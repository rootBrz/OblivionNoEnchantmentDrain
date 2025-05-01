#pragma once

#include <cstdint>

typedef struct
{
  unsigned char byte;
  unsigned char mask;
} Pattern;

uintptr_t FindPattern(const char *pat);
void LogToFile(const char *format, ...);
float ReadFloatIniSetting(const char *setting);