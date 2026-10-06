# STM32F446RE - OV7670 (No FIFO) & ILI9488 Display Integration

Proyek ini dibuat untuk menghubungkan modul sensor kamera OV7670 (versi murah/generik tanpa chip FIFO AL422B) ke layar LCD ILI9488 (SPI) menggunakan mikrokontroler STM32F446RE. Pengembangan ini merupakan bagian awal dari persiapan **Kompetisi Chip Peruri PulseAI**.

Sistem ini didesain agar dapat mengambil gambar langsung dari kamera dan menampilkannya menutupi layar LCD secara penuh (480x320) dengan *frame rate* yang tinggi, tanpa menggunakan memori eksternal tambahan.

---

## 🛠 Daftar Pin yang Digunakan (Pinout)

### Kamera OV7670 (DCMI & I2C2)
| Pin Kamera | STM32 Pin | Fungsi / Deskripsi |
| :--- | :--- | :--- |
| **3V3 & GND** | 3.3V & GND | Supply Daya Kamera |
| **SIOC / SCL** | PB10 | I2C2 SCL (Wajib tambah *Pull-up resistor* 3.3kΩ / 4.7kΩ) |
| **SIOD / SDA** | PB11 | I2C2 SDA (Wajib tambah *Pull-up resistor* 3.3kΩ / 4.7kΩ) |
| **XCLK** | PA8 | DCMI Clock output (MCO1) - 16 MHz untuk sensor |
| **PCLK** | PA6 | DCMI Pixel Clock input |
| **VSYNC** | PB7 | DCMI VSYNC (Awal Frame) |
| **HREF** | PA4 | DCMI HSYNC (Awal Baris/Data Valid) |
| **D0 - D7** | PC6 - PC11, PB8, PB9 | DCMI Data [0:7] (8-bit Paralel) |
| **RET / RESET** | (Tergantung Konfigurasi) | Tarik ke HIGH atau kendalikan via GPIO |
| **PWDN** | (Tergantung Konfigurasi) | Tarik ke LOW agar kamera selalu aktif |

### Layar ILI9488 (SPI2)
| Pin LCD | STM32 Pin | Fungsi / Deskripsi |
| :--- | :--- | :--- |
| **VCC & GND** | 3.3V & GND | Supply Daya Layar |
| **CS** | PB12 | SPI2 Chip Select |
| **SCK** | PB13 | SPI2 Clock |
| **MOSI (SDI)** | PB15 | SPI2 MOSI (Data In) |
| **DC** | (Pin Khusus, Misal PC4)| Data / Command selection |
| **RST** | (Pin Khusus, Misal PC5)| Hardware Reset LCD |
| **LED / BL** | PA7 | Backlight PWM (TIM3_CH2) |

---

## 🚧 Hambatan & Solusi Saat Menggunakan Kamera OV7670 Clone (Generik)

Sebagian besar tutorial di internet ditujukan untuk OV7670 asli atau varian FIFO yang dipakai dengan Arduino. Saat menggunakan versi *Clone* murah tanpa FIFO pada STM32 yang sangat cepat, terdapat banyak masalah teknis yang tidak didokumentasikan di internet:

### 1. Masalah Protokol SCCB (Berbeda dengan I2C Standar)
- **Masalah:** Fungsi standar `HAL_I2C_Mem_Read()` STM32 selalu gagal membaca register dari kamera.
- **Penyebab:** Sensor OV7670 menggunakan bus SCCB (*Serial Camera Control Bus*). Meskipun mirip I2C, SCCB **TIDAK** mendukung sinyal *Repeated Start* di sela-sela fase penulisan alamat dan pembacaan data.
- **Solusi:** Proses baca (*Read*) tidak bisa dilakukan dalam satu fungsi. Kita harus secara eksplisit menggunakan `HAL_I2C_Master_Transmit()` untuk menulis alamat register, lalu *Stop*, dan dilanjutkan dengan `HAL_I2C_Master_Receive()` dengan sinyal *Start* baru. Selain itu, bus ini rentan putus/hilang sinyal sehingga membutuhkan *pull-up* resistor keras (~3.3kΩ).

### 2. "Death by Scaling" (Kamera Hang/Crash Saat Resolusi Dikecilkan)
- **Masalah:** DCMI hanya mendeteksi pin VSYNC tinggi (High) secara permanen, dan PCLK tetap ada, tetapi pin D0-D7 beserta HREF tetap rendah selamanya. Modul terkunci sepenuhnya (*Deadlock*).
- **Penyebab:** Modul *clone* ini memiliki *Digital Signal Processor* (DSP) yang cacat atau lemah. Saat register *Downsampling/Scaling* diaktifkan (seperti `COM3`, `COM14`, dan fitur DCW), atau ketika register **PCLK Divider** dihidupkan untuk memperlambat *output* ke 1 MHz, *pipeline* sirkuit analog kamera bentrok dan memicu *overflow* pada arsitektur pipa internalnya. 
- **Solusi:** **Jangan pernah gunakan Scaling Hardware Kamera!** Biarkan kamera berlari pada resolusi penuh bawaannya (VGA atau QVGA) tanpa fitur *scaling* internal sama sekali.

