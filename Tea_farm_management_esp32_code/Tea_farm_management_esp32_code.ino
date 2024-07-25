/* Declaration from the Author:
The portions of the codes responsible for interfacing the load cell with ESP32 Microcontroller is adapted from the works posted by Rui Santos on RandomNerdTutorials.com
The portions of the code related to RFID and uploading to google sheet is adapted from Himanshu Sharma. The GitHub project is available at https://github.com/himanshus2847/IoT-Attendance-System-using-RFID
*/

// Message from Rui Santos
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-load-cell-hx711/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Library HX711 by Bogdan Necula: https://github.com/bogde/HX711
// Library: pushbutton by polulu: https://github.com/pololu/pushbutton-arduino

#include <Arduino.h>
#include "HX711.h"  // Library for using the HX711 amplifier for reading the data from a load cell
#include "soc/rtc.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Library for using a OLED Display
#include <Pushbutton.h> // Library for simplifying the code of pushbutton

#include <SPI.h>
#include <MFRC522.h> // Library for using RFID Card

#include "WiFi.h"
#include <HTTPClient.h>

/* Edit Your WiFi Name and Password. */
const char* ssid = "Your_WiFi_Name";
const char* password = "Your_WiFi_Password";
String upload_link = "YOUR_GOOGLE_SHEET_UPLOAD_LINK"; /* For example: "https://script.google.com/macros/s/xxxxxxx__xx_xxxxxxxxxxxxxxx_xxxx_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/exec" */


int httpCode = 0;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 5000;

/* HX711 circuit wiring */
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;

/*---------------rfid initials---------------*/
#define SS_PIN  5  // ESP32 pin GIOP5 
#define RST_PIN 27 // ESP32 pin GIOP27 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

/* Setting the block to which we want to write data */
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;
/* Create an array of 16 Bytes and fill it with data */

/* Length of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

/* The RFID card has ID numbers written on it which corresponds to Staffs of the Facility. 
The ID is 8 digits long. For example "00000008".
  */
String convertedID = " ";
String idOld = " "; // The previously scanned ID
bool newCardScanned = false;
MFRC522::StatusCode status;


#define LED_PIN 26

HX711 scale;
int reading;
int lastReading;

//REPLACE WITH YOUR CALIBRATION FACTOR
#define CALIBRATION_FACTOR 391.33

//OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Button
#define BUTTON_PIN 15
#define TARE_PIN 33
Pushbutton button(BUTTON_PIN);
Pushbutton buttonTare(TARE_PIN);

#define PIN_RED 13
#define PIN_GREEN 14
#define PIN_BLUE 26

//#include <Fonts/FreeSans9pt7b.h>
void displayText(String message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println(message);
  display.display();
  delay(1000);
}

void displayIdWeight(String id, int weight) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("ID:");
  display.println(id);
  display.println("Weight: ");
  display.print(weight);
  display.print(" g");
  display.display();

}

void ReadDataFromBlock(int blockNum, byte readBlockData[])
{
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF; /* The key for reading the data is 11111111 */
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    Serial.println("Error from Card Authentication Failure.");
    displayText("Scan Again!");
    setLedColor('r'); // Utilizing an RGB LED for warning signal.
    return;
  }


  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  bufferLen = 18; //Remember to re-initialize bufferlen, because it gets reset after the read call
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    displayText("Scan Again!");
    setLedColor('r');
    return;
  }

}

/* A function to detect when the same card is being scanned more than one time  */
bool dataMatches(String idOldLocal, String idNew)
{

  if (idOldLocal == idNew)
  {
    return true;
  }
  else
  {
    idOld = idNew;
    return false;
  }
}


/* Uploads the scanned ID and weight reading from the load cell to the Google Sheet. 
To upload to the Google Sheet, we need the uploading link obtained from the Sheet which is stored in the variable upload_link.
The Instructions to generate the upload link will be described in the ReadMe file.  */

