#include <MacRocketry_SD_Logger.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include "RTClib.h"

//RTC and Data Logging
RTC_PCF8523 rtc;
MacRocketry_SD_Logger logger = MacRocketry_SD_Logger();

//Atmospheric Sensors
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(12345);
float pressure, temp, altitude;
const float SEA_LEVEL_PRESSURE = 101325; //COnstant for sea level pressure

int baseTime; //Starting time as we enter the launch loop
int currentTime;

//debug
int timesRun = 0;

void setup() {
  Serial.begin(9600);

  //Connect to serial
  while (!Serial)
  {
    ;
  }

  //Begin BMP sensor
  if (!bmp.begin())
  {
    Serial.println("BMP open failed!");
  }

  //Check that new file was successfully loaded
  if(logger.connectFile == NULL)
    Serial.println("Failed to open log file");

  baseTime = millis(); //Set start time
  Serial.println("Starting");
}

void loop()
{
  //Error-trapping to check if the logger first should always be there. The run limit of 50 can be adjusted to be whatever.
  if (timesRun < 50)
  {
    timesRun++;
    logger.writeFile(getSensorData()); //What we can probably do is use writeBuffer for the faster sensors (Like the altitude) and writeFile once we receive data from the slow ones.
    delay(500);
    //Might be a good idea to use a non-blocking delay here... that way we can poll sensors every 500 ms and skip ones that are slow
  }
}

String getSensorData()
{
  //Get new data from the sensor. Uses the adafruit library
  currentTime = millis() - baseTime;
  bmp.getPressure(&pressure);
  bmp.getTemperature(&temp);

  //Consider doing this in post. We have pressure so we can just calculate when we process the data
  altitude = bmp.pressureToAltitude(SEA_LEVEL_PRESSURE, pressure);

  //Convert to string and store in CSV format
  String data = String(currentTime) + ",";
  data += String(pressure) + ",";
  data += String(temp) + ",";
  data += String(altitude) + ",";

  Serial.println(data); //Placeholder variable

  return data;
}

//Begins and Initializes the RTC (If we are using it)
bool initRTC()
{
  if (rtc.begin())
  {
    //check that the RTC has been initialized. If it hasn't then set the time to the upload time.
    if (!rtc.initialized())
    {
      Serial.println("RTC not initialized!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      return true;
    }
    return true;
  }
  return false;
}

//Gets a timestamp from the RTC
String getTimestamp()
{
    DateTime currentTime = rtc.now();

    String timeStamp;
    //Convert to a timestamp
    timeStamp += String(currentTime.year());
    timeStamp += String(currentTime.month());
    timeStamp += String(currentTime.day()) + "_";
    timeStamp += String(currentTime.hour());
    timeStamp += String(currentTime.minute());
    timeStamp += String(currentTime.second());

    return timeStamp;
}

