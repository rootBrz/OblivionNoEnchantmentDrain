#include "main.h"
#include "MinHook.h"
#include "utils.h"
#include <cstdint>
#include <memoryapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>

float multiplier = 0.0f;

void *oMeleeFunc = nullptr;
static void MeleeHook(void) __attribute__((naked));
static void MeleeHook(void)
{
  __asm__ volatile(
      ".intel_syntax noprefix\n\t"
      "mulss  xmm6, DWORD PTR [rip + multiplier]\n\t"
      "jmp   QWORD PTR [rip + oMeleeFunc]\n\t"
      ".att_syntax prefix\n\t"
      :
      :
      : "cc", "memory");
}

void *oStaffFunc = nullptr;
static void StaffHook(void) __attribute__((naked));
static void StaffHook(void)
{
  __asm__ volatile(
      ".intel_syntax noprefix\n\t"
      "mulss  xmm7, DWORD PTR [rip + multiplier]\n\t"
      "jmp   QWORD PTR [rip + oStaffFunc]\n\t"
      ".att_syntax prefix\n\t"
      :
      :
      : "cc", "memory");
}

void *oBowFunc = nullptr;
static void BowHook(void) __attribute__((naked));
static void BowHook(void)
{
  __asm__ volatile(
      ".intel_syntax noprefix\n\t"
      "mulss  xmm7, DWORD PTR [rip + multiplier]\n\t"
      "jmp   QWORD PTR [rip + oBowFunc]\n\t"
      ".att_syntax prefix\n\t"
      :
      :
      : "cc", "memory");
}

DWORD WINAPI InitThread(LPVOID lpParam)
{
  SetFileAttributesA(LOG_NAME, FILE_ATTRIBUTE_NORMAL);
  DeleteFile(LOG_NAME);

  // If OBSE initialized, do not run from PROCESS_ATTACH
  if (lpParam && OBSE_MESSAGE)
    return true;

  multiplier = ReadFloatIniSetting("ChargeMultiplier");

  uintptr_t meleeAddr = FindPattern("0F 2F FE ?? ?? 48 8B 4C 24 70 48 8B 1F 48 81 C1 88");
  uintptr_t staffAddr = FindPattern("0F 2F FE ?? ?? 48 8B 0D ?? ?? ?? ??");
  uintptr_t bowAddr = FindPattern("0F 2F F7 ?? ?? 49 8B 1E 48 8D 8E");

  MH_Initialize();

  MH_CreateHook((LPVOID)meleeAddr, (LPVOID)MeleeHook, (LPVOID *)&oMeleeFunc);
  MH_EnableHook((LPVOID)meleeAddr);

  MH_CreateHook((LPVOID)staffAddr, (LPVOID)StaffHook, (LPVOID *)&oStaffFunc);
  MH_EnableHook((LPVOID)staffAddr);

  MH_CreateHook((LPVOID)bowAddr, (LPVOID)BowHook, (LPVOID *)&oBowFunc);
  MH_EnableHook((LPVOID)bowAddr);

  LogToFile("Found melee address: 0x%p\n", meleeAddr);
  LogToFile("Found staff address: 0x%p\n", staffAddr);
  LogToFile("Found bow address: 0x%p\n", bowAddr);
  LogToFile("Charge multiplier: %f\n", multiplier);

  return true;
}

// OBSE
void MessageHandler(OBSEMessagingInterface::Message *msg)
{
  if (msg->type == OBSEMessagingInterface::kMessage_PostPostLoad)
    InitThread(nullptr);
}
extern "C"
{
  OBSEPluginVersionData OBSEPlugin_Version =
      {
          OBSEPluginVersionData::kVersion,

          10,
          "Configurable Enchantment Charge Cost",
          "rootBrz",

          OBSEPluginVersionData::kAddressIndependence_Signatures,
          OBSEPluginVersionData::kStructureIndependence_NoStructs};

  bool OBSEPlugin_Load(const OBSEInterface *obse)
  {
    PLUGIN_HANDLE = obse->GetPluginHandle();
    OBSE_MESSAGE = (OBSEMessagingInterface *)obse->QueryInterface(kInterface_Messaging);
    OBSE_MESSAGE->RegisterListener(PLUGIN_HANDLE, "OBSE", MessageHandler);

    return true;
  }
};
