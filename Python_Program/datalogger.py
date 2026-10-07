import serial
import numpy as np
import cv2
import os
import time

# =============================================================================
# KONFIGURASI
# =============================================================================
# Ganti 'COM3' dengan port COM STM32 (cek di Device Manager Windows)
COM_PORT = 'COM23' 
BAUD_RATE = 1000000 # Baudrate tidak terlalu berpengaruh pada USB CDC, tapi wajib diisi
DATASET_DIR = 'dataset'

# Definisi Label
LABELS = {
    '1': 'segitiga',
    '2': 'segiempat',
    '3': 'lingkaran',
    '4': 'lainnya'
}

# =============================================================================
# INISIALISASI
# =============================================================================
# Buat direktori dataset jika belum ada
if not os.path.exists(DATASET_DIR):
    os.makedirs(DATASET_DIR)

for key, label in LABELS.items():
    label_path = os.path.join(DATASET_DIR, label)
    if not os.path.exists(label_path):
        os.makedirs(label_path)

print(f"Menghubungkan ke {COM_PORT}...")
try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print("Berhasil terhubung!")
except Exception as e:
    print(f"Gagal membuka port {COM_PORT}: {e}")
    exit(1)

print("\n=== LIVE LABELING MODE ===")
print("Tekan angka untuk menyimpan frame ke dalam kelas/folder:")
for key, label in LABELS.items():
    print(f"  '{key}' -> {label}")
print("Tekan 'q' atau 'ESC' untuk keluar.\n")

# =============================================================================
# LOOP UTAMA
# =============================================================================
try:
    while True:
        # 1. Cari Header Sinkronisasi (0xAA, 0x55)
        # Jika data yang masuk tidak rata, kita harus mencari headernya
        header_found = False
        while not header_found:
            # Baca 1 byte
            if ser.in_waiting > 0:
                b = ser.read(1)
                if b == b'\xAA':
                    b2 = ser.read(1)
                    if b2 == b'\x55':
                        header_found = True

        # 2. Baca 784 byte setelah header (Gambar 28x28 Grayscale)
        raw_data = ser.read(784)
        
        if len(raw_data) != 784:
            print("Peringatan: Data tidak lengkap, melewatkan frame.")
            continue
            
        # 3. Ubah byte menjadi array numpy (28x28 grayscale)
        img_array = np.frombuffer(raw_data, dtype=np.uint8).reshape((28, 28))
        
        # 4. Tampilkan gambar ke layar (diperbesar agar mudah dilihat)
        img_display = cv2.resize(img_array, (280, 280), interpolation=cv2.INTER_NEAREST)
        cv2.imshow('Live Camera (28x28)', img_display)
        
        # 5. Baca input keyboard
        # cv2.waitKey(1) mengembalikan nilai ASCII dari tombol yang ditekan selama 1ms
        key = cv2.waitKey(1) & 0xFF
        
        if key == ord('q') or key == 27: # 'q' atau ESC
            print("Keluar...")
            break
            
        # Cek apakah tombol yang ditekan ada di dictionary LABELS
        char_key = chr(key)
        if char_key in LABELS:
            label_name = LABELS[char_key]
            
            # Buat nama file berdasarkan timestamp
            filename = f"{label_name}_{int(time.time() * 1000)}.png"
            filepath = os.path.join(DATASET_DIR, label_name, filename)
            
            # Simpan file gambar
            cv2.imwrite(filepath, img_array)
            print(f"[{time.strftime('%H:%M:%S')}] Tersimpan: {filepath}")

except KeyboardInterrupt:
    print("\nDihentikan oleh pengguna.")
finally:
    ser.close()
    cv2.destroyAllWindows()
    print("Port serial ditutup.")
