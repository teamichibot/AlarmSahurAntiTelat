# 🚀 Alarm Sahur Anti Telat  
**⏰ Pastikan Sahur Tepat Waktu!**

Alarm ini **tidak akan berhenti berbunyi** sampai Anda benar-benar **minum sahur**! Dibangun menggunakan **Arduino Nano**, sensor **Load Cell**, dan berbagai komponen pendukung lainnya, alarm ini **memaksa Anda bangun** dengan suara buzzer **super kencang** dan efek cahaya LED **strobo** yang tidak bisa diabaikan.

---

## 🎯 Fitur Utama
✅ **Alarm berbasis sensor berat** - Alarm hanya mati setelah gelas dikosongkan.  
✅ **Buzzer 12V yang super keras** - Tidak ada opsi snooze, Anda *harus* bangun!  
✅ **Efek LED strobo & breathing** - Lampu akan berkedip dan bertransisi agar lebih mengganggu.  
✅ **Navigasi mudah dengan rotary encoder** - Set waktu alarm, kalibrasi sensor, dan atur warna LED dengan mudah.  
✅ **Real-Time Clock (RTC) DS3231** - Alarm tetap berjalan meskipun perangkat dimatikan.  
✅ **Tampilan 7-segment** - Menampilkan jam & mode pengaturan dengan jelas.  

---

## 🛠 Komponen yang Dibutuhkan
Berikut adalah daftar komponen yang digunakan beserta link pembelian:

| No  | Komponen                    | Tipe / Model | Link Pembelian |
|-----|-----------------------------|--------------|----------------|
| 1   | **Mikrokontroler**           | Arduino Nano | [🔗 Shopee](https://s.shopee.co.id/2qGSRJIe4f) |
| 2   | **Sensor Berat**             | Load Cell 1kg + HX711 | [🔗 Shopee](https://s.shopee.co.id/9KTwB2d11t) |
| 3   | **Buzzer**                   | Buzzer 12V | [🔗 Shopee](https://s.shopee.co.id/50Kx0rDKUg) |
| 4   | **Modul RTC**                | DS3231 | [🔗 Shopee](https://s.shopee.co.id/9UnMNJvCUA) |
| 5   | **Rotary Encoder**           | KY-040 | [🔗 Shopee](https://s.shopee.co.id/2B0le0JHhh) |
| 6   | **Tampilan 7-Segment**       | TM1637 (4 digit) | [🔗 Shopee](https://s.shopee.co.id/1Vl4qj5A0m) |
| 7   | **WS2812 LED Ring**          | 8-bit | [🔗 Shopee](https://s.shopee.co.id/8fEFNlKlba) |
| 8   | **MOSFET**                   | IRF540 | [🔗 Shopee](https://s.shopee.co.id/VsXedAtS8) |
| 9   | **Dioda**                    | 1N4007 | [🔗 Shopee](https://s.shopee.co.id/3LCj36Ubjt) |
| 10  | **Step-down Converter**      | Mini 12V-5V | [🔗 Shopee](https://s.shopee.co.id/6AWuOBxwYA) |
| 11  | **Resistor Pull-up**         | 10KΩ | [🔗 Shopee](https://s.shopee.co.id/10ooG16pal) |
| 12  | **Jack DC Female**           | 5.5mm x 2.1mm | [🔗 Shopee](https://s.shopee.co.id/3AtIpYe0Id) |

---

## ⚡ Skema Rangkaian
📌 *(Tambahkan gambar skematik di sini, bisa dibuat dengan Fritzing atau KiCad.)*  

---

## 🖥 Instalasi & Setup
### 🔹 **1. Instalasi Library**
Pastikan Anda telah menginstal library berikut sebelum mengunggah kode ke Arduino:  
- `HX711_ADC` - Untuk membaca data dari sensor Load Cell.  
- `RTClib` - Untuk mengakses Real-Time Clock DS3231.  
- `Adafruit_NeoPixel` - Untuk mengontrol LED WS2812.  
- `TM1637Display` - Untuk mengontrol display 7-segment.  

#### 🔹 **Cara Install Library**
1. Buka **Arduino IDE**.  
2. Pergi ke **Sketch** ➝ **Include Library** ➝ **Manage Libraries**.  
3. Cari dan **install** semua library yang dibutuhkan di atas.  

---

### 🔹 **2. Wiring & Koneksi**
Berikut adalah koneksi antar komponen pada Arduino Nano:  

| Komponen          | Pin Arduino |
|------------------|------------|
| HX711 DOUT      | A1         |
| HX711 SCK       | A0         |
| TM1637 CLK      | 3          |
| TM1637 DIO      | 2          |
| Rotary CLK      | 4          |
| Rotary DT       | 5          |
| Rotary SW       | 6          |
| LED WS2812      | 7          |
| Buzzer          | 9 (via MOSFET) |

---

### 🔹 **3. Upload Kode ke Arduino**
1. Buka **Arduino IDE**.  
2. Pilih **Board: "Arduino Nano"**.  
3. Pilih **Processor: "ATmega328P (Old Bootloader)"** *(Jika menggunakan versi non-original)*.  
4. **Pilih Port** sesuai dengan Arduino Anda.  
5. Klik **Upload**.  

---

## 🚀 Cara Menggunakan
### **🔸 1. Menyetel Alarm**
- Putar **Rotary Encoder** untuk memilih mode.  
- Pilih menu **"Set Alarm"**, lalu tekan tombol untuk konfirmasi.  
- Atur jam & menit dengan memutar rotary, tekan untuk menyimpan.  

### **🔸 2. Kalibrasi Load Cell**
- Pilih menu **"Calibrate Tare"**, lalu tekan tombol.  
- Pastikan gelas dalam keadaan kosong, tekan lagi untuk menyimpan.  

### **🔸 3. Mengatur Warna LED**
- Pilih menu **"Set LED Color"**, putar rotary untuk memilih warna.  
- Tekan tombol untuk menyimpan.  

### **🔸 4. Cara Mematikan Alarm**
- Saat alarm berbunyi, **angkat gelas dan minum sampai sensor mendeteksi perubahan berat**.  
- Jika gelas tetap penuh, alarm **tidak akan mati**.  

---

## 🔥 Kode Arduino
Kode lengkap tersedia di repositori ini. Unduh dan upload ke Arduino Anda untuk menjalankan alarm.

---

## 📢 Contribute & Feedback
- Jika ingin berkontribusi, silakan buat **pull request**!  
- Jika menemukan bug atau masalah, **buat issue baru** di tab **Issues**.  
- Jangan lupa ⭐ **star repository ini** jika bermanfaat!  

📌 **Author**: Ichibot 🚀
