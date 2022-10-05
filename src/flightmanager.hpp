#include <TinyGPSPlus.h>
#include <SoftWire.h>
#include <OneWire.h>
#include <Wire.h>
#include "Adafruit_MPRLS.h"
#include <DallasTemperature.h>
#include <AsyncDelay.h>
#include "SdFat.h"
#include <config.hpp>

#define GPSSerial Serial1

const uint8_t I2C_ADDRESS = 0x68;

class FlightManager {
public:
    FlightManager(TinyGPSPlus& gps, SoftWire& softWire, SdFat& sd, DallasTemperature& sensors, Adafruit_MPRLS& mpr) : gps(gps), softWire(softWire), sd(sd), sensors(sensors), mpr(mpr) {}
    void updateGPS();
    void printGPS();
    void printTime();
    void printTemperature();
    void printPressure();
    void printSensorsData();
    void saveGPSDataOnSDCard();
private:
    void printTwoDigit(int n, bool sdcard = false);
    void readTime();
    void readPressure();

    TinyGPSPlus& gps;
    SoftWire& softWire;
    int year;
    int month;
    int dateOfMonth;
    int hour;
    int minute;
    int second;
    File file;
    SdFat& sd;
    DallasTemperature& sensors;
    Adafruit_MPRLS& mpr;
    float pressure_hPa;
};