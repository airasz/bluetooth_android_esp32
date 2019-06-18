#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

String pushmsg;

int indexOfBody;
String header;
String body1, body2, body3, body4;
String msgg[] = {body1, body2, body3, body4};
// String msgg[4]={};

bool flashInfo = 0;
bool showmsg = 0;
int previousMillis = 0;
// String body1, body2,body3,body4;
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{

  String info;
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0)
    {
      info = "";
      Serial.println("*********");
      Serial.print("Received Value: ");

      for (int i = 0; i < rxValue.length(); i++)
      {

        Serial.print(rxValue[i]);
        info += (rxValue[i]);
      }
      pushmsg = info;
      // oled(info);
      showmsg = 1;
      Serial.println();
      Serial.print("info = ");
      Serial.println(info);
      Serial.println();
      Serial.println("*********");
      splittingMsg();
    }
  }

  void oled(String text)
  {

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(info);
    display.display();
  }
  void splittingMsg()
  {
    int ss1=60,ss2=ss1*2,ss3=ss1*3,ss4=ss1*4;
    int nl = pushmsg.indexOf(0x0A);
    header = pushmsg.substring(0, nl);
    Serial.print("header");
    Serial.println(header);
    String msgbody = pushmsg.substring(nl + 1);
    Serial.print("msgbody =");
    Serial.println(msgbody);

    int msglength = msgbody.length();
    Serial.print("msglength = ");
    Serial.println(msglength);
    if (msglength < 200 && msglength > 150)
    {
      body1 = msgbody.substring(0, ss1);
      body2 = msgbody.substring(ss1, ss2);
      body3 = msgbody.substring(ss2, ss3);
      body4 = msgbody.substring(ss3);
      // msgg[4]={body1,body2,body3,body4};
      indexOfBody = 4;
    }
    else if (msglength < 150 && msglength > 100)
    {
      body1 = msgbody.substring(0, ss1);
      body2 = msgbody.substring(ss1, ss2);
      body3 = msgbody.substring(ss2, ss3);
      indexOfBody = 3;
    }
    else if (msglength < 120 && msglength > 60)
    {
      body1 = msgbody.substring(0, ss1);
      body2 = msgbody.substring(ss1, ss2);
      indexOfBody = 2;
    }
    else if (msglength < 60)
    {
      body1 = msgbody;
      // msgg[]={body1};
      // strncpy(msgg,msgbody,0);
      indexOfBody = 1;
    }
    indexOfBody=(msglength/ss1)+1;
  }
};

void setup()
{
  Serial.begin(115200);

  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 128x32)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.println("ESP assisty!");
  display.display();
  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}
void loop()
{
  // notify changed value
  if (deviceConnected)
  {
    //pCharacteristic->setValue(&value, 1);
    //pCharacteristic->notify();
    // value++;
    // delay(10); // bluetooth stack will go into congestion, if too many packets are sent
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    oled("reconnected!");
    flashInfo = 1;
  }
  displaymsg();
}
void oled(String txt)
{
  if (flashInfo)
  {
    display.setCursor(0, 0);
    display.print(txt);
    display.display();
    flashInfo = false;
  }
}
int countindex;
void displaymsg()
{
  unsigned long currentMillis = millis();
  if (previousMillis + 3000 == millis())
  {
    if (showmsg)
    {
      countindex++;
      if (countindex < indexOfBody + 1)
      {
        if (countindex == 1)
          displayinfo(body1);
        if (countindex == 2)
          displayinfo(body2);
        if (countindex == 3)
          displayinfo(body3);
        if (countindex == 4)
          displayinfo(body4);
        // displayinfo(body+String(countindex));
        // Serial.print("countindex>");
        // Serial.println(countindex);
        // Serial.print("indexOfBody>");
        // Serial.println(indexOfBody);
      }
      else
      {

        countindex = 0;
      }
    }
    previousMillis = millis();
  }
}
void displayinfo(String txt)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(header);
  display.setCursor(0, 9);
  // Serial.print("msgg = ");
  // Serial.println(msgg[countindex-1]);
  display.print(txt);
  display.display();
}
// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// BLEServer *pServer = NULL;
// BLECharacteristic * pTxCharacteristic;
// bool deviceConnected = false;
// bool oldDeviceConnected = false;
// uint8_t txValue = 0;

// // See the following for generating UUIDs:
// // https://www.uuidgenerator.net/

// #define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
// #define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
// #define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//       deviceConnected = true;
//     };

//     void onDisconnect(BLEServer* pServer) {
//       deviceConnected = false;
//     }
// };

// class MyCallbacks: public BLECharacteristicCallbacks {
//     void onWrite(BLECharacteristic *pCharacteristic) {
//       std::string rxValue = pCharacteristic->getValue();

//       if (rxValue.length() > 0) {
//         Serial.println("*********");
//         Serial.print("Received Value: ");
//         for (int i = 0; i < rxValue.length(); i++)
//           Serial.print(rxValue[i]);

//         Serial.println();
//         Serial.println("*********");
//       }
//     }
// };

// void setup() {
//   Serial.begin(115200);

//   // Create the BLE Device
//   BLEDevice::init("SR2");

//   // Create the BLE Server
//   pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());

//   // Create the BLE Service
//   BLEService *pService = pServer->createService(SERVICE_UUID);

//   // Create a BLE Characteristic
//   pTxCharacteristic = pService->createCharacteristic(
// 										CHARACTERISTIC_UUID_TX,
// 										BLECharacteristic::PROPERTY_NOTIFY
// 									);

//   pTxCharacteristic->addDescriptor(new BLE2902());

//   BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
// 											 CHARACTERISTIC_UUID_RX,
// 											BLECharacteristic::PROPERTY_WRITE
// 										);

//   pRxCharacteristic->setCallbacks(new MyCallbacks());

//   // Start the service
//   pService->start();

//   // Start advertising
//   pServer->getAdvertising()->start();
//   Serial.println("Waiting a client connection to notify...");
// }

// void loop() {

//     if (deviceConnected) {
//         pTxCharacteristic->setValue(&txValue, 1);
//         pTxCharacteristic->notify();
//         txValue++;
// 		delay(10); // bluetooth stack will go into congestion, if too many packets are sent
// 	}

//     // disconnecting
//     if (!deviceConnected && oldDeviceConnected) {
//         delay(500); // give the bluetooth stack the chance to get things ready
//         pServer->startAdvertising(); // restart advertising
//         Serial.println("start advertising");
//         oldDeviceConnected = deviceConnected;
//     }
//     // connecting
//     if (deviceConnected && !oldDeviceConnected) {
// 		// do stuff here on connecting
//         oldDeviceConnected = deviceConnected;
//     }
// }
