#include "mbed.h"
#include "base.h"
#include "IMU.h"
#include "main.h"
#include "Screen.h"
#include "CANController.h"
#include "ButtonManager.h"
#include "TestInfo.h"
#include "Analog.h"
#include <cstdint>
#include <cstdio>
#include "SDCardManager.h"

#define SPI_FREQ 4000000

const int TestMaxCount = 4;
TestInfoFull FullTests[TestMaxCount];
int FullTestCount = 0;

bool kickout = false;

bool Start = false;
int CurrentTest = -1;
int CurrentStep = -1;
int CurrentStepTime = 0;

bool MAIN_IMU_DATA_VALID = false;
bool MAIN_GPS_DATA_VALID = false;
bool MAIN_CAN_DATA_VALID = false;
bool MAIN_ANGLE_DATA_VALID = false;

SDCard SD("sd", p5, p6, p7, p20, p17, SPI_FREQ);
SPI mainSPI(p5, p6, p7); //SPI to control IMU/SD Card

Screen screen;
IMU_Control IMU;

AnalogIn wheelAngle(p16);
AnalogIn transducer(p15);

DigitalIn quadA(p11, PullUp);
DigitalIn quadB(p12, PullUp);

Main_State currentState = Main_State::Initializing;
Testing_State testingState = Testing_State::Waiting;

int LastGPSTime = 0;
int LastAngleTime = 0;
int LastIMUTime = 0;
int LastCANTime = 0;

uint32_t lastTransducerValue = 0;
uint32_t lastWheelAngleValue = 0;

bool wheelAngleUpdated = false;

int lastQuadA = 0;
int lastQuadB = 0;

int quadAChanges = 0;
int quadBChanges = 0;

void CANDispatch(CANLine line, CANMessage msg)
{
    int time = Base_GetTime_ms();
    Base_WriteSerialFile(true,"$CAN,%d,0x%08X,%d,%d,%d\r\n", line, msg.id, msg.len, msg.data[2], time);
    LastCANTime = time;
    MAIN_CAN_DATA_VALID = true;
}

//Called when NMEA data recieved
void NMEASerialDispatch(char* line)
{
    int time = Base_GetTime_ms();
    Base_WriteSerialFile(true,"%s,%d\r\n", line, time);
    LastGPSTime = time;
    MAIN_GPS_DATA_VALID = true;
}

void StartMenu()
{
    screen.SetState(Screen_State::MainMenu);
}

void StartTest()
{
    testingState = Testing_State::Running;
    screen.SetState(Screen_State::RunningTest);
}

void TestActivated(TestInfoBasic testInfo)
{
    //Test Has been activated, save for next thing
    testingState = Testing_State::Select;
    CurrentTest = testInfo.TestID;
    CurrentStep = -1;
    CurrentStepTime = Base_GetTime_ms();
    Start = true;
}

void MainInitialize()
{
    BaseInit();
    SD.Init();

    //Startup
    //Initialize SPI
    mainSPI.format(16, 3); //16 bits
    mainSPI.frequency((int) SPI_FREQ);

    //Initialize IMU
    IMU.IMU_initialize(&mainSPI);
    
    Analog::SetPWMFrequency();
}

