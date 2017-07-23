#include <TaskScheduler.h>
#include <CayenneDefines.h>
#include <BlynkSimpleEsp8266.h>
#include <CayenneWiFiClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "CayenneAuth.h"

#define LED_PIN D0
#define WATER_PIN D7
#define TEMP_SENSOR_PIN D1

int minutesPassed = -1;
int maxRuntime;
bool timeChanged = false;
bool autoShutOff = false;

Scheduler scheduler;
Task tWater;
Task tTemp;

DHT dht(TEMP_SENSOR_PIN, DHT22);

void timerCallback() {
  minutesPassed++;
  if (minutesPassed >= maxRuntime) {
      shutOff();
      autoShutOff = true;
    }
  
  timeChanged = true;
}

void setup() {
  Serial.begin(115200);
  Cayenne.begin(token, ssid, pwd);
  dht.begin();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(WATER_PIN, OUTPUT);
  digitalWrite(WATER_PIN, HIGH);

  tWater.set(TASK_MINUTE, TASK_FOREVER, timerCallback);
  tTemp.set(2000, TASK_FOREVER, sendDHT);
  scheduler.init();
  scheduler.addTask(tWater);
  scheduler.addTask(tTemp);
  tTemp.enable();
}

void sendDHT()
{
  float temp = dht.readTemperature(true);
  float humidity = dht.readHumidity();

  if (isnan(humidity) || isnan(temp)) {
    Cayenne.virtualWrite(V3, -1);
    Cayenne.virtualWrite(V4, -1);
    return;
  }
  
  Cayenne.virtualWrite(V3, temp);
  Cayenne.virtualWrite(V4, humidity);
}

void loop() {
  scheduler.execute();
  Cayenne.run();

  if(autoShutOff){
    Cayenne.virtualWrite(V0, 0);
    autoShutOff = false;
  }

  if(timeChanged){
    Cayenne.virtualWrite(V2, minutesPassed);
    timeChanged = false;
  }
}

void shutOff(){
  digitalWrite(WATER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  tWater.disable();
}

CAYENNE_CONNECTED() {
  static int isFirstConnect = true;
  if (isFirstConnect) {
    Cayenne.syncAll();
    blinkLed();
  }
  isFirstConnect = false;
}

CAYENNE_IN(V0)
{
  // get value sent from dashboard
  int currentValue = getValue.asInt(); // 0 to 1

  // low trigger
  if (currentValue == 0) {
    shutOff();
  } else {
    minutesPassed = -1;
    tWater.enable();
    digitalWrite(WATER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
}

CAYENNE_IN(V1)
{
  maxRuntime = getValue.asInt();
}

//CAYENNE_OUT(V3)
//{
//  Cayenne.virtualWrite(V3, dht.readTemperature(true));
//}
//
//CAYENNE_OUT(V4)
//{
//  Cayenne.virtualWrite(V4, dht.readHumidity());
//}

void blinkLed() {
  digitalWrite(LED_PIN, LOW);    // turn the LED on (LOW)
  delay(250);
  digitalWrite(LED_PIN, HIGH);  // turn the LED off by making the voltage HIGH
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
}
