#include <M5StickC.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <Adafruit_SGP30.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

VL53L0X sensor;
Adafruit_SGP30 sgp;
BLEServer *pServer;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }  // Wait for serial console to open!
  Wire.begin();
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
  Serial.println("SGP30");
  if (!sgp.begin()) {
    Serial.println("Failed to detect and initialize sensor!");
    while (1)
      ;
  }
  Serial.println("initialized");
  Serial.println("Ble");
  BLEDevice::init("EnvSensor");         // デバイスを初期化
  pServer = BLEDevice::createServer();  // サーバーを生成
}

int counter = 0;
void loop() {
  M5.update();
  if (!sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  uint16_t range = sensor.readRangeSingleMillimeters();
  uint16_t tvoc = sgp.TVOC;
  uint16_t co2 = sgp.eCO2;

  Serial.print(range);
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.println();
  if (M5.BtnB.wasPressed()) {
    esp_restart();
  }


  Serial.print("TVOC ");
  Serial.print(sgp.TVOC);
  Serial.print(" ppb\t");
  Serial.print("eCO2 ");
  Serial.print(sgp.eCO2);
  Serial.println(" ppm");

  if (!sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.print("Raw H2 ");
  Serial.print(sgp.rawH2);
  Serial.print(" \t");
  Serial.print("Raw Ethanol ");
  Serial.print(sgp.rawEthanol);
  Serial.println("");


  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x");
    Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x");
    Serial.println(TVOC_base, HEX);
  }
  //print
  M5.Lcd.fillRect(0, 20, 319, 239, BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print("range:");
  M5.Lcd.println(range);
  M5.Lcd.print("tvoc:");
  M5.Lcd.println(tvoc);
  M5.Lcd.print("co2:");
  M5.Lcd.println(co2);
  // ble


  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x06);  // BR_EDR_NOT_SUPPORTED | LE General Discoverable Mode

  std::string strServiceData = "";
  strServiceData += (char)0x09;                   // length
  strServiceData += (char)0x16;                   // AD Type 0xFF: Manufacturer specific data
  strServiceData += (char)0xff;                   // Test manufacture ID low byte
  strServiceData += (char)0xff;                   // Test manufacture ID high byte
  strServiceData += (char)((range >> 8) & 0xff);  //range
  strServiceData += (char)(range & 0xff);
  strServiceData += (char)((tvoc >> 8) & 0xff);  // tvoc
  strServiceData += (char)(tvoc & 0xff);
  strServiceData += (char)((co2 >> 8) & 0xff);  // co2
  strServiceData += (char)(co2 & 0xff);

  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->start();
  Serial.println("Advertizing started...");

  delay(5000);
  pAdvertising->stop();
}