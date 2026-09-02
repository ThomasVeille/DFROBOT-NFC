#include <DFRobot_PN532.h>

#define PN532_IRQ 2
#define POLLING 0

#define BLOCK_SIZE 16

DFRobot_PN532_IIC nfc(PN532_IRQ, POLLING);

uint8_t dataRead[BLOCK_SIZE];

// Le message à écrire dans le bloc 2 (16 caractères max, complété par des zéros si plus court)
uint8_t dataWrite[BLOCK_SIZE] = "Atelier 3D-01"; // <-- change ici l'info que tu veux stocker

bool alreadyWritten = false; // pour n'écrire qu'une seule fois par passage de carte

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

// Nouvelle fonction : écrire 16 octets dans un bloc donné
void writeBlock(uint8_t blockNumber, uint8_t *data) {

  Serial.print("Writing block ");
  Serial.print(blockNumber);
  Serial.println("...");

  if (nfc.writeData(blockNumber, data) == 1) {
    Serial.println("Write successful!");
  } else {
    Serial.println("ERROR: write failed");
  }
  Serial.println();
}

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" DFR0231 NFC READER/WRITER");
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
  Serial.println();
  Serial.println("Place your NFC card on the reader...");
}

void loop() {

  if (!nfc.scan()) {
    alreadyWritten = false; // carte retirée, on réarme pour la prochaine
    return;
  }

  Serial.println();
  Serial.println("==============================");
  Serial.println("       CARD DETECTED");
  Serial.println("==============================");

  // Lecture avant écriture (pour comparer)
  printBlock(0);
  printBlock(2);

  // Écriture du bloc 2 (une seule fois par passage de carte)
  if (!alreadyWritten) {
    writeBlock(2, dataWrite);
    alreadyWritten = true;

    // Relecture pour confirmer que l'écriture a bien pris
    Serial.println("Vérification après écriture :");
    printBlock(2);
  }

  Serial.println("==============================");
  Serial.println("Finished reading card.");
  Serial.println();

  delay(2000);
}