#include <DFRobot_PN532.h>
#include <HKDF.h>
#include <SHA256.h>

#define PN532_IRQ 2
#define POLLING 0
#define BLOCK_SIZE 16

DFRobot_PN532_IIC nfc(PN532_IRQ, POLLING);

uint8_t dataRead[BLOCK_SIZE];
uint8_t derivedKeys[96]; // 16 keys x 6 bytes, computed once per tag

void deriveKeys(uint8_t *uid) {
  uint8_t master[16] = {
    0x9A, 0x75, 0x9C, 0xF2, 0xC4, 0xF7, 0xCA, 0xFF,
    0x22, 0x2C, 0xB9, 0x76, 0x9B, 0x41, 0xBC, 0x96
  };
  uint8_t context[] = { 'R', 'F', 'I', 'D', '-', 'A', 0x00 };

  HKDF<SHA256> hkdf;
  hkdf.setKey(uid, 4, master, sizeof(master));
  hkdf.extract(derivedKeys, sizeof(derivedKeys), context, sizeof(context));
}

// Reads a block, automatically using its sector's derived key
bool readBambuBlock(uint8_t blockNumber, uint8_t *buffer) {
  uint8_t sector = blockNumber / 4;
  for (int i = 0; i < 6; i++) {
    nfc.nfcPassword[i] = derivedKeys[sector * 6 + i];
  }
  return (nfc.readData(buffer, blockNumber) == 1);
}

void printAsText(uint8_t *buf, int len) {
  for (int i = 0; i < len; i++) {
    Serial.print((buf[i] >= 32 && buf[i] <= 126) ? (char)buf[i] : '.');
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" DFR0231 - Bambu Spool Reader");
  Serial.println(" Arduino UNO R4");
  Serial.println("==============================");

  while (!nfc.begin()) {
    Serial.println("NFC init failed");
    delay(2000);
  }

  Serial.println("Ready. Please present a spool...");
}

void loop() {
  if (!nfc.scan()) return;

  deriveKeys(nfc.nfcUid);

  Serial.println();
  Serial.println("==============================");
  Serial.println("       SPOOL DETECTED");
  Serial.println("==============================");

  // --- Block 1: Material variant / material ID ---
  if (readBambuBlock(1, dataRead)) {
    Serial.print("Material variant: ");
    printAsText(dataRead, 8);
    Serial.print(" | Material ID: ");
    printAsText(dataRead + 8, 8);
    Serial.println();
  }

  // --- Block 2: Filament type ---
  if (readBambuBlock(2, dataRead)) {
    Serial.print("Filament type: ");
    printAsText(dataRead, 16);
    Serial.println();
  }

  // --- Block 4: Detailed type ---
  if (readBambuBlock(4, dataRead)) {
    Serial.print("Detailed type: ");
    printAsText(dataRead, 16);
    Serial.println();
  }

  // --- Block 5: Color, weight, diameter ---
  if (readBambuBlock(5, dataRead)) {
    Serial.print("Color (RGBA): ");
    for (int i = 0; i < 4; i++) {
      if (dataRead[i] < 0x10) Serial.print("0");
      Serial.print(dataRead[i], HEX);
    }
    Serial.println();

    uint16_t weight;
    memcpy(&weight, dataRead + 4, 2);
    Serial.print("Spool weight: ");
    Serial.print(weight);
    Serial.println(" g");

    float diameter;
    memcpy(&diameter, dataRead + 8, 4);
    Serial.print("Filament diameter: ");
    Serial.print(diameter, 2);
    Serial.println(" mm");
  }

  // --- Block 6: Temperatures / drying ---
  if (readBambuBlock(6, dataRead)) {
    uint16_t dryTemp, dryTime, bedTempType, bedTemp, maxHotend, minHotend;
    memcpy(&dryTemp, dataRead + 0, 2);
    memcpy(&dryTime, dataRead + 2, 2);
    memcpy(&bedTempType, dataRead + 4, 2);
    memcpy(&bedTemp, dataRead + 6, 2);
    memcpy(&maxHotend, dataRead + 8, 2);
    memcpy(&minHotend, dataRead + 10, 2);

    Serial.print("Drying: "); Serial.print(dryTemp); Serial.print(" C for "); Serial.print(dryTime); Serial.println("h");
    Serial.print("Bed temperature: "); Serial.print(bedTemp); Serial.println(" C");
    Serial.print("Nozzle temperature: "); Serial.print(minHotend); Serial.print(" - "); Serial.print(maxHotend); Serial.println(" C");
  }

  // --- Block 9: Tray UID ---
  if (readBambuBlock(9, dataRead)) {
    Serial.print("Tray UID: ");
    for (int i = 0; i < 16; i++) {
      if (dataRead[i] < 0x10) Serial.print("0");
      Serial.print(dataRead[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }

  // --- Block 10: Spool width ---
  if (readBambuBlock(10, dataRead)) {
    uint16_t width;
    memcpy(&width, dataRead + 4, 2);
    Serial.print("Spool width: ");
    Serial.print(width / 100.0, 2);
    Serial.println(" mm");
  }

  // --- Block 12: Production date ---
  if (readBambuBlock(12, dataRead)) {
    Serial.print("Production date: ");
    printAsText(dataRead, 16);
    Serial.println();
  }

  // --- Block 14: Filament length ---
  if (readBambuBlock(14, dataRead)) {
    uint16_t length;
    memcpy(&length, dataRead + 4, 2);
    Serial.print("Filament length (approx): ");
    Serial.print(length);
    Serial.println(" m");
  }

  Serial.println("==============================");
  Serial.println();

  delay(3000);
}