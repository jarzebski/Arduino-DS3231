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


#ifndef DS3231_h
#define DS3231_h

#include <Wire.h>

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DS3231_ADDRESS              (0x68)

#define DS3231_REG_TIME             (0x00)
#define DS3231_REG_ALARM_1          (0x07)
#define DS3231_REG_ALARM_2          (0x0B)
#define DS3231_REG_CONTROL          (0x0E)
#define DS3231_REG_STATUS           (0x0F)
#define DS3231_REG_TEMPERATURE      (0x11)

#ifndef RTCDATETIME_STRUCT_H
#define RTCDATETIME_STRUCT_H
struct RTCDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    uint32_t unixtime;
};

struct RTCAlarmTime
{
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};
#endif

typedef enum
{
    DS3231_1HZ          = 0x00,
    DS3231_4096HZ       = 0x01,
    DS3231_8192HZ       = 0x02,
    DS3231_32768HZ      = 0x03
} DS3231_sqw_t;

typedef enum
{
    DS3231_EVERY_SECOND   = 0b00001111,
    DS3231_MATCH_S        = 0b00001110,
    DS3231_MATCH_M_S      = 0b00001100,
    DS3231_MATCH_H_M_S    = 0b00001000,
    DS3231_MATCH_DT_H_M_S = 0b00000000,
    DS3231_MATCH_DY_H_M_S = 0b00010000
} DS3231_alarm1_t;

typedef enum
{
    DS3231_EVERY_MINUTE   = 0b00001110,
    DS3231_MATCH_M        = 0b00001100,
    DS3231_MATCH_H_M      = 0b00001000,
    DS3231_MATCH_DT_H_M   = 0b00000000,
    DS3231_MATCH_DY_H_M   = 0b00010000
} DS3231_alarm2_t;

class DS3231
{
public:
    DS3231(TwoWire *theWire = &Wire);

	bool begin(void);

	void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
	void setDateTime(uint32_t t);
	void setDateTime(const char* date, const char* time);
	RTCDateTime getDateTime(void);
	uint8_t isReady(void);

	DS3231_sqw_t getOutput(void);
	void setOutput(DS3231_sqw_t mode);
	void enableOutput(bool enabled);
	bool isOutput(void);
	void enable32kHz(bool enabled);
	bool is32kHz(void);

	void forceConversion(void);
	float readTemperature(void);

	void setAlarm1(uint8_t dydw, uint8_t hour, uint8_t minute, uint8_t second, DS3231_alarm1_t mode, bool armed = true);
	RTCAlarmTime getAlarm1(void);
	DS3231_alarm1_t getAlarmType1(void);
	bool isAlarm1(bool clear = true);
	void armAlarm1(bool armed);
	bool isArmed1(void);
	void clearAlarm1(void);

	void setAlarm2(uint8_t dydw, uint8_t hour, uint8_t minute, DS3231_alarm2_t mode, bool armed = true);
	RTCAlarmTime getAlarm2(void);
	DS3231_alarm2_t getAlarmType2(void);
	bool isAlarm2(bool clear = true);
	void armAlarm2(bool armed);
	bool isArmed2(void);
	void clearAlarm2(void);

	void setBattery(bool timeBattery, bool squareBattery);

	char* dateFormat(const char* dateFormat, RTCDateTime dt);
	char* dateFormat(const char* dateFormat, RTCAlarmTime dt);

	static RTCDateTime loadDateTimeFromLong(uint32_t t);

private:
	RTCDateTime t;

	char *strDayOfWeek(uint8_t dayOfWeek);
	char *strMonth(uint8_t month);
	char *strAmPm(uint8_t hour, bool uppercase);
	char *strDaySufix(uint8_t day);

	uint8_t hour12(uint8_t hour24);
	uint8_t bcd2dec(uint8_t bcd);
	uint8_t dec2bcd(uint8_t dec);

	long time2long(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);
	uint16_t date2days(uint16_t year, uint8_t month, uint8_t day);
	uint8_t daysInMonth(uint16_t year, uint8_t month);
	uint16_t dayInYear(uint16_t year, uint8_t month, uint8_t day);
	bool isLeapYear(uint16_t year);
	uint8_t dow(uint16_t y, uint8_t m, uint8_t d);

	uint32_t unixtime(void);
	uint8_t conv2d(const char* p);

	void writeRegister8(uint8_t reg, uint8_t value);
	uint8_t readRegister8(uint8_t reg);
    TwoWire *wire;
};

#endif
