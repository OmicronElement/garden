#include <TaskScheduler.h>
#include "CayenneDefines.h"
#include "BlynkSimpleEsp8266.h"
#include "CayenneWiFiClient.h"
#include "CayenneAuth.h"
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#define LED_PIN D0
#define WATER_PIN D7

int minutesPassed = -1;
int maxRuntime;

Scheduler scheduler;
Task tWater;

void timerCallback() {
  minutesPassed++;

  Cayenne.virtualWrite(V2, minutesPassed);

  if (minutesPassed >= maxRuntime) {
    Cayenne.virtualWrite(V0, 0);
    shutOff();
  }
}

void setup() {
  Serial.begin(115200);
  Cayenne.begin(token, ssid, pwd);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(WATER_PIN, OUTPUT);
  digitalWrite(WATER_PIN, HIGH);

  tWater.set(TASK_MINUTE, TASK_FOREVER, timerCallback, false);
  scheduler.init();
  scheduler.addTask(tWater);
}

void loop() {
  scheduler.execute();
  Cayenne.run();
}

CAYENNE_CONNECTED() {
  static int isFirstConnect = true;
  if (isFirstConnect) {
    Cayenne.syncAll();
    blinkLed();
  }
  isFirstConnect = false;
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

void shutOff(){
  digitalWrite(WATER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  tWater.disable();
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
