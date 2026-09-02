#include <DFRobot_PN532.h>
#include <HKDF.h>
#include <SHA256.h>

#define PN532_IRQ 2
#define POLLING 0

#define BLOCK_SIZE 16

DFRobot_PN532_IIC nfc(PN532_IRQ, POLLING);

uint8_t dataRead[BLOCK_SIZE];

void printBlock(uint8_t blockNumber) {

  Serial.print("Reading block ");
  Serial.print(blockNumber);
  Serial.println("...");

  if (nfc.readData(dataRead, blockNumber) == 1) {

    Serial.print("Block ");
    Serial.print(blockNumber);
    Serial.println(" HEX:");

    for (int i = 0; i < BLOCK_SIZE; i++) {

      if (dataRead[i] < 0x10)
        Serial.print("0");

      Serial.print(dataRead[i], HEX);
      Serial.print(" ");
    }

    Serial.println();

    Serial.print("Block ");
    Serial.print(blockNumber);
    Serial.println(" TEXT:");

    for (int i = 0; i < BLOCK_SIZE; i++) {

      if (dataRead[i] >= 32 && dataRead[i] <= 126)
        Serial.print((char)dataRead[i]);
      else
        Serial.print(".");
    }

    Serial.println();
    Serial.println();

  } else {

    Serial.print("ERROR: Cannot read block ");
    Serial.println(blockNumber);
    Serial.println();
  }
}

void deriveKey(uint8_t *uid) {

  // Master key
  uint8_t master[16] = {
    0x9A, 0x75, 0x9C, 0xF2,
    0xC4, 0xF7, 0xCA, 0xFF,
    0x22, 0x2C, 0xB9, 0x76,
    0x9B, 0x41, 0xBC, 0x96
  };

  // Context
  uint8_t context[] = {
    'R', 'F', 'I', 'D', '-', 'A', 0x00
  };

  // 16 clés × 6 octets = 96 octets
  uint8_t derivedKeys[96];

  HKDF<SHA256> hkdf;

  // Dérivation avec l'UID du TAG
  hkdf.setKey(uid, 4, master, sizeof(master));

  hkdf.extract(
    derivedKeys,
    sizeof(derivedKeys),
    context,
    sizeof(context)
  );

  // Key 1
  // Key 0 = octets 0-5
  // Key 1 = octets 6-11
  for (int i = 0; i < 6; i++) {
    nfc.nfcPassword[i] = derivedKeys[6 + i];
  }
}

void setup() {

  Serial.begin(115200);



  delay(1000);

  //testKeyDerivation();

  Serial.println();
  Serial.println("==============================");
  Serial.println(" DFR0231 NFC READER");
  Serial.println(" Arduino UNO R4");
  Serial.println("==============================");
  Serial.println();

  Serial.println("Starting NFC reader...");

  while (!nfc.begin()) {

    Serial.println("NFC initialization failed!");
    Serial.println("Check wiring and I2C mode.");

    delay(2000);
  }

  Serial.println("NFC reader OK!");
  // nfc.nfcPassword[0] = 0xC9;
  // nfc.nfcPassword[1] = 0x39;
  // nfc.nfcPassword[2] = 0x57;
  // nfc.nfcPassword[3] = 0xB3;
  // nfc.nfcPassword[4] = 0xD1;
  // nfc.nfcPassword[5] = 0xCF;
  Serial.println();
  Serial.println("Place your NFC card on the reader...");
}


void loop() {
  if (!nfc.scan()) return;

  deriveKey(nfc.nfcUid);

  Serial.println();
  Serial.println("==============================");
  Serial.println("       CARD DETECTED");
  Serial.println("==============================");

  Serial.print("UID: ");

  for (int i = 0; i < 4; i++) {
    if (nfc.nfcUid[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(nfc.nfcUid[i], HEX);
    if (i < 3) {
      Serial.print(" ");
    }
  }

  
  Serial.println();
  Serial.println();

  printBlock(4);

  delay(2000);
}