void RunTest()
{
    //TESTS START
    static float deadBandLeft = -0.28;
    static float deadBandRight = 0.28;
    static uint32_t leftWheel = 13000;
    static uint32_t rightWheel = 35000;

    static uint32_t totalRange = rightWheel - leftWheel;
    static uint32_t halfRange = leftWheel + (totalRange / 2.0f);
    static bool newStep = true;

    if(CurrentTest >= 0)
    {
        if(CurrentStep < 0)
        {
            CurrentStep = 0;
            Base_WriteSerial("NEWSTEP-START\r\n");
            newStep = true;
        }

        if(CurrentStep >= 0)
        {
            TestStep step = FullTests[CurrentTest].steps[CurrentStep];
            int currentTime = Base_GetTime_ms();

            if(step.stopWithWheelPercentage)
            {
                if(step.wheelPercentage >= 0)
                {
                    if(lastWheelAngleValue >=  halfRange + ((rightWheel - halfRange) * step.wheelPercentage))
                    {
                        Base_WriteSerial("NEWSTEP-POSITIVE\r\n");
                        newStep = true;
                    }
                }
                else if(step.wheelPercentage < 0)
                {
                    if(lastWheelAngleValue <=  leftWheel + ((halfRange - leftWheel) * step.wheelPercentage))
                    {
                        Base_WriteSerial("NEWSTEP-NEGATIVE\r\n");
                        newStep = true;
                    }
                }
            }

            if(currentTime - CurrentStepTime >= step.durationMS)
            {
                Base_WriteSerial("NEWSTEP-TIME\r\n");
                newStep = true;
            }
        }

        if(newStep)
        {
            CurrentStep++;
            newStep = false;
            screen.TestInfoStatus(CurrentStep, FullTests[CurrentTest].numberOfSteps);

            if(CurrentStep >= FullTests[CurrentTest].numberOfSteps)
            {
                Analog::SetSteer(Analog::Analog_type::Type2, 0.0f);
                
                Base_WriteSerialFile(true,"$VNEW,%0.2f,%0.2f,%d\r\n", 0.0f, Base_GetTime_ms());
                Base_WriteSerialFile(true,"$INFO,TEST ENDED,%d\r\n", Base_GetTime_ms());
            
                testingState = Testing_State::Finished;
            }
            else 
            {
                TestStep step = FullTests[CurrentTest].steps[CurrentStep];

                Base_WriteSerial("$NEWSTEP %d, %d,%f,%d,%d,%f\r\n",CurrentTest,CurrentStep, step.SteerValue, step.durationMS, step.stopWithWheelPercentage, step.wheelPercentage);

                Analog::SetSteer(Analog::Analog_type::Type2, step.SteerValue);
                CurrentStepTime = Base_GetTime_ms();
                Base_WriteSerialFile(true,"$VNEW,%0.2f,%0.2f,%d\r\n", step.SteerValue, CurrentStepTime);
            }
        }
        
    }
}

