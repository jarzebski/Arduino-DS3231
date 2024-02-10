/*

The MIT License

Copyright (c) 2014-2023 Korneliusz Jarzębski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

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

DS3231::DS3231(TwoWire *theWire) {
    wire = theWire;
}

bool DS3231::begin(void)
{
    wire->begin();

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
    wire->beginTransmission(DS3231_ADDRESS);

    #if ARDUINO >= 100
        wire->write(DS3231_REG_TIME);
    #else
        wire->send(DS3231_REG_TIME);
    #endif

    #if ARDUINO >= 100
        wire->write(dec2bcd(second));
        wire->write(dec2bcd(minute));
        wire->write(dec2bcd(hour));
        wire->write(dec2bcd(dow(year, month, day)));
        wire->write(dec2bcd(day));
        wire->write(dec2bcd(month));
        wire->write(dec2bcd(year - 2000));
    #else
        wire->send(dec2bcd(second));
        wire->send(dec2bcd(minute));
        wire->send(dec2bcd(hour));
        wire->send(dec2bcd(dow(year, month, day)));
        wire->send(dec2bcd(day));
        wire->send(dec2bcd(month));
        wire->send(dec2bcd(year-2000));
    #endif

    wire->endTransmission();
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


RTCDateTime DS3231::loadDateTimeFromLong(uint32_t t)
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

    RTCDateTime temp;
    temp.year = year+2000;
    temp.month = month;
    temp.day = day;
    temp.hour = hour;
    temp.minute = minute;
    temp.second = second;

    return temp;
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
    static char buffer[255];

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
    static char buffer[255];

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_TIME);
    #else
        wire->send(DS3231_REG_TIME);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 7);

    for (int i = 6; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(wire->read());
        #else
            values[i] = bcd2dec(wire->receive());
        #endif
    }

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_TEMPERATURE);
    #else
        wire->send(DS3231_REG_TEMPERATURE);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 2);

    #if ARDUINO >= 100
    msb = wire->read();
    lsb = wire->read();
    #else
    msb = wire->receive();
    lsb = wire->receive();
    #endif

    return ((((short)msb << 8) | (short)lsb) >> 6) / 4.0f;
}

RTCAlarmTime DS3231::getAlarm1(void)
{
    uint8_t values[4];
    RTCAlarmTime a;

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_ALARM_1);
    #else
        wire->send(DS3231_REG_ALARM_1);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 4);

    for (int i = 3; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(wire->read() & 0b01111111);
        #else
            values[i] = bcd2dec(wire->receive() & 0b01111111);
        #endif
    }

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_ALARM_1);
    #else
        wire->send(DS3231_REG_ALARM_1);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 4);

    for (int i = 3; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(wire->read());
        #else
            values[i] = bcd2dec(wire->receive());
        #endif
    }

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_ALARM_1);
        wire->write(second);
        wire->write(minute);
        wire->write(hour);
        wire->write(dydw);
    #else
        wire->send(DS3231_REG_ALARM_1);
        wire->send(second);
        wire->send(minute);
        wire->send(hour);
        wire->send(dydw);
    #endif

    wire->endTransmission();

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_ALARM_2);
    #else
        wire->send(DS3231_REG_ALARM_2);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 3);

    for (int i = 2; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(wire->read() & 0b01111111);
        #else
            values[i] = bcd2dec(wire->receive() & 0b01111111);
        #endif
    }

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_ALARM_2);
    #else
        wire->send(DS3231_REG_ALARM_2);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 3);

    for (int i = 2; i >= 0; i--)
    {
        #if ARDUINO >= 100
            values[i] = bcd2dec(wire->read());
        #else
            values[i] = bcd2dec(wire->receive());
        #endif
    }

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

    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(DS3231_REG_ALARM_2);
        wire->write(minute);
        wire->write(hour);
        wire->write(dydw);
    #else
        wire->send(DS3231_REG_ALARM_2);
        wire->send(minute);
        wire->send(hour);
        wire->send(dydw);
    #endif

    wire->endTransmission();

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
            return (char*) "Monday";
            break;
        case 2:
            return (char*) "Tuesday";
            break;
        case 3:
            return (char*) "Wednesday";
            break;
        case 4:
            return (char*) "Thursday";
            break;
        case 5:
            return (char*) "Friday";
            break;
        case 6:
            return (char*) "Saturday";
            break;
        case 7:
            return (char*) "Sunday";
            break;
        default:
            return (char*) "Unknown";
    }
}

char *DS3231::strMonth(uint8_t month)
{
    switch (month) {
        case 1:
            return (char*) "January";
            break;
        case 2:
            return (char*) "February";
            break;
        case 3:
            return (char*) "March";
            break;
        case 4:
            return (char*) "April";
            break;
        case 5:
            return (char*) "May";
            break;
        case 6:
            return (char*) "June";
            break;
        case 7:
            return (char*) "July";
            break;
        case 8:
            return (char*) "August";
            break;
        case 9:
            return (char*) "September";
            break;
        case 10:
            return (char*) "October";
            break;
        case 11:
            return (char*) "November";
            break;
        case 12:
            return (char*) "December";
            break;
        default:
            return (char*) "Unknown";
    }
}

char *DS3231::strAmPm(uint8_t hour, bool uppercase)
{
    if (hour < 12)
    {
        if (uppercase)
        {
            return (char*) "AM";
        } else
        {
            return (char*) "am";
        }
    } else
    {
        if (uppercase)
        {
            return (char*) "PM";
        } else
        {
            return (char*) "pm";
        }
    }
}

char *DS3231::strDaySufix(uint8_t day)
{
     if (day >= 11 && day <= 13)
     {
         return (char*) "th";
     }

     switch (day % 10)
     {
         case 1:
             return (char*) "st";
             break;
         case 2:
             return (char*) "nd";
             break;
         case 3:
             return (char*) "rd";
             break;
         default:
             return (char*) "th";
         break;
      }
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
        if ((month == 1) && isLeapYear(year))
        {
            ++days16;
        }
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
    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(reg);
        wire->write(value);
    #else
        wire->send(reg);
        wire->send(value);
    #endif
    wire->endTransmission();
}

uint8_t DS3231::readRegister8(uint8_t reg)
{
    uint8_t value;
    wire->beginTransmission(DS3231_ADDRESS);
    #if ARDUINO >= 100
        wire->write(reg);
    #else
        wire->send(reg);
    #endif
    wire->endTransmission();

    wire->requestFrom(DS3231_ADDRESS, 1);

    #if ARDUINO >= 100
        value = wire->read();
    #else
        value = wire->receive();
    #endif

    return value;
}