void upload(String id,int weight)
{

    if(WiFi.status()== WL_CONNECTED)
    {
        HTTPClient http;
        setLedColor('b');
        String serverPath = String(upload_link) + "?id="+ id +"&tea-leaf-weight="+String(weight);
        Serial.print("POST data to spreadsheet:");
        Serial.println(serverPath);
        http.begin(serverPath.c_str());
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        httpCode = http.GET(); 
        if (httpCode != 200)
        {
          displayText("Scan Again");
          setLedColor('n');
          setLedColor('r');
        }
        else
        {
          displayText("Upload Successful");
          setLedColor('n');
          setLedColor('g');
        }
        Serial.print("HTTP Status Code: ");
        Serial.println(httpCode);
        //---------------------------------------------------------------------
        //getting response from google sheet
        String payload;
        if (httpCode > 0) {
            payload = http.getString();
            Serial.println("Payload: "+payload);    
        }
        http.end();
        delay(1000);
        setLedColor('n');
    }
}


void setLedColor(char code)
{
  switch (code)
  {
    case 'r': // Red
      ledfn(255,0,0);
      break;
    case 'g': // Green
      ledfn(0, 255, 0);
      break;
    case 'b': // Blue
      ledfn(0, 0, 255);
      break;
    case 'y': // Yellow
      ledfn(255,200,0);
      break;
    case 'v': // Violet
      ledfn(143, 0, 255);
      break;
    case 'n': // LED Off
      ledfn(0,0,0);
      break;
  }
}

// Function to write the PWM values to RGB LED Pins.
void ledfn(int r, int g, int b)
{
  analogWrite(PIN_RED, r);
  analogWrite(PIN_GREEN, g);
  analogWrite(PIN_BLUE, b);
}

void setup() {

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  
  WiFi.begin(ssid, password);
  Serial.begin(9600);
  SPI.begin();
  startMillis = millis();
  pinMode(LED_PIN, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) 
  {
    setLedColor('n');
    setLedColor('v');
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  setLedColor('n');
  Serial.println("Connected to the WiFi network");
  delay(500);


  mfrc522.PCD_Init();
  Serial.println("rfid reader initiated");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(CALIBRATION_FACTOR);   // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
}

void loop() {

  mfrc522.PCD_Init();
  newCardScanned = false;
  if (mfrc522.PICC_IsNewCardPresent())
  {
    if (mfrc522.PICC_ReadCardSerial())
    {
      newCardScanned = true;
      ReadDataFromBlock(blockNum, readBlockData);
      convertedID = String((char*) readBlockData);
      convertedID.trim();
      currentMillis = millis();

        displayIdWeight(convertedID, lastReading);
        startMillis = currentMillis;
      
    }
  }
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if ((button.getSingleDebouncedPress() && currentMillis - startMillis <= period) || httpCode == -1)  /* The upload button must be pressed with period */
  {
    if (!dataMatches(idOld, convertedID))
    {
    Serial.println("Upload Data to Sheet");
    upload(convertedID, lastReading);      
    }
    else
    {
      displayText("Already Uploaded");
      for(int i=1; i<4; i++) // Show a pattern of LED light to detect this state
      {
        setLedColor('r');
        delay(100);
        setLedColor('n');
        delay(100);
      }
      
      Serial.println(idOld);
      Serial.println(convertedID);
    }
  }
  

  if (scale.wait_ready_timeout(200)) {
    if (buttonTare.getSingleDebouncedPress())
    {
      scale.tare();
/*
This line of code is important. The load cell you will be using may not be perfect.
It may gradually show an offset in reading. Taring before getting a reading solves this problem. 
*/

    }
    reading = round(scale.get_units());

    currentMillis = millis();
    if (reading != lastReading && currentMillis - startMillis >= period ) {

      displayIdWeight("-------", lastReading);
    } 
    /* The display showing the readings resets after the time specified by "period"  */
    lastReading = reading;
  }
  else {
    Serial.println("HX711 not found.");
  }
  
}
