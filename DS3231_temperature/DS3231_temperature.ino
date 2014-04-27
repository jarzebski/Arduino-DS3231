/*
  DS3231: Real-Time Clock. Temperature example
  Read more: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-ds3231.html
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
}

void loop()
{
  // The temperature registers are updated after every 64-second conversion.
  // If you want force temperature conversion use forceConversion()
  clock.forceConversion();

  Serial.print("Temperature: ");
  Serial.println(clock.readTemperature());

  delay(1000);
}
