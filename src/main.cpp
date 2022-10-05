#include <Arduino.h>
#include <flightmanager.hpp>


#if SPI_DRIVER_SELECT == 2  // Must be set in SdFat/SdFatConfig.h

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI

SdFat sd;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int sdaPin = A4;
const int sclPin = A5;

SoftWire softWire(sdaPin, sclPin);

char swTxBuffer[16];
char swRxBuffer[16];

Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

TinyGPSPlus gps;
FlightManager flightManager(gps, softWire, sd, sensors, mpr);


AsyncDelay readInterval;

unsigned long lastGPS = 0;

void setup() {
  Serial.begin(9600);
  GPSSerial.begin(9600);
  softWire.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
  softWire.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
  softWire.setDelay_us(5);
  softWire.setTimeout(1000);
  softWire.begin();
  if (!sd.begin(SD_CONFIG)) {
      sd.initErrorHalt();
  }
  if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
    while (1) {
      delay(10);
    }
  }
  sensors.begin();
}

void loop() {
  flightManager.updateGPS();
  if(millis() - lastGPS > GPSdelay){
    flightManager.printSensorsData();
    lastGPS = millis();
  }
}
#else  // SPI_DRIVER_SELECT
#error SPI_DRIVER_SELECT must be two in SdFat/SdFatConfig.h
#endif  //SPI_DRIVER_SELECT