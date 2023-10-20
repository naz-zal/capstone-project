#ifndef CANCONTROLLER_H
#define CANCONTROLLER_H

#include "mbed.h"

enum class CANLine
{
    Main = 0,
    Secondary = 1
};

int CanReceive(CANLine line, CANMessage* msg);
void CANProcess();
void CANSend(CANLine line, CANMessage msg);


#endif