### 3. Masalah Geometri "Diagonal Slant" dan Tearing
- **Masalah:** Gambar pita warna (*Color Bar*) yang ditangkap tampak miring menyamping, bergaris diagonal, dan selalu bergeser dari satu *frame* ke *frame* lainnya.
- **Penyebab:** Kamera memancarkan garis 640 piksel, tetapi DCMI dan memori LCD diatur untuk menangkap panjang yang lebih kecil (misal 160). Akibatnya, pemotongan memori (*buffer wrap*) tidak sesuai dengan *Carriage Return* (VSYNC), menyebabkan data piksel dari baris sebelumnya terus merembes secara acak ke baris di bawahnya.
- **Solusi:** Total *Word* (DMA Data Length) yang diambil melalui instruksi `HAL_DCMI_Start_DMA` harus 100% kongruen dan habis dibagi sempurna dengan resolusi jendela tangkapan (*Crop Window*).

---

## ⚙️ Cara Kerja Sistem (Arsitektur Akhir yang Sukses)

Bagaimana STM32F446RE dengan RAM internal (128 KB) yang sangat terbatas bisa menangkap data kamera resolusi besar dan memetakannya tanpa batasan penuh ke LCD resolusi 480x320?

1. **Native Sensor Output:** Kamera dikonfigurasi melalui I2C (`COM7 = 0x14`) untuk mengeluarkan resolusi **QVGA (320x240)** pada spektrum warna RGB565. Fitur Scaling dan Test Pattern dimatikan penuh.
2. **STM32 DCMI Hardware Cropping:** Karena memori kita tidak cukup menampung 320x240 secara utuh (yang membutuhkan 150 KB), kita menggunakan fitur DCMI CROP berbasis piranti keras (*Hardware*). DCMI diatur untuk membuang 40 piksel dari kiri/kanan, dan 40 baris dari atas/bawah, hanya menangkap **bidang 240x160 persis di tengah-tengah lensa (*Center FOV*)**. 
3. **DMA to Internal RAM:** Bidang 240x160 tersebut berukuran sekitar ~75 KB, muat secara sempurna dan aman di dalam SRAM STM32 kita, disisipkan oleh *Direct Memory Access* (DMA) agar CPU (Cortex-M4) dapat beristirahat.
4. **Software Upscaling (SPI ke LCD):** Saat CPU mendeteksi `frame_ready` dari DCMI, CPU membaca memori 240x160 tersebut, menggandakan setiap piksel di sumbu horisontal sebanyak dua kali (2x), dan mengirimkan perintah untuk mengirim setiap garis sebanyak dua kali (2x) kepada layar. Hasil akhirnya: **240x160 mekar secara simetris menjadi 480x320 (Full Screen)** tanpa menyisakan margin sedikit pun!

---

## 🚀 Potensi Pengembangan Masa Depan (Peruri PulseAI)

Sistem tangkapan ini sudah meletakkan *bedrock* (pondasi perangkat lunak) yang sangat solid. Berikut adalah apa yang bisa Anda bangun di atas kode ini untuk memenangkan kompetisi tersebut:

1. **Implementasi Ping-Pong (Double Buffering) DMA:** Saat ini, pembacaan kamera harus menunggu penggambaran ke LCD selesai, sehingga FPS maksimal kita adalah `~8-12 FPS`. Jika Anda menggunakan konfigurasi `DCMI_MODE_CONTINUOUS` dan membagi *array* RAM menjadi 2 blok (*Front-Buffer* dan *Back-Buffer*), FPS tampilan akan meningkat secara drastis (*60 FPS+ limit kamera*).
2. **Computer Vision & TinyML Inference:** Karena kita berhasil mengunci resolusi `240x160` secara bersih di dalam RAM STM32, data yang sama bisa langsung di-*feed* (disuapkan) ke algoritma *TensorFlow Lite for Microcontrollers* atau modul CMSIS-NN tanpa perlu mengubah bentuk dimensinya. 
3. **Auto Exposure & White Balance Tuning:** Kondisi pencahayaan dunia nyata sangat liar. Diperlukan pengujian parameter register `AWB` (Auto White Balance), `AEC` (Auto Exposure), dan `AGC` (Auto Gain) agar modul tidak *overexposed* (putih buta) atau terlalu gelap.
4. **DMA Timeout Recovery:** Walaupun penanganan SPI DMA Timeout sudah diatasi, ke depannya Anda bisa menyusun sistem *Health Check* periodik untuk mereset DCMI secara langsung tanpa mengulang siklus `HAL_Init` apabila DMA kamera terindikasi terjebak (*stall*).
