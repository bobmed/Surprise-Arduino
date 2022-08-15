#include "rtc.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif


bool RTC::begin(uint16_t addr){        
  setEEPROMaddress(addr);          
  EEPROM.get(this->_memAddress, tempEEPROM);
  if((tempEEPROM.rtcAddress < 8)||(tempEEPROM.rtcAddress >126)){
    tempEEPROM.rtcAddress = 0x68;
    EEPROM.put(this->_memAddress, tempEEPROM);
  }
  _dataEEPROM = tempEEPROM;

  bool beginSuccessful;
     clock.begin(_dataEEPROM.rtcAddress);
     beginSuccessful = true;  
  return beginSuccessful;
}

void RTC::restart(){
  clock.begin(_dataEEPROM.rtcAddress);
}

void RTC::setEEPROMaddress(uint16_t addr){
  this->_memAddress = addr;
}


uint8_t RTC::getAlarmStatus(){
  if(_dataEEPROM.bitsWrd & enable){
  uint8_t out = _dataEEPROM.bitsWrd >> 1;
  updateDateTime();
  if(dt.year>_dataEEPROM.year){
    if(dt.year>2021) return out;
  }else if(dt.year==_dataEEPROM.year){
	if(dt.month>_dataEEPROM.month){
      return out;
    }else if(dt.month==_dataEEPROM.month){
	  if(dt.day>_dataEEPROM.day){
	    return out;
	  }else if(dt.day==_dataEEPROM.day){
	    if(dt.hour>_dataEEPROM.hour){
		  return out;
		}else if(dt.hour==_dataEEPROM.hour){
		  if(dt.minute>_dataEEPROM.minute){
		    return out;
		  }else if(dt.minute==_dataEEPROM.minute){
		    if(dt.second>=_dataEEPROM.second){
			  return out;	
			}
		  }
		}
	  }
	}
  } 
  }
  return 0;
}

void RTC::updateDateTime(){
  bool h12; bool PM_time; bool Century;
    dt.year   = clock.getYear()+2000;
    dt.month  = clock.getMonth(Century);
    dt.day    = clock.getDate(); 
    dt.hour   = clock.getHour(h12, PM_time);
    dt.minute = clock.getMinute();
    dt.second = clock.getSecond();
}

void RTC::setRtcInfoEEPROM(int &operNumb,char *valIn){
  bool ok;
  switch(operNumb){
    case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);   
      tempEEPROM.bitsWrd = 0;
      tempEEPROM.bitsWrd = readInt(valIn,ok);	  
      if(ok){operNumb=10;
	  }else{break;}
	case 10: 
	  tempEEPROM.rtcAddress = readInt(valIn,ok);	  
      if(ok){operNumb=20;
	  }else{break;}		
	case 20:  
	  tempEEPROM.year = readInt(valIn,ok);	  
      if(ok){operNumb=30;
	  }else{break;}
	case 30:  
	  tempEEPROM.month = readInt(valIn,ok);	  
      if(ok){operNumb=40;
	  }else{break;}
	case 40:  
	  tempEEPROM.day = readInt(valIn,ok);	  
      if(ok){operNumb=50;
	  }else{break;}
	case 50:  
	  tempEEPROM.hour = readInt(valIn,ok);	  
      if(ok){operNumb=60;
	  }else{break;}
	case 60:  
	  tempEEPROM.minute = readInt(valIn,ok);	  
      if(ok){operNumb=70;
	  }else{break;}
	case 70: 
	  tempEEPROM.second = readInt(valIn,ok);
	  if(ok){
	  EEPROM.put(this->_memAddress, tempEEPROM);
    EEPROM.put(_memAddress+EEPROM_MEMORY_SHIFT, tempEEPROM);
	  _dataEEPROM = tempEEPROM;
	  operNumb=-1;
	  Serial.println(SERIAL_OUT_OPERATION_FINISHED);
	  }
	  break;
		
    default: 
      operNumb = -2;
      break;
  }
}

void RTC::getRtcInfoEEPROM(int &operNumb){
	switch(operNumb){
    case 0:
		EEPROM.get(this->_memAddress, tempEEPROM);
		Serial.println(tempEEPROM.bitsWrd);
		Serial.println(tempEEPROM.rtcAddress);
		Serial.println(tempEEPROM.year);
		Serial.println(tempEEPROM.month);
		Serial.println(tempEEPROM.day);
		Serial.println(tempEEPROM.hour);
		Serial.println(tempEEPROM.minute);
		Serial.println(tempEEPROM.second);
		operNumb = -1;
		Serial.println(SERIAL_OUT_OPERATION_FINISHED);
		break;
	default: 
		operNumb = -2;
		break;
  }
}

void RTC::setRtcTime(int &operNumb,char *valIn){
  bool ok;
	switch(operNumb){
  case 0:
	  operNumb = 20;
	case 20:  
	  dtTemp.year = readInt(valIn,ok);	  
      if(ok){operNumb=30;
	  }else{break;}
	case 30:  
	  dtTemp.month = readInt(valIn,ok);	  
      if(ok){operNumb=40;
	  }else{break;}
	case 40:  
	  dtTemp.day = readInt(valIn,ok);	  
      if(ok){operNumb=50;
	  }else{break;}
	case 50:  
	  dtTemp.hour = readInt(valIn,ok);	  
      if(ok){operNumb=60;
	  }else{break;}
	case 60:  
	  dtTemp.minute = readInt(valIn,ok);	  
      if(ok){operNumb=70;
	  }else{break;}
	case 70: 
	  dtTemp.second = readInt(valIn,ok);
	  if(ok){
	  clock.setClockMode(false);
    clock.setSecond(dtTemp.second);
    clock.setMinute(dtTemp.minute);
    clock.setHour(dtTemp.hour); 
    clock.setDate(dtTemp.day); 
    clock.setMonth(dtTemp.month); 
    clock.setYear((dtTemp.year>2000)?(dtTemp.year-2000):dtTemp.year); 

	  dt = dtTemp;
	  operNumb=-1;
	  Serial.println(SERIAL_OUT_OPERATION_FINISHED);
	  }
	  break;
		
    default: 
      operNumb = -2;
      break;
  }
}

void RTC::getRtcTime(int &operNumb){  
	switch(operNumb){

    case 0:
    updateDateTime();    
		Serial.println(dt.year);
		Serial.println(dt.month);
		Serial.println(dt.day);
		Serial.println(dt.hour);
		Serial.println(dt.minute);
		Serial.println(dt.second);
		operNumb = -1;
		Serial.println(SERIAL_OUT_OPERATION_FINISHED);
		break;
	default: 
		operNumb = -2;
		break;
    }
}

void RTC::setDefaultMemory(){
  EEPROM.get(_memAddress+EEPROM_MEMORY_SHIFT, tempEEPROM);
  EEPROM.put(this->_memAddress, tempEEPROM);
  _dataEEPROM = tempEEPROM;
}

int16_t RTC::get_I2C_Addr(){
  return _dataEEPROM.rtcAddress;
}

uint8_t RTC::getUsedOuts(){
  uint8_t outWrd = 0;
  if(_dataEEPROM.bitsWrd & enable){
    outWrd |= (_dataEEPROM.bitsWrd >> 1);
  }
  return outWrd;
}

static int RTC::sizeOfEEPROM(){
  return (int)sizeof(RtcEEPROMInfo);
}
