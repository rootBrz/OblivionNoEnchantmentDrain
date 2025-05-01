#pragma once

#include "OBSE/obse64/PluginAPI.h"
#include <minwindef.h>

inline constexpr const char *LOG_NAME = "enchantmentdrain.log";
inline constexpr const char *INI_NAME = "enchantmentdrain.ini";
inline OBSEMessagingInterface *OBSE_MESSAGE = nullptr;
inline PluginHandle PLUGIN_HANDLE = kPluginHandle_Invalid;
inline HMODULE DLL_HANDLE = nullptr;

DWORD WINAPI InitThread(LPVOID lpParam);