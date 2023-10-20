#include "mbed.h"
#include "Screen.h"
#include "TextLCD.h"
#include "ButtonManager.h"
#include "base.h"

bool BUTTON_UP = false; 
bool BUTTON_DOWN = false; 
bool BUTTON_SELECT = false; 
bool BUTTON_CANCEL = false;

bool IMU_DATA_VALID = false;
bool GPS_DATA_VALID = false;
bool CAN_DATA_VALID = false;
bool ANGLE_DATA_VALID = false;

bool Screen_Change_Init = true;
float SPEED_FROM_GPS;
int Selected_test;
int Hold_Speed = 0;
int Maintain_Speed = 0;

int CURRENT_STEP = 0;
int TOTAL_STEP = 0;


bool clearScreen = true; 
bool UpdateStatus = true;

Callback<void(TestInfoBasic)> TestCallback;
Callback<void(void)> MenuStartCallBack;
Callback<void(void)> TestStartCallBack;

const char* Logo[] = {"ATGCS = V1.1",
                      "Build Date: April 11/23",
                      "Group 21 - Init...",
                      "Press Select!" };

const char* Menu_Bar= "Select a Test";

const int TestMaxCount = 16;
TestInfoBasic Tests[TestMaxCount];
int TestsCount = 0;

int Screen_Position = 0;

#define SLAVEADDRESS 0x4E

I2C i2c_lcd(p28,p27); // SDA, SCL
TextLCD_I2C lcd(&i2c_lcd, SLAVEADDRESS,  TextLCD::LCD20x4);

Screen_State state = Screen_State::Logo;

void Screen::Initialize()
{
    lcd.setCursor(TextLCD::CurOff_BlkOff);
    lcd.setBacklight(TextLCD::LightOn);
    lcd.cls();
    clearScreen = true;

    state = Screen_State::Logo;
};

void Screen::UpdateFunction(Callback<void(TestInfoBasic)> TestStart)
{
    TestCallback = TestStart;
}

void Screen::StartMenuFunction(Callback<void(void)> StartMenu)
{
    MenuStartCallBack = StartMenu;
}

void Screen::StartTestFunction(Callback<void(void)> StartMenu)
{
    TestStartCallBack = StartMenu;
}

void Screen::ButtonAction(ButtonCommand command) // check to see if a button has been pressed
{
    if(command == ButtonCommand::Cancel){
        BUTTON_CANCEL = true;
    } else if(command == ButtonCommand::Down){
        BUTTON_DOWN = true;
    }else if(command == ButtonCommand::Up){
        BUTTON_UP = true;
    }else if(command == ButtonCommand::Select){
        BUTTON_SELECT = true;
    }
}

bool Screen::AddTest(TestInfoBasic testInfo)
{
    if(TestsCount < TestMaxCount)
    {
        memcpy(&Tests[TestsCount], &testInfo, sizeof(TestInfoBasic));
        TestsCount++;
        return true;
    }
    return false;
}

void Screen::TestInfoStatus(int currentStep, int TotalSteps)
{
    CURRENT_STEP = currentStep;
    TOTAL_STEP = TotalSteps;
}

void Screen::DataInputStatus(bool GPS, bool IMU, bool CAN, bool ANGLE)
{
    if(GPS_DATA_VALID != GPS)
    {
        GPS_DATA_VALID = GPS;
        UpdateStatus = true;
    }

    if(IMU_DATA_VALID != IMU)
    {
        IMU_DATA_VALID = IMU;
        UpdateStatus = true;
    }

    if(CAN_DATA_VALID != CAN)
    {
        CAN_DATA_VALID = CAN;
        UpdateStatus = true;
    }

    if(ANGLE_DATA_VALID != ANGLE)
    {
        ANGLE_DATA_VALID = ANGLE;
        UpdateStatus = true;
    }
}

void Screen::PrintDataStatus()
{

    static int lastTime = 0;

    int currentTime = Base_GetTime_ms();

    if(currentTime - lastTime > 500)
    {
        UpdateStatus = true;
        lastTime = currentTime;
    }

    if(!UpdateStatus)
        return;
    
    UpdateStatus = false;

    //GPS
    char writeGPS = GPS_DATA_VALID ? 'G' : '.';
    char writeIMU = IMU_DATA_VALID ? 'I' : '.';
    char writeCAN = CAN_DATA_VALID ? 'C' : '.';
    char writeAngle = ANGLE_DATA_VALID ? 'W' : '.';

    lcd.setAddress(16, 0);
    lcd.printf("%c%c%c%c", writeGPS, writeIMU, writeCAN, writeAngle);
}

void Screen::DisplayLogo()
{
    static int lastTime = 0;

    if(BUTTON_SELECT)
    {
        MenuStartCallBack();
    }

    int currentTime = Base_GetTime_ms();

    if(currentTime - lastTime < 500)
    {
        return;
    }

    lastTime = currentTime;

    if(clearScreen)
    {
        clearScreen = false;
        lcd.cls();
        for(int i = 0; i < 4; i++)
        {
            lcd.setAddress(0, i);
            lcd.printf("%s", Logo[i]);
        }
    }
}

void Screen::SetState(Screen_State states)
{
    state = states;
    clearScreen = true;
}


void Screen::GPSSpeed(float ActualSpeed){
    SPEED_FROM_GPS =ActualSpeed;

}

