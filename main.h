#ifndef MAIN_H
#define MAIN_H

enum class Main_State
{
    Restart,
    Initializing,
    Configuring,
    Running,
    FatalError
};

enum class Testing_State
{
    Restart,
    Waiting,
    Select,
    WaitingForSpeed,
    Running,
    Finished,
    FatalError
};

#endif //MAIN_H