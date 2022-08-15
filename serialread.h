#ifndef SERIAL_READ_H
#define SERIAL_READ_H

#define SERIAL_OUT_OPERATION_FINISHED F("<,.>")
#define EEPROM_MEMORY_SHIFT 128
#include "Wire.h"
//#include "longliveEEPROM.h"

class SerialRead{ 

private:

public:
  static int readInt(char *str, bool &ok);
  static int32_t readLInt(char *str, bool &ok);
  static float readFloat(char *str, bool &ok);
  static void readLn(char *str, bool &ok);
  static int readCmd(char *str, bool &ok);   
};

#endif
