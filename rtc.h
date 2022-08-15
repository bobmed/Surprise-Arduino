#ifndef REAL_TIME_CLOCK_H
#define REAL_TIME_CLOCK_H


#include "serialread.h"
#include "DS3231.h"
#include <EEPROM.h>

class RTC : private SerialRead{ 

private:

  enum BitsArr{
  enable = 0b00000001,
	ch1_on = 0b00000010,
	ch2_on = 0b00000100,
	ch3_on = 0b00001000,
	ch4_on = 0b00010000
  };

  struct RTCDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};
  
	struct RtcEEPROMInfo{	
		uint16_t  	bitsWrd = 0;
		int16_t	rtcAddress = 0x68;
		uint16_t	year	= 0;
		uint8_t		month	= 0;
		uint8_t		day		= 0;
		uint8_t		hour	= 0;
		uint8_t		minute	= 0;
		uint8_t		second	= 0;
	};
  
  int _memAddress;
  RtcEEPROMInfo tempEEPROM;
  RtcEEPROMInfo _dataEEPROM;
  DS3231 clock;
  RTCDateTime dt;
  RTCDateTime dtTemp;

  void updateDateTime();
 
public:
  bool begin(uint16_t addr);
  void restart();
  void setRtcInfoEEPROM(int &operNumb,char *valIn);
  void getRtcInfoEEPROM(int &operNumb);
  void setRtcTime(int &operNumb,char *valIn);
  void getRtcTime(int &operNumb);
  void setDefaultMemory();
  uint8_t getAlarmStatus();
  int16_t get_I2C_Addr();
  void setEEPROMaddress(uint16_t memAddress);
  uint8_t getUsedOuts();
  static int sizeOfEEPROM();

 
};

 

#endif
