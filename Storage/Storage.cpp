#include "mbed.h"
#include <stdio.h>
#include "Storage.h"

LocalFileSystem local("local"); // defining fs

void Storage::open(const char* fileName, int partNumber) {
    

    char fileNameBuffer[256];

    sprintf(fileNameBuffer, "/local/blahblahblahblah%s_%03d.txt", fileName, partNumber);

    this->fp = fopen(fileNameBuffer, "a");
    if (this->fp == NULL) {
        printf("Failed to open file\r\n");
        return;
    }
}

void Storage::write() {

    char test[256] = "yey"; 

    if (this->fp == NULL) {
        printf("File not open\r\n");
        return;
    }

    fwrite(test , 1 , sizeof(test) , this->fp);
    
};

void Storage::close() {

    fclose(this->fp);  

}

void Storage::read() {


    // Open the file for reading
    FILE* fp = fopen("/local/name{}.txt", "r");
    if (!fp) {
        printf("Failed to open file for reading\n");
    }



    // Read the contents of the file and output to the terminal
    char buf[256];
    while (fgets(buf, sizeof(buf), fp)) {
        printf("%s", buf);
    }

    // Close the file
    fclose(fp);
}