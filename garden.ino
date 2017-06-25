#include <TaskScheduler.h>
#include <CayenneDefines.h>
#include <BlynkSimpleEsp8266.h>
#include <CayenneWiFiClient.h>
#include "CayenneAuth.h"

#define LED_PIN D0
#define WATER_PIN D7

int minutesPassed = -1;
int maxRuntime;
bool timeChanged = false;
bool autoShutOff = false;

Scheduler scheduler;
Task tWater;

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

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(WATER_PIN, OUTPUT);
  digitalWrite(WATER_PIN, HIGH);

  tWater.set(TASK_SECOND, TASK_FOREVER, timerCallback);
  scheduler.init();
  scheduler.addTask(tWater);
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
  // get value sent from dashboard
  maxRuntime = getValue.asInt()/1000;
}

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