void Screen::CheckSpeed() {

    //Tracking how long the machine is within the desire speed
    if((SPEED_FROM_GPS-3) < Tests[Selected_test].TestSpeed && Tests[Selected_test].TestSpeed < (SPEED_FROM_GPS+3)){
        Hold_Speed++;
    }else{
        Hold_Speed=0;
    }

}
void Screen::MaintainSpeed(){

    return;

    if((SPEED_FROM_GPS-3) < Tests[Selected_test].TestSpeed && Tests[Selected_test].TestSpeed< (SPEED_FROM_GPS+3)){
        Maintain_Speed= 0;
    }else{
        Maintain_Speed++;
    }
    if (Maintain_Speed==10){
        Maintain_Speed =0;
        Hold_Speed=0;
    }

}
//Call When Test Has Been selected
void Screen::StartTest()
{

}

void Screen::Run()
{
    switch(state)
    {
        case Screen_State::Logo:
            DisplayLogo();
            break;
        case Screen_State::MainMenu:
            RunMenu();
            break;
        case Screen_State::WaitingOnSpeed:
            DisplayWaitingOnSpeed();
            break;
         case Screen_State::RunningTest:
            DisplayRunningTest();
            break;
        case Screen_State::TestFinished:
            DisplayTestFinished();
            break;
        case Screen_State::KickoutOccured:
            DisplayKickout();
            break;
        case Screen_State::FatalError:
            DisplayFatalError();
            break;
    }

    //reset pin status
    BUTTON_UP = false; 
    BUTTON_DOWN = false; 
    BUTTON_SELECT = false; 
    BUTTON_CANCEL = false;

    Screen::PrintDataStatus();
}

void Screen::RunMenu(){

    Screen::Scroll();
};

void Screen::DisplayWaitingOnSpeed(){

    static int lastTime = 0;
    int currentTime = Base_GetTime_ms();

    if(Hold_Speed >10 && BUTTON_SELECT)
    {
        TestStartCallBack();
    }


    if( currentTime - lastTime >= 500){

        lastTime = currentTime;

        if(Screen_Change_Init){ 
            // Clear the screen one time 
            Hold_Speed= 0;
            Screen_Change_Init= false; 
            lcd.cls();
        }else if(Hold_Speed<10) {
            // Updating the speed from the GPS every 0.5 seconds
            lcd.setAddress(0, 0);
            lcd.printf("Waiting on Speed");
            lcd.setAddress(0, 1);
            lcd.printf("Required Speed: %0.1f",Tests[Selected_test].TestSpeed);
            lcd.setAddress(0, 2);
            lcd.printf("Current Speed below:");
            lcd.setAddress(0, 3);
            lcd.printf("       %.2f", SPEED_FROM_GPS);
        
            // This updates Hold_Speed everytime it is called. If the speed is within a given range Hold_Speed will increase by 1
            Screen::CheckSpeed();

        }else if(Hold_Speed==10){
            Screen::CheckSpeed();
            lcd.cls();
        }else if(Hold_Speed>10){

            Screen::MaintainSpeed();
            lcd.setAddress(0, 0);
            lcd.printf("Hold Speed: %0.1f", Tests[Selected_test].TestSpeed);
            lcd.setAddress(0, 1);
            lcd.printf("Your Speed: %.2f", SPEED_FROM_GPS);
            lcd.setAddress(0, 2);
            lcd.printf("Press to Start:");
            lcd.setAddress(0, 3);
            lcd.printf("-> BEGIN");
            lcd.setAddress(0, 4);
        }

    }
};

void Screen::DisplayRunningTest(){

    if(clearScreen)
    {
        lcd.cls();
        lcd.setAddress(0, 0);
        lcd.printf("Running Test:");
        lcd.setAddress(0, 1);
        lcd.printf("%s (%d)",Tests[Selected_test].TestName,Tests[Selected_test].TestID);
        clearScreen = false;
    }

    static int lastTime = 0;

    int currentTime = Base_GetTime_ms();

    if(currentTime - lastTime < 500)
    {
        return;
    }
    lastTime = currentTime;

    lcd.setAddress(0, 2);
    lcd.printf("Test Step %d of %d", CURRENT_STEP, TOTAL_STEP);
    lcd.setAddress(0, 3);
    lcd.printf("Progress: %0.1f %c", (float) CURRENT_STEP * 100.0 / (float) TOTAL_STEP, '%');
};
void Screen::DisplayTestFinished(){};
void Screen::DisplayKickout(){};
void Screen::DisplayFatalError(){};

void Screen::Menu()
{

}

void Screen::Scroll(){

    if(BUTTON_DOWN)
    {
         Screen_Position++;
         clearScreen = true;
    }
       
    if(BUTTON_UP)
    {
        Screen_Position--;
        clearScreen = true;
    }
        
    Screen_Position = Base_BoundValue(Screen_Position, 0, TestsCount - 1);

    //If a test is selected 
    if(BUTTON_SELECT)
    {
        Selected_test = Screen_Position; 
        Screen_Position= 0;

        if(TestCallback)
        {
            //Start Test!
            TestCallback(Tests[Selected_test]);
        }
        
    }

    if(!clearScreen)
    {
        return;
    }


    lcd.cls();
    lcd.setAddress(0, 0);
    lcd.printf("%s",Menu_Bar); 
    
    for(int i = 0; i < 3; i++)
    {
        int testIndex = Screen_Position + i - 2;
        if(testIndex >= 0 && TestsCount > testIndex)
        {
            lcd.setAddress(0, i + 1);
            lcd.printf("%s%s", Screen_Position == testIndex ? "->" : "", Tests[testIndex].TestName); 
        }
    }

    clearScreen = false;

    

};