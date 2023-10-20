#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "mbed.h"

#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

class SDCard
{
    public:
        SDCard(const char *mountPoint, PinName mosi, PinName miso, PinName clk, PinName cs, PinName detect, int freq);

        bool Init();
        bool Deinit();
        bool RemoveFile(char *name);
        char * GetMountPoint(bool shrt);
        bool IsInitialized();
        int TestSD(int testSeed);

    protected:
        char mountPointShort[8];  // Mountpoint name in format "fs"
        char mountPointFull[10];  // Mountpoint name in format "/fs/"
        bool initialized = false; // Initialization status of object
        BlockDevice *_blockDevice; // Blockdevice object
        FATFileSystem *_fileSystem;   // Filesystem object
        DigitalIn *_SDDet; //SD Detecct Pin
};

#endif