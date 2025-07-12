#pragma once
#include<Arduino.h>
#if DEBUG
   #define LOG(message) do { Serial.print(F(message)); } while(0)
   #define LOG_NEW_LINE(message) do { Serial.println(F(message)); } while(0)
   #define LOG_DYNAMIC(message) do { Serial.println(message); } while(0)
   #define LOG_PAIR_DEC(name, val) do { Serial.print(F(name ": ")); Serial.println(val, DEC); } while(0)
   #define LOG_PAIR_HEX(name, val) do { Serial.print(F(name ": 0x")); Serial.println(val, HEX); } while(0)
   #define LOG_PAIR_BIN(name, val) do { Serial.print(F(name ": 0b")); Serial.println(val, BIN); } while(0)
   #define NEW_LINE() do { Serial.println(); } while(0)
#else
   #define LOG(message)
   #define LOG_NEW_LINE(message)
   #define LOG_DYNAMIC(message)
   #define LOG_PAIR_DEC(name, val)
   #define LOG_PAIR_HEX(name, val)
#endif