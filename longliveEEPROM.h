#ifndef LONG_LEAVE_EEPROM_h
#define LONG_LEAVE_EEPROM_h
#include <EEPROM.h>
//#define __TEST_LONG_LIVE__

class LL_EEPROM{
  
public:
void LL_EEPROM::clrDataEEPROM(int16_t const _begin, int16_t const _end){
  for(int i = _begin; i < _end; i++){
    EEPROM.update(i,0xFF);
  }
   #ifdef __TEST__
      Serial.println('c');
   #endif
}

template<typename T>
void LL_EEPROM::putDataMultiEEPROM(int16_t const _begin, int16_t const _end, const T _in){//
  int _addr;
  bool flag = false;
  int8_t const _size = sizeof(_in);
  const T _mask = ((1 << (_size * 8))-1);//FF mask
  T _out = 0;
  T _temp;
  T _tempOld;
  
  for(int i = _begin; i <= (_end-_size); i+=_size){
    EEPROM.get(i, _temp);
    if(_temp == _mask){
      flag = true;
      if(_tempOld != _in)_addr = i;
      break;
    }
    _addr = i;
    _tempOld = _temp;   
  }
  if(!flag){
    if(_tempOld != _in){
      clrDataEEPROM(_begin, _end);
      _addr = _begin;
    }
  }
  #ifdef __TEST_LONG_LIVE__
   Serial.print('w'); Serial.print(_addr); Serial.print('\t'); Serial.println(_in); 
  #endif
  EEPROM.put(_addr, _in);
}

template<typename T>
void LL_EEPROM::readDataMultiEEPROM(int16_t const _begin, int16_t const _end, T& _in){//
  bool flag = false;
  int8_t const _size = sizeof(_in);
  const T _mask = ((1 << (_size * 8))-1);//FF mask
  T _out = 0;
  T _temp;

  for(int i = _begin; i <= (_end-_size); i+=_size){
    EEPROM.get(i, _temp);
    if(_temp == _mask){
      #ifdef __TEST_LONG_LIVE__
       Serial.print('r'); Serial.print(i-_size); 
      #endif 
      break;}
    _out = _temp;
  }

  #ifdef __TEST_LONG_LIVE__
   Serial.print('\t'); Serial.println(_in); 
  #endif
  _in = _out;
}
};

#endif
