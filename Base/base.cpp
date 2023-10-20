#include "mbed.h"
#include "base.h"
#include <cstdio>

Serial pc(USBTX, USBRX); //Connection to PC over Serial
Serial NMEA(p13, p14); //Connection to NMEA input

Ticker Ticker_Timer; //Timer Variable
uint64_t TimeCount = 0; //Current Time, in 100us increments

bool LocalFileOpen = false;
char testFileName[70];

FILE *testFile;

char* Base_GetFileName()
{
    return testFileName;
}

bool Base_NewFile(const char* str, ...)
{
    if(LocalFileOpen)
    {
        Base_CloseFile();
    }

    if(LocalFileOpen)
        return false;

    char newFileName[64]; //Buffer for writing serial

    //contstrcut message
    va_list args;
    va_start(args, str);
    vsprintf(newFileName, str, args);
    va_end(args);

    snprintf(testFileName, sizeof(testFileName), "/sd/%s.txt", newFileName);
    testFile = fopen(testFileName, "w");


    Base_WriteSerial("$BASE, OPEN FILE: %s,%d", testFileName, Base_GetTime_ms());

    if(testFile == NULL)
    {
        Base_WriteSerial("$BASE, NULL NULL NULL OPEN FILE: %s,%d", testFileName, Base_GetTime_ms());
    }

    LocalFileOpen = true;
    return true;
}

bool Base_CloseFile()
{
    if(LocalFileOpen)
    {
        fclose(testFile);
        LocalFileOpen = false;
        testFile = NULL;
        return true;
    }
    return false;
}

int Base_BoundValue(int value, int min, int max)
{
    if(value > max)
        return max;

    if(value < min)
        return min;

    return value;
}

float Base_BoundFloat(float value, float min, float max)
{
    if(value > max)
        return max;

    if(value < min)
        return min;

    return value;
}

void BaseInit()
{
    Ticker_Timer.attach(&TimeTick, 0.001); //Start time tracking ticker, run at 100us interval

    //Set baud rate (Idk why is red, ignore)
    pc.baud(460800);
    NMEA.baud(460800);

    Base_WriteSerial("\r\n**********************\r\n");
    Base_WriteSerial("ATGCS Startup!\r\n");
    Base_WriteSerial("**********************\r\n\r\n");

    Base_WriteSerial("Base: Initialization Complete\r\n");
}

void Base_strcpy(char *dest, const char *src)
{
    memcpy(dest, src, strlen (src) + 1);
}

void BaseReadNMEA(Callback<void(char*)> dispatch)
{   
    static uint8_t serialCharIndex = 0; //current index of array
    static char serialBuffer[256]; //buffer to hold incoming characters

    while (NMEA.readable())
    {         
        //grab next character
        char charIn = NMEA.getc();        

        //wait for message start identifier ($)
        if (!serialCharIndex && charIn != '$')
            continue;
            
        //end of messgae 
        if (charIn == '\n' || charIn == '\r')
        {
            //terminate string in buffer
            serialBuffer[serialCharIndex] = 0;
            
            //reset index
            serialCharIndex = 0; 
            
            //send message to given dispatch function
            dispatch(serialBuffer);                  
        }
        else
        {
            //save char
            serialBuffer[serialCharIndex] = charIn;
            
            //increment to thr next index, unless we've hit the end of the array
            if (serialCharIndex < 255)
                serialCharIndex++;
        }
    }
}

//Usage: BaseWriteSerial("Hello! It is %d o'clock! \r\n", time);
void Base_WriteSerial(const char* str, ...)
{
    char msg[512]; //Buffer for writing serial

    //contstrcut message
    va_list args;
    va_start(args, str);
    vsprintf(msg, str, args);
    va_end(args);

    pc.puts(msg);
}

void Base_WriteSerialFile(bool file, const char* str, ...)
{
    char msg[512]; //Buffer for writing serial

    //contstrcut message
    va_list args;
    va_start(args, str);
    vsprintf(msg, str, args);
    va_end(args);

    pc.puts(msg);

    if(file && LocalFileOpen)
    {
        fputs(msg, testFile);
    } 
}

void TimeTick()
{
    TimeCount++;
}

uint32_t Base_GetTime_ms()
{
    return TimeCount;
}