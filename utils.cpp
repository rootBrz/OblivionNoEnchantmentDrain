#include "utils.h"
#include "main.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <psapi.h>

// Return float based on ini setting
float ReadFloatIniSetting(const char *setting)
{
  char *path = (char *)malloc(MAX_PATH);

  DWORD len = GetModuleFileName(DLL_HANDLE, path, MAX_PATH);
  for (DWORD i = len; i > 0; --i)
    if (path[i - 1] == '\\' || path[i - 1] == '/')
    {
      path[i] = '\0';
      break;
    }
  snprintf(path + strlen(path), MAX_PATH - strlen(path), "\\%s", INI_NAME);

  FILE *iniSettings = fopen(path, "r");
  free(path);

  if (!iniSettings)
    return 0.0f;

  char line[1024];
  while (fgets(line, sizeof(line), iniSettings))
  {
    if (strncmp(line, setting, strlen(setting)))
      continue;

    char *equalSign = strchr(line, '=');
    if (equalSign)
      return strtof(equalSign + 1, NULL);
  }

  fclose(iniSettings);
  return 0.0f;
}

// Find memory address based on pattern
uintptr_t FindPattern(const char *pat)
{
  size_t patLen = strlen(pat) / 3;
  unsigned char *patBytes = (unsigned char *)_alloca(patLen);
  unsigned char *mask = (unsigned char *)_alloca(patLen);

  for (size_t i = 0; i < patLen; ++i)
  {
    const char *p = pat + i * 3;
    bool isWildcard = (*p == '?');

    patBytes[i] = isWildcard ? 0 : (unsigned char)strtoul(p, nullptr, 16);
    mask[i] = isWildcard ? 0 : 1;
  }

  size_t badShift[256];
  for (auto &value : badShift)
    value = patLen;
  for (size_t i = 0; i < patLen - 1; ++i)
    if (mask[i])
      badShift[patBytes[i]] = patLen - 1 - i;

  HMODULE hMod = GetModuleHandle(NULL);
  MODULEINFO mi;
  GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
  unsigned char *base = (unsigned char *)hMod;
  size_t max = mi.SizeOfImage - patLen;
  size_t i = 0;
  while (i <= max)
  {
    int j = patLen - 1;

    while (j >= 0 && (!mask[j] || base[i + j] == patBytes[j]))
      --j;

    if (j < 0)
      return (uintptr_t)(base + i);

    i += badShift[base[i + patLen - 1]];
  }
  return 0;
}

void LogToFile(const char *format, ...)
{
  FILE *file = fopen(LOG_NAME, "a");
  if (file)
  {
    va_list args;
    va_start(args, format);

    vfprintf(file, format, args);

    va_end(args);
    fclose(file);
  }
}