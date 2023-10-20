#ifndef BASE_H
#define BASE_H

#include "mbed.h"

void BaseInit();
void Base_WriteSerial(const char* str, ...);
void Base_WriteSerialFile(bool file, const char* str, ...);
void TimeTick();
uint32_t Base_GetTime_ms();
void BaseReadNMEA(Callback<void(char*)> dispatch);
void Base_strcpy (char *dest, const char *src);
int Base_BoundValue(int value, int min, int max);
float Base_BoundFloat(float value, float min, float max);

bool Base_NewFile(const char* str, ...);
bool Base_CloseFile();
char* Base_GetFileName();




#endif // BASE_H