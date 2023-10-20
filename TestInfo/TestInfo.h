#ifndef TESTINFO_H
#define TESTINFO_H

#include "mbed.h"
#include <cstdint>

struct TestStep
{
    float SteerValue = 0.0f;
    uint32_t durationMS = 0;
    bool stopWithWheelPercentage = false;
    float wheelPercentage = 0.0;
};

struct TestInfoBasic
{
    int TestID = -1;
    char TestName[32];
    float TestSpeed = 0.0;
};

struct TestInfoFull
{
    TestInfoBasic info;
    TestStep steps[128];
    int numberOfSteps = 0;
};

#endif // TESTINFO_H