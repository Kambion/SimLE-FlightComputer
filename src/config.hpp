//BASIC SETTINGS
struct Settings {
    bool debugLogs = true;
    bool temperatureSensors = true;
    bool pressureSensors = true;
    bool gpsModule = true;
    bool SDcard = false;
};
//ONEWIRE TEMPERATURE CONFIG
#define ONE_WIRE_BUS 5
//

//GENERAL CONFIG
constexpr unsigned long GPSSavePeriod = 5000;
constexpr unsigned long SensorsSavePeriod = 5000;
constexpr unsigned long LoRaTransmissionPeriod = 10000;
//


//SD SOFTSPI CONFIG
const uint8_t SD_CS_PIN = 10;
const uint8_t SOFT_MISO_PIN = 12;
const uint8_t SOFT_MOSI_PIN = 11;
const uint8_t SOFT_SCK_PIN  = 13;
//

//PRESSURE SENSOR CONFIG
#define RESET_PIN  -1
#define EOC_PIN    -1
//
