#include "serialread.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif


static int SerialRead::readInt(char *str, bool &ok){
  ok = false;
  if(strlen(str)>0){ 
    ok = true; 
    for(int i = 0; i < strlen(str); i++){
    char b = str[i];
      if(b!='0' && b!='1' && b!='2' &&
        b!='3' && b!='4' && b!='5' &&
        b!='6' && b!='7' && b!='8' &&
        b!='9' && b!='-'){ok = false; break;}
    }
  }else{ok = false;}
  int k = atoi(str);
  *str = '\0';
  return k;
}

static int32_t SerialRead::readLInt(char *str, bool &ok){
  ok = false;
  if(strlen(str)>0){ 
    ok = true; 
    for(int i = 0; i < strlen(str); i++){
    char b = str[i];
      if(b!='0' && b!='1' && b!='2' &&
        b!='3' && b!='4' && b!='5' &&
        b!='6' && b!='7' && b!='8' &&
        b!='9' && b!='-'){ok = false; break;}
    }
  }else{ok = false;}
  int32_t k = atol(str);
  *str = '\0';
  return k;
}

static float SerialRead::readFloat(char *str, bool &ok){
  ok = true;
  if(strlen(str)>0){
  for(int i = 0; i < strlen(str); i++){
    char b = str[i];
      if(b!='0' && b!='1' && b!='2' &&
        b!='3' && b!='4' && b!='5' &&
        b!='6' && b!='7' && b!='8' &&
        b!='9' && b!='.' && b!='-' && 
        b!='E' && b!='e'){ok = false; break;}
  }
  }else{ok = false;}
  float k = atof(str);
  *str = '\0';
  return k;
}


static void SerialRead::readLn(char *str, bool &ok){
  ok = false;
  if (Serial.available() > 0) {     
      int strLen = Serial.readBytesUntil('\n',str,64);
      if (strLen > 0) {
        str[strLen] = '\0';
        ok = true;
      }else {
          str[0] = '\0';
          ok = false;
      }
  } 
}


static int SerialRead::readCmd(char *str, bool &ok){
  ok = false;
  if(str[0]=='&'){
    ok = true;
    for(int i = 1; i < strlen(str); i++){
      char b = str[i];
      if (b!='0' && b!='1' && b!='2' &&
        b!='3' && b!='4' && b!='5' &&
        b!='6' && b!='7' && b!='8' &&
        b!='9' && b!='-'){ok = false; break;}
    }
  }
  int k = atoi(&str[1]);
  *str = '\0';
  return k;
}
  
