/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/home/jhosen/p2/Argon_Air_Quality/src/Argon_Air_Quality.ino"
/*
 * Air Quality Kit for FNR498
 * Setup sensor and connect to MyGeoHub
 * 
 * 
 */
#include "Particle.h"

void setup();
void loop();
int getAirQualityNum();
int getBMEValues(int &temp, int &pressure, int &humidity);
void updateDisplay(int temp, int humidity, int pressure, String airQuality, int qual_int);
#line 9 "/home/jhosen/p2/Argon_Air_Quality/src/Argon_Air_Quality.ino"
SYSTEM_THREAD(ENABLED);

#include "Air_Quality_Sensor.h"
#include "Adafruit_BME280.h"
#include "SeeedOLED.h"
#include "JsonParserGeneratorRK.h"

#define AQS_PIN A2
#define DUST_SENSOR_PIN D4
#define SENSOR_READING_INTERVAL 900000

AirQualitySensor aqSensor(AQS_PIN);
Adafruit_BME280 bme;

unsigned long lastInterval;
unsigned long lowpulseoccupancy = 0;
unsigned long last_lpo = 0;
unsigned long duration;

float ratio = 0;
float concentration = 0;

const char * eventName =            "env-vals";    
const char * MySiteID = "South Oaklan Lafayette IN";


float field1;  
float field2;  
float field3;
float field4;
float field5;
float field6;
float field7;
float field8;
char msg[256];       // Character array for the snprintf Publish Payload



int getBMEValues(int &temp, int &humidity, int &pressure);
void getDustSensorReadings();
String getAirQuality();
void createEventPayload(int temp, int humidity, int pressure, String airQuality);
void updateDisplay(int temp, int humidity, int pressure, String airQuality);

void setup()
{
  Serial.begin(9600);
  delay(50);

  // Configure the dust sensor pin as an input
  pinMode(DUST_SENSOR_PIN, INPUT);

  if (aqSensor.init())
  {
    Serial.println("Air Quality Sensor ready.");
  }
  else
  {
    Serial.println("Air Quality Sensor ERROR!");
  }

  Wire.begin();
  SeeedOled.init();

  SeeedOled.clearDisplay();
  SeeedOled.setNormalDisplay();
  SeeedOled.setPageMode();

  SeeedOled.setTextXY(2, 0);
  SeeedOled.putString("FNR498");
  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString("Sensors");
  SeeedOled.setTextXY(4, 0);
  SeeedOled.putString("Initializing");

  if (bme.begin())
  {
    Serial.println("BME280 Sensor ready.");
  }
  else
  {
    Serial.println("BME280 Sensor ERROR!");
  }

delay(5000);

  SeeedOled.setTextXY(5, 0);
  SeeedOled.putString("Warm-Up");
  

  lastInterval = millis();
}




void loop()
{
  int temp, pressure, humidity;

  duration = pulseIn(DUST_SENSOR_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;

  if ((millis() - lastInterval) > SENSOR_READING_INTERVAL)
  {
    String quality = getAirQuality();
    int qual_int = getAirQualityNum();
    Serial.printlnf("Air Quality: %s", quality.c_str());

    getBMEValues(temp, pressure, humidity);
    Serial.printlnf("Temp: %d", temp);
    Serial.printlnf("Pressure: %d", pressure);
    Serial.printlnf("Humidity: %d", humidity);

    getDustSensorReadings();

    updateDisplay(temp, humidity, pressure, quality, qual_int);


    field1 = temp;
    field2 = pressure;
    field3 = humidity;
    field4 = aqSensor.slope();
    field5 = aqSensor.getValue();
      if (lowpulseoccupancy > 0)
  {
    field6 = lowpulseoccupancy;
    field7 = ratio;
  }
    lowpulseoccupancy = 0;
    snprintf(msg, sizeof(msg),"{\"temp\":\"%.2f\", \"pressure\":\"%.1f\", \"humidity\":\"%.2f\", \"aq_slope\":\"%.2f\", \"voc_level\":\"%.3f\", \"lowpulseoccupancy\":\"%.2f\", \"dust\":\"%.1f\", \"deviceid\":\"%s\"}", field1, field2, field3, field4, field5, field6, field7, MySiteID);
    Particle.publish(eventName, msg, PRIVATE, NO_ACK);



    lowpulseoccupancy = 0;
    lastInterval = millis();







    
  }
}







int getAirQualityNum()
{
  int qual_int = aqSensor.slope();
  return qual_int;
}

String getAirQuality()
{
  int quality = aqSensor.slope();
  String qual = "None";

  if (quality == AirQualitySensor::FORCE_SIGNAL)
  {
    qual = "Danger";
  }
  else if (quality == AirQualitySensor::HIGH_POLLUTION)
  {
    qual = "High Pollution";
  }
  else if (quality == AirQualitySensor::LOW_POLLUTION)
  {
    qual = "Low Pollution";
  }
  else if (quality == AirQualitySensor::FRESH_AIR)
  {
    qual = "Fresh Air";
  }

  return qual;
}

int getBMEValues(int &temp, int &pressure, int &humidity)
{
  temp = (int)bme.readTemperature();
  pressure = (int)(bme.readPressure() / 100.0F);
  humidity = (int)bme.readHumidity();

  return 1;
}

void getDustSensorReadings()
{
  // This particular dust sensor returns 0s often, so let's filter them out by making sure we only
  // capture and use non-zero LPO values for our calculations once we get a good reading.
  if (lowpulseoccupancy == 0)
  {
    lowpulseoccupancy = last_lpo;
  }
  else
  {
    last_lpo = lowpulseoccupancy;
  }

  ratio = lowpulseoccupancy / (SENSOR_READING_INTERVAL * 10.0);                   // Integer percentage 0=>100
  concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve

  Serial.printlnf("LPO: %d", lowpulseoccupancy);
  Serial.printlnf("Ratio: %f%%", ratio);
  Serial.printlnf("Concentration: %f pcs/L", concentration);
}






void updateDisplay(int temp, int humidity, int pressure, String airQuality, int qual_int)
{
  SeeedOled.clearDisplay();

  SeeedOled.setTextXY(0, 3);
  SeeedOled.putString(airQuality);

  SeeedOled.setTextXY(1, 0);
  SeeedOled.putString("VOC Levels: ");
  SeeedOled.putNumber(qual_int);


  SeeedOled.setTextXY(2, 0);
  SeeedOled.putString("Temp: ");
  SeeedOled.putNumber(temp);
  SeeedOled.putString("C");

  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString("Humidity: ");
  SeeedOled.putNumber(humidity);
  SeeedOled.putString("%");

  SeeedOled.setTextXY(4, 0);
  SeeedOled.putString("Press: ");
  SeeedOled.putNumber(pressure);
  SeeedOled.putString(" hPa");

  if (concentration > 1)
  {
    SeeedOled.setTextXY(5, 0);
    SeeedOled.putString("Dust: ");
    SeeedOled.putNumber(concentration); // Will cast our float to an int to make it more compact
    SeeedOled.putString(" pcs/L");
  }
}