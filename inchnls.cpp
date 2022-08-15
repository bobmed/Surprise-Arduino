#include "inchnls.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif


void InChannels::begin(uint16_t addr){
  setEEPROMaddress(addr);
  EEPROM.get(this->_memAddress, tempEEPROM);
  _dataEEPROM = tempEEPROM;
  
  pinMode(CHANNEL_1_IN, INPUT_PULLUP);
  pinMode(CHANNEL_2_IN, INPUT_PULLUP);
  pinMode(CHANNEL_3_IN, INPUT_PULLUP);
  pinMode(CHANNEL_4_IN, INPUT_PULLUP);
    
}

void InChannels::setEEPROMaddress(uint16_t addr){
  this->_memAddress = addr;
}


uint8_t InChannels::getAlarmStatus(){
	uint8_t out = 0;
  for(int i=0; i<4; i++){
	if(_dataEEPROM.bitsWrd[i] & chnl_on){
		bool status = false;
		bool signal = (_dataEEPROM.bitsWrd[i] 
						& chnl_nc);
		switch(i){
			case 0: 
				status = (digitalRead(CHANNEL_1_IN)==signal); 
				break;
			case 1:  
				status = (digitalRead(CHANNEL_2_IN)==signal); 
				break;
			case 2:  
				status = (digitalRead(CHANNEL_3_IN)==signal); 
				break;
			case 3:  
				status = (digitalRead(CHANNEL_4_IN)==signal); 
				break;
			default: break;
		}
		if(status) out |= _dataEEPROM.bitsWrd[i] >> 2;
	}
  }
  return out;
}


void InChannels::setIOInfoEEPROM(int &operNumb,char *valIn){
  bool ok;
  switch(operNumb){
  case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);   
      //tempEEPROM.bitsWrd = 0;
      tempEEPROM.bitsWrd[0] = readInt(valIn,ok);	  
      if(ok){operNumb=10;
	  }else{break;}
	case 10: 
	  tempEEPROM.bitsWrd[1] = readInt(valIn,ok);	  
      if(ok){operNumb=20;
	  }else{break;}		
	case 20:  
	  tempEEPROM.bitsWrd[2] = readInt(valIn,ok);	  
      if(ok){operNumb=30;
	  }else{break;}
	case 30: 
	  tempEEPROM.bitsWrd[3] = readInt(valIn,ok);
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

void InChannels::getIOInfoEEPROM(int &operNumb){
	switch(operNumb){
    case 0:
		EEPROM.get(this->_memAddress, tempEEPROM);
		Serial.println(tempEEPROM.bitsWrd[0]);
		Serial.println(tempEEPROM.bitsWrd[1]);
		Serial.println(tempEEPROM.bitsWrd[2]);
		Serial.println(tempEEPROM.bitsWrd[3]);
		operNumb = -1;
		Serial.println(SERIAL_OUT_OPERATION_FINISHED);
		break;
	default: 
		operNumb = -2;
		break;
  }
}


void InChannels::setDefaultMemory(){
  EEPROM.get(_memAddress+EEPROM_MEMORY_SHIFT, tempEEPROM);
  EEPROM.put(this->_memAddress, tempEEPROM);
  _dataEEPROM = tempEEPROM;
}

uint8_t InChannels::getUsedOuts(){
  uint8_t outWrd = 0;
  for(int i = 0; i< 4 ; i++){
    if(_dataEEPROM.bitsWrd[i]&chnl_on){ 
      outWrd |= (_dataEEPROM.bitsWrd[i] >> 2);
    }
  }
  return outWrd;
}

static int InChannels::sizeOfEEPROM(){
  return (int)sizeof(InChEEPROMInfo);
}
