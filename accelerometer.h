#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <EEPROM.h>
#include "Wire.h"
#include "serialread.h"



class Accelerometer : private SerialRead
{ 
  private:
    
    struct eepromInfo{      
      int16_t bitsWrd     = 0;    //=2 bytes
      int16_t accAddress  = 0x53; //=2 bytes
      float sensGSP       = 0.5;  //=4 bytes
      float sensCosSP     = 0.9;  //=4 bytes
      float xMem          = 0.0;  //=4 bytes
      float yMem          = 0.0;  //=4 bytes
      float zMem          = 0.0;  //=4 bytes
      float gVal          = 0.0;  //=4 bytes
    };                    //=28 bytes in total
             
    enum BitsArr{
      enable            = 0b00000001,
      cosEnable         = 0b00000010,
      zeroingCompleted  = 0b00000100,
      ch1_on            = 0b00001000,
      ch2_on            = 0b00010000,
      ch3_on            = 0b00100000,
      ch4_on            = 0b01000000
    };

    eepromInfo tempEEPROM;
    eepromInfo _dataEEPROM;
    
	  float getCosVal();
    float getGVal();
    
    float _accVal, _cosVal,
          _x, _y, _z;
    int _memAddress; 
    

  public:
  
    bool begin(int memAddress);
    void restart();
    void setEEPROMaddress(int memAddress);
    void updateEEPROMaddress(int memAddress);    
    void refreshData();
    void printInfoSerial(int &operNumb);
    void getAccInfoEEPROM(int &operNumb);
    void setAccInfoEEPROM(int &operNumb, char *valIn);
    void set_XYZ_PC_EEPROM(int &operNumb,char *valIn);
    void set_zeroXYZ_EEPROM();
    void setDefaultMemory();
    uint8_t getAlarmStatus();
    int16_t get_I2C_Addr();
    uint8_t getUsedOuts();
    static int sizeOfEEPROM();
    
    
};

#endif
