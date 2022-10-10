#include <flightmanager.hpp>

void FlightManager::updateGPS(){
    if(GPSSerial.available()){
        gps.encode(GPSSerial.read());
    }
}

void FlightManager::printGPS(){
  if(settings.gpsModule){
    Serial.print(F("Location: ")); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(", "));
    Serial.print(gps.location.lng(), 6);
    Serial.println();
  }
  else {
    Serial.println("No GPS Module, change settings in config.hpp");
  }
}

void FlightManager::readTime(){
  softWire.beginTransmission(I2C_ADDRESS);
  softWire.write(uint8_t(0));
  softWire.endTransmission();

  uint8_t registers[7];
  int numBytes = softWire.requestFrom(I2C_ADDRESS, (uint8_t)7);
  for (int i = 0; i < numBytes; ++i) {
    registers[i] = softWire.read();
  }
  if (numBytes != 7) {
    Serial.print("Read wrong number of bytes: ");
    Serial.println((int)numBytes);
    return;
  }

  int tenYear = (registers[6] & 0xf0) >> 4;
  int unitYear = registers[6] & 0x0f;
  year = (10 * tenYear) + unitYear;

  int tenMonth = (registers[5] & 0x10) >> 4;
  int unitMonth = registers[5] & 0x0f;
  month = (10 * tenMonth) + unitMonth;

  int tenDateOfMonth = (registers[4] & 0x30) >> 4;
  int unitDateOfMonth = registers[4] & 0x0f;
  dateOfMonth = (10 * tenDateOfMonth) + unitDateOfMonth;

  bool twelveHour = registers[2] & 0x40;
  bool pm = false;
  int unitHour;
  int tenHour;
  if (twelveHour) {
    pm = registers[2] & 0x20;
    tenHour = (registers[2] & 0x10) >> 4;
  } else {
    tenHour = (registers[2] & 0x30) >> 4;
  }
  unitHour = registers[2] & 0x0f;
  hour = (10 * tenHour) + unitHour;
  if (twelveHour) {
    hour += 12;
  }

  int tenMinute = (registers[1] & 0xf0) >> 4;
  int unitMinute = registers[1] & 0x0f;
  minute = (10 * tenMinute) + unitMinute;

  int tenSecond = (registers[0] & 0xf0) >> 4;
  int unitSecond = registers[0] & 0x0f;
  second = (10 * tenSecond) + unitSecond;

}

void FlightManager::printTime(){
    readTime();
    Serial.print("Time: ");
    Serial.print(year);
    Serial.print('-');
    printTwoDigit(month);
    Serial.print('-');
    printTwoDigit(dateOfMonth);
    Serial.print(' ');
    printTwoDigit(hour);
    Serial.print(':');
    printTwoDigit(minute);
    Serial.print(':');
    printTwoDigit(second);
    Serial.println();
}

void FlightManager::printTwoDigit(int n, bool sdcard)
{
  if(sdcard){
    if (n < 10) {
      file.print('0');
    }
    file.print(n);
  }
  else{
    if (n < 10) {
      Serial.print('0');
    }
    Serial.print(n);
  }
}

void FlightManager::saveGPSDataOnSDCard(){
  if(settings.gpsModule && settings.SDcard) {
    readTime();
    if(file.open("gpsData.txt", O_RDWR | O_CREAT | O_AT_END)){
        file.print("Time: ");
        file.print(year);
        file.print('-');
        printTwoDigit(month, true);
        file.print('-');
        printTwoDigit(dateOfMonth, true);
        file.print(' ');
        printTwoDigit(hour, true);
        file.print(':');
        printTwoDigit(minute, true);
        file.print(':');
        printTwoDigit(second, true);
        file.print(F(" Location: ")); 
        file.print(gps.location.lat(), 6);
        file.print(F(", "));
        file.print(gps.location.lng(), 6);
        file.println();
        file.close();
        if(settings.debugLogs)
          Serial.println("GPS Data saved on SD card");
      }
      else {
        if(settings.debugLogs)
          Serial.println("SD card file save failed");
    }
  }
}

void FlightManager::printTemperature() {
  if(settings.temperatureSensors){
    sensors.requestTemperatures();
    for (int i = 0; i < sensors.getDeviceCount(); i++){
      Serial.print("Temp ");
      Serial.print(i+1);
      Serial.print(" : ");
      Serial.print(sensors.getTempCByIndex(i));
      Serial.println(" C");
    }
  }
}

void FlightManager::readPressure() {
  if(settings.pressureSensors)
    pressure_hPa = mpr.readPressure();
}

void FlightManager::printPressure(){
  if(settings.pressureSensors){
    readPressure();
    Serial.print("Pressure: ");
    Serial.print(pressure_hPa);
    Serial.println(" hPa");
  }
}

void FlightManager::printSensorsData(){
  printTemperature();
  printPressure();
}

void FlightManager::saveSensorsDataOnSDCard() {
  if(settings.SDcard){
    readTime();
    if(file.open("sensorsData.txt", O_RDWR | O_CREAT | O_AT_END)){
      file.print("Time: ");
      file.print(year);
      file.print('-');
      printTwoDigit(month, true);
      file.print('-');
      printTwoDigit(dateOfMonth, true);
      file.print(' ');
      printTwoDigit(hour, true);
      file.print(':');
      printTwoDigit(minute, true);
      file.print(':');
      printTwoDigit(second, true);
      if(settings.temperatureSensors){
        sensors.requestTemperatures();
        for (int i = 0; i < sensors.getDeviceCount(); i++){
          file.print(" Temp ");
          file.print(i+1);
          file.print(" : ");
          file.print(sensors.getTempCByIndex(i));
          file.print(" C");
        }
      }
      if(settings.pressureSensors){
        readPressure();
        file.print(" Pressure: ");
        file.print(pressure_hPa);
        file.println(" hPa");
      }
      file.close();
      if(settings.debugLogs)
        Serial.println("Sensors Data saved on SD card");
    }
    else {
      if(settings.debugLogs)
        Serial.println("SD card file save failed");
    }
  }
}