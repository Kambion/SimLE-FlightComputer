#include <flightmanager.hpp>

void FlightManager::updateGPS(){
    if(GPSSerial.available()){
        gps.encode(GPSSerial.read());
    }
}

void FlightManager::printGPS(){
  Serial.print(F("Location: ")); 
  Serial.print(gps.location.lat(), 6);
  Serial.print(F(", "));
  Serial.print(gps.location.lng(), 6);
  Serial.println();
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

void FlightManager::printTemperature() {
  sensors.requestTemperatures();
  for (int i = 0; i < sensors.getDeviceCount(); i++){
    Serial.print("Temp ");
    Serial.print(i+1);
    Serial.print(" : ");
    Serial.print(sensors.getTempCByIndex(i));
    Serial.println(" C");
  }
}

void FlightManager::readPressure() {
  pressure_hPa = mpr.readPressure();
}

void FlightManager::printPressure(){
  readPressure();
  Serial.print("Pressure: ");
  Serial.print(pressure_hPa);
  Serial.println(" hPa");
}

void FlightManager::printSensorsData(){
  printTemperature();
  printPressure();
}


///*YYYY-MM-DDTHH:MM:SS,lattitude,longtitude,height, speed, course, numberOfSatelites,pressure,temp1,temp2*/
void FlightManager::saveFrame() {
  readTime();
  readPressure();
  sensors.requestTemperatures();
  if(file.open("frameData.txt", O_RDWR | O_CREAT | O_AT_END)){
    // file.print("Time: ");
    String frame = "/*" 
                  + String(year)
                  + "-"
                  + String(month)
                  + "-"
                  + String(dateOfMonth)
                  + "T"
                  + String(hour)
                  + ":"
                  + String(minute)
                  + ":"
                  + String(second)
                  + ","
                  + String(gps.location.lat(), 6)
                  + ","
                  + String(gps.location.lng(), 6)
                  + ","
                  + String(gps.altitude.meters())
                  + ","
                  + String(gps.speed.kmph(),6)
                  + ","
                  + String(gps.course.deg(),6)
                  + ","
                  + String(gps.satellites.value())
                  + ","
                  + String(pressure_hPa)
                  + ",";
                  for (int i = 0; i < sensors.getDeviceCount(); i++){
                    frame += String(sensors.getTempCByIndex(i));
                    if(i != sensors.getDeviceCount() - 1){
                      frame += ",";
                    }
                  }
                  + "*/";
    file.println(frame);  
    file.close();
    Serial.println("Sensors Data saved on SD card");
  }
  else {
    Serial.println("SD card file save failed");
  }
}

void FlightManager::cutdownNOW(){
    cutdownStatus = ITSTHEFINALCUTDOWN;
    digitalWrite(CUTDOWN_PIN, HIGH);
    Serial.println("Cutdown Begin");
    readTime();
    if(file.open("cutdownLog.txt", O_RDWR | O_CREAT | O_AT_END)){
      String info = String(hour)
                  + ":"
                  + String(minute)
                  + ":"
                  + String(second)
                  + " Cutdown Begin";
      file.println(info);
      file.close();
    }
    cutdownStartTime = millis();
}

void FlightManager::checkCutdown(){
  if(!cutdownHappend){
    if ((gps.altitude.meters() >= 0) && (cutdownStatus == DISARMED) && gps.location.isValid() && gps.altitude.isValid())
    {
      cutdownStatus = READY;      // Armed
      Serial.println("GPS Armed");
    }
    if(cutdownStatus == ITSTHEFINALCUTDOWN){
      if(millis() - cutdownStartTime > CutdownTime){
        digitalWrite(CUTDOWN_PIN, LOW);
        Serial.println("Cutdown End");
        readTime();
      if(file.open("cutdownLog.txt", O_RDWR | O_CREAT | O_AT_END)){
        String info = String(hour)
                    + ":"
                    + String(minute)
                    + ":"
                    + String(second)
                    + " Cutdown End";
        file.println(info);
      file.close();
    }
        cutdownHappend = true;
      }
    }

    // Trigger only if armed
    if (cutdownStatus == READY && gps.location.isValid() && gps.altitude.isValid()) 
    {
      // Uncomment/modify the following code to trigger the cutdown appropriately for your flight
    
      // ALTITUDE TEST
      
      if (gps.altitude.meters() > 31000)
      {
        // Altitude trigger
        Serial.println("Cutdown Altitude Trigger");
        cutdownNOW();
      }
      
    
      // LONGITUDE TEST
      //if (GPS.Latitude >= 0.0)
      if (gps.location.lat() > 54.0 || gps.location.lng() > 22.0)
      {
        // Longitude trigger
        Serial.println("Cutdown position trigger");
        cutdownNOW();
      }
      
    }
  }
}

void FlightManager::sendPosition(){
  LoRaSerial.print(gps.location.lat(), 6);
  LoRaSerial.print(F(" "));
  LoRaSerial.println(gps.location.lng(), 6);
  //For debug purpose:
  //printGPS();
}
