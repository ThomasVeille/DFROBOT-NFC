#include <SPI.h>
#include <MFRC522.h>
#include "Arduino_LED_Matrix.h"

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
// LED MATRIX

ArduinoLEDMatrix matrix;

// Maximum number of authorized TAGs
#define MAX_AUTHORIZED_TAGS 10

// Maximum UID size
#define MAX_UID_SIZE 10


// List of authorized UIDs
byte authorizedUIDs[MAX_AUTHORIZED_TAGS][MAX_UID_SIZE] = {

  // TAG 1
  {0xF7, 0x66, 0xA9, 0x5F},

  // TAG 2
  // Example:
  {0x47, 0xB8, 0x44, 0x62},

};


// UID size of each authorized TAG
byte authorizedUIDSizes[MAX_AUTHORIZED_TAGS] = {

  // TAG 1
  4,

  // TAG 2
  4,

};


// Number of authorized TAGs
byte authorizedTagCount = 2;

// LED MATRIX

// CHECK MARK
const uint32_t accessGranted[] = {
  0x08004002,
  0x00100080,
  0x04008010
};

// CROSS
const uint32_t accessDenied[] = {
  0x20410809,
  0x00600600,
  0x90108204
};

// EMPTY MATRIX
const uint32_t matrixClear[] = {
  0x00000000,
  0x00000000,
  0x00000000
};


void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  matrix.begin();

  // Clear matrix
  matrix.loadFrame(matrixClear);


  Serial.println();
  Serial.println("=================================");
  Serial.println("       RFID ACCESS CONTROL");
  Serial.println("=================================");
  Serial.println();

  Serial.println("Place your card on the RC522...");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;


  Serial.println();
  Serial.println("=================================");
  Serial.println("          CARD DETECTED");
  Serial.println("=================================");


  Serial.print("UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10)
      Serial.print("0");

    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();


  if (isAuthorized()) {


    Serial.println();
    Serial.println("---------------------------------");
    Serial.println("        ACCESS GRANTED");
    Serial.println("---------------------------------");
    Serial.println("This TAG is authorized.");


    matrix.loadFrame(accessGranted);

  }
  else {

    Serial.println();
    Serial.println("---------------------------------");
    Serial.println("        ACCESS DENIED");
    Serial.println("---------------------------------");
    Serial.println("This TAG is NOT authorized.");

    matrix.loadFrame(accessDenied);
  }


  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(3000);

  matrix.loadFrame(matrixClear);


  Serial.println();
  Serial.println("Place another card on the RC522...");
}


bool isAuthorized() {

  // Check every authorized TAG
  for (byte tag = 0; tag < authorizedTagCount; tag++) {

    bool match = true;

    // Check UID length
    if (rfid.uid.size != authorizedUIDSizes[tag]) {

      match = false;
    }
    else {

      // Compare every UID byte
      for (byte i = 0; i < rfid.uid.size; i++) {

        if (rfid.uid.uidByte[i] != authorizedUIDs[tag][i]) {

          match = false;
          break;
        }
      }
    }

    // UID matches an authorized TAG
    if (match) {

      Serial.print("Authorized TAG number: ");
      Serial.println(tag + 1);

      return true;
    }
  }

  return false;
}
