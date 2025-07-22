#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_CS   5
#define SD_PWR  4  // PNP-Transistor: HIGH = AUS, LOW = EIN

void powerCycleSDCard() {
  Serial.println("Power cycling SD card...");

  // SPI-Bus entkoppeln, CS auf HIGH halten
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);   // Verhindert SPI-Start im falschen Zustand

  // SPI stoppen, falls aktiv
  SPI.end();
  delay(50);

  // SD-Karte stromlos schalten
  pinMode(SD_PWR, OUTPUT);
  Serial.println("Karte AUS");
  digitalWrite(SD_PWR, HIGH);   // PNP sperrt -> Karte AUS
  delay(10000);                   // mindestens 500 ms stromlos

  // SD-Karte wieder einschalten
  Serial.println("Karte EIN");
  digitalWrite(SD_PWR, LOW);    // PNP leitet -> Karte EIN
  delay(500);                   // kurz warten

  // SPI neu initialisieren
  SPI.begin();                  // startet SCLK, MOSI, MISO
  delay(100);                   // SD-Karte braucht Zeit nach Einschalten
}



void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ESP32 SD Card Test mit echtem Power Reset ===");

  powerCycleSDCard();  // SD-Karte hart zurücksetzen

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Init fehlgeschlagen – prüfe Stromversorgung oder Karte.");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) Serial.println("MMC");
  else if (cardType == CARD_SD) Serial.println("SDSC");
  else if (cardType == CARD_SDHC) Serial.println("SDHC");
  else Serial.println("UNKNOWN");

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 1);
  writeFile(SD, "/hello.txt", "Hello with true reset\n");
  readFile(SD, "/hello.txt");
}

void loop() {
}
