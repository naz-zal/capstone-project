#ifndef SCREEN_H
#define SCREEN_H

#include "mbed.h"
#include "ButtonManager.h"
#include "TestInfo.h"

enum class Screen_State
{
    Logo,
    MainMenu,
    WaitingOnSpeed,
    RunningTest,
    TestFinished,
    KickoutOccured,
    FatalError
};

class Screen {
public:

    void Initialize();
    void Run();
    void SetState(Screen_State states);
    void ButtonAction(ButtonCommand command);
    void StartTest();
    void GPSSpeed(float ActualSpeed);
    bool AddTest(TestInfoBasic testInfo);
    void UpdateFunction(Callback<void(TestInfoBasic)> TestStart);
    void StartMenuFunction(Callback<void(void)> StartMenu);
    void StartTestFunction(Callback<void(void)> StartMenu);
    void DataInputStatus(bool GPS, bool IMU, bool CAN, bool ANGLE);
    void TestInfoStatus(int currentStep, int TotalSteps);

 private:
    void DisplayLogo();
    void RunMenu();
    void DisplayWaitingOnSpeed();
    void DisplayRunningTest();
    void DisplayTestFinished();
    void DisplayKickout();
    void DisplayFatalError();
    void CheckSpeed();
    void MaintainSpeed();
    void PrintDataStatus();

    void LCD_TEST_SCREEN();
    void Menu();
    void Scroll();
};

#endif // SCREEN_H