#include "device.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


WholeDevice::WholeDevice() {
  valStr[0] = '\0';
}

void WholeDevice::begin() {
  wdt_enable(WDTO_8S);
  pinMode(CHANNEL_1_OUT, OUTPUT);
  pinMode(CHANNEL_2_OUT, OUTPUT);
  pinMode(CHANNEL_3_OUT, OUTPUT);
  pinMode(CHANNEL_4_OUT, OUTPUT);
  pinMode(SAFETY_IN_PIN,    INPUT_PULLUP);
  pinMode(RESET_MEM_BTN_PIN,INPUT_PULLUP);
  pinMode(READ_SERIAL_PERMISSION, INPUT_PULLUP);

  digitalWrite(CHANNEL_1_OUT, false);
  digitalWrite(CHANNEL_2_OUT, false);
  digitalWrite(CHANNEL_3_OUT, false);
  digitalWrite(CHANNEL_4_OUT, false);
  
  beginDevices();  

  EEPROM.get(this->_memAddress, tempEEPROM);

  if (tempEEPROM.delaySleep < 25)tempEEPROM.delaySleep = 25;
  else if (tempEEPROM.delaySleep > 500)tempEEPROM.delaySleep = 500;

  if (!(tempEEPROM.bitsWrd & cockingStrtd)) {
    if (tempEEPROM.distCockingMechTime < 10)tempEEPROM.distCockingMechTime = 10;
    putSlfTrigData(tempEEPROM.selfTriggTime);
    _selfTrigTime = tempEEPROM.selfTriggTime;
    EEPROM.put(OLD_TRIG_EEPROM_ADDR, (uint8_t)0);
    EEPROM.put(TRIGGERED_OUTS_ADDR,  (uint8_t)0);
  }

  power.hardwareDisable(PWR_SPI);
  wdt_disable();
    sleepTmrZeroing();
  wdt_enable(WDTO_8S);
  tmr.setInterval(1000);
  
  EEPROM.put(this->_memAddress, tempEEPROM);
  _dataEEPROM = tempEEPROM;
}

void WholeDevice::beginDevices(){
  uint16_t eepromMemStartAddr;
  EEPROM.get(0, eepromMemStartAddr);

  
  if (eepromMemStartAddr > 256) {
    eepromMemStartAddr = 256;
  } else if (eepromMemStartAddr < START_DATA_EEPROM_ADDR) {
    eepromMemStartAddr = START_DATA_EEPROM_ADDR;
  }
  
  this->_memAddress = eepromMemStartAddr;
  eepromMemStartAddr += WholeDevice::sizeOfEEPROM();
  accBeginSuccess = acc.begin(eepromMemStartAddr);
  eepromMemStartAddr += acc.sizeOfEEPROM();
  rtcBeginSuccess = rtc.begin(eepromMemStartAddr);
  eepromMemStartAddr += rtc.sizeOfEEPROM();
  inch.begin(eepromMemStartAddr);
  eepromMemStartAddr += inch.sizeOfEEPROM();
}

void WholeDevice::setEEPROMaddresses(uint16_t addr) {
  this->_memAddress = addr;
  addr += WholeDevice::sizeOfEEPROM();
  acc.setEEPROMaddress(addr);
  addr += acc.sizeOfEEPROM();
  rtc.setEEPROMaddress(addr);
  addr += rtc.sizeOfEEPROM();
  inch.setEEPROMaddress(addr);
  addr += inch.sizeOfEEPROM();
}


void WholeDevice::sleepTmrZeroing() {

  if (tempEEPROM.bitsWrd & slpTmrCalibrtd) {
    power.calibrate(tempEEPROM.sleepTmrZeroing);
  } else {
    tempEEPROM.sleepTmrZeroing = power.getMaxTimeout();
    power.calibrate(tempEEPROM.sleepTmrZeroing);
    tempEEPROM.bitsWrd |= slpTmrCalibrtd;
  }
  power.correctMillis(true);
  //_dataEEPROM = tempEEPROM;
}

void WholeDevice::mainLoop() {//////////////////////////////MAIN_LOOP/////////////////////////
    
  checkTimer();
  if (funcId == 0) getAlarmStatuses();
  
  const bool _pwrSfty = _dataEEPROM.bitsWrd & pwrSftyMode;
  
  if(_pwrSfty && digitalRead(READ_SERIAL_PERMISSION)){
    power.sleepDelay(_dataEEPROM.delaySleep);//////////////DEEP_SLEEP//////////////////////////
    Wire.begin();
    _selfTrigTime = readSlfTrigData();      
  }else
  checkSerial();

}

