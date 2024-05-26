/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>


//BLE server name
#define bleServerName "ESP32_BME280"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Default UUID for Environmental Sensing Service
// https://www.bluetooth.com/specifications/assigned-numbers/
#define SERVICE_UUID (BLEUUID((uint16_t)0x181A))

// Temperature Characteristic and Descriptor (default UUID)
// Check the default UUIDs here: https://www.bluetooth.com/specifications/assigned-numbers/
BLECharacteristic bmeTemperatureCharacteristic(BLEUUID((uint16_t)0x2A6E), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor (default UUID)
BLECharacteristic bmeHumidityCharacteristic(BLEUUID((uint16_t)0x2A6F), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic button1Characteristic(BLEUUID("479be236-4615-46d5-91ad-89ab8ce29773"), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor button1Descriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic bmePressureCharacteristic(BLEUUID("bab4a75f-95d3-45c7-b1c7-7b3e395f4610"), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmePressureDescriptor(BLEUUID((uint16_t)0x2902));

// DHT Sensor
/*
const int DHTPin = 14;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
*/
Adafruit_BME280 bme; // I2C
const int button1 = 16;


bool deviceConnected = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
};

void setup() {
  // Start DHT sensor
  //dht.begin();
  pinMode(button1, INPUT);
  // Start serial communication 
  Serial.begin(115200);


  //START BME280 SENSOR
  if (!bme.begin(0x77)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }


  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and corresponding Descriptors
  bmeService->addCharacteristic(&bmeTemperatureCharacteristic);
  bmeTemperatureCharacteristic.addDescriptor(&bmeTemperatureDescriptor);
  
  bmeService->addCharacteristic(&bmeHumidityCharacteristic);
  bmeHumidityCharacteristic.addDescriptor(&bmeHumidityDescriptor);

  bmeService->addCharacteristic(&bmePressureCharacteristic);
  bmePressureCharacteristic.addDescriptor(&bmePressureDescriptor);
  
  bmeService->addCharacteristic(&button1Characteristic);
  button1Characteristic.addDescriptor(&button1Descriptor);

  // Start the service
  bmeService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

int button1_state = 0;

void loop() {
  if (deviceConnected) {
    // Read temperature as Celsius (the default)
    float t = bme.readTemperature();

    // Read humidity
    float h = bme.readHumidity();

    //read pressure
    float p = (bme.readPressure()/100.0F);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(p) ) {
      Serial.println("Failed to read from  BME280 sensor!");
      return;
    }
    
    //Notify temperature reading from BME sensor
    uint16_t temperatureCTemp = (uint16_t)t;
    //Set temperature Characteristic value and notify connected client
    bmeTemperatureCharacteristic.setValue(temperatureCTemp);
    bmeTemperatureCharacteristic.notify();
    Serial.print("Temperature Celsius: ");
    Serial.print(t);
    Serial.print(" *C");
   
    
    //Notify humidity reading from BME
    uint16_t humidityTemp = (uint16_t)h;
    //Set humidity Characteristic value and notify connected client
    bmeHumidityCharacteristic.setValue(humidityTemp);
    bmeHumidityCharacteristic.notify();   
    Serial.print(" - Humidity: ");
    Serial.print(h);
    Serial.println(" %");

  
    uint16_t pressure = (uint16_t)p;
    //Set humidity Characteristic value and notify connected client
    bmePressureCharacteristic.setValue(pressure);
    bmePressureCharacteristic.notify();   
    Serial.print(" - Pressure: ");
    Serial.print(p);
    Serial.println(" Pa");


    if(digitalRead(button1) == HIGH){
      while(digitalRead(button1) == HIGH){}
      button1_state ++;
      if(button1_state > 1){
        button1_state = 0;
      }
      Serial.print(button1_state);
      uint16_t button_state = (uint16_t)button1_state;
      button1Characteristic.setValue(button_state);
      button1Characteristic.notify();  
    }

    delay(1000);
  }
}