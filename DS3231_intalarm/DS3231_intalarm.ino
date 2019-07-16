/*
  DS3231: Real-Time Clock. Alarm simple
  Read more: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-DS3231.html
  GIT: https://github.com/jarzebski/Arduino-DS3231
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;
boolean isAlarm = false;
int alarmLED = 4;

void alarmFunction()
{
  Serial.println("*** INT 0 ***");
  isAlarm = true;
}

void setup()
{
  Serial.begin(9600);
  
  // Initialize DS3231
  Serial.println("Initialize DS3231");;
  clock.begin();
 
  // Disarm alarms and clear alarms for this example, because alarms is battery backed.
  // Under normal conditions, the settings should be reset after power and restart microcontroller.
  clock.armAlarm1(false);
  clock.armAlarm2(false);
  clock.clearAlarm1();
  clock.clearAlarm2();
  clock.enableOutput(false);
  
  // Manual (Year, Month, Day, Hour, Minute, Second)
  clock.setDateTime(2014, 4, 25, 0, 0, 0);

  // Set Alarm1 - Every 20s in each minute
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  clock.setAlarm1(0, 0, 0, 10, DS3231_MATCH_S);

  // Attach Interrput to Arduino Pin 2
  attachInterrupt(digitalPinToInterrupt(2), alarmFunction, FALLING);

  // Setup LED Pin
  pinMode(alarmLED, OUTPUT);
}

void loop()
{
  dt = clock.getDateTime();
  Serial.println(clock.dateFormat("d-m-Y H:i:s - l", dt));
  
  digitalWrite(alarmLED, isAlarm);
  if (isAlarm)
  {
    isAlarm = !isAlarm;
    clock.clearAlarm1();
  }
  
  delay(1000);
}

