#ifndef WHOLE_DEVICE_H
#define WHOLE_DEVICE_H

#define TRIGGERING_TIME       1500    //ms

////////////////////////////EEPROM_STATIC_ADDRESSES////////////////////////////////////
#define OLD_TRIG_EEPROM_ADDR    4
#define TRIGGERED_OUTS_ADDR     10      //uint8_t
#define START_DATA_EEPROM_ADDR  128
#define SELFTRIG_LEFT_ADDR      512   //Do not record EEPROM info between this addresses
#define EEPROM_MEMORY_SIZE      1024   //Do not record EEPROM info between this addresses
////////////////////////////EEPROM_STATIC_ADDRESSES////////////////////////////////////

#define SERIAL_IN_HELLO "#@($0"
#define SERIAL_OUT_HELLO F("^%*+")
#define SERIAL_OUT_START_OPERATION F("!612!")
#define SERIAL_IN_GOODBYE "-/';"

#define CHANNEL_1_OUT 2
#define CHANNEL_2_OUT 3
#define CHANNEL_3_OUT 4
#define CHANNEL_4_OUT 5
#define SAFETY_IN_PIN 6
#define RESET_MEM_BTN_PIN 11
#define READ_SERIAL_PERMISSION 12



#include <avr/wdt.h>
#include "Wire.h"
#include "accelerometer.h"
#include "inchnls.h"
#include "rtc.h"
#include "serialread.h"
#include "Timer.h"
#include "Power.h"
#include "longliveEEPROM.h"


class WholeDevice : private SerialRead, private LL_EEPROM{ 

private:

  enum cmdNum{
    SERIAL_IN_GET_ALL_INFO          = 10000,
    SERIAL_IN_SET_ALL_INFO          = 10050,
    
    SERIAL_IN_GET_I2C_ADDRESSES     = 9999,
    SERIAL_IN_GET_DEV_INFO          = 10001,
    SERIAL_IN_GET_DEV_ZERO_EEPROM   = 10002,
    SERIAL_IN_GET_DEV_COCK_TIME_LFT = 10003,
    SERIAL_IN_GET_DEV_TRIG_TIME_LFT = 10004,
    SERIAL_IN_GET_DEV_TRIG_STATES   = 10005,
    SERIAL_IN_SET_DEV_INFO          = 10051,
    SERIAL_IN_SET_DEV_ZERO_EEPROM   = 10052,
    SERIAL_IN_RESET_DEV_TRIGG       = 10053,
    SERIAL_IN_SET_DEV_TRIGG         = 10054,
    SERIAL_IN_RESET_DEV_TOTALY      = 10055,
    
    SERIAL_IN_GET_ACC_INFO          = 11001,
    SERIAL_IN_GET_ACC_VALUE         = 11002,
    SERIAL_IN_SET_ACC_INFO          = 11051,
    SERIAL_IN_SET_ACC_ZERO_SP_INFO  = 11052,
    
    SERIAL_IN_GET_RTC_INFO          = 12001,    
    SERIAL_IN_GET_RTC_TIME          = 12002,
    SERIAL_IN_SET_RTC_INFO          = 12051, 
    SERIAL_IN_SET_RTC_CURR_TIME     = 12052,
    SERIAL_IN_SET_RTC_MANU_TIME     = 12053,

    SERIAL_IN_GET_INCH_INFO         = 13001,  
    SERIAL_IN_SET_INCH_INFO         = 13051
  };

  enum BitsArr{
    selftrigEnabled = 0b00000001,
    pwrSftyMode     = 0b00000010,
    slpTmrCalibrtd  = 0b00000100,
    cockingStrtd    = 0b00001000,
    cockingCompltd  = 0b00010000
  };
  
	struct DeviceEEPROMInfo{
		int       programVersion    = 0;
    uint16_t  bitsWrd           = 0;
    uint16_t  sleepTmrZeroing   = 0;
    uint16_t  delaySleep        = 100;//msecs
		int16_t   distCockingMechTime = 150;//secs
		int32_t   selfTriggTime     = 86400;//secs
		uint8_t  chnlsTotalTrig     = 0;
		uint8_t  chnlsInTrig        = 0;
    uint8_t  chnlsSelfTrig      = 0;
		uint8_t  chnlsAccTrig       = 0;
		uint8_t  chnlsRtcTrig       = 0;
	};

  bool connectionOk = false;
  char valStr[64];
  int _memAddress = 3;  
  int funcId = 0;
  int operationNumber = 0;
  int32_t _selfTrigTime = 86400;
  int8_t cmdRstCount = 0;
  int8_t devRstCount = 0;
  uint8_t _tmprdPrm = 0;
  DeviceEEPROMInfo tempEEPROM;
  DeviceEEPROMInfo _dataEEPROM;
  
  bool accBeginSuccess = false;
  bool rtcBeginSuccess = false;
  
  Accelerometer acc;
  RTC rtc;
  InChannels inch;
  GTimer tmr;

  void checkFunc();
  void setZeroEEPROMAddr();
  void getZeroEEPROMAddr();
  void setDevInfoEEPROM();
  void getDevInfoEEPROM();
  void getCockTimeLeft();
  void getTrigTimeLeft();
  void getTrigStates();
  void resetTriggers();
  void setTrigger();
  void sleepTmrZeroing();
  void beginDevices();
  void setEEPROMaddresses(uint16_t addr);
  void checkTimer();
  void getAlarmStatuses();
  uint8_t getAlarmStatus();
  void setOutPins();
  void setPinsOutNow(uint8_t in);
  bool wireConnectOk(int16_t addrI2C);
  void get_I2C_Addresses();
  void resetDevice();
  void resetDeviceNow();
  void setDefaultMemory();
  void blinkDiode(const uint8_t _nbrs, const int _glowsMs, const int _fadeMs);
  void clearSlfTrigData();
  void putSlfTrigData(int32_t const _in);
  int32_t readSlfTrigData();
public:
  WholeDevice();
  void begin();
	void checkSerial();
  void refreshData();
  void mainLoop();
  uint8_t getUsedOuts();
  static int sizeOfEEPROM();

 
};

 

#endif