void WholeDevice::checkTimer() {
  if (tmr.isReady()) { //every second timer
       
    if(!digitalRead(READ_SERIAL_PERMISSION)){//Can use UART in power safety mode
      if(_tmprdPrm == 0){
        Serial.begin(19200);
        operationNumber = -2;
    //if(accBeginSuccess)acc.restart();
    //if(rtcBeginSuccess)rtc.restart();
        blinkDiode(5,200,127);/////////////////////////////////////////////////
      }
      _tmprdPrm = 1;
    }else _tmprdPrm = 0;

    
    if(funcId != 0) { //reset cmd if it is no finished in 5 sec
      cmdRstCount++;
      if (cmdRstCount > 5) {
        operationNumber = -2;
        cmdRstCount = 0;
      }
    }


    if (!digitalRead(RESET_MEM_BTN_PIN)
        && !digitalRead(SAFETY_IN_PIN)) { //RESET EEPROM values if button pressed during 5 sec
      devRstCount++;
      if (devRstCount > 5) {
        setDefaultMemory();
        devRstCount = 0;
        blinkDiode(3,512,512);
          digitalWrite(CHANNEL_1_OUT, false);
          digitalWrite(CHANNEL_2_OUT, false);
          digitalWrite(CHANNEL_3_OUT, false);
          digitalWrite(CHANNEL_4_OUT, false);
        resetDeviceNow();
      }
    }

    EEPROM.get(this->_memAddress, tempEEPROM);
    _selfTrigTime = readSlfTrigData();
      
    if (!(tempEEPROM.bitsWrd & cockingCompltd)) {
      if (!(tempEEPROM.bitsWrd & cockingStrtd) && digitalRead(SAFETY_IN_PIN))
        tempEEPROM.bitsWrd |= cockingStrtd;

      if ((tempEEPROM.distCockingMechTime > 0)
          && (tempEEPROM.bitsWrd & cockingStrtd)) {
        tempEEPROM.distCockingMechTime--;

      } else if (tempEEPROM.distCockingMechTime <= 0) {
        tempEEPROM.bitsWrd |= cockingCompltd;
      }
    } else if (tempEEPROM.bitsWrd & selftrigEnabled) {
      if (_selfTrigTime > 0) {        
        putSlfTrigData(--_selfTrigTime);
      }
    }

    EEPROM.put(this->_memAddress, tempEEPROM);
    _dataEEPROM = tempEEPROM;
  }
}

bool WholeDevice::wireConnectOk(int16_t addrI2C) {
  bool connectionOk;
  Wire.beginTransmission(addrI2C);
  connectionOk = (Wire.endTransmission() == 0) ? true : false;
  return connectionOk;
}

void WholeDevice::getAlarmStatuses() {

  EEPROM.get(this->_memAddress, this->tempEEPROM);
  if (tempEEPROM.bitsWrd & cockingCompltd) {

    uint8_t accAlStat = 0;
    uint8_t rtcAlStat = 0;
    if (accBeginSuccess) {
      if (wireConnectOk(acc.get_I2C_Addr()))accAlStat = acc.getAlarmStatus();
    }
    if (rtcBeginSuccess) {
      if (wireConnectOk(rtc.get_I2C_Addr()))rtcAlStat = rtc.getAlarmStatus();
    }

    tempEEPROM.chnlsAccTrig   |= accAlStat;
    tempEEPROM.chnlsRtcTrig   |= rtcAlStat;
    tempEEPROM.chnlsInTrig    |= inch.getAlarmStatus();
    tempEEPROM.chnlsSelfTrig  |= getAlarmStatus();

    tempEEPROM.chnlsTotalTrig |= tempEEPROM.chnlsAccTrig
                                 | tempEEPROM.chnlsRtcTrig
                                 | tempEEPROM.chnlsInTrig
                                 | tempEEPROM.chnlsSelfTrig
                                 ;
    if (tempEEPROM.chnlsTotalTrig & 1)
      tempEEPROM.chnlsTotalTrig = getUsedOuts();
    EEPROM.put(this->_memAddress, tempEEPROM);
    _dataEEPROM = tempEEPROM;
    setOutPins();
  }
}

uint8_t WholeDevice::getAlarmStatus() {
  if ((_dataEEPROM.bitsWrd & selftrigEnabled)
      && (_selfTrigTime <= 0)) {
    return getUsedOuts();//0b00001111;
  } else {
    return 0;
  }
}

void WholeDevice::setOutPins() {
  uint8_t trigOld;
  uint8_t trig = _dataEEPROM.chnlsTotalTrig;
  uint8_t triggered;
  EEPROM.get(OLD_TRIG_EEPROM_ADDR, trigOld);
  EEPROM.get(TRIGGERED_OUTS_ADDR, triggered);

  if(trig == 0){
    trigOld = 0;
    triggered = 0;
  }else if (trig != trigOld){
    if (trig == 0) trigOld = 0;
    else{
      if (trig & 0b1110 & (~triggered)){
        setPinsOutNow(trig & 0b1110 & (~triggered));
        triggered = trig;
        delay(TRIGGERING_TIME);
      }
      if (trig & 1){
        setPinsOutNow(1);
        delay(TRIGGERING_TIME);
      }
    }
  }else{
    setPinsOutNow(trig);
    /*if ((_dataEEPROM.bitsWrd & pwrSftyMode) 
        && (trig & 1)
        && digitalRead(READ_SERIAL_PERMISSION)) 
        delay(TRIGGERING_TIME);*/
  }

  trigOld = trig;
  EEPROM.put(TRIGGERED_OUTS_ADDR, triggered);
  EEPROM.put(OLD_TRIG_EEPROM_ADDR, trigOld);
}

void WholeDevice::setPinsOutNow(uint8_t in) {
  if (in & 0b00000001) {
    digitalWrite(CHANNEL_1_OUT, true);
  }
  if (in & 0b00000010) {
    digitalWrite(CHANNEL_2_OUT, true);
  }
  if (in & 0b00000100) {
    digitalWrite(CHANNEL_3_OUT, true);
  }
  if (in & 0b00001000) {
    digitalWrite(CHANNEL_4_OUT, true);
  }
}

void WholeDevice::checkFunc() {
  switch (funcId) {
    case SERIAL_IN_GET_ALL_INFO:                                                          break;
    case SERIAL_IN_SET_ALL_INFO:                                                          break;
    case SERIAL_IN_GET_I2C_ADDRESSES:     get_I2C_Addresses();                            break;
    case SERIAL_IN_GET_DEV_INFO:          getDevInfoEEPROM();                             break;
    case SERIAL_IN_GET_DEV_ZERO_EEPROM:   getZeroEEPROMAddr();                            break;
    case SERIAL_IN_GET_DEV_COCK_TIME_LFT: getCockTimeLeft();                              break;
    case SERIAL_IN_GET_DEV_TRIG_TIME_LFT: getTrigTimeLeft();                              break;
    case SERIAL_IN_GET_DEV_TRIG_STATES:   getTrigStates();                                break;
    case SERIAL_IN_SET_DEV_INFO:          setDevInfoEEPROM();                             break;
    case SERIAL_IN_RESET_DEV_TRIGG:       resetTriggers();                                break;
    case SERIAL_IN_SET_DEV_ZERO_EEPROM:   setZeroEEPROMAddr();                            break;
    case SERIAL_IN_SET_DEV_TRIGG:         setTrigger();                                   break;
    case SERIAL_IN_RESET_DEV_TOTALY:      resetDevice();                                  break;

    case SERIAL_IN_GET_ACC_INFO:          acc.getAccInfoEEPROM(operationNumber);          break;
    case SERIAL_IN_GET_ACC_VALUE:         acc.printInfoSerial(operationNumber);           break;
    case SERIAL_IN_SET_ACC_INFO:          acc.setAccInfoEEPROM(operationNumber, valStr);   break;
    case SERIAL_IN_SET_ACC_ZERO_SP_INFO:  acc.set_XYZ_PC_EEPROM(operationNumber, valStr);  break;

    case SERIAL_IN_GET_RTC_INFO:          rtc.getRtcInfoEEPROM(operationNumber);          break;
    case SERIAL_IN_GET_RTC_TIME:          rtc.getRtcTime(operationNumber);                break;
    case SERIAL_IN_SET_RTC_INFO:          rtc.setRtcInfoEEPROM(operationNumber, valStr);   break;
    case SERIAL_IN_SET_RTC_CURR_TIME:     rtc.setRtcTime(operationNumber, valStr);         break;
    case SERIAL_IN_SET_RTC_MANU_TIME:     rtc.setRtcTime(operationNumber, valStr);         break;

    case SERIAL_IN_GET_INCH_INFO:         inch.getIOInfoEEPROM(operationNumber);          break;
    case SERIAL_IN_SET_INCH_INFO:         inch.setIOInfoEEPROM(operationNumber, valStr);   break;
    default: break;
  }
}

