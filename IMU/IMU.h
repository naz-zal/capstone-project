#ifndef IMU_H
#define IMU_H

#include "mbed.h"

struct IMUData
{
    int Status;
    float xRate;
    float yRate;
    float zRate;
    float xAccel;
    float yAccel;
    float zAccel;
    float BoardTemp;

    int ProgramTime;
};

class IMU_Control {
    public:

        uint32_t CurrentTime;
        bool IWantToPrint = true;
        int PrintTime_ms = 0;
        // Pass = new DigitalOut(pIL);
        // do_indexLeft = new SPI(Pin1,Pin2,Pin3);

        void IMU_initialize(SPI* boardSPI);
        void Run();


        short FlipShort(short input);
        void PrintIMUData(IMUData data);
        void Convert(bool IWantToPrint = false); 
        bool NewDataAvailable();
        IMUData GetData();
        void UpdatedIMUData();
        void IMUNoise(int reset);

    private:
        SPI* PassedSPI; //Reference to boardSPI
};


#endif // IMU_H