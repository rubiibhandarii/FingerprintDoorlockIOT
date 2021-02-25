#define BLYNK_PRINT Serial


#include <BlynkSimpleEsp8266.h>

#include <Servo.h>

#include <Adafruit_Fingerprint.h>
#include <ESP8266WiFi.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(D7, D8);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

const char auth[] = "Md0deOJQms1xxQJkgbduATyCP7rExASj"; //Auth code sent via Email
const char ssid[] = "Jamc"; //Wifi name
const char pass[] = "9841227604";  //Wifi Password


int pos = 0; 
const int buzzer=D0;
const int servoPIN=D1;

void setup()
{
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

  myservo.attach(servoPIN);  
  pinMode(buzzer, OUTPUT); 

  Blynk.begin(auth, ssid, pass);
}

void loop()                     // run over and over again
{
  Blynk.run();
  getFingerprintID();
  delay(50);            //don't ned to run this at full speed.
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");

    Blynk.notify("Alert : Welcome home.");

 for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  delay(5000);
  
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }



    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
     tone(buzzer, 1000); // Send 1KHz sound signal...
     delay(500);        // ...for 1 sec
     noTone(buzzer);     // Stop sound...
     delay(500);        // ...for 1sec
     Serial.println("Communication error");
     return p;
  } else if (p == FINGERPRINT_NOTFOUND) {

    Blynk.notify("Alert : Unknown fingerprint! Unknown access!");
    
     tone(buzzer, 1000); // Send 1KHz sound signal...
     delay(500);        // ...for 1 sec
     noTone(buzzer);     // Stop sound...
     delay(500);
     tone(buzzer, 1000);// ...for 1sec
     delay(500);
     noTone(buzzer);     // Stop sound...
     delay(500);
     tone(buzzer, 1000);// ...for 1sec
     delay(500);
     noTone(buzzer);     // Stop sound...
     delay(500);
     Serial.println("Did not find a match");
     return p;
  } else {
     tone(buzzer, 1000); // Send 1KHz sound signal...
     delay(1000);        // ...for 1 sec
     noTone(buzzer);     // Stop sound...
     delay(500);        // ...for 1sec
     Serial.println("Unknown error");
     return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
