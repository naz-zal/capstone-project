#include "SDCardManager.h"
#include "mbed.h"
#include "base.h"

SDCard::SDCard(const char *mountPoint, PinName mosi, PinName miso, PinName clk, PinName cs, PinName detect, int freq)
{
    _blockDevice = new SDBlockDevice(mosi, miso, clk, cs, freq);

    _fileSystem = new FATFileSystem(mountPoint);

    _SDDet = new DigitalIn(detect, PullUp);

    if (mountPoint == NULL || strlen(mountPoint) < 1)
        return;

    strncpy(mountPointShort, mountPoint, sizeof(mountPointShort));
    snprintf(mountPointFull, sizeof(mountPointFull), "/%s/", mountPointShort);
}

bool SDCard::Init()
{
    Base_WriteSerial("$SD,%s,Starting Initialization,%d\r\n", mountPointShort, Base_GetTime_ms());

    if(!_SDDet)
    {
        Base_WriteSerial("$SD,%s,No SD Card Detected, Canceling,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }

    if (!_blockDevice || !_fileSystem)
    {
        Base_WriteSerial("$SD,ERR,%s,Block Device or File System NULL,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }

    if(_blockDevice->init())
    {
        Base_WriteSerial("$SD,ERR,%s,Could not initialize block device,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }

    if(_fileSystem->mount(_blockDevice))
    {
        Base_WriteSerial("$SD,%s,Filesystem could not mount, reformatting,%d\r\n", mountPointShort, Base_GetTime_ms());
        
        if (_fileSystem->reformat(_blockDevice))
        {
            Base_WriteSerial("$SD,ERR,%s,Could not format block device,%d\r\n", mountPointShort, Base_GetTime_ms());
            return false;
        }
        else 
        {
            Base_WriteSerial("$SD,%s,Formatting complete,%d\r\n", mountPointShort, Base_GetTime_ms());
        }
        return false;
    }

    Base_WriteSerial("SD,%s,SD initialization complete,%d\r\n", mountPointShort, Base_GetTime_ms());
    initialized = true;
    return true;
}


bool SDCard::Deinit()
{
    printf("$SD,%s,Starting DeInitialize,%d\r\n", mountPointShort, Base_GetTime_ms());
    if (!_blockDevice || !_fileSystem)
    {
        printf("$SD,ERR,%s,Block Device or File System NULL,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }

    if (!initialized)
    {
        printf("$SD,ERR,%s,Device Not Initialized,%d\r\n", mountPointShort, Base_GetTime_ms());
        return true;
    }

 
    if (_fileSystem->unmount())
    {
        printf("$SD,ERR,%s,Could not unmount block device,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }

    if (_blockDevice->deinit())
    {
        printf("$SD,ERR,%s,Could not deinitialize block device,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }

    printf("$SD,%s,Deinitialization complete,%d\r\n", mountPointShort, Base_GetTime_ms());
    return true;
}

bool SDCard::RemoveFile(char *name)
{
    if (!initialized)
    {
        printf("$SD,ERR,%s,FS not initialized,%d\r\n", mountPointShort, Base_GetTime_ms());
        return false;
    }
    if (_fileSystem->remove(name))
    {
        printf("$SD,ERR,%s,Could not remove %s,%d\r\n", mountPointShort, name, Base_GetTime_ms());
        return false;
    }
    printf("$SD,%s,Deleted %s,%d\r\n", mountPointShort, name, Base_GetTime_ms());
    return true;
}

char * SDCard::GetMountPoint(bool shrt){
    static char returnString[10];
    if(shrt)
        strcpy(returnString, mountPointShort);
    else 
        strcpy(returnString, mountPointFull);
    
    return returnString;
}

bool SDCard::IsInitialized()
{
    return initialized;
}

int SDCard::TestSD(int testSeed)
{
    int returnVal = 1;

    if (!initialized)
        return returnVal; // 1 - Not initialized

    printf("$FS,%s,Test Start\r\n", mountPointShort);

    char testData[32];
    char testFileName[32];
    snprintf(testFileName, sizeof(testFileName), "%sTest.txt", mountPointFull);

    FILE *testFile;
    testFile = fopen(testFileName, "w");

    if (testFile != NULL)
    {
        printf("$FS,%s,File open for writing\r\n", mountPointShort);
        sprintf(testData, "T%d", testSeed); // Generate random test string
        fputs(testData, testFile);          // Write random string to file
        // Close and reset
        fclose(testFile);
        testFile = NULL;

        // Open same file for reading
        testFile = fopen(testFileName, "r");

        if (testFile != NULL)
        {
            printf("$FS,%s,File open for reading\r\n", mountPointShort);

            char readData[32];
            if (fgets(readData, sizeof(readData), testFile) != NULL)
            {
                printf("$FS,%s,Testing data read: %s, Test reference data: %s\r\n", mountPointShort, readData, testData);
                if (!strcmp(readData, testData))
                {
                    printf("$FS,%s,Test read success\r\n", mountPointShort);
                    returnVal = 0; // 0 - Test success
                }
                else
                {
                    printf("$FS,%s,Test data mismatch\r\n", mountPointShort);
                    returnVal = 5; // 5 - Test Data Mismatch
                }
            }
            else
            {
                printf("$FS,%s,Test data read NULL\r\n", mountPointShort);
                returnVal = 4; // 4 - Bad test read
            }
            fclose(testFile);
            testFile = NULL;
        }
        else
        {
            printf("$FS,%s,ERR,Could not open test file for reading\r\n", mountPointShort);
            returnVal = 3; // 3 - Could not open test file for reading
        }

        printf("$FS,%s,Removed test file\r\n", mountPointShort);
        //remove(testFileName);
    }
    else
    {
        printf("$FS,%s,ERR,Could not open test file for writing\r\n", mountPointShort);
        returnVal = 2; // 2 - Could not open test file for writing
    }

    return returnVal;
}