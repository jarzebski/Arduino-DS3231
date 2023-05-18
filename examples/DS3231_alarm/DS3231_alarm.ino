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
 
  // Manual (Year, Month, Day, Hour, Minute, Second)
  clock.setDateTime(2014, 4, 25, 0, 0, 0);

  // Set Alarm - Every second.
  // DS3231_EVERY_SECOND is available only on Alarm1.
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(0, 0, 0, 0, DS3231_EVERY_SECOND);

  // Set Alarm - Every full minute.
  // DS3231_EVERY_MINUTE is available only on Alarm2.
  // setAlarm2(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm2(0, 0, 0, 0, DS3231_EVERY_MINUTE);
  
  // Set Alarm1 - Every 20s in each minute
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  clock.setAlarm1(0, 0, 0, 20, DS3231_MATCH_S);

  // Set Alarm2 - Every 01m in each hour
  // setAlarm2(Date or Day, Hour, Minute, Mode, Armed = true)
  clock.setAlarm2(0, 0, 1,     DS3231_MATCH_M);

  // Set Alarm - Every 01m:25s in each hour
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(0, 0, 1, 25, DS3231_MATCH_M_S);

  // Set Alarm - Every 01h:10m:30s in each day
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(0, 1, 10, 30, DS3231_MATCH_H_M_S);

  // Set Alarm - 07h:00m:00s in 25th day in month
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(25, 7, 0, 0, DS3231_MATCH_DT_H_M_S);

  // Set Alarm - 10h:45m:30s in every Friday (1 - Mon, 7 - Sun)
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(5, 10, 40, 30, DS3231_MATCH_DY_H_M_S);
  
  // Check alarm settings
  checkAlarms();
}

void checkAlarms()
{
  RTCAlarmTime a1;  
  RTCAlarmTime a2;

  if (clock.isArmed1())
  {
    a1 = clock.getAlarm1();

    Serial.print("Alarm1 is triggered ");
    switch (clock.getAlarmType1())
    {
      case DS3231_EVERY_SECOND:
        Serial.println("every second");
        break;
      case DS3231_MATCH_S:
        Serial.print("when seconds match: ");
        Serial.println(clock.dateFormat("__ __:__:s", a1));
        break;
      case DS3231_MATCH_M_S:
        Serial.print("when minutes and sencods match: ");
        Serial.println(clock.dateFormat("__ __:i:s", a1));
        break;
      case DS3231_MATCH_H_M_S:
        Serial.print("when hours, minutes and seconds match: ");
        Serial.println(clock.dateFormat("__ H:i:s", a1));
        break;
      case DS3231_MATCH_DT_H_M_S:
        Serial.print("when date, hours, minutes and seconds match: ");
        Serial.println(clock.dateFormat("d H:i:s", a1));
        break;
      case DS3231_MATCH_DY_H_M_S:
        Serial.print("when day of week, hours, minutes and seconds match: ");
        Serial.println(clock.dateFormat("l H:i:s", a1));
        break;
      default: 
        Serial.println("UNKNOWN RULE");
        break;
    }
  } else
  {
    Serial.println("Alarm1 is disarmed.");
  }

  if (clock.isArmed2())
  {
    a2 = clock.getAlarm2();

    Serial.print("Alarm2 is triggered ");
    switch (clock.getAlarmType2())
    {
      case DS3231_EVERY_MINUTE:
        Serial.println("every minute");
        break;
      case DS3231_MATCH_M:
        Serial.print("when minutes match: ");
        Serial.println(clock.dateFormat("__ __:i:s", a2));
        break;
      case DS3231_MATCH_H_M:
        Serial.print("when hours and minutes match:");
        Serial.println(clock.dateFormat("__ H:i:s", a2));
        break;
      case DS3231_MATCH_DT_H_M:
        Serial.print("when date, hours and minutes match: ");
        Serial.println(clock.dateFormat("d H:i:s", a2));
        break;
      case DS3231_MATCH_DY_H_M:
        Serial.println("when day of week, hours and minutes match: ");
        Serial.print(clock.dateFormat("l H:i:s", a2));
        break;
      default: 
        Serial.println("UNKNOWN RULE"); 
        break;
    }
  } else
  {
    Serial.println("Alarm2 is disarmed.");
  }
}

void loop()
{
  dt = clock.getDateTime();

  Serial.println(clock.dateFormat("d-m-Y H:i:s - l", dt));

  // Call isAlarm1(false) if you want clear alarm1 flag manualy by clearAlarm1();
  if (clock.isAlarm1())
  {
    Serial.println("ALARM 1 TRIGGERED!");
  }

  // Call isAlarm2(false) if you want clear alarm1 flag manualy by clearAlarm2();
  if (clock.isAlarm2())
  {
    Serial.println("ALARM 2 TRIGGERED!");
  }
 
  delay(1000);
}


