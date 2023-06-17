/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with Atom-Lite/Matrix sample source code
*                          配套  Atom-Lite/Matrix 示例源代码
* Visit the website for more
*information：https://docs.m5stack.com/en/core/atom_matrix
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/atom_matrix
*
* Product: Button example.  按键示例
* Date: 2021/7/21
*******************************************************************************
*/
#include "M5Atom.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("connect");
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    Serial.println("disconnect");
    deviceConnected = false;
    BLEAdvertising* pAdvertising;
    pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) {
    Serial.println("read");
    pCharacteristic->setValue("Hello World!");
  }

  void onWrite(BLECharacteristic* pCharacteristic) {
    Serial.println("write");
    std::string value = pCharacteristic->getValue();
    Serial.println(value.c_str());
    if (value.c_str()[0] == '0') {
      M5.dis.drawpix(0, 0xff0000);
    } else {
      M5.dis.drawpix(0, 0x00ff00);
    }
  }
  void onNotify(BLECharacteristic* pCharacteristic){
Serial.println("notify");    
  }
};

void setup() {
  Serial.begin(115200);
  M5.begin(true, false, true);

  BLEDevice::init("m5-stack");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising* pAdvertising;
  setAdvertisementData();
  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

uint8_t pushCount = 0;
uint8_t loopCount = 0;
void setAdvertisementData() {
  BLEAdvertising* pAdvertising;
  pAdvertising = pServer->getAdvertising();
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04);  // BR_EDR_NOT_SUPPORTED | LE General Discoverable Mode
  std::string strServiceData = "";
  strServiceData += (char)0x05;  // length
  strServiceData += (char)0x16;  // AD Type 0xFF: Manufacturer specific data
  strServiceData += (char)0x01;  // Test manufacture ID low byte
  strServiceData += (char)0x23;  // Test manufacture ID high byte
  strServiceData += (char)'X';   // Test manufacture ID high byte
  strServiceData += (char)pushCount;
  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);
}
void loop() {
  if (M5.Btn.wasPressed()) {
    if (pushCount == 0xff) {
      pushCount = 0;
    } else {
      pushCount++;
    }
    pCharacteristic->setValue(&pushCount,sizeof(uint8_t));
    pCharacteristic->notify(true);
  }
  if (loopCount == 20) {
    setAdvertisementData();
  }
  if (loopCount == 20) {
    loopCount = 0;
  } else {
    loopCount++;
  }
  M5.update();
  delay(50);
}
