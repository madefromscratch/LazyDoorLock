#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// For Motor
//Declare pin functions on Redboard
#define stp 0
#define dir 0
#define MS1 0
#define MS2 0
#define EN  0

// For NFC Reader
// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
// TODO: redo these values since the board has changed
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
//Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
// also change #define in Adafruit_PN532.cpp library file
   #define Serial SerialUSB
#endif


// Some initialization
uint8_t OPEN[] = {0x4F, 0x50, 0x45, 0x4E};
uint8_t CLOSE[] = {0x43, 0x4C, 0X4F, 0X53, 0X45};
uint8_t HELLO[] = {0x48, 0x45, 0x4C, 0x4C, 0x4F};
uint8_t selectApdu[] = {0x00, /* CLA */
                            0xA4, /* INS */
                            0x04, /* P1 */
                            0x00, /* P2 */
                            0x07, /* Length of AID */ 0xF0, 0x69, 0x69, 0x69, 0x69, 0x80, 0x08, /* AID as defined on Android App */
                            0x00 /* Le */};
uint8_t queryApdu[] = {0x51, 0x55, 0x45, 0x52, 0x59}; // "QUERY"
uint32_t DELAYTIMER = 2000;
int x;

void setup(void) {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}

// Reset Easy Driver Motor pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}

//Default microstep mode function
void StepForwardDefault()
{
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  // TODO: figure out what's the right number
  for(x= 1; x<500; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

//Reverse default microstep mode function
void StepBackwardDefault()
{
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  // TODO: figure out what's the right number
  for(x= 1; x<500; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  bool success;
  uint8_t responseLength = 16;
  uint8_t resposne[responseLength];

// 0 - Start
// 1 - Connected
// 2 - Command
// 3 - Performing
// NFC
//              0 - inListPassiveTarget()
//      success -> 1    failure -> 0
//          
//              1 - send AID()
//      success -> 2    failure -> 0
//          
//              2 - send queryCommand()
//      success -> 4    failure -> 0
// 
//              3 - do action
//                  set timer, expires -> 0
// 
// Android
//          0 - waiting for Hello, send response
//      received -> 1   failure -> 0
//          
//          1 - waiting for query command, send response
//      received -> 0 failure -> 0

  success = nfc.inListPassiveTarget();
  if (success) {
    Serial.println("Found a card!");
    success = nfc.inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength);
    nfc.PrintHexChar(response, responseLength);
    if (success && memcmp(response, HELLO, sizeof(HELLO) / sizeof(uint8_t)) == 0) {
      success = nfc.inDataExchange(queryApdu, sizeof(queryApdu), response, &responseLength);
      nfc.PrintHexChar(response, responseLength);
      responseLength = 16;
      if (success) {
        if (memcmp(response, OPEN, sizeof(OPEN) / sizeof(uint8_t)) == 0) {
          // do open rotation
//          StepForwardDefault();
          success = nfc.inDataExchange(OPEN, sizeof(OPEN), response, &responseLength);
          nfc.PrintHexChar(response, responseLength);
          delay(DELAYTIMER);
        } else if (memcmp(response, CLOSE, sizeof(CLOSE) / sizeof(uint8_t)) == 0) {
          // do close rotation
//          StepBackwardDefault();
          success = nfc.inDataExchange(CLOSE, sizeof(CLOSE), response, &responseLength);
          nfc.PrintHexChar(response, responseLength);
          delay(DELAYTIMER);
        } else {
            Serial.println("NOT OPEN OR CLOSE");
        }
      }
    }
  }
  resetEDPins();
}
