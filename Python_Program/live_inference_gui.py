import serial
import numpy as np
import cv2
import time
import threading
import tkinter as tk
from PIL import Image, ImageTk
import tensorflow as tf

# =============================================================================
# KONFIGURASI KUANTISASI (C-Style Define)
# Pastikan tipe ini SAMA PERSIS dengan yang ada di train_bnn.py
# Pilihan: 'BNN', 'INT4', 'INT8', 'FP8'
# =============================================================================
QUANT_MODE = 'BNN'

# =============================================================================
# IMPLEMENTASI CUSTOM LAYER UNTUK LOAD MODEL
# =============================================================================
@tf.custom_gradient
def ste_quantize(x):
    if QUANT_MODE == 'BNN':
        out = tf.where(x >= 0, 1.0, -1.0)
    elif QUANT_MODE == 'INT4':
        scale = 7.0
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    elif QUANT_MODE == 'INT8' or QUANT_MODE == 'FP8':
        scale = 127.0
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    else:
        out = x

    def grad(dy, variables=None):
        g = dy * tf.cast(tf.abs(x) <= 1.0, dy.dtype)
        if variables:
            return g, [None] * len(variables)
        return g
    return out, grad

class QuantizedDense(tf.keras.layers.Layer):
    def __init__(self, units, **kwargs):
        super(QuantizedDense, self).__init__(**kwargs)
        self.units = units
    def build(self, input_shape):
        self.kernel = self.add_weight(shape=(input_shape[-1], self.units), initializer="glorot_normal", trainable=True, name="kernel")
    def call(self, inputs):
        return tf.matmul(ste_quantize(inputs), ste_quantize(self.kernel))

# =============================================================================
# KONFIGURASI
# =============================================================================
COM_PORT = 'COM23'
BAUD_RATE = 1000000
MODEL_PATH = "bnn_model.h5"

LABELS = ['Lainnya', 'Lingkaran', 'Segiempat', 'Segitiga']
COLORS = ['#9C27B0', '#2196F3', '#4CAF50', '#FF5722']

