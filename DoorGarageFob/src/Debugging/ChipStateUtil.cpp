#include "Debugging/ChipStateUtil.h"


/// @brief
/// @param chipState 
/// @return 
String chipStateToString(uint8_t chipState)
{
   
    String stateStr ;

    switch ( (chipState >> 4) & 0x0F ) {
        case 0x00: stateStr = "IDLE"; break;
        case 0x01: stateStr = "RX"; break;
        case 0x02: stateStr = "TX"; break;
        case 0x03: stateStr = "FSTXON"; break;
        case 0x04: stateStr = "CALIBRATING"; break;
        case 0x05: stateStr = "SETTLING"; break;
        case 0x06: stateStr = "RX FIFO OVERFLOW"; break;
        case 0x07: stateStr = "TX FIFO UNDERFLOW"; break;
        default:   stateStr = "UNKNOWN"; break;
    }
    return stateStr;
}