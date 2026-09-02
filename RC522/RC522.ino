#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Default MIFARE key
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Place your card on the RC522...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.println("\n--- CARD DETECTED ---");

  // Print UID
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Only read MIFARE Classic
  MFRC522::PICC_Type type = rfid.PICC_GetType(rfid.uid.sak);

  if (type != MFRC522::PICC_TYPE_MIFARE_MINI &&
      type != MFRC522::PICC_TYPE_MIFARE_1K &&
      type != MFRC522::PICC_TYPE_MIFARE_4K) {

    Serial.println("Not a MIFARE Classic card.");
    rfid.PICC_HaltA();
    return;
  }

  Serial.println("Reading memory...");

  // Read blocks 1-63
  for (byte block = 1; block < 64; block++) {

    byte buffer[18];
    byte size = sizeof(buffer);

    // Sector trailer blocks are 3, 7, 11, etc.
    if ((block + 1) % 4 == 0)
      continue;

    MFRC522::StatusCode status;

    status = rfid.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A,
      block,
      &key,
      &(rfid.uid)
    );

    if (status != MFRC522::STATUS_OK) {
      Serial.print("Block ");
      Serial.print(block);
      Serial.print(": Authentication failed - ");
      Serial.println(rfid.GetStatusCodeName(status));
      continue;
    }

    status = rfid.MIFARE_Read(block, buffer, &size);

    if (status != MFRC522::STATUS_OK) {
      Serial.print("Block ");
      Serial.print(block);
      Serial.print(": Read failed");
      Serial.println();
      continue;
    }

    Serial.print("Block ");
    Serial.print(block);
    Serial.print(": ");

    for (byte i = 0; i < 16; i++) {
      Serial.print(buffer[i] < 0x10 ? "0" : "");
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }

    Serial.println();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  Serial.println("--- DONE ---");

  delay(2000);
}