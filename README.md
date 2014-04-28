Arduino-DS3231
==============

DS3231 Real-Time Clock.

![DS3231](http://www.jarzebski.pl/media/big/publish/2014/04/ds3231-dateformat.png)

Date formats (Day)
------------------

 * d : Day of the month, 2 digits with leading zeros (01 to 31)
 * D : A textual representation of a day, three letters (Mon through Sun)
 * j : Day of the month without leading zeros (1 to 31)
 * l : A full textual representation of the day of the week (Sunday through Saturday)
 * N : ISO-8601 numeric representation of the day of the week (1 for Monday through 7 for Sunday)
 * S : English ordinal suffix for the day of the month, 2 characters (st, nd, rd or th. Works well with j)
 * w : Numeric representation of the day of the week (0 for Sunday through 6 for Saturday)
 * z : The day of the year (0 through 365)

Date formats (Month)
--------------------

 * F : A full textual representation of a month, such as January or March (January through December)
 * m : Numeric representation of a month, with leading zeros (01 through 12)
 * M : A short textual representation of a month, three letters (Jan through Dec)
 * n : Numeric representation of a month, without leading zeros (1 through 12)
 * t : Number of days in the given month (28 through 31)

Date formats (Year)
-------------------

 * L : Whether it's a leap year (1 if it is a leap year, 0 otherwise)
 * Y : A full numeric representation of a year, 4 digits (Examples: 1999 or 2003)
 * y : A two digit representation of a year (Examples: 99 or 03)

Date formats (Time)
-------------------

 * a : Lowercase Ante meridiem and Post meridiem (am or pm)
 * A : Uppercase Ante meridiem and Post meridiem (AM or PM)
 * g : 2-hour format of an hour without leading zeros (1 through 12)
 * G : 24-hour format of an hour without leading zeros (0 through 23)
 * h : 12-hour format of an hour with leading zeros (01 through 12)
 * H : 24-hour format of an hour with leading zeros (00 through 23)
 * i : Minutes with leading zeros (00 to 59)
 * s : Seconds, with leading zeros (00 through 59)

Dare formats (Full Date/Time)
-----------------------------

 * U : Seconds since the Unix Epoch (January 1 1970 00:00:00 GMT)

More info
---------

Tutorials: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-ds3231.html

This library use I2C to communicate, 2 pins are required to interface.

Credits
-------

Original Code by JeeLabs http://news.jeelabs.org/code/

First fork by adafruit https://github.com/adafruit/RTClib