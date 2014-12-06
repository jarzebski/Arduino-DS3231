/*
DS3231.cpp - Class file for the DS3231 Real-Time Clock

Version: 1.0.1
(c) 2014 Korneliusz Jarzebski
www.jarzebski.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "DS3231.h"

const uint8_t daysArray [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
const uint8_t dowArray[] PROGMEM = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

bool DS3231::begin(void)
{
    Wire.begin();

    setBattery(true, false);

    t.year = 2000;
    t.month = 1;
    t.day = 1;
    t.hour = 0;
    t.minute = 0;
    t.second = 0;
    t.dayOfWeek = 6;
    t.unixtime = 946681200;

    return true;
}

void DS3231::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    Wire.beginTransmission(DS3231_ADDRESS);

    #if ARDUINO >= 100
        Wire.write(DS3231_REG_TIME);
    #else
        Wire.send(DS3231_REG_TIME);
    #endif

    #if ARDUINO >= 100
        Wire.write(dec2bcd(second));
        Wire.write(dec2bcd(minute));
        Wire.write(dec2bcd(hour));
        Wire.write(dec2bcd(dow(year, month, day)));
        Wire.write(dec2bcd(day));
        Wire.write(dec2bcd(month));
        Wire.write(dec2bcd(year-2000));
    #else
        Wire.send(dec2bcd(second));
        Wire.send(dec2bcd(minute));
        Wire.send(dec2bcd(hour));
        Wire.send(dec2bcd(dow(year, month, day)));
        Wire.send(dec2bcd(day));
        Wire.send(dec2bcd(month));
        Wire.send(dec2bcd(year-2000));
    #endif

    #if ARDUINO >= 100
        Wire.write(DS3231_REG_TIME);
    #else
        Wire.send(DS3231_REG_TIME);
    #endif

    Wire.endTransmission();
}

void DS3231::setDateTime(uint32_t t)
{
    t -= 946681200;

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    second = t % 60;
    t /= 60;

    minute = t % 60;
    t /= 60;

    hour = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;

    for (year = 0; ; ++year)
    {
        leap = year % 4 == 0;
        if (days < 365 + leap)
        {
            break;
        }
        days -= 365 + leap;
    }

    for (month = 1; ; ++month)
    {
        uint8_t daysPerMonth = pgm_read_byte(daysArray + month - 1);

        if (leap && month == 2)
        {
            ++daysPerMonth;
        }

        if (days < daysPerMonth)
        {
            break;
        }
        days -= daysPerMonth;
    }

    day = days + 1;

    setDateTime(year+2000, month, day, hour, minute, second);
}

void DS3231::setDateTime(const char* date, const char* time)
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    year = conv2d(date + 9);

    switch (date[0])
    {
        case 'J': month = date[1] == 'a' ? 1 : month = date[2] == 'n' ? 6 : 7; break;
        case 'F': month = 2; break;
        case 'A': month = date[2] == 'r' ? 4 : 8; break;
        case 'M': month = date[2] == 'r' ? 3 : 5; break;
        case 'S': month = 9; break;
        case 'O': month = 10; break;
        case 'N': month = 11; break;
        case 'D': month = 12; break;
    }

    day = conv2d(date + 4);
    hour = conv2d(time);
    minute = conv2d(time + 3);
    second = conv2d(time + 6);

    setDateTime(year+2000, month, day, hour, minute, second);
}

char* DS3231::dateFormat(const char* dateFormat, RTCDateTime dt)
{
    char buffer[255];

    buffer[0] = 0;

    char helper[11];

    while (*dateFormat != '\0')
    {
        switch (dateFormat[0])
        {
            // Day decoder
            case 'd':
                sprintf(helper, "%02d", dt.day); 
                strcat(buffer, (const char *)helper); 
                break;
            case 'j':
                sprintf(helper, "%d", dt.day);
                strcat(buffer, (const char *)helper);
                break;
            case 'l':
                strcat(buffer, (const char *)strDayOfWeek(dt.dayOfWeek));
                break;
            case 'D':
                strncat(buffer, strDayOfWeek(dt.dayOfWeek), 3);
                break;
            case 'N':
                sprintf(helper, "%d", dt.dayOfWeek);
                strcat(buffer, (const char *)helper);
                break;
            case 'w':
                sprintf(helper, "%d", (dt.dayOfWeek + 7) % 7);
                strcat(buffer, (const char *)helper);
                break;
            case 'z':
                sprintf(helper, "%d", dayInYear(dt.year, dt.month, dt.day));
                strcat(buffer, (const char *)helper);
                break;
            case 'S':
                strcat(buffer, (const char *)strDaySufix(dt.day));
                break;

            // Month decoder
            case 'm':
                sprintf(helper, "%02d", dt.month);
                strcat(buffer, (const char *)helper);
                break;
            case 'n':
                sprintf(helper, "%d", dt.month);
                strcat(buffer, (const char *)helper);
                break;
            case 'F':
                strcat(buffer, (const char *)strMonth(dt.month));
                break;
            case 'M':
                strncat(buffer, (const char *)strMonth(dt.month), 3);
                break;
            case 't':
                sprintf(helper, "%d", daysInMonth(dt.year, dt.month));
                strcat(buffer, (const char *)helper);
                break;

            // Year decoder
            case 'Y':
                sprintf(helper, "%d", dt.year); 
                strcat(buffer, (const char *)helper); 
                break;
            case 'y': sprintf(helper, "%02d", dt.year-2000);
                strcat(buffer, (const char *)helper);
                break;
            case 'L':
                sprintf(helper, "%d", isLeapYear(dt.year)); 
                strcat(buffer, (const char *)helper); 
                break;

            // Hour decoder
            case 'H':
                sprintf(helper, "%02d", dt.hour);
                strcat(buffer, (const char *)helper);
                break;
            case 'G':
                sprintf(helper, "%d", dt.hour);
                strcat(buffer, (const char *)helper);
                break;
            case 'h':
                sprintf(helper, "%02d", hour12(dt.hour));
                strcat(buffer, (const char *)helper);
                break;
            case 'g':
                sprintf(helper, "%d", hour12(dt.hour));
                strcat(buffer, (const char *)helper);
                break;
            case 'A':
                strcat(buffer, (const char *)strAmPm(dt.hour, true));
                break;
            case 'a':
                strcat(buffer, (const char *)strAmPm(dt.hour, false));
                break;

            // Minute decoder
            case 'i': 
                sprintf(helper, "%02d", dt.minute);
                strcat(buffer, (const char *)helper);
                break;

            // Second decoder
            case 's':
                sprintf(helper, "%02d", dt.second); 
                strcat(buffer, (const char *)helper); 
                break;

            // Misc decoder
            case 'U': 
                sprintf(helper, "%lu", dt.unixtime);
                strcat(buffer, (const char *)helper);
                break;

            default: 
                strncat(buffer, dateFormat, 1);
                break;
        }
        dateFormat++;
    }

    return buffer;
}

char* DS3231::dateFormat(const char* dateFormat, RTCAlarmTime dt)
{
    char buffer[255];

    buffer[0] = 0;

    char helper[11];

    while (*dateFormat != '\0')
    {
        switch (dateFormat[0])
        {
            // Day decoder
            case 'd':
                sprintf(helper, "%02d", dt.day); 
                strcat(buffer, (const char *)helper); 
                break;
            case 'j':
                sprintf(helper, "%d", dt.day);
                strcat(buffer, (const char *)helper);
                break;
            case 'l':
                strcat(buffer, (const char *)strDayOfWeek(dt.day));
                break;
            case 'D':
                strncat(buffer, strDayOfWeek(dt.day), 3);
                break;
            case 'N':
                sprintf(helper, "%d", dt.day);
                strcat(buffer, (const char *)helper);
                break;
            case 'w':
                sprintf(helper, "%d", (dt.day + 7) % 7);
                strcat(buffer, (const char *)helper);
                break;
            case 'S':
                strcat(buffer, (const char *)strDaySufix(dt.day));
                break;

            // Hour decoder
            case 'H':
                sprintf(helper, "%02d", dt.hour);
                strcat(buffer, (const char *)helper);
                break;
            case 'G':
                sprintf(helper, "%d", dt.hour);
                strcat(buffer, (const char *)helper);
                break;
            case 'h':
                sprintf(helper, "%02d", hour12(dt.hour));
                strcat(buffer, (const char *)helper);
                break;
            case 'g':
                sprintf(helper, "%d", hour12(dt.hour));
                strcat(buffer, (const char *)helper);
                break;
            case 'A':
                strcat(buffer, (const char *)strAmPm(dt.hour, true));
                break;
            case 'a':
                strcat(buffer, (const char *)strAmPm(dt.hour, false));
                break;

            // Minute decoder
            case 'i': 
                sprintf(helper, "%02d", dt.minute);
                strcat(buffer, (const char *)helper);
                break;

            // Second decoder
            case 's':
                sprintf(helper, "%02d", dt.second); 
                strcat(buffer, (const char *)helper); 
                break;

            default: 
                strncat(buffer, dateFormat, 1);
                break;
        }
        dateFormat++;
    }

    return buffer;
}

RTCDateTime DS3231::getDateTime(void)
{
    int values[7];

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_TIME);
    #else
        Wire.send(DS3231_REG_TIME);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 7);

    while(!Wire.available()) {};

    for (int i = 6; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(Wire.read());
        #else
            values[i] = bcd2dec(Wire.receive());
        #endif
    }

    Wire.endTransmission();

    t.year = values[0] + 2000;
    t.month = values[1];
    t.day = values[2];
    t.dayOfWeek = values[3];
    t.hour = values[4];
    t.minute = values[5];
    t.second = values[6];
    t.unixtime = unixtime();

    return t;
}

uint8_t DS3231::isReady(void) 
{
    return true;
}

void DS3231::enableOutput(bool enabled)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b11111011;
    value |= (!enabled << 2);

    writeRegister8(DS3231_REG_CONTROL, value);
}

void DS3231::setBattery(bool timeBattery, bool squareBattery)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    if (squareBattery)
    {
        value |= 0b01000000;
    } else
    {
        value &= 0b10111111;
    }

    if (timeBattery)
    {
        value &= 0b01111011;
    } else
    {
        value |= 0b10000000;
    }

    writeRegister8(DS3231_REG_CONTROL, value);
}

bool DS3231::isOutput(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b00000100;
    value >>= 2;

    return !value;
}

void DS3231::setOutput(DS3231_sqw_t mode)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b11100111;
    value |= (mode << 3);

    writeRegister8(DS3231_REG_CONTROL, value);
}

DS3231_sqw_t DS3231::getOutput(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b00011000;
    value >>= 3;

    return (DS3231_sqw_t)value;
}

void DS3231::enable32kHz(bool enabled)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);

    value &= 0b11110111;
    value |= (enabled << 3);

    writeRegister8(DS3231_REG_STATUS, value);
}

bool DS3231::is32kHz(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);

    value &= 0b00001000;
    value >>= 3;

    return value;
}

void DS3231::forceConversion(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value |= 0b00100000;

    writeRegister8(DS3231_REG_CONTROL, value);

    do {} while ((readRegister8(DS3231_REG_CONTROL) & 0b00100000) != 0);
}

float DS3231::readTemperature(void)
{
    uint8_t msb, lsb;

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_TEMPERATURE);
    #else
        Wire.send(DS3231_REG_TEMPERATURE);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 2);

    while(!Wire.available()) {};

    #if ARDUINO >= 100
    msb = Wire.read();
    lsb = Wire.read();
    #else
    msb = Wire.receive();
    lsb = Wire.receive();
    #endif

    return ((((short)msb << 8) | (short)lsb) >> 6) / 4.0f;
}

RTCAlarmTime DS3231::getAlarm1(void)
{
    uint8_t values[4];
    RTCAlarmTime a;

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_ALARM_1);
    #else
        Wire.send(DS3231_REG_ALARM_1);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 4);

    while(!Wire.available()) {};

    for (int i = 3; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(Wire.read() & 0b01111111);
        #else
            values[i] = bcd2dec(Wire.receive() & 0b01111111);
        #endif
    }

    Wire.endTransmission();

    a.day = values[0];
    a.hour = values[1];
    a.minute = values[2];
    a.second = values[3];

    return a;
}

DS3231_alarm1_t DS3231::getAlarmType1(void)
{
    uint8_t values[4];
    uint8_t mode = 0;

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_ALARM_1);
    #else
        Wire.send(DS3231_REG_ALARM_1);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 4);

    while(!Wire.available()) {};

    for (int i = 3; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(Wire.read());
        #else
            values[i] = bcd2dec(Wire.receive());
        #endif
    }

    Wire.endTransmission();

    mode |= ((values[3] & 0b01000000) >> 6);
    mode |= ((values[2] & 0b01000000) >> 5);
    mode |= ((values[1] & 0b01000000) >> 4);
    mode |= ((values[0] & 0b01000000) >> 3);
    mode |= ((values[0] & 0b00100000) >> 1);

    return (DS3231_alarm1_t)mode;
}

void DS3231::setAlarm1(uint8_t dydw, uint8_t hour, uint8_t minute, uint8_t second, DS3231_alarm1_t mode, bool armed)
{
    second = dec2bcd(second);
    minute = dec2bcd(minute);
    hour = dec2bcd(hour);
    dydw = dec2bcd(dydw);

    switch(mode)
    {
        case DS3231_EVERY_SECOND:
            second |= 0b10000000;
            minute |= 0b10000000;
            hour |= 0b10000000;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_S:
            second &= 0b01111111;
            minute |= 0b10000000;
            hour |= 0b10000000;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_M_S:
            second &= 0b01111111;
            minute &= 0b01111111;
            hour |= 0b10000000;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_H_M_S:
            second &= 0b01111111;
            minute &= 0b01111111;
            hour &= 0b01111111;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_DT_H_M_S:
            second &= 0b01111111;
            minute &= 0b01111111;
            hour &= 0b01111111;
            dydw &= 0b01111111;
            break;

        case DS3231_MATCH_DY_H_M_S:
            second &= 0b01111111;
            minute &= 0b01111111;
            hour &= 0b01111111;
            dydw &= 0b01111111;
            dydw |= 0b01000000;
            break;
    }

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_ALARM_1);
        Wire.write(second);
        Wire.write(minute);
        Wire.write(hour);
        Wire.write(dydw);
    #else
        Wire.send(DS3231_REG_ALARM_1);
        Wire.send(second);
        Wire.send(minute);
        Wire.send(hour);
        Wire.send(dydw);
    #endif

    Wire.endTransmission();

    armAlarm1(armed);

    clearAlarm1();
}

bool DS3231::isAlarm1(bool clear)
{
    uint8_t alarm;

    alarm = readRegister8(DS3231_REG_STATUS);
    alarm &= 0b00000001;

    if (alarm && clear)
    {
        clearAlarm1();
    }

    return alarm;
}

void DS3231::armAlarm1(bool armed)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);

    if (armed)
    {
        value |= 0b00000001;
    } else
    {
        value &= 0b11111110;
    }

    writeRegister8(DS3231_REG_CONTROL, value);
}

bool DS3231::isArmed1(void)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);
    value &= 0b00000001;
    return value;
}

void DS3231::clearAlarm1(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);
    value &= 0b11111110;

    writeRegister8(DS3231_REG_STATUS, value);
}

RTCAlarmTime DS3231::getAlarm2(void)
{
    uint8_t values[3];
    RTCAlarmTime a;

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_ALARM_2);
    #else
        Wire.send(DS3231_REG_ALARM_2);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 3);

    while(!Wire.available()) {};

    for (int i = 2; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(Wire.read() & 0b01111111);
        #else
            values[i] = bcd2dec(Wire.receive() & 0b01111111);
        #endif
    }

    Wire.endTransmission();

    a.day = values[0];
    a.hour = values[1];
    a.minute = values[2];
    a.second = 0;

    return a;
}

DS3231_alarm2_t DS3231::getAlarmType2(void)
{
    uint8_t values[3];
    uint8_t mode = 0;

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_ALARM_2);
    #else
        Wire.send(DS3231_REG_ALARM_2);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 3);

    while(!Wire.available()) {};

    for (int i = 2; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(Wire.read());
        #else
            values[i] = bcd2dec(Wire.receive());
        #endif
    }

    Wire.endTransmission();

    mode |= ((values[2] & 0b01000000) >> 5);
    mode |= ((values[1] & 0b01000000) >> 4);
    mode |= ((values[0] & 0b01000000) >> 3);
    mode |= ((values[0] & 0b00100000) >> 1);

    return (DS3231_alarm2_t)mode;
}

void DS3231::setAlarm2(uint8_t dydw, uint8_t hour, uint8_t minute, DS3231_alarm2_t mode, bool armed)
{
    minute = dec2bcd(minute);
    hour = dec2bcd(hour);
    dydw = dec2bcd(dydw);

    switch(mode)
    {
        case DS3231_EVERY_MINUTE:
            minute |= 0b10000000;
            hour |= 0b10000000;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_M:
            minute &= 0b01111111;
            hour |= 0b10000000;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_H_M:
            minute &= 0b01111111;
            hour &= 0b01111111;
            dydw |= 0b10000000;
            break;

        case DS3231_MATCH_DT_H_M:
            minute &= 0b01111111;
            hour &= 0b01111111;
            dydw &= 0b01111111;
            break;

        case DS3231_MATCH_DY_H_M:
            minute &= 0b01111111;
            hour &= 0b01111111;
            dydw &= 0b01111111;
            dydw |= 0b01000000;
            break;
    }

    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(DS3231_REG_ALARM_2);
        Wire.write(minute);
        Wire.write(hour);
        Wire.write(dydw);
    #else
        Wire.send(DS3231_REG_ALARM_2);
        Wire.send(minute);
        Wire.send(hour);
        Wire.send(dydw);
    #endif

    Wire.endTransmission();

    armAlarm2(armed);

    clearAlarm2();
}

void DS3231::armAlarm2(bool armed)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);

    if (armed)
    {
        value |= 0b00000010;
    } else
    {
        value &= 0b11111101;
    }

    writeRegister8(DS3231_REG_CONTROL, value);
}

bool DS3231::isArmed2(void)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);
    value &= 0b00000010;
    value >>= 1;
    return value;
}


void DS3231::clearAlarm2(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);
    value &= 0b11111101;

    writeRegister8(DS3231_REG_STATUS, value);
}


bool DS3231::isAlarm2(bool clear)
{
    uint8_t alarm;

    alarm = readRegister8(DS3231_REG_STATUS);
    alarm &= 0b00000010;

    if (alarm && clear)
    {
        clearAlarm2();
    }

    return alarm;
}

uint8_t DS3231::bcd2dec(uint8_t bcd)
{
    return ((bcd / 16) * 10) + (bcd % 16);
}

uint8_t DS3231::dec2bcd(uint8_t dec)
{
    return ((dec / 10) * 16) + (dec % 10);
}

char *DS3231::strDayOfWeek(uint8_t dayOfWeek)
{
    switch (dayOfWeek) {
        case 1:
            return "Monday";
            break;
        case 2:
            return "Tuesday";
            break;
        case 3:
            return "Wednesday";
            break;
        case 4:
            return "Thursday";
            break;
        case 5:
            return "Friday";
            break;
        case 6:
            return "Saturday";
            break;
        case 7:
            return "Sunday";
            break;
        default:
            return "Unknown";
    }
}

char *DS3231::strMonth(uint8_t month)
{
    switch (month) {
        case 1:
            return "January";
            break;
        case 2:
            return "February";
            break;
        case 3:
            return "March";
            break;
        case 4:
            return "April";
            break;
        case 5:
            return "May";
            break;
        case 6:
            return "June";
            break;
        case 7:
            return "July";
            break;
        case 8:
            return "August";
            break;
        case 9:
            return "September";
            break;
        case 10:
            return "October";
            break;
        case 11:
            return "November";
            break;
        case 12:
            return "December";
            break;
        default:
            return "Unknown";
    }
}

char *DS3231::strAmPm(uint8_t hour, bool uppercase)
{
    if (hour < 12)
    {
        if (uppercase)
        {
            return "AM";
        } else
        {
            return "am";
        }
    } else
    {
        if (uppercase)
        {
            return "PM";
        } else
        {
            return "pm";
        }
    }
}

char *DS3231::strDaySufix(uint8_t day)
{
    if (day % 10 == 1)
    {
        return "st";
    } else
    if (day % 10 == 2)
    {
        return "nd";
    }
    if (day % 10 == 3)
    {
        return "rd";
    }

    return "th";
}

uint8_t DS3231::hour12(uint8_t hour24)
{
    if (hour24 == 0)
    {
        return 12;
    }

    if (hour24 > 12)
    {
       return (hour24 - 12);
    }

    return hour24;
}

long DS3231::time2long(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    return ((days * 24L + hours) * 60 + minutes) * 60 + seconds;
}

uint16_t DS3231::dayInYear(uint16_t year, uint8_t month, uint8_t day)
{
    uint16_t fromDate;
    uint16_t toDate;

    fromDate = date2days(year, 1, 1);
    toDate = date2days(year, month, day);

    return (toDate - fromDate);
}

bool DS3231::isLeapYear(uint16_t year)
{
    return (year % 4 == 0);
}

uint8_t DS3231::daysInMonth(uint16_t year, uint8_t month)
{
    uint8_t days;

    days = pgm_read_byte(daysArray + month - 1);

    if ((month == 2) && isLeapYear(year))
    {
        ++days;
    }

    return days;
}

uint16_t DS3231::date2days(uint16_t year, uint8_t month, uint8_t day)
{
    year = year - 2000;

    uint16_t days16 = day;

    for (uint8_t i = 1; i < month; ++i)
    {
        days16 += pgm_read_byte(daysArray + i - 1);
    }

    if ((month == 2) && isLeapYear(year))
    {
        ++days16;
    }

    return days16 + 365 * year + (year + 3) / 4 - 1;
}

uint32_t DS3231::unixtime(void)
{
    uint32_t u;

    u = time2long(date2days(t.year, t.month, t.day), t.hour, t.minute, t.second);
    u += 946681200;

    return u;
}

uint8_t DS3231::conv2d(const char* p)
{
    uint8_t v = 0;

    if ('0' <= *p && *p <= '9')
    {
        v = *p - '0';
    }

    return 10 * v + *++p - '0';
}

uint8_t DS3231::dow(uint16_t y, uint8_t m, uint8_t d)
{
    uint8_t dow;

    y -= m < 3;
    dow = ((y + y/4 - y/100 + y/400 + pgm_read_byte(dowArray+(m-1)) + d) % 7);

    if (dow == 0)
    {
        return 7;
    }

    return dow;
}

void DS3231::writeRegister8(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
        Wire.write(value);
    #else
        Wire.send(reg);
        Wire.send(value);
    #endif
    Wire.endTransmission();
}

uint8_t DS3231::readRegister8(uint8_t reg)
{
    uint8_t value;
    Wire.beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 1);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif;
    Wire.endTransmission();

    return value;
}
