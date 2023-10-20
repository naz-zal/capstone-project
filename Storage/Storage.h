#ifndef STORAGE_H
#define STORAGE_H

#include "mbed.h"
#include "LocalFileSystem.h"


class Storage {
    
    public:

        Storage() {
            fp = NULL; 
        }

        void open(const char*, int); 
        void write(); 
        void read(); 
        void close(); 

    private:
        std::FILE* fp;
};

#endif // STORAGE_H