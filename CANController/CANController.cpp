#include "mbed.h"
#include "base.h"
#include "CANController.h"
#include "queue.h"

CAN can1(p9,p10);
CAN can2(p30,p29);

Queue sendQueue1(sizeof(CANMessage), 50);
Queue sendQueue2(sizeof(CANMessage), 50);

void CANSend(CANLine line, CANMessage msg)
{
    switch (line) {
        case CANLine::Main:
            sendQueue1.Put(&msg);
            break;
        case CANLine::Secondary:
            sendQueue2.Put(&msg);
    }
}

void CANProcess()
{
    static CANMessage sendMSG;
    while(sendQueue1.GetNumberOfItems() > 0)
    {
        sendQueue1.Get(&sendMSG);
        can1.write(sendMSG);
    }

    while(sendQueue2.GetNumberOfItems() > 0)
    {
        sendQueue2.Get(&sendMSG);
        can2.write(sendMSG);
    }
}

int CanReceive(CANLine line, CANMessage* msg)
{
     switch (line) {
        case CANLine::Main:
            return can1.read(*msg);
            break;
        case CANLine::Secondary:
            return can2.read(*msg);
    }

    return false;
}