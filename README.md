# NFC Communication

## Overview

This projet group few test code to understand how works NFC communication with utilisation like Bambu spool; NFC card or TAG. For this projet, the DFR0231-H and the RFID-RC522 boards were used.
The DRF0231-H board have been used to add some message and also read the information of the different NFC TAG. For the RFID-RC522, it was only used to read the information of the NFC TAG.
To use the two NFC card, Arduino UNO R4 WIFI have been used during all the test.

## Connection with the Arduino UNO board

### 1. DFR0231-H

This board can be used with to communication solution : I2C and UART. For all the solution, the communication used is the **I2C** communication.
For the connection with the Arduino UNO board need to use the following table :

| DRF0231-H Pin| Arduino UNO Pin |
|--------------------|--------|
|**D/T**              | A4     |
|**C/R**              | A5     |
|**GND**              | GND     |
|**VCC**              | 5V     |

### 2. RFID-RC522

For the connection with the Arduino UNO board need to use the following table and the following picture :

| RFID-RC522 Pin| Arduino UNO Pin |
|--------------------|--------|
|**SDA**              | ~10     |
|**SCK**              | 13     |
|**MOSI**              | ~11     |
|**MISO**              | 12     |
|**IRQ**              | No connected     |
|**GND**              | GND     |
|**RST**              | ~9     |
|**3V3**              | 3V3     |


<p align="center">
  <img src="./RC522-Arduino.png" width="500">
  <br>
  RC522 connection with Arduino UNO
</p>

## What does the code do ?

### A-DRFOBOT

For this code, the DRF0231-H have been use during the test.
The code **DFROBOT** allow to know information about the Bambu Lab spool. The following information can be obtained :

- Material variant
- Material ID
- Filament type
- Detailed type
- Color (RGBA)
- Spool weight
- Filament diameter
- Drying
- Bed temperature
- Nozzle temperature
- Spool width
- Production date
- Filament length

To have the information, need to place on the NFC card the Bambu spool few secondes. The information of the Bambu Lab will be write in the serial monitoring between **========== line**.
All the information come from the following link : https://github.com/Bambu-Research-Group/RFID-Tag-Guide/blob/main/BambuLabRfid.md#block-5 . All the spool can't be print in the Arduino code because they don't have a lot of information (for the moment).
### B-WRITE

For this code, the DFR0231-H have been use during the test.
The code **WRITE** allow to add some information into a NFC card or TAG. When you place the TAG on the NFC card, you can see the different BLOCK with their HEXA message. After, we can enter the number of the BLOCK you want know the message.
In the code we can change the message you want to have in one BLOCK (16 characters max for one BLOCK). 

```cpp
uint8_t dataWrite[BLOCK_SIZE] = "CHANGE MESSAGE"; 
```

You can also edit each BLOCK, just need to edit the printblock() argument with the number of the BLOCK you want

```cpp
printBlock(2);
```
### C-RC522

For this code need to use the RFID-RC522 board.
The code **RC522** allow to read information of the NFC card or TAG. When a card or TAG is placed on the NFC board, all the information of the different BLOCK can be saw in HEXA. After we can enter a number to see the translate message in text.
