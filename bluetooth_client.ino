/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/


#include "BLEDevice.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "ESP32_BME280"


//UUID's of the service, characteristic that we want to read
static BLEUUID bmeServiceUUID(BLEUUID((uint16_t)0x181A));


//Temperature Characteristic
static BLEUUID temperatureCharacteristicUUID((uint16_t)0x2A6E);


//Humidity Characteristic
static BLEUUID humidityCharacteristicUUID((uint16_t)0x2A6F);

static BLEUUID pressureCharacteristicUUID("bab4a75f-95d3-45c7-b1c7-7b3e395f4610");
static BLEUUID button1CharacteristicUUID("479be236-4615-46d5-91ad-89ab8ce29773");


//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;


//Address of the peripheral device. Address will be found during scanning..
static BLEAddress *pServerAddress;
 
//Characteristic that we want to read and characteristic that we want to write.
static BLERemoteCharacteristic* temperatureCharacteristic;
static BLERemoteCharacteristic* humidityCharacteristic;
static BLERemoteCharacteristic* pressureCharacteristic;
static BLERemoteCharacteristic* button1Characteristic;

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

int  button_value;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels


//Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire,-1);


//Variables to store temperature and humidity
#define MAX_STRING_LENGTH 10
char temperatureR[MAX_STRING_LENGTH];
char humidityR[MAX_STRING_LENGTH];
char pressureR[16];
char button1R[MAX_STRING_LENGTH];


//Flags to check whether new temperature and humidity readings are available
boolean newTemperatureR = false;
boolean newHumidityR = false;
boolean newPressureR = false;
boolean newButton1R =false;

const int  led = 26;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bmeServiceUUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
  humidityCharacteristic = pRemoteService->getCharacteristic(humidityCharacteristicUUID);
  pressureCharacteristic = pRemoteService->getCharacteristic(pressureCharacteristicUUID);
  button1Characteristic = pRemoteService->getCharacteristic(button1CharacteristicUUID);


  if (temperatureCharacteristic == nullptr || humidityCharacteristic == nullptr || pressureCharacteristic == nullptr || button1Characteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  //Assign callback functions for the Characteristics
  temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
  humidityCharacteristic->registerForNotify(humidityNotifyCallback);
  pressureCharacteristic->registerForNotify(pressureNotifyCallback);
  button1Characteristic->registerForNotify(button1NotifyCallback);

  return true;
}


//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};
 
//When the BLE Server sends a new temperature reading with the notify property
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                      uint8_t* pData, size_t length, bool isNotify) {
  // Reinterpret the received data as a uint16_t value
  uint16_t temperatureValue = *(uint16_t*)pData;
    Serial.print("Temp: ");
  Serial.println(temperatureValue);
  // Convert the temperature value to a string for display
  snprintf(temperatureR, MAX_STRING_LENGTH, "%d", temperatureValue);
  newTemperatureR = true;
}


//When the BLE Server sends a new humidity reading with the notify property
static void humidityNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                   uint8_t* pData, size_t length, bool isNotify) {
  // Reinterpret the received data as a uint16_t value
  uint16_t humidityValue = *(uint16_t*)pData;
  // Convert the humidity value to a string for display
  snprintf(humidityR, MAX_STRING_LENGTH, "%d", humidityValue);
  newHumidityR = true;
}

//When the BLE Server sends a new humidity reading with the notify property
static void pressureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                   uint8_t* pData, size_t length, bool isNotify) {
  // Reinterpret the received data as a uint16_t value
  float pressureValue = *(uint16_t*)pData;
  Serial.print("pressure: ");
  Serial.println(pressureValue);
  // Convert the humidity value to a string for display
  snprintf(pressureR, MAX_STRING_LENGTH, "%f", pressureValue);
  newPressureR = true;
}

//When the BLE Server sends a new humidity reading with the notify property
static void button1NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                   uint8_t* pData, size_t length, bool isNotify) {
  // Reinterpret the received data as a uint16_t value
  
  
  int button1Value = *(uint16_t*)pData;
  Serial.println("Button 1 value: ");
  Serial.print(button1Value);
  button_value = button1Value;
  Serial.println("Inside function button value: ");
  Serial.print(button_value);

  if (button_value == 1) {
  digitalWrite(led, HIGH); // Turn on LED
  Serial.println("led button: ");
  Serial.print(button_value);
  } else if (button_value == 0) {
  digitalWrite(led, LOW); // Turn off LED
  Serial.println("led button: ");
  Serial.print(button_value);
  }

  // Convert the humidity value to a string for display
  snprintf(button1R, MAX_STRING_LENGTH, "%d", button1Value);
  newButton1R = true;
  
}

//function that prints the latest sensor readings in the OLED display
void printbmeReadings(){
  display.clearDisplay();
  display.setTextColor(WHITE);
 
  //display temperature
  display.setTextSize(1.5);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(1.5);
  display.setCursor(80,0);
  display.print(temperatureR);
  display.print(" ");
  display.setTextSize(1.5);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1.5);
  display.print("C");
  Serial.print("Temperature:");
  Serial.print(temperatureR);
  Serial.print("ºC");


  //display humidity
  display.setTextSize(1.5);
  display.setCursor(0, 8);
  display.print("Humidity: ");
  display.setTextSize(1.5);
  display.setCursor(60, 8);
  display.print(humidityR);
  display.print(" %");
  Serial.print(" Humidity:");
  Serial.print(humidityR);
  Serial.println("%");


  //display humidity
  display.setTextSize(1.5);
  display.setCursor(0, 16);
  display.print("Pressure: ");
  display.setTextSize(1.5);
  display.setCursor(60, 16);
  display.print(pressureR);
  display.print(" Pa");
  Serial.print(" Pressure:");
  Serial.print(pressureR);
  Serial.println("Pa");

  display.display();
}


void setup() {

  pinMode(led, OUTPUT);
  //Start serial communication
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");




  //OLED display setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
 
  delay(2000);
  display.clearDisplay();
  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}


void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      humidityCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      pressureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      button1Characteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);


      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  //if new temperature readings are available, print in the OLED
  if (newTemperatureR && newHumidityR && newPressureR){
    newTemperatureR = false;
    newHumidityR = false;
    newPressureR = false;
    printbmeReadings();
  }

  delay(1000); // Delay a second between loops.
}