# =============================================================================
# GUI INFERENCE
# =============================================================================
class LiveInferenceGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("PulseAI - BNN Live Inference")
        self.root.geometry("600x850")
        self.root.configure(bg="#f0f0f0")
        
        self.is_running = True
        self.latest_frame = np.zeros((28, 28), dtype=np.uint8)
        self.frame_updated = False
        self.ser = None
        
        # Load Model
        print("Memuat Model BNN...")
        try:
            self.model = tf.keras.models.load_model(MODEL_PATH, custom_objects={'QuantizedDense': QuantizedDense, 'ste_quantize': ste_quantize})
            print("Model berhasil dimuat!")
        except Exception as e:
            print(f"Gagal memuat model: {e}")
            self.model = None

        self.setup_ui()
        self.connect_serial()
        
        # Thread Serial
        self.serial_thread = threading.Thread(target=self.read_serial_loop, daemon=True)
        self.serial_thread.start()
        
        # Mulai loop inference
        self.update_inference()
        
    def setup_ui(self):
        # Judul
        tk.Label(self.root, text="Live Inference (PC)", font=("Arial", 16, "bold"), bg="#f0f0f0").pack(pady=10)
        
        # Tampilan Gambar
        self.image_label = tk.Label(self.root, text="Menunggu Kamera...", bg="black", fg="white")
        self.image_label.pack(pady=10)
        
        # Prediksi Teratas
        self.pred_label = tk.Label(self.root, text="-", font=("Arial", 24, "bold"), fg="#333", bg="#f0f0f0")
        self.pred_label.pack(pady=10)
        
        # Grafik Probabilitas (Bar Chart)
        self.canvas = tk.Canvas(self.root, width=500, height=200, bg="white", highlightthickness=1, highlightbackground="#ccc")
        self.canvas.pack(pady=10)
        
        # Inisialisasi bar chart
        self.bars = []
        self.bar_texts = []
        bar_width = 80
        spacing = 40
        start_x = 35
        
        for i, label in enumerate(LABELS):
            x0 = start_x + i * (bar_width + spacing)
            x1 = x0 + bar_width
            y0 = 180
            y1 = 180
            
            # Gambar Bar (awal 0)
            bar = self.canvas.create_rectangle(x0, y0, x1, y1, fill=COLORS[i], outline="")
            self.bars.append(bar)
            
            # Label Bawah
            self.canvas.create_text(x0 + bar_width/2, 190, text=label, font=("Arial", 9, "bold"))
            
            # Label Persentase
            pct = self.canvas.create_text(x0 + bar_width/2, 170, text="0%", font=("Arial", 9))
            self.bar_texts.append(pct)

        # Status Label
        self.status_label = tk.Label(self.root, text="Status: Menghubungkan...", font=("Arial", 10), bg="#f0f0f0")
        self.status_label.pack(side=tk.BOTTOM, pady=10)

    def connect_serial(self):
        try:
            self.ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
            self.status_label.config(text=f"Status: Terhubung ke {COM_PORT}", fg="green")
        except Exception as e:
            self.status_label.config(text=f"Status: Gagal membuka {COM_PORT}", fg="red")
            
    def read_serial_loop(self):
        if self.ser is None: return
        while self.is_running:
            try:
                if self.ser.in_waiting > 0:
                    b = self.ser.read(1)
                    if b == b'\xAA':
                        if self.ser.read(1) == b'\x55':
                            raw_data = self.ser.read(784)
                            if len(raw_data) == 784:
                                self.latest_frame = np.frombuffer(raw_data, dtype=np.uint8).reshape((28, 28))
                                self.frame_updated = True
            except Exception as e:
                time.sleep(1)
                
    def update_inference(self):
        if not self.is_running: return
        
        if self.frame_updated and self.model:
            self.frame_updated = False
            
            # Update Tampilan Gambar Kamera
            img_resized = cv2.resize(self.latest_frame, (400, 400), interpolation=cv2.INTER_NEAREST)
            pil_img = Image.fromarray(img_resized)
            self.photo = ImageTk.PhotoImage(image=pil_img)
            self.image_label.config(image=self.photo, text="")
            
            # PREPROCESSING SAMA DENGAN SAAT TRAINING
            img_float = self.latest_frame.astype(np.float32) / 255.0
            
            if QUANT_MODE == 'BNN':
                img_proc = np.where(img_float > 0.5, 1.0, -1.0).astype(np.float32)
            else:
                img_proc = (img_float * 2.0 - 1.0).astype(np.float32)
            
            # Expand dims menjadi (1, 28, 28, 1)
            img_tensor = np.expand_dims(img_proc, axis=(0, -1))
            
            # Lakukan INFERENCE! (Menggunakan call langsung agar sangat cepat)
            preds = self.model(img_tensor, training=False).numpy()[0]
            
            # Update Bar Chart dan UI
            self.update_chart(preds)
            
        # Panggil kembali fungsi ini setelah 30ms
        self.root.after(30, self.update_inference)
        
    def update_chart(self, preds):
        bar_width = 80
        spacing = 40
        start_x = 35
        max_height = 150 # Tinggi maksimal bar pada kanvas
        
        best_idx = np.argmax(preds)
        best_val = preds[best_idx] * 100
        
        # Jika probabilitas > 50%, tampilkan tebakan utama
        if best_val > 50:
            self.pred_label.config(text=f"{LABELS[best_idx]} ({best_val:.1f}%)", fg=COLORS[best_idx])
        else:
            self.pred_label.config(text="Tidak Yakin...", fg="gray")
        
        # Animasi Bar Chart
        for i, val in enumerate(preds):
            x0 = start_x + i * (bar_width + spacing)
            x1 = x0 + bar_width
            
            # Tinggi Bar
            h = val * max_height
            y0 = 180 - h
            y1 = 180
            
            # Update posisi Rectangle
            self.canvas.coords(self.bars[i], x0, y0, x1, y1)
            
            # Update Teks Persentase
            self.canvas.coords(self.bar_texts[i], x0 + bar_width/2, y0 - 10)
            self.canvas.itemconfig(self.bar_texts[i], text=f"{val*100:.0f}%")

    def on_close(self):
        self.is_running = False
        if self.ser:
            self.ser.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = LiveInferenceGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
