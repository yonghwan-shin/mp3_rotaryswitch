#include <OneButton.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

volatile int number = 0;                // Testnumber, print it when it changes value,
// used in loop and both interrupt routines
int oldnumber = number;

bool playStarted = false;
bool justStarted = false;
volatile boolean halfleft = false;      // Used in both interrupt routines
volatile boolean halfright = false;
uint16_t volume = 10;
OneButton button(A0, true);


void setup() {


  mySoftwareSerial.begin(9600);
  Serial.begin(115200);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  pinMode(2, INPUT);
  digitalWrite(2, HIGH);                // Turn on internal pullup resistor
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);                // Turn on internal pullup resistor
  attachInterrupt(0, isr_2, FALLING);   // Call isr_2 when digital pin 2 goes LOW
  attachInterrupt(1, isr_3, FALLING);   // Call isr_3 when digital pin 3 goes LOW
  button.attachClick(singleclick);
  button.attachDoubleClick([]() {
    Serial.println("Double pressed");
    myDFPlayer.next();
  });

  myDFPlayer.setTimeOut(500);
  Serial.print("Volume ");
  Serial.println(volume);
  myDFPlayer.volume(10);  //Set volume value (0~30).
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.loopFolder(1);
}

void loop() {
  if (number != oldnumber) {            // Change in value ?

    Serial.println(-number);             // Yes, print it (or whatever)
    oldnumber = number;
    myDFPlayer.volume(volume - number);
    Serial.print("Volume changed:");
    Serial.println(volume - number);
  }
  button.tick() ;

  if (justStarted == false) {
    Serial.println("Folder 1 started");
    myDFPlayer.loopFolder(1);
    delay(500);
    myDFPlayer.pause();
    Serial.println("Paused,, waiting for signal");
    justStarted = true;
  }
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}


void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}

void singleclick() {
  
  if ( playStarted == false) {
    playStarted = true;
    myDFPlayer.start();
    Serial.println("clicked: start");
  } else {
    playStarted = false;
    myDFPlayer.pause();
    Serial.println("clicked: pause");
  }

}
void isr_2() {                                             // Pin2 went LOW
  delay(1);                                                // Debounce time
  if (digitalRead(2) == LOW) {                             // Pin2 still LOW ?
    if (digitalRead(3) == HIGH && halfright == false) {    // -->
      halfright = true;                                    // One half click clockwise
    }
    if (digitalRead(3) == LOW && halfleft == true) {       // <--
      halfleft = false;                                    // One whole click counter-
      number--;                                            // clockwise
    }
  }
}
void isr_3() {                                            // Pin3 went LOW
  delay(1);                                               // Debounce time
  if (digitalRead(3) == LOW) {                            // Pin3 still LOW ?
    if (digitalRead(2) == HIGH && halfleft == false) {    // <--
      halfleft = true;                                    // One half  click counter-
    }                                                     // clockwise
    if (digitalRead(2) == LOW && halfright == true) {     // -->
      halfright = false;                                  // One whole click clockwise
      number++;
    }
  }
}
