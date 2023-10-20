#include "IMU.h"
#include "base.h"
#include "mbed.h"

DigitalOut IMU_CS(p18, 1); //chip seelect
DigitalOut IMU_NRST(p8, 0); //reset pin - might need  to stay down - or maybe it's up
DigitalIn  IMU_DataReady(p19); //data ready

bool initialized = false;

static short STORE_IMU_BURST[8];
static float IMU_NOISE_REDUCED[8];
static float STORE_IMU_CONVERTED[8];


static IMUData lastDataReceived;
static int lastDataReceivedTime;
bool newDataReceieved = false;

void IMU_Control::IMU_initialize(SPI* boardSPI){
    
    //Assign SPI
    PassedSPI = boardSPI;
    
    //Turn on IMU;
    IMU_NRST = 1;

    initialized = true;
}

IMUData IMU_Control::GetData()
{
    IMUData newData;

    newData.Status = lastDataReceived.Status;
    newData.xRate = lastDataReceived.xRate;
    newData.yRate = lastDataReceived.yRate;
    newData.zRate = lastDataReceived.zRate;
    newData.xAccel = lastDataReceived.xAccel;
    newData.yAccel = lastDataReceived.yAccel;
    newData.zAccel = lastDataReceived.zAccel;
    newData.BoardTemp = lastDataReceived.BoardTemp;
    newData.ProgramTime = lastDataReceived.ProgramTime;

    newDataReceieved = false;
    return newData;
}

void IMU_Control::IMUNoise(int reset){

    if(reset==1)
    {
        for(int i=0; i<9; i++)
        {
            IMU_NOISE_REDUCED[i]=0;
        }
    }
    else
    {
        for(int i=0; i<9; i++)
        {
                if(i==0)
                {
                    IMU_NOISE_REDUCED[i]=STORE_IMU_CONVERTED[i];
                }
                else if (i <8)
                {
                    IMU_NOISE_REDUCED[i]=(STORE_IMU_CONVERTED[i]+IMU_NOISE_REDUCED[i])/2;
                }
        }
    }
}

bool IMU_Control::NewDataAvailable()
{
    return newDataReceieved;
}

void IMU_Control::PrintIMUData(IMUData data)
{
    Base_WriteSerial("$IMU_DATA,%d,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f\r\n", 
                                    data.Status, data.xRate, data.yRate, data.zRate, 
                                    data.xAccel, data.yAccel, data.zAccel, data.BoardTemp);
}

void IMU_Control::Convert(bool IWantToPrint){

    static IMUData newData;

    newData.Status = STORE_IMU_BURST[0];

    newData.xRate = (float) STORE_IMU_BURST[1]/64.0f;
    newData.yRate = (float) STORE_IMU_BURST[2]/64.0f;
    newData.zRate = (float) STORE_IMU_BURST[3]/64.0f;

    newData.xAccel = (float) STORE_IMU_BURST[4]/4000.0f;
    newData.yAccel = (float) STORE_IMU_BURST[5]/4000.0f;
    newData.zAccel = (float) STORE_IMU_BURST[6]/4000.0f;

    newData.BoardTemp = (STORE_IMU_BURST[7] * 0.073111172849435) + 31.0;

    newData.ProgramTime = Base_GetTime_ms();

    lastDataReceived = newData;
    newDataReceieved = true;

    //Printing of Data
    if(IWantToPrint)
    {
        PrintIMUData(newData);
    }
}

void IMU_Control::Run()
{
    //Check for new Data
    UpdatedIMUData();
}

short IMU_Control::FlipShort(short input)
{
    return input;
    short output = 0;
    output = (( input & 0xFF00 ) >> 8 ) | ((input & 0x00FF ) << 8);
    return output;
}

void IMU_Control::UpdatedIMUData()
{
    static int lastState = 0;
    static int lastTime = 0;

    int currentState = IMU_DataReady.read();
    if(currentState != lastState)
    {
        if(currentState == 0)
        {
            PassedSPI->lock();

            IMU_CS.write(0);
            
            short Write = (short) PassedSPI->write(0x3E00); // start burst mode
            wait_us(15);

            short Status = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[0]= FlipShort(Status);
            wait_us(15);

            short xRate = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[1]= FlipShort(xRate);
            wait_us(15);

            short yRate = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[2] = FlipShort(yRate);
            wait_us(15);

            short zRate = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[3] = FlipShort(zRate);
            wait_us(15);

            short x_acc = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[4] = FlipShort(x_acc);
            wait_us(15);

            short y_acc = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[5] = FlipShort(y_acc);
            wait_us(15);

            short z_acc = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[6] = FlipShort(z_acc);
            wait_us(15);

            short Board_temp = (short) PassedSPI->write(0x0000);
            STORE_IMU_BURST[7] = FlipShort(Board_temp);
            wait_us(15);

            IMU_CS.write(1);
            PassedSPI->unlock();

            Convert(false);
            //Storing the IMU data and the current time 
            //IMU_Control::ConvertAndPrint(IWantToPrint,PrintTime_ms);
        }


        lastState = currentState;
    }
}