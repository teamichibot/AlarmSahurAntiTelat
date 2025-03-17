#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h>
#include <HX711_ADC.h>
#include <EEPROM.h>  // 🔹 Tambahkan library EEPROM

#define EEPROM_ADDR_HOUR 0    // Alamat penyimpanan jam alarm
#define EEPROM_ADDR_MINUTE 1  // Alamat penyimpanan menit alarm
#define EEPROM_ADDR_HUE 2

// **Pin WS2812 LED Ring**
#define RGB_PIN 7
#define LED_COUNT 8  // **Menggunakan LED Ring 12 LED**

// **Pin TM1637 (Display)**
#define CLK 3
#define DIO 2

// **Pin Rotary Encoder (KY-040)**
#define ENCODER_CLK 4
#define ENCODER_DT 5
#define ENCODER_SW 6

// **Pin HX711 (Load Cell)**
#define LOADCELL_DOUT A1
#define LOADCELL_SCK A0

// **Pin WS2812 LED Ring**
#define RGB_PIN 7
#define LED_COUNT 12

// **Pin Buzzer**
#define BUZZER_PIN 9
#define BUZZER_DURATION 1000  // **Durasi buzzer aktif (1 detik)**
#define BUZZER_INTERVAL 500   // **Waktu jeda antara bunyi buzzer (500 ms)**

// **Batas Berat untuk Aktivasi Alarm**
#define WEIGHT_THRESHOLD 100
#define EMPTY_THRESHOLD 50
#define DEBOUNCE_DELAY 5  // Debounce untuk Rotary Encoder (dalam ms)


// **Deklarasi Objek**
TM1637Display display(CLK, DIO);
RTC_DS3231 rtc;
HX711_ADC LoadCell(LOADCELL_DOUT, LOADCELL_SCK);
Adafruit_NeoPixel strip(LED_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

// **Variabel Global**
int alarmHour = 4, alarmMinute = 0;
bool alarmActive = false;
int ledColor = 100;
int ledHue = 0;
bool isEditing = false;
unsigned long lastEncoderMillis = 0;
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
int lastStateCLK;
int currentStateCLK;
int counter = 0;
int buttonCount = 0;
unsigned long blinkTimer = 0;
bool blinkState = true;
bool buttonPressed = false;
unsigned long lastRead = 0;
int weight = 0;
int tareWeight = 0;  // Menyimpan berat gelas kosong
int currentHour = 0;
int currentMinute = 0;
unsigned long lastCounterChangeTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai setup...");

  Wire.begin();
  // **Baca Alarm dari EEPROM**
  alarmHour = EEPROM.read(EEPROM_ADDR_HOUR);
  alarmMinute = EEPROM.read(EEPROM_ADDR_MINUTE);
  ledHue = EEPROM.read(EEPROM_ADDR_HUE);

  // **Validasi nilai dari EEPROM (Harus dalam rentang yang benar)**
  if (alarmHour > 23) alarmHour = 0;
  if (alarmMinute > 59) alarmMinute = 0;
  if (ledHue > 255) ledHue = 0;

  Serial.print("⏰ Alarm dari EEPROM: ");
  Serial.print(alarmHour);
  Serial.print(":");
  Serial.println(alarmMinute);
  Serial.print("🌈 Hue LED dari EEPROM: ");
  Serial.println(ledHue);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  lastStateCLK = digitalRead(ENCODER_CLK);
  digitalWrite(BUZZER_PIN, LOW);

  display.setBrightness(7);
  display.showNumberDec(8888);
  delay(1000);
  display.clear();

  LoadCell.begin();
  LoadCell.setReverseOutput();           // Membalik arah output HX711
  unsigned long stabilizingtime = 2000;  // Waktu stabilisasi
  boolean _tare = true;                  // Tare otomatis di awal
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("⚠️ Timeout! Periksa wiring HX711");
    while (1)
      ;
  } else {
    LoadCell.setCalFactor(696.0);  // Gunakan faktor kalibrasi
    Serial.println("✅ Load Cell Siap!");
  }
  if (!rtc.begin()) {
    Serial.println("RTC tidak ditemukan! Stuck di loop.");
    while (1)
      ;
  }

  if (rtc.lostPower()) {
    Serial.println("RTC kehilangan daya, mengatur ulang waktu.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  strip.begin();
  strip.show();
  updateLedColor();  // **Tampilkan warna awal dari EEPROM**

  Serial.println("Setup selesai!");
}

void loop() {
  currentStateCLK = digitalRead(ENCODER_CLK);
  unsigned long currentMillis = millis();
  DateTime now = rtc.now();
  resetCounterIfInactive();
  static boolean newDataReady = 0;

  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
    weight = LoadCell.getData() - tareWeight;  // Ambil data stabil
    newDataReady = 0;
  }


  // 🔹 Cek Alarm dan Berat
  currentHour = now.hour();
  currentMinute = now.minute();

  if (weight > WEIGHT_THRESHOLD) {
    checkAlarm();
    if (currentHour == alarmHour && currentMinute == alarmMinute) {
      alarmActive = true;
    }
  } else if (weight < EMPTY_THRESHOLD) {
    alarmActive = false;
    digitalWrite(BUZZER_PIN, LOW);
  }

  // 🔹 Navigasi Mode Rotary Encoder
  if (buttonCount == 0) {
    if (currentMillis - lastEncoderMillis > DEBOUNCE_DELAY) {
      if (currentStateCLK != lastStateCLK) {
        if (digitalRead(ENCODER_DT) != currentStateCLK) {
          counter++;
        } else {
          counter--;
        }

        // **Batasi nilai counter agar tetap dalam 0 - 4**
        if (counter < 0) {
          counter = 4;
        } else if (counter > 4) {
          counter = 0;
        }

        Serial.print("Counter: ");
        Serial.println(counter);
        lastEncoderMillis = currentMillis;  // Simpan waktu terakhir pembacaan
        lastCounterChangeTime = currentMillis;
      }
    }
  } else if (counter == 1) {
    setAlarmTime();
  } else if (counter == 2) {
    setCurrentTime();
  } else if (counter == 3) {
    LedC();
  } else if (counter == 4) {
    calibrateTare();
  }

  // 🔹 Tampilan TM1637
  if (counter == 0) {
    displayTime();
    if (buttonCount != 0) {
      buttonCount = 0;
    }
  } else if (counter == 1 && buttonCount == 0) {
    displaySetA();
  } else if (counter == 2 && buttonCount == 0) {
    displaySetC();
  } else if (counter == 3 && buttonCount == 0) {
    displayLedC();
  } else if (counter == 4 && buttonCount == 0) {
    displayTare();
  }
  lastStateCLK = currentStateCLK;


  // 🔹 Cek jika tombol rotary encoder ditekan (Dengan Debounce)
  if (digitalRead(ENCODER_SW) == LOW && buttonPressed == false) {
    buttonPressed = true;
    delay(100);
    buttonCount++;
    if (buttonCount > 3) buttonCount = 0;
    Serial.print("Button Count: ");
    Serial.println(buttonCount);
  }
  if (digitalRead(ENCODER_SW) == HIGH) {
    buttonPressed = false;
  }
  updateLedColor();

  delay(10);
}

// 🔹 Fungsi LedC untuk Mengatur Hue LED WS2812
void LedC() {
  static int lastStateCLK = digitalRead(ENCODER_CLK);
  int currentStateCLK = digitalRead(ENCODER_CLK);
  unsigned long currentMillis = millis();

  if (currentMillis - lastEncoderMillis > DEBOUNCE_DELAY) {
    if (buttonCount == 1) {
      if (currentStateCLK != lastStateCLK) {
        if (digitalRead(ENCODER_DT) != currentStateCLK) {
          ledHue = (ledHue + 5) % 256;
        } else {
          ledHue = (ledHue - 5 + 256) % 256;
        }
        Serial.print("🌈 Hue LED: ");
        Serial.println(ledHue);
        updateLedColor();
      }
      lastStateCLK = currentStateCLK;

      display.showNumberDecEx(ledHue, 0x00, true);
    } else if (buttonCount == 2) {
      Serial.println("✅ Hue LED disimpan ke EEPROM!");

      if (EEPROM.read(EEPROM_ADDR_HUE) != ledHue) {
        EEPROM.update(EEPROM_ADDR_HUE, ledHue);
      }

      buttonCount = 0;
    }
  }
}

// 🔹 Fungsi untuk Mengubah Warna LED WS2812 Berdasarkan Hue
void updateLedColor() {
  uint32_t color = strip.ColorHSV(ledHue * 65536 / 255);
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void displayTare() {
  uint8_t Tare[] = { 0b01111000, 0b01011111, 0b00110001, 0b01111011 };  // "T", "A", "R", "E"
  display.setSegments(Tare);
}
// 🔹 Menampilkan "LedC" di TM1637 (Pengaturan LED Color)
void displayLedC() {
  uint8_t LedC[] = {
    0b00111000,  // L
    0b01111001,  // E
    0b01011110,  // d (bentuk kecil, tampak seperti 'd')
    0b00111001   // C
  };
  display.setSegments(LedC);
}

// 🔹 Menampilkan "SetA" di TM1637 (Set Alarm)
void displaySetA() {
  uint8_t SetA[] = { 0b00111001, 0b00111000, 0b00000000, 0b01011111 };  // S, E, t, A
  display.setSegments(SetA);
}

// 🔹 Menampilkan "SetC" di TM1637 (Set Current Time)
void displaySetC() {
  uint8_t SetC[] = { 0b00111001, 0b00111000, 0b00000000, 0b01111000 };  // S, E, t, C
  display.setSegments(SetC);
}

// 🔹 Fungsi Set Alarm
void setAlarmTime() {
  static int lastStateCLK = digitalRead(ENCODER_CLK);
  int currentStateCLK = digitalRead(ENCODER_CLK);
  unsigned long currentMillis = millis();

  if (currentMillis - lastEncoderMillis > DEBOUNCE_DELAY) {
    if (buttonCount == 1) {
      if (currentStateCLK != lastStateCLK) {
        if (digitalRead(ENCODER_DT) != currentStateCLK) {
          alarmHour = (alarmHour + 1) % 24;
        } else {
          alarmHour = (alarmHour - 1 + 24) % 24;
        }
        Serial.print("Jam Alarm: ");
        Serial.println(alarmHour);
      }
      lastStateCLK = currentStateCLK;

      if (millis() - blinkTimer >= 500) {
        blinkTimer = millis();
        blinkState = !blinkState;
      }
      if (blinkState) {
        display.showNumberDecEx(alarmHour * 100 + alarmMinute, 0x40, true);
      } else {
        display.showNumberDecEx(alarmMinute, 0x00, true);
      }
    } else if (buttonCount == 2) {
      if (currentStateCLK != lastStateCLK) {
        if (digitalRead(ENCODER_DT) != currentStateCLK) {
          alarmMinute = (alarmMinute + 1) % 60;
        } else {
          alarmMinute = (alarmMinute - 1 + 60) % 60;
        }
        Serial.print("Menit Alarm: ");
        Serial.println(alarmMinute);
      }
      lastStateCLK = currentStateCLK;

      if (millis() - blinkTimer >= 500) {
        blinkTimer = millis();
        blinkState = !blinkState;
      }
      if (blinkState) {
        display.showNumberDecEx(alarmHour * 100 + alarmMinute, 0x40, true);
      } else {
        display.showNumberDecEx(alarmHour * 100, 0x00, true);
      }
    } else if (buttonCount == 3) {
      Serial.println("✅ Alarm disimpan ke EEPROM!");

      // **Simpan ke EEPROM hanya jika ada perubahan**
      if (EEPROM.read(EEPROM_ADDR_HOUR) != alarmHour) {
        EEPROM.update(EEPROM_ADDR_HOUR, alarmHour);
      }
      if (EEPROM.read(EEPROM_ADDR_MINUTE) != alarmMinute) {
        EEPROM.update(EEPROM_ADDR_MINUTE, alarmMinute);
      }

      buttonCount = 0;
    }
  }
}

// 🔹 Fungsi Set Current Time
void setCurrentTime() {
  static int lastStateCLK = digitalRead(ENCODER_CLK);
  int currentStateCLK = digitalRead(ENCODER_CLK);
  static int setHour = 0, setMinute = 0;
  static bool firstRun = true;  // Ambil waktu dari RTC hanya saat pertama kali masuk

  // 🔹 Ambil waktu saat ini dari RTC **hanya saat pertama kali masuk**
  if (firstRun) {
    DateTime now = rtc.now();
    setHour = now.hour();
    setMinute = now.minute();
    firstRun = false;
  }

  if (buttonCount == 1) {
    if (currentStateCLK != lastStateCLK) {
      if (digitalRead(ENCODER_DT) != currentStateCLK) {
        setHour = (setHour + 1) % 24;
      } else {
        setHour = (setHour - 1 + 24) % 24;
      }
      Serial.print("Jam Sekarang: ");
      Serial.println(setHour);
    }
    lastStateCLK = currentStateCLK;

    // 🔹 Efek berkedip pada 2 digit pertama
    if (millis() - blinkTimer >= 500) {
      blinkTimer = millis();
      blinkState = !blinkState;
    }
    if (blinkState) {
      display.showNumberDecEx(setHour * 100 + setMinute, 0x40, true);
    } else {
      display.showNumberDecEx(setMinute, 0x00, true);
    }
  } else if (buttonCount == 2) {
    if (currentStateCLK != lastStateCLK) {
      if (digitalRead(ENCODER_DT) != currentStateCLK) {
        setMinute = (setMinute + 1) % 60;
      } else {
        setMinute = (setMinute - 1 + 60) % 60;
      }
      Serial.print("Menit Sekarang: ");
      Serial.println(setMinute);
    }
    lastStateCLK = currentStateCLK;

    // 🔹 Efek berkedip pada 2 digit terakhir
    if (millis() - blinkTimer >= 500) {
      blinkTimer = millis();
      blinkState = !blinkState;
    }
    if (blinkState) {
      display.showNumberDecEx(setHour * 100 + setMinute, 0x40, true);
    } else {
      display.showNumberDecEx(setHour * 100, 0x00, true);
    }
  } else if (buttonCount == 3) {
    DateTime now = rtc.now();  // 🔹 Ambil tanggal sekarang dari RTC
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), setHour, setMinute, 0));
    Serial.println("✅ Waktu berhasil diperbarui di RTC!");

    // 🔹 Pastikan nilai RTC terbaru langsung ditampilkan
    delay(200);
    now = rtc.now();  // Perbarui nilai setelah rtc.adjust()
    display.showNumberDecEx(now.hour() * 100 + now.minute(), 0x40, true);

    buttonCount = 0;
    firstRun = true;  // Reset agar bisa mengambil waktu dari RTC lagi saat masuk menu ini
  }
}

// 🔹 Menampilkan Waktu di TM1637
void displayTime() {
  DateTime now = rtc.now();
  display.showNumberDecEx(now.hour() * 100 + now.minute(), 0x40, true);
}

void calibrateTare() {
  static int lastStateCLK = digitalRead(ENCODER_CLK);
  int currentStateCLK = digitalRead(ENCODER_CLK);

  // **Jika tombol ditekan, set berat saat ini sebagai tare**
  if (buttonCount == 1) {
    display.showNumberDecEx(weight, 0x00, true);
    Serial.print("✅ Berat: ");
    Serial.print(weight);
    Serial.println(" g");

  } else if (buttonCount == 2) {
    Serial.println("⚙️ Tare sedang dilakukan...");
    LoadCell.tareNoDelay();
    if (LoadCell.getTareStatus() == true) {
      Serial.println("✅ Tare Selesai!");
    }

    // **Tampilkan konfirmasi di TM1637**
    display.showNumberDecEx(0, 0x00, true);  // Menampilkan 0000 sebagai indikator tare berhasil

    // **Reset buttonCount untuk keluar dari menu Tare**
    delay(1000);
    buttonCount = 0;
  }
}

// 🔹 Fungsi untuk Mengecek Alarm
void checkAlarm() {
  Serial.print("Waktu Sekarang : ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(" | Waktu Alarm : ");
  Serial.print(alarmHour);
  Serial.print(":");
  Serial.print(alarmMinute);
  Serial.print(" | alarm status : ");
  Serial.print(alarmActive);
  Serial.print(" | Berat : ");
  Serial.println(weight);
  if (alarmActive == true) {
    Serial.println("🔔 ALARM AKTIF! Berat Melebihi Batas!");
    activateBuzzer();      // **Nyalakan Buzzer**
    ledBreathingEffect();  // **Aktifkan efek LED breathing**
  }
}

// 🔹 Fungsi untuk Mengaktifkan Buzzer
void activateBuzzer() {
  static unsigned long lastBuzzerMillis = 0;
  static bool buzzerState = false;

  unsigned long currentMillis = millis();

  if (!buzzerActive) {
    buzzerActive = true;
    lastBuzzerMillis = currentMillis;
    buzzerState = true;
    digitalWrite(BUZZER_PIN, HIGH);
  }

  // 🔹 Matikan buzzer setelah BUZZER_DURATION ms
  if (buzzerState && (currentMillis - lastBuzzerMillis >= BUZZER_DURATION)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
    lastBuzzerMillis = currentMillis;  // Mulai jeda
  }

  // 🔹 Aktifkan kembali setelah BUZZER_INTERVAL ms
  if (!buzzerState && (currentMillis - lastBuzzerMillis >= BUZZER_INTERVAL)) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerState = true;
    lastBuzzerMillis = currentMillis;  // Mulai hitungan ulang
  }
}


void ledBreathingEffect() {
  static int brightness = 0;
  static int fadeAmount = 5;  // Besar perubahan brightness setiap step
  static unsigned long lastUpdate = 0;

  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= 50) {  // Perbarui setiap 50ms untuk efek halus
    lastUpdate = currentMillis;

    // 🔹 Ubah kecerahan (breathing)
    brightness += fadeAmount;
    if (brightness <= 50 || brightness >= 255) {  // Batasi agar tidak terlalu gelap
      fadeAmount = -fadeAmount;                   // Balik arah fade
    }

    // 🔹 Konversi warna Hue ke RGB dengan kecerahan yang bervariasi
    uint32_t color = strip.ColorHSV(ledHue * 65536 / 255, 255, brightness);
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
  }
}

void resetCounterIfInactive() {
  if (millis() - lastCounterChangeTime >= 100000) {  // 10 detik tanpa perubahan
    if (counter != 0) {                             // Reset hanya jika counter bukan 0
      Serial.println("⏳ Tidak ada perubahan selama 10 detik, counter kembali ke 0");
      counter = 0;
    }
    lastCounterChangeTime = millis();  // Reset timer agar tidak terus mencetak log
  }
}