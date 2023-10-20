#ifndef BUTTONMANAGER_H
#define BUTTONMANAGER_H

#include "mbed.h"

enum class ButtonCommand
{
    Invalid = -1,
    Up = 0,
    Down = 1,
    Select = 2,
    Cancel = 3
};

enum class ButtonAction
{
    Release = 0,
    Press = 1
};

namespace BM {
    void ButtonManagerInit();
    int ButtonsInQueue();
    ButtonCommand GetNextButton();
    void ClearButtons();
}

#endif // BUTTONMANAGER_H