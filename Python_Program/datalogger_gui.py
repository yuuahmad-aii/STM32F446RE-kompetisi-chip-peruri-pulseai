import serial
import numpy as np
import cv2
import os
import time
import threading
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk

# =============================================================================
# KONFIGURASI
# =============================================================================
COM_PORT = 'COM23' 
BAUD_RATE = 1000000 
DATASET_DIR = 'dataset'

LABELS = ['segitiga', 'segiempat', 'lingkaran']

# =============================================================================
# INISIALISASI FOLDER
# =============================================================================
if not os.path.exists(DATASET_DIR):
    os.makedirs(DATASET_DIR)

for label in LABELS:
    label_path = os.path.join(DATASET_DIR, label)
    if not os.path.exists(label_path):
        os.makedirs(label_path)

# =============================================================================
# APLIKASI GUI
# =============================================================================
class DataLoggerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("PulseAI - Dataset Logger")
        self.root.geometry("400x550")
        
        # Variabel Status
        self.is_running = True
        self.latest_frame = np.zeros((28, 28), dtype=np.uint8)
        self.ser = None
        self.counts = {label: len(os.listdir(os.path.join(DATASET_DIR, label))) for label in LABELS}
        
        self.setup_ui()
        self.connect_serial()
        
        # Thread untuk membaca serial agar GUI tidak freeze
        self.serial_thread = threading.Thread(target=self.read_serial_loop, daemon=True)
        self.serial_thread.start()
        
        # Timer untuk update layar
        self.update_display()
        
    def setup_ui(self):
        # Tampilan Gambar
        self.image_label = tk.Label(self.root, text="Menunggu Kamera...", bg="black", fg="white")
        self.image_label.pack(pady=20)
        
        # Status Label
        self.status_label = tk.Label(self.root, text="Status: Menghubungkan...", font=("Arial", 10))
        self.status_label.pack(pady=5)
        
        # Tombol Dataset
        btn_frame = tk.Frame(self.root)
        btn_frame.pack(pady=10)
        
        self.count_labels = {}
        for i, label in enumerate(LABELS):
            btn = tk.Button(btn_frame, text=f"Simpan {label.capitalize()}", font=("Arial", 12, "bold"), 
                            bg="#4CAF50", fg="white", width=20, height=2,
                            command=lambda l=label: self.save_image(l))
            btn.grid(row=i, column=0, pady=5)
            
            lbl_count = tk.Label(btn_frame, text=f"Total: {self.counts[label]}", font=("Arial", 10))
            lbl_count.grid(row=i, column=1, padx=10)
            self.count_labels[label] = lbl_count
            
            # Bind keyboard (1, 2, 3, 4)
            self.root.bind(str(i+1), lambda event, l=label: self.save_image(l))
            
    def connect_serial(self):
        try:
            self.ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
            self.status_label.config(text=f"Status: Terhubung ke {COM_PORT}", fg="green")
        except Exception as e:
            self.status_label.config(text=f"Status: Gagal membuka {COM_PORT}", fg="red")
            
    def read_serial_loop(self):
        if self.ser is None:
            return
            
        while self.is_running:
            try:
                # Cari header 0xAA 0x55
                if self.ser.in_waiting > 0:
                    b = self.ser.read(1)
                    if b == b'\xAA':
                        b2 = self.ser.read(1)
                        if b2 == b'\x55':
                            # Header ditemukan, baca 784 byte!
                            raw_data = self.ser.read(784)
                            if len(raw_data) == 784:
                                self.latest_frame = np.frombuffer(raw_data, dtype=np.uint8).reshape((28, 28))
            except Exception as e:
                print(f"Serial Error: {e}")
                time.sleep(1)
                
    def update_display(self):
        if self.is_running:
            # Perbesar gambar 28x28 menjadi 280x280 menggunakan Nearest Neighbor agar pixel art jelas
            img_resized = cv2.resize(self.latest_frame, (280, 280), interpolation=cv2.INTER_NEAREST)
            
            # Konversi array numpy ke PIL Image lalu ke PhotoImage
            pil_img = Image.fromarray(img_resized)
            self.photo = ImageTk.PhotoImage(image=pil_img)
            
            self.image_label.config(image=self.photo, text="")
            
            # Panggil fungsi ini lagi setelah 30ms (~33 FPS)
            self.root.after(30, self.update_display)
            
    def save_image(self, label):
        filename = f"{label}_{int(time.time() * 1000)}.png"
        filepath = os.path.join(DATASET_DIR, label, filename)
        
        cv2.imwrite(filepath, self.latest_frame)
        
        # Update Counter
        self.counts[label] += 1
        self.count_labels[label].config(text=f"Total: {self.counts[label]}")
        print(f"Tersimpan: {filepath}")

    def on_close(self):
        self.is_running = False
        if self.ser:
            self.ser.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = DataLoggerGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