int main() 
{    //Add Tests
    //Left DeadBand Test
    TestInfoBasic leftTest
    {
        .TestID = FullTestCount,
        .TestSpeed = 0.0
    };
    Base_strcpy(leftTest.TestName, "Left DeadBand");
    FullTests[FullTestCount].info = leftTest;

    float testValue = 0.0f;
    for(int i = 0; i < 100; i++)
    {
        FullTests[FullTestCount].steps[i].durationMS = 1000;
        FullTests[FullTestCount].steps[i].SteerValue = testValue;
        FullTests[FullTestCount].steps[i].stopWithWheelPercentage = false;
        FullTests[FullTestCount].steps[i].wheelPercentage = -0.95f; //Stop at 95% left
        
        if(testValue >= -0.10)
            testValue -= 0.05;
        else if(testValue >= -0.20)
            testValue -= 0.025;
        else
            testValue -= 0.01;

        if(testValue < -1.0f)
        {
            FullTests[FullTestCount].numberOfSteps = i + 1;
            Base_WriteSerial("LEFT NUM STEPS %d\r\n", FullTests[FullTestCount].numberOfSteps);
            break;
        }

        testValue = Base_BoundFloat(testValue, -1.0f, 1.0f);
    }
    FullTestCount += 1;
    screen.AddTest(leftTest);
    //End LeftBandTest

    //Right DeadBand Test
    TestInfoBasic rightTest
    {
        .TestID = FullTestCount,
        .TestSpeed = 0.0
    };
    Base_strcpy(rightTest.TestName, "Right DeadBand");
    FullTests[FullTestCount].info = rightTest;

    testValue = 0.0f;
    for(int i = 0; i < 100; i++)
    {
        FullTests[FullTestCount].steps[i].durationMS = 1000;
        FullTests[FullTestCount].steps[i].SteerValue = testValue;
        FullTests[FullTestCount].steps[i].stopWithWheelPercentage = false;
        FullTests[FullTestCount].steps[i].wheelPercentage = 0.95f; //Stop at 95% right
        
        if(testValue <= 0.10)
            testValue += 0.05;
        else if(testValue <= 0.20)
            testValue += 0.025;
        else 
            testValue += 0.01;

        if(testValue > 1.0f)
        {
            Base_WriteSerial("RIGHT NUM STEPS %d\r\n", FullTests[FullTestCount].numberOfSteps);
            FullTests[FullTestCount].numberOfSteps = i + 1;
            break;
        }

        testValue = Base_BoundFloat(testValue, -1.0f, 1.0f);
    }
    FullTestCount += 1;
    screen.AddTest(rightTest);
    //End RightBandTest

    //Left And Right Test
    TestInfoBasic wheelRateTest
    {
        .TestID = FullTestCount,
        .TestSpeed = 0.0
    };
    Base_strcpy(wheelRateTest.TestName, "Wheel Rate Test");
    wheelRateTest.TestSpeed = 0.0;
    FullTests[FullTestCount].info = wheelRateTest;

    testValue = 0.28f;
    int stepCounter = 0;

    //Start by moving wheels fully to the right
    FullTests[FullTestCount].steps[stepCounter].durationMS = 5000;
    FullTests[FullTestCount].steps[stepCounter].SteerValue = 0.5f;
    FullTests[FullTestCount].steps[stepCounter].stopWithWheelPercentage = false;
    FullTests[FullTestCount].steps[stepCounter].SteerValue = 0.95f; //Stop at 95% right
    stepCounter++;
    float direction = -1.0f; //-1 = left, -1 = right.

    for(int i = 0; i < 100; i++)
    {
        testValue += 0.01f;

        if(testValue > 1.0f)
        {
            FullTests[FullTestCount].numberOfSteps = stepCounter;
            break;
        }

        //Left/Right
        FullTests[FullTestCount].steps[stepCounter].durationMS = 5000;
        FullTests[FullTestCount].steps[stepCounter].SteerValue = direction * testValue;
        FullTests[FullTestCount].steps[stepCounter].stopWithWheelPercentage = false;
        FullTests[FullTestCount].steps[stepCounter].wheelPercentage = direction * 0.95f; //Stop at 95% right/left

        stepCounter++;

        //Stop for 2s
        FullTests[FullTestCount].steps[stepCounter].durationMS = 2000;
        FullTests[FullTestCount].steps[stepCounter].SteerValue = 0.0f;
        FullTests[FullTestCount].steps[stepCounter].stopWithWheelPercentage = false;
        FullTests[FullTestCount].steps[stepCounter].wheelPercentage = 0.5;

        stepCounter++;
        direction = -1.0f * direction;
    }

    FullTestCount += 1;
    screen.AddTest(wheelRateTest);
    //End Left And Right Test

    //Initialize Scren
    screen.Initialize();
    screen.UpdateFunction(TestActivated);
    screen.StartMenuFunction(StartMenu);
    screen.StartTestFunction(StartTest);

    //Initialize Button Manager
    BM::ButtonManagerInit();

    MainInitialize();

    while(1)
    {
        while(kickout)
        {
            //100 ms Loop
            static int lastTime100 = 0;
            int currentTime100 = Base_GetTime_ms();

            if(currentTime100 - lastTime100 >= 100)
            {
                Analog::EmergencyStop();
                Base_WriteSerialFile(true,"$KICKOUT,%d\r\n", Base_GetTime_ms());

                lastTime100 = currentTime100;
            }      
        }


        IMU.Run();
        screen.Run();
        
        CANMessage msg;
        if(CanReceive(CANLine::Main, &msg))
        {
            CANDispatch(CANLine::Main, msg);
        }

        if(CanReceive(CANLine::Secondary, &msg))
        {
            CANDispatch(CANLine::Secondary, msg);
        }

        static int readCounter = 0;
        static uint32_t wheelAngleRunning = 0;
        static uint32_t transducerRunning = 0;

        //1 ms Loop
        static int lastTime1 = 0;
        int currentTime1 = Base_GetTime_ms();
        if(currentTime1 - lastTime1 >= 1)
        {
            readCounter++;

            wheelAngleRunning += wheelAngle.read_u16();
            transducerRunning += transducer.read_u16();

            //10ms
            if(readCounter % 10 == 0)
            {
                int currentQuadA = quadA.read();
                int currentQuadB = quadB.read();

                if(currentQuadA != lastQuadA)
                {
                    quadAChanges++;
                    lastQuadA = currentQuadA;
                }

                if(currentQuadB != lastQuadB)
                {
                    quadBChanges++;
                    lastQuadB = currentQuadB;
                }

                if(quadAChanges > 2 && quadBChanges > 2)
                {
                    //kickout = true;
                    Base_WriteSerialFile(true,"$CGS,%d,%d,%d\r\n", quadAChanges, quadBChanges, Base_GetTime_ms());
                }
            }

            if(readCounter == 55)
            {
                Base_WriteSerialFile(true,"$VALVE,%0.2f,%0.2f,%d\r\n", Analog::GetCurrentValue(),  Base_GetTime_ms());
            }

            if(readCounter >= 100)
            {
                readCounter = 0;
                lastWheelAngleValue = wheelAngleRunning / 100.0f;
                lastTransducerValue = transducerRunning / 100.0f;

                if(lastWheelAngleValue > 100)
                {
                    LastAngleTime = Base_GetTime_ms();
                }

                wheelAngleUpdated = true;
                quadAChanges = 0;
                quadBChanges = 0;

                wheelAngleRunning = 0;
                transducerRunning = 0;
                Base_WriteSerialFile(true,"$AN,%d,%d,%d\r\n", lastWheelAngleValue, lastTransducerValue, Base_GetTime_ms());

            }

            lastTime1 = currentTime1;
        }

        BaseReadNMEA(&NMEASerialDispatch);

        int numberOfButtons = BM::ButtonsInQueue();

        for(int i = 0; i < numberOfButtons; i++)
        {
            ButtonCommand nextButton = BM::GetNextButton();
            screen.ButtonAction(nextButton);
        }

        if(IMU.NewDataAvailable())
        {
            IMUData newData = IMU.GetData();
            
            Base_WriteSerialFile(true,"$IMU_DATA,%d,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%d\r\n", 
                                    newData.Status, newData.xRate, newData.yRate, newData.zRate, 
                                    newData.xAccel, newData.yAccel, newData.zAccel, newData.BoardTemp, newData.ProgramTime);
                                   
            LastIMUTime = newData.ProgramTime;
        }
       
        int time = Base_GetTime_ms();

        MAIN_IMU_DATA_VALID = !(time - LastIMUTime > 2000);
        MAIN_GPS_DATA_VALID = !(time - LastGPSTime > 2000);
        MAIN_ANGLE_DATA_VALID = !(time - LastAngleTime > 2000);
        MAIN_CAN_DATA_VALID = !(time - LastCANTime > 2000);

        screen.DataInputStatus(MAIN_GPS_DATA_VALID, MAIN_IMU_DATA_VALID, MAIN_CAN_DATA_VALID, MAIN_ANGLE_DATA_VALID);
        static int currTime; 
        if(Start)
        {
            switch(testingState)
            {
                case Testing_State::Restart:
                    Start = false;
                    screen.Initialize();
                    break;
                case Testing_State::Waiting:
                    break;
                case Testing_State::Select:
                     CurrentStep = -1;
                     CurrentStepTime = Base_GetTime_ms();

                     //Start New Test
                     Base_CloseFile();
                     currTime = Base_GetTime_ms();
                     Base_NewFile("Test-%d-%s",currTime, FullTests[CurrentTest].info.TestName);
                     Base_WriteSerialFile(true, "Test Start (Test-%d-%s),%d\r\n", currTime, FullTests[CurrentTest].info.TestName, Base_GetTime_ms());
                     testingState = Testing_State::WaitingForSpeed;
                     break;
                case Testing_State::WaitingForSpeed:
                     screen.SetState(Screen_State::WaitingOnSpeed);
                     break;
                case Testing_State::Running:
                     RunTest();
                     break;
                case Testing_State::Finished:
                     Base_WriteSerialFile(true, "Test END (Test-%d-%s),%d\r\n", currTime, FullTests[CurrentTest].info.TestName, Base_GetTime_ms());
                     Base_CloseFile();
                    
                     testingState = Testing_State::Restart;
                     break;
                case Testing_State::FatalError:
                    break;
            }

           
       }
    }
}


