#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println("Place your card on the RC522...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.println("\n--- CARD DETECTED ---");

  // UID
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // SAK (Select Acknowledge) - utile pour identifier le type précisément
  Serial.print("SAK: 0x");
  Serial.println(rfid.uid.sak, HEX);

  // Type de carte
  MFRC522::PICC_Type type = rfid.PICC_GetType(rfid.uid.sak);
  Serial.print("Card type: ");
  Serial.println(rfid.PICC_GetTypeName(type));

  rfid.PICC_HaltA();

  delay(2000);
}