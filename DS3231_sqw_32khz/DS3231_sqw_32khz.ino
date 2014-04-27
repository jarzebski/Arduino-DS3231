/*
  DS3231: Real-Time Clock. SQW & 32kHz Example
  Read more: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-ds3231.html
  GIT: https://github.com/jarzebski/Arduino-DS3231
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <DS3231.h>

DS3231 clock;

boolean state;

void setup()
{
  Serial.begin(9600);

  // Initialize DS3231
  Serial.println("Initialize DS3231");;
  clock.begin();

  // disable 32kHz 
  clock.enable32kHz(false);

  // Select output as rate to 1Hz
  clock.setOutput(DS3231_1HZ);
  
  // Enable output
  clock.enableOutput(true);

  // Check config

  if (clock.isOutput())
  {
    Serial.println("Oscilator is enabled");
  } else
  {
    Serial.println("Oscilator is disabled");
  }

  switch (clock.getOutput())
  {
    case DS3231_1HZ:     Serial.println("SQW = 1Hz"); break;
    case DS3231_4096HZ:  Serial.println("SQW = 4096Hz"); break;
    case DS3231_8192HZ:  Serial.println("SQW = 8192Hz"); break;
    case DS3231_32768HZ: Serial.println("SQW = 32768Hz"); break;
    default: Serial.println("SQW = Unknown"); break; }
}

void loop()
{
   // Togle 32kHz output
   clock.enable32kHz(state);
   state = !state;
   delay(2000);
}
