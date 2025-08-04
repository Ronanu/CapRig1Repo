#include <Arduino.h>
#include <pb.h>
#include <pb_common.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <messages.pb.h>

// Globale Nachrichten
FromEsp32 fromMsg;
ToEsp32 toMsg;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("ESP32 Protobuf ready.");
}

void sendStatus() {
  fromMsg.timestamp = millis();
  fromMsg.temperature = 22.5f;
  fromMsg.humidity = 55.5f;
  strncpy(fromMsg.status, "OK", sizeof(fromMsg.status));
  fromMsg.status[sizeof(fromMsg.status) - 1] = '\0'; // Sicherheit
  fromMsg.hash = 0; // optional berechnen

  uint8_t buffer[128];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, FromEsp32_fields, &fromMsg)) {
    Serial.println("Encoding failed!");
    return;
  }

  // Länge senden
  Serial.write((uint8_t)(stream.bytes_written >> 8));
  Serial.write((uint8_t)(stream.bytes_written & 0xFF));
  Serial.write(buffer, stream.bytes_written);
}

void loop() {
  // Warte auf 2 Bytes für die Länge
  if (Serial.available() >= 2) {
    uint16_t len = ((uint16_t)Serial.read()) << 8;
    len |= Serial.read();

    if (len > 128) return;

    uint8_t buffer[128];
    size_t i = 0;
    while (i < len) {
      if (Serial.available()) {
        buffer[i++] = Serial.read();
      }
    }

    pb_istream_t stream = pb_istream_from_buffer(buffer, len);
    if (!pb_decode(&stream, ToEsp32_fields, &toMsg)) {
      Serial.println("Decoding failed!");
      return;
    }

    if (strcmp(toMsg.command, "get_status") == 0) {
      sendStatus();
    }
  }
}