void WholeDevice::checkSerial() {

  bool dataReceived = false;
  SerialRead::readLn(valStr, dataReceived);
  if (dataReceived) {
    if (strcmp(valStr, SERIAL_IN_HELLO) == 0) {
      funcId = 0;
      operationNumber = 0;
      Serial.println(SERIAL_OUT_HELLO);
      connectionOk = true;
    } else if (strcmp(valStr, SERIAL_IN_GOODBYE) == 0) {
      funcId = 0;
      operationNumber = 0;
      connectionOk = false;
    } else if (valStr[0] == '&' && connectionOk) {
      funcId = SerialRead::readCmd(valStr, dataReceived);
      if (dataReceived) {
        operationNumber = 0;
        Serial.println(SERIAL_OUT_START_OPERATION);
        checkFunc();
      }
    } else if (funcId != 0 && connectionOk) {
      checkFunc();
    }
  }

  if (operationNumber == -1) { //operation ok
    funcId = 0;
    operationNumber = 0;
  } else if (operationNumber == -2) { //operation failed
    if (Serial.available() > 0) Serial.readString(); //clear buffer
    funcId = 0;
    operationNumber = 0;
  }
}

void WholeDevice::resetDevice() {
  switch (operationNumber) {
    case 0:
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      delay(45);
      resetDeviceNow();
      break;
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::resetDeviceNow() {
  void(* resetFunc) (void) = 0;
  resetFunc();
}

void WholeDevice::setDefaultMemory() {
  EEPROM.get(_memAddress + EEPROM_MEMORY_SHIFT, tempEEPROM);
  EEPROM.put(this->_memAddress, tempEEPROM);
  _dataEEPROM = tempEEPROM;
  acc.setDefaultMemory();
  rtc.setDefaultMemory();
  inch.setDefaultMemory();
}

void WholeDevice::refreshData() {
  acc.refreshData();
}



void WholeDevice::setZeroEEPROMAddr() {
  bool ok;
  uint16_t addr;
  switch (operationNumber) {
    case 0:
      addr = readInt(valStr, ok);
      if (ok) {
        if (addr > 250) {
          addr = 250;
        } else if (addr < 3) {
          addr = 3;
        }
        EEPROM.put(0, addr);
        setEEPROMaddresses(addr);
        operationNumber = -1;
        Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      }
      break;
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::getZeroEEPROMAddr() {
  uint16_t addr;
  switch (operationNumber) {
    case 0:
      EEPROM.get(0, addr);
      Serial.println(addr);
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::get_I2C_Addresses() {
  for (int16_t address = 9; address < 120; address++ )
  {
    Wire.beginTransmission(address);
    int error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.println(address);
    }
    //delay(1);
  }
  Serial.println(SERIAL_OUT_OPERATION_FINISHED);
  operationNumber = -1;
}

void WholeDevice::setDevInfoEEPROM() {
  bool ok;
  switch (operationNumber) {
    case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);
      tempEEPROM.programVersion = 1;
      tempEEPROM.bitsWrd = 0;
      tempEEPROM.bitsWrd = readInt(valStr, ok);
      if (ok) {
        operationNumber = 5;
      }
      else {
        break;
      }
    case 5:
      tempEEPROM.delaySleep = readInt(valStr, ok);
      if (ok) {
        operationNumber = 10;
      }
      else {
        break;
      }
    case 10:
      tempEEPROM.distCockingMechTime = readInt(valStr, ok);
      if (ok) {
        if (tempEEPROM.distCockingMechTime < 10) {
          tempEEPROM.distCockingMechTime = 10;/////////////////////////////////////////////
        }
        operationNumber = 20;
      } else {
        break;
      }
    case 20:
      tempEEPROM.selfTriggTime = readLInt(valStr, ok);
      if (ok) {
        clearSlfTrigData();
        putSlfTrigData(tempEEPROM.selfTriggTime);
        EEPROM.put(this->_memAddress, tempEEPROM);
        EEPROM.put(_memAddress + EEPROM_MEMORY_SHIFT, tempEEPROM);
        _selfTrigTime = tempEEPROM.selfTriggTime;
        _dataEEPROM = tempEEPROM;
        operationNumber++;
        ok = false;
        operationNumber = -1;
        Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      }
      break;

    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::getDevInfoEEPROM() {
  switch (operationNumber) {
    case 0:
      EEPROM.get(_memAddress, tempEEPROM);

      Serial.println(tempEEPROM.programVersion);
      Serial.println(tempEEPROM.bitsWrd);
      Serial.println(tempEEPROM.sleepTmrZeroing);
      Serial.println(tempEEPROM.delaySleep);
      Serial.println(tempEEPROM.distCockingMechTime);
      Serial.println(_selfTrigTime);
      Serial.println(tempEEPROM.chnlsTotalTrig);
      Serial.println(tempEEPROM.chnlsInTrig);
      Serial.println(tempEEPROM.chnlsSelfTrig);
      Serial.println(tempEEPROM.chnlsAccTrig);
      Serial.println(tempEEPROM.chnlsRtcTrig);
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);

    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::getCockTimeLeft() {
  switch (operationNumber) {
    case 0:
      EEPROM.get(_memAddress, tempEEPROM);
      Serial.println(tempEEPROM.distCockingMechTime);
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::getTrigTimeLeft() {
  switch (operationNumber) {
    case 0:
      //EEPROM.get(_memAddress, tempEEPROM);
      Serial.println(_selfTrigTime);
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::getTrigStates() {
  switch (operationNumber) {
    case 0:
      EEPROM.get(_memAddress, tempEEPROM);
      Serial.println(tempEEPROM.chnlsTotalTrig);
      Serial.println(tempEEPROM.chnlsInTrig);
      Serial.println(tempEEPROM.chnlsSelfTrig);
      Serial.println(tempEEPROM.chnlsAccTrig);
      Serial.println(tempEEPROM.chnlsRtcTrig);
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::resetTriggers() {
  switch (operationNumber) {
    case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);
      tempEEPROM.chnlsTotalTrig = 0;
      tempEEPROM.chnlsInTrig = 0;
      tempEEPROM.chnlsSelfTrig = 0;
      tempEEPROM.chnlsAccTrig = 0;
      tempEEPROM.chnlsRtcTrig = 0;
      EEPROM.put(this->_memAddress, tempEEPROM);
      EEPROM.put(_memAddress + EEPROM_MEMORY_SHIFT, tempEEPROM);
      _dataEEPROM = tempEEPROM;
      operationNumber = -1;
      Serial.println(SERIAL_OUT_OPERATION_FINISHED);
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::setTrigger() {
  bool ok;
  switch (operationNumber) {
    case 0:
      EEPROM.get(this->_memAddress, tempEEPROM);
      tempEEPROM.chnlsTotalTrig = readInt(valStr, ok);
      if (ok) {
        EEPROM.put(this->_memAddress, tempEEPROM);
        _dataEEPROM = tempEEPROM;
        operationNumber = -1;
        Serial.println(SERIAL_OUT_OPERATION_FINISHED);
      } else {
        break;
      }
    default:
      operationNumber = -2;
      break;
  }
}

void WholeDevice::blinkDiode(const uint8_t _nbrs, const int _glowsMs, const int _fadeMs) {
  for (int i = 0; i < _nbrs; i++) {
    if(i>0)delay(_fadeMs);
    digitalWrite(13, HIGH);
    delay(_glowsMs);
    digitalWrite(13, LOW);
  }
}

void WholeDevice::clearSlfTrigData(){
  clrDataEEPROM(SELFTRIG_LEFT_ADDR, EEPROM_MEMORY_SIZE);
}

void WholeDevice::putSlfTrigData(int32_t const _in){
  putDataMultiEEPROM(SELFTRIG_LEFT_ADDR, EEPROM_MEMORY_SIZE, _in);
}


int32_t WholeDevice::readSlfTrigData(){
  int32_t _temp;
  readDataMultiEEPROM(SELFTRIG_LEFT_ADDR, EEPROM_MEMORY_SIZE, _temp);
  return _temp;
}

uint8_t WholeDevice::getUsedOuts() {
  uint8_t outWrd = 1;
  uint8_t uo1 = 0, uo2 = 0, uo3 = 0;
  uo1 = acc.getUsedOuts();
  uo2 = rtc.getUsedOuts();
  uo3 = inch.getUsedOuts();
  outWrd |= uo1
            |  uo2
            |  uo3
            ;
  return outWrd;
}

static int WholeDevice::sizeOfEEPROM() {
  return (int)sizeof(DeviceEEPROMInfo);
}
