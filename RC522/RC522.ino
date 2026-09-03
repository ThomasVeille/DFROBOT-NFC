#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Stockage des blocs lus
byte blockData[64][16];
bool blockValid[64] = {false};


void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  // Clé MIFARE par défaut
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


  // =========================
  // AFFICHER L'UID
  // =========================

  Serial.print("UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();


  // =========================
  // VERIFIER LE TYPE DE CARTE
  // =========================

  MFRC522::PICC_Type type = rfid.PICC_GetType(rfid.uid.sak);

  if (type != MFRC522::PICC_TYPE_MIFARE_MINI &&
      type != MFRC522::PICC_TYPE_MIFARE_1K &&
      type != MFRC522::PICC_TYPE_MIFARE_4K) {

    Serial.println("Not a MIFARE Classic card.");

    rfid.PICC_HaltA();

    return;
  }


  Serial.println("Reading memory...");


  // =========================
  // LECTURE DES BLOCS
  // =========================

  // Ici on lit les blocs 1 à 63
  for (byte block = 1; block < 64; block++) {

    byte buffer[18];
    byte size = sizeof(buffer);


    // Les blocs 3, 7, 11, etc.
    // sont les Sector Trailers.
    if ((block + 1) % 4 == 0)
      continue;


    MFRC522::StatusCode status;


    // =========================
    // AUTHENTIFICATION
    // =========================

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


    // =========================
    // LECTURE DU BLOC
    // =========================

    status = rfid.MIFARE_Read(block, buffer, &size);


    if (status != MFRC522::STATUS_OK) {

      Serial.print("Block ");
      Serial.print(block);
      Serial.print(": Read failed");
      Serial.println();

      continue;
    }


    // =========================
    // SAUVEGARDER LE BLOC
    // =========================

    for (byte i = 0; i < 16; i++) {

      blockData[block][i] = buffer[i];
    }

    blockValid[block] = true;


    // =========================
    // AFFICHER LE BLOC
    // =========================

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


  // =========================
  // FIN DE COMMUNICATION AVEC LE TAG
  // =========================

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  Serial.println("--- DONE ---");


  // =========================
  // DEMANDER UN BLOC
  // =========================

  Serial.println();
  Serial.println("Enter block number to convert to text :");


  while (Serial.available() == 0) {
    // Attendre que l'utilisateur entre un numéro
  }


  int blockNumber = Serial.parseInt();


  // Nettoyer les éventuels caractères restants
  while (Serial.available() > 0) {
    Serial.read();
  }


  // =========================
  // AFFICHER LE BLOC CHOISI
  // =========================

  readAndConvertBlock(blockNumber);


  Serial.println();
  Serial.println("Place another card on the RC522...");

  delay(2000);
}


// =====================================================
// FONCTION POUR AFFICHER UN BLOC SAUVEGARDE
// =====================================================

void readAndConvertBlock(byte blockNumber) {

  // Vérifier que le numéro est valide
  if (blockNumber >= 64) {

    Serial.println("Invalid block number.");

    return;
  }


  // Vérifier que le bloc a bien été lu
  if (!blockValid[blockNumber]) {

    Serial.println("This block was not read.");

    return;
  }


  Serial.println();

  Serial.print("Reading saved block ");
  Serial.println(blockNumber);


  // =========================
  // AFFICHAGE HEX
  // =========================

  Serial.print("HEX : ");

  for (byte i = 0; i < 16; i++) {

    if (blockData[blockNumber][i] < 0x10)
      Serial.print("0");

    Serial.print(blockData[blockNumber][i], HEX);
    Serial.print(" ");
  }

  Serial.println();


  // =========================
  // AFFICHAGE TEXT / ASCII
  // =========================

  Serial.print("TEXT: ");

  for (byte i = 0; i < 16; i++) {

    byte value = blockData[blockNumber][i];


    // Caractères ASCII imprimables
    if (value >= 32 && value <= 126) {

      Serial.print((char)value);

    } else {

      // Pour les 00, caractères non imprimables, etc.
      Serial.print(".");
    }
  }

  Serial.println();
}