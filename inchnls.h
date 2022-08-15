#ifndef INPUT_CHANNELS_H
#define INPUT_CHANNELS_H

#define CHANNEL_1_IN  7
#define CHANNEL_2_IN  8
#define CHANNEL_3_IN  9
#define CHANNEL_4_IN  10


#include "serialread.h"
#include <EEPROM.h>

class InChannels : private SerialRead{ 

private:

  enum BitsArr{
    chnl_on 	  = 0b00000001,
    chnl_nc 	  = 0b00000010,
    chnl_out1 	= 0b00000100,
    chnl_out2	  = 0b00001000,
    chnl_out3 	= 0b00010000,
    chnl_out4 	= 0b00100000
  };
  
	struct InChEEPROMInfo{	
		uint16_t  bitsWrd[4] = {0,0,0,0};
	};
  
  //uint16_t tempTrig = 0;
  int _memAddress;
  InChEEPROMInfo tempEEPROM;
  InChEEPROMInfo _dataEEPROM;

  
 
public:
  void begin(uint16_t addr);
  void setIOInfoEEPROM(int &operNumb,char *valIn);
  void getIOInfoEEPROM(int &operNumb);
  void setDefaultMemory();
  uint8_t getAlarmStatus();
  void setEEPROMaddress(uint16_t addr);
  uint8_t getUsedOuts();
  static int sizeOfEEPROM();

 
};

 

#endif
