#include <M5StickC.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

VL53L0X sensor;
BLEServer* pServer;
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }  // Wait for serial console to open!
  Wire.begin(0, 26, 100000UL);
  M5.begin();
  M5.Lcd.setRotation(3);
  Serial.println("Start");
  sensor.setTimeout(500);
  Serial.println("VL53L0X");
  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }
  Serial.println("initialized");

  Serial.println("Ble");
  BLEDevice::init("EnvSensor");         // デバイスを初期化
  pServer = BLEDevice::createServer();  // サーバーを生成
  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

uint8_t counter = 0;
void loop() {
  M5.update();

  uint16_t range = sensor.readRangeSingleMillimeters();

  Serial.print(range);
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.println();
  if (M5.BtnB.wasPressed()) {
    esp_restart();
  }

  //print
  M5.Lcd.fillRect(0, 20, 319, 239, BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print("range:");
  M5.Lcd.println(range);

  // ble


  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x06);  // BR_EDR_NOT_SUPPORTED | LE General Discoverable Mode

  std::string strServiceData = "";
  strServiceData += (char)0x07;                   // length
  strServiceData += (char)0x16;                   // AD Type 0xFF: Manufacturer specific data
  strServiceData += (char)0x01;                   // Test manufacture ID low byte
  strServiceData += (char)0x23;                   // Test manufacture ID high byte
  strServiceData += (char)0x5a;                   //count
  strServiceData += (char)((range >> 8) & 0xff);  //range
  strServiceData += (char)(range & 0xff);
  strServiceData += (char)counter;

  oAdvertisementData.addData(strServiceData);
  BLEAdvertising* pAdvertising;
  pAdvertising = pServer->getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  if (counter == 0xff) {
    counter = 0;
  } else {
    counter++;
  }
  delay(1000);
}