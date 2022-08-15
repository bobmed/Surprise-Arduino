
#include "accelerometer.h"


#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif



bool Accelerometer::begin(int memAddress){
  this->_memAddress = memAddress;
   
  EEPROM.get(this->_memAddress, tempEEPROM);
  if((tempEEPROM.accAddress < 8)||(tempEEPROM.accAddress >127)){
    tempEEPROM.accAddress = 0x53;
    EEPROM.put(this->_memAddress, tempEEPROM);
  }
  _dataEEPROM = tempEEPROM;

  bool beginSuccessful;
  bool connectionOk;
                                                          
  connectionOk = true;
  if(connectionOk){
    Wire.begin(); // Initiate the Wire library
    // Set ADXL345 in measuring mode
    Wire.beginTransmission(_dataEEPROM.accAddress); // Start communicating with the device 
    Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
    // Enable measurement
    Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
    Wire.endTransmission();
    delay(10);
    beginSuccessful = true;
  }else beginSuccessful = false;
  
  return beginSuccessful;
}

void Accelerometer::restart(){
  Wire.begin(); // Initiate the Wire library
    // Set ADXL345 in measuring mode
    Wire.beginTransmission(_dataEEPROM.accAddress); // Start communicating with the device 
    Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
    // Enable measurement
    Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
    Wire.endTransmission();
    delay(10);
}

void Accelerometer::setEEPROMaddress(int memAddress){
  this->_memAddress = memAddress;  
}

void Accelerometer::updateEEPROMaddress(int memAddress){
  EEPROM.get(this->_memAddress, _dataEEPROM);
  setEEPROMaddress(memAddress);
  EEPROM.put(this->_memAddress, _dataEEPROM);
}

void Accelerometer::refreshData(){
  // === Read acceleromter data === //
  Wire.beginTransmission(_dataEEPROM.accAddress);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(_dataEEPROM.accAddress, 6, true); // Read 6 registers total, each axis value is stored in 2 registers

  this->_x = (Wire.read()|Wire.read() << 8); // X-axis value
  this->_x /= 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  
  this->_y = (Wire.read()|Wire.read() << 8); // Y-axis value
  this->_y /= 256;
    
  this->_z = (Wire.read()|Wire.read() << 8); // Z-axis value
  this->_z /= 256;
  
  _accVal = getGVal();

  _cosVal = getCosVal();
}

uint8_t Accelerometer::getAlarmStatus(){
  refreshData();
  uint8_t out = 0;
  bool alrmStatus = false;
  if(_dataEEPROM.bitsWrd & enable){//if enabled
    if(_dataEEPROM.bitsWrd & zeroingCompleted){
      if(_dataEEPROM.bitsWrd & cosEnable){
        if (_cosVal < _dataEEPROM.sensCosSP) alrmStatus = true;
      }
      if(abs(_dataEEPROM.gVal - _accVal) > _dataEEPROM.sensGSP){
        alrmStatus = true;
      }
      
    }else{set_zeroXYZ_EEPROM();return 0;}
  if(alrmStatus) out |= _dataEEPROM.bitsWrd >> 3;
  }

  return out;
}

void Accelerometer::printInfoSerial(int &operNumb){
  switch(operNumb){
  case 0:  
  refreshData();
  //Serial.print("Xa= ");
  Serial.println(this->_x);
  //Serial.print("   Ya= ");
  Serial.println(this->_y);
  //Serial.print("   Za= ");
  Serial.println(this->_z);
  //Serial.print("   Acc val= ");
  Serial.println(this->_accVal);
  //Serial.print("   cos a= ");
  Serial.println(this->_cosVal);
  Serial.println(SERIAL_OUT_OPERATION_FINISHED);
  operNumb = -1;
  default:
      operNumb = -2;
      break;
  }  
}


void Accelerometer::getAccInfoEEPROM(int &operNumb){
  switch(operNumb){
    case 0:
      EEPROM.get(_memAddress, tempEEPROM);

      Serial.println(tempEEPROM.bitsWrd);
      Serial.println(tempEEPROM.accAddress);
      Serial.println(tempEEPROM.sensGSP);
      Serial.println(tempEEPROM.sensCosSP);
      Serial.println(tempEEPROM.xMem);
      Serial.println(tempEEPROM.yMem);
      Serial.println(tempEEPROM.zMem);
      Serial.println(tempEEPROM.gVal);
      operNumb = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      
    default:
      operNumb = -2;
      break;
  }
}


void Accelerometer::setAccInfoEEPROM(int &operNumb,char *valIn){
  bool ok;
  switch(operNumb){
    case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);    
      tempEEPROM.bitsWrd = 0;
      tempEEPROM.bitsWrd = readInt(valIn,ok);
      if(ok){operNumb=10; ok = false;}
      else{break;}
    case 10:
      tempEEPROM.accAddress = readInt(valIn,ok);
      if(ok){operNumb=20; ok = false;}
      else{break;}
    case 20:  
      tempEEPROM.sensGSP = readFloat(valIn,ok);
      if(ok){operNumb=30; ok = false;}
      else{break;}
    case 30:
      tempEEPROM.sensCosSP = readFloat(valIn,ok);
      if(ok){
        EEPROM.put(this->_memAddress, tempEEPROM);
        EEPROM.put(_memAddress+EEPROM_MEMORY_SHIFT, tempEEPROM);
        _dataEEPROM = tempEEPROM;
        ok = false;
        operNumb = -1;
        Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      }
      break;
    
    default: 
      operNumb = -2;
      break;
  }
}

void Accelerometer::set_XYZ_PC_EEPROM(int &operNumb,char *valIn){
  bool ok;
  switch(operNumb){
  case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);  
      tempEEPROM.xMem = readFloat(valIn,ok);
      if(ok){operNumb=10;}
      else{break;}
    case 10:  
      tempEEPROM.yMem = readFloat(valIn,ok);
      if(ok){operNumb=20;}
      else{break;}
    case 20:
      tempEEPROM.zMem = readFloat(valIn,ok);
      if(ok){
        tempEEPROM.gVal = sqrt(sq(tempEEPROM.xMem) + sq(tempEEPROM.yMem) + sq(tempEEPROM.zMem));
        EEPROM.put(this->_memAddress, tempEEPROM);
        EEPROM.put(_memAddress+EEPROM_MEMORY_SHIFT, tempEEPROM);
        _dataEEPROM = tempEEPROM;
        operNumb++; 
        ok = false;
        operNumb = -1;
        Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      }
      break;
    default: 
      operNumb = -2;
      break;
  }
}

void Accelerometer::set_zeroXYZ_EEPROM(){
      
  EEPROM.get(this->_memAddress, tempEEPROM);
  if(!(tempEEPROM.bitsWrd & zeroingCompleted)){
    refreshData();
    tempEEPROM.xMem = this->_x;
    tempEEPROM.yMem = this->_y;
    tempEEPROM.zMem = this->_z;
    tempEEPROM.gVal = this->_accVal;
    
    tempEEPROM.bitsWrd |= zeroingCompleted;
    
    EEPROM.put(this->_memAddress, tempEEPROM);
    _dataEEPROM = tempEEPROM;
  }
   
}

void Accelerometer::setDefaultMemory(){
  EEPROM.get(_memAddress+EEPROM_MEMORY_SHIFT, tempEEPROM);
  EEPROM.put(this->_memAddress, tempEEPROM);
  _dataEEPROM = tempEEPROM;
}

float Accelerometer::getCosVal(){
  //EEPROM.get(this->_memAddress, tempEEPROM);
  if (_accVal*_dataEEPROM.gVal != 0.0){
    return (_x*_dataEEPROM.xMem + _y*_dataEEPROM.yMem + _z*_dataEEPROM.zMem) / (_accVal*_dataEEPROM.gVal);
  }else return 0.0;
}

float Accelerometer::getGVal(){
  float out = sqrt(sq(this->_x) + sq(this->_y) + sq(this->_z));
  return out;
}

int16_t Accelerometer::get_I2C_Addr(){
  return _dataEEPROM.accAddress;
}

uint8_t Accelerometer::getUsedOuts(){
  uint8_t outWrd = 0;
  if(_dataEEPROM.bitsWrd & enable){ 
    outWrd |= (_dataEEPROM.bitsWrd >> 3);
  }
  return outWrd;
}

static int Accelerometer::sizeOfEEPROM(){
  return (int)sizeof(eepromInfo);
}
