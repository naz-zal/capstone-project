#include "mbed.h"
#include "base.h"
#include "PinDetect.h"
#include "ButtonManager.h"
#include "queue.h"


namespace BM 
{
    PinDetect Scroll_Down(p24,PullUp);
    PinDetect Scroll_Up(p23,PullUp);
    PinDetect Select_Button(p11,PullUp);
    PinDetect Cancel_Button(p12,PullUp);
    Queue buttonQueue(sizeof(ButtonCommand), 100);

    ButtonAction states[] = {ButtonAction::Release, ButtonAction::Release, ButtonAction::Release, ButtonAction::Release};

    void ButtonChangedState(ButtonCommand command, ButtonAction action)
    {
        ButtonAction oldState = states[(int) command];

        if(oldState != action)
        {
            states[(int) command ] = action;

            //state has changed from released to press
            if(action == ButtonAction::Press)
            {
                //add button to queue (to be pulled later)
                buttonQueue.Put(&command);
            }
        }
    }

    void callback_up_press(void) { ButtonChangedState(ButtonCommand::Up, ButtonAction::Press); }
    void callback_up_release(void) { ButtonChangedState(ButtonCommand::Up, ButtonAction::Release); }

    void callback_down_press(void) { ButtonChangedState(ButtonCommand::Down, ButtonAction::Press); }
    void callback_down_release(void) { ButtonChangedState(ButtonCommand::Down, ButtonAction::Release); }

    void callback_select_press(void) { ButtonChangedState(ButtonCommand::Select, ButtonAction::Press); }
    void callback_select_release(void) { ButtonChangedState(ButtonCommand::Select, ButtonAction::Release); }

    void callback_cancel_press(void) { ButtonChangedState(ButtonCommand::Cancel, ButtonAction::Press); }
    void callback_cancel_release(void) { ButtonChangedState(ButtonCommand::Cancel, ButtonAction::Release); }

    void ButtonManagerInit()
    {
        // Attach callback functions to button events
        Scroll_Up.attach_asserted(&callback_up_press);
        Scroll_Up.attach_deasserted(&callback_up_release);
        Scroll_Down.attach_asserted(&callback_down_press);
        Scroll_Down.attach_deasserted(&callback_down_release);
        Select_Button.attach_asserted(&callback_select_press);
        Select_Button.attach_deasserted(&callback_select_release);
        Cancel_Button.attach_asserted(&callback_cancel_press);
        Cancel_Button.attach_deasserted(&callback_cancel_release);

        // Set the sampling frequency
        Scroll_Up.setSampleFrequency();
        Scroll_Down.setSampleFrequency();
        Select_Button.setSampleFrequency();
        Cancel_Button.setSampleFrequency();
    }

    int ButtonsInQueue()
    {
        return buttonQueue.GetNumberOfItems();
    }

    ButtonCommand GetNextButton()
    {
        ButtonCommand command = ButtonCommand::Invalid;

        if(buttonQueue.GetNumberOfItems() > 0)
        {
            buttonQueue.Get(&command);
        }

        return command;
    }

    void ClearButtons()
    {
        buttonQueue.Flush();
    }
}


