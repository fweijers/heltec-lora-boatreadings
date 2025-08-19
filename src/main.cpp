#include <Arduino.h>
#include <Wire.h>
#include <Seeed_BME280.h>
#include <LoRaWan_APP.h>
#include "ttnparams.h"


// === LoRaWAN instellingen ===
uint32_t appTxDutyCycle = 180000; // Verzending elke 3 minuten 
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = LORAWAN_CLASS;
bool overTheAirActivation = LORAWAN_NETMODE;
bool loraWanAdr = LORAWAN_ADR;
bool keepNet = LORAWAN_NET_RESERVE;
bool isTxConfirmed = LORAWAN_UPLINKMODE;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;

// === Sensorwaarden ===
int temperature, humidity, batteryVoltage, batteryLevel;
long pressure;
uint16_t distance = 0;
uint16_t externalVoltage = 0;

BME280 bme280;

// === JSN-SR04T pinconfiguratie ===
#define trigPin GPIO5  // J2-9
#define echoPin GPIO0  // J2-8


// === Ultrasoon meting ===
long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(30);
  digitalWrite(trigPin, LOW);

  unsigned long startMicros = micros();
  while (digitalRead(echoPin) == LOW) {
    if (micros() - startMicros > 30000) return 0;
  }

  unsigned long echoStart = micros();
  while (digitalRead(echoPin) == HIGH) {
    if (micros() - echoStart > 30000) return 0;
  }

  long duration = micros() - echoStart;
  return (uint16_t)(duration * 0.034 / 2); // in cm
}


// === Spanningsdeler uitlezen met gemeten referentiespanning ===
#define ADC_VREF 2.38  // 🔧 Gemeten referentiespanning, i.p.v. 3.3V
#define EXTERNAL_VOLTAGE_PIN ADC
#define EXTERNAL_VOLTAGE_R1 11500  // bovenste weerstand in spanningsdeler
#define EXTERNAL_VOLTAGE_R2 4700   // onderste weerstand in spanningsdeler

uint16_t readExternalVoltage() {
  delay(50);  // Tijd geven aan spanning
  uint16_t raw = analogRead(EXTERNAL_VOLTAGE_PIN); // 12-bit waarde tussen 0-4095

  Serial.print("🧪 Raw ADC: "); Serial.println(raw);

  // Gebruik gemeten referentiespanning
  float vMeasured = raw * (ADC_VREF / 4095.0);  // spanning op de ADC-pin
  Serial.print("🔍 Gemeten op pin: "); Serial.print(vMeasured); Serial.println(" V");

  float vReal = vMeasured * ((EXTERNAL_VOLTAGE_R1 + EXTERNAL_VOLTAGE_R2) / (float)EXTERNAL_VOLTAGE_R2);
  Serial.print("🔌 Teruggerekend: "); Serial.print(vReal); Serial.println(" V");

  return (uint16_t)(vReal * 1000);  // millivolt
}


// === Payload voorbereiden ===
static void prepareTxFrame(uint8_t port) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW); // Vext aan
  delay(500);

  Wire.begin();
  if (!bme280.init()) {
    Serial.println("⚠️ BME280 niet gevonden!");
  }
  delay(500);

  temperature = bme280.getTemperature() * 100;
  humidity = bme280.getHumidity();
  pressure = bme280.getPressure();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  distance = measureDistance();

  externalVoltage = readExternalVoltage();

  digitalWrite(Vext, HIGH); // Vext uit
  Wire.end();

  batteryVoltage = getBatteryVoltage();
  batteryLevel = (BoardGetBatteryLevel() / 254.0) * 100;

  // === Payload: 16 bytes ===
  appDataSize = 16;
  appData[0]  = highByte(temperature);
  appData[1]  = lowByte(temperature);
  appData[2]  = highByte(humidity);
  appData[3]  = lowByte(humidity);
  appData[4]  = (pressure >> 24) & 0xFF;
  appData[5]  = (pressure >> 16) & 0xFF;
  appData[6]  = (pressure >> 8) & 0xFF;
  appData[7]  = pressure & 0xFF;
  appData[8]  = highByte(batteryVoltage);
  appData[9]  = lowByte(batteryVoltage);
  appData[10] = highByte(batteryLevel);
  appData[11] = lowByte(batteryLevel);
  appData[12] = highByte(distance);
  appData[13] = lowByte(distance);
  appData[14] = highByte(externalVoltage);
  appData[15] = lowByte(externalVoltage);

  // === Logging ===
  Serial.print("🌡️ Temp: "); Serial.print(temperature / 100.0); Serial.print(" °C, ");
  Serial.print("💧 Hum: "); Serial.print(humidity); Serial.print(" %, ");
  Serial.print("⏲️ Pres: "); Serial.print(pressure / 100.0); Serial.print(" hPa, ");
  Serial.print("🔋 Vbat: "); Serial.print(batteryVoltage); Serial.print(" mV, ");
  Serial.print("⚡ Level: "); Serial.print(batteryLevel); Serial.print(" %, ");
  Serial.print("📏 Dist: "); Serial.print(distance); Serial.print(" cm, ");
  Serial.print("🔌 ExtV: "); Serial.print(externalVoltage); Serial.println(" mV");
}

// === Setup en LoRaWAN loop ===
void setup() {
  boardInitMcu();
  Serial.begin(115200);
  delay(2000);
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();
}

void loop() {
  switch (deviceState) {
    case DEVICE_STATE_INIT:
      printDevParam();
      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;
    case DEVICE_STATE_JOIN:
      LoRaWAN.join();
      break;
    case DEVICE_STATE_SEND:
      prepareTxFrame(appPort);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    case DEVICE_STATE_CYCLE:
      txDutyCycleTime = appTxDutyCycle + randr(0, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    case DEVICE_STATE_SLEEP:
      LoRaWAN.sleep();
      break;   
    default:
      deviceState = DEVICE_STATE_INIT;
      break;
  }
}
