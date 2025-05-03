#include "main.h"
#include "MinHook.h"
#include "utils.h"
#include <cstdint>
#include <memoryapi.h>
#include <minwindef.h>
#include <process.h>
#include <processthreadsapi.h>
#include <stdio.h>

extern "C" float multiplier;
extern "C" void *oMeleeFunc;
extern "C" void *oStaffFunc;
extern "C" void *oBowFunc;

float multiplier = 0.0f;

void *oMeleeFunc = nullptr;
static void MeleeHook(void) __attribute__((naked));
static void MeleeHook(void)
{
  __asm__ volatile(
      ".intel_syntax noprefix\n\t"
      "mulss  xmm6, multiplier[rip]\n\t"
      "jmp   oMeleeFunc[rip]\n\t"
      ".att_syntax prefix\n\t"
      :
      :
      : "cc", "memory", "xmm6");
}

void *oStaffFunc = nullptr;
static void StaffHook(void) __attribute__((naked));
static void StaffHook(void)
{
  __asm__ volatile(
      ".intel_syntax noprefix\n\t"
      "mulss  xmm7, multiplier[rip]\n\t"
      "jmp   oStaffFunc[rip]\n\t"
      ".att_syntax prefix\n\t"
      :
      :
      : "cc", "memory", "xmm7");
}

void *oBowFunc = nullptr;
static void BowHook(void) __attribute__((naked));
static void BowHook(void)
{
  __asm__ volatile(
      ".intel_syntax noprefix\n\t"
      "mulss  xmm7, multiplier[rip]\n\t"
      "jmp   oBowFunc[rip]\n\t"
      ".att_syntax prefix\n\t"
      :
      :
      : "cc", "memory", "xmm7");
}

typedef struct
{
  LPVOID addr;
  LPVOID detourFunc;
  LPVOID *origFunc;
  const char *name;
} Patch;

unsigned __stdcall InitThread(void *)
{
  FILE *log = fopen(LOG_NAME, "w");

  fprintf(log, "Logging started.\n");
  multiplier = ReadFloatIniSetting("ChargeMultiplier");
  fprintf(log, "Charge multiplier: %f\n", multiplier);

  uintptr_t meleeAddr = FindPattern("0F 2F FE ?? ?? 48 8B 4C");
  uintptr_t staffAddr = FindPattern("0F 2F FE ?? ?? 48 8B 0D");
  uintptr_t bowAddr = FindPattern("0F 2F F7 ?? ?? 49 8B 1E 48 8D 8E");

  MH_Initialize();

  Patch patches[] = {
      {(LPVOID)meleeAddr, (LPVOID)MeleeHook, (LPVOID *)&oMeleeFunc, "melee"},
      {(LPVOID)staffAddr, (LPVOID)StaffHook, (LPVOID *)&oStaffFunc, "staff"},
      {(LPVOID)bowAddr, (LPVOID)BowHook, (LPVOID *)&oBowFunc, "bow"}};

  for (const auto &patch : patches)
  {
    if (patch.addr != 0)
    {
      fprintf(log, "Found %s address: 0x%p\n", patch.name, patch.addr);

      MH_CreateHook(patch.addr, patch.detourFunc, patch.origFunc);

      if (MH_EnableHook(patch.addr) == MH_OK)
        fprintf(log, "SUCCESS: Hook %s successfully enabled. \n", patch.name);
      else
        fprintf(log, "ERROR: Failed to enable %s hook. \n", patch.name);
    }
    else
      fprintf(log, "ERROR: Pattern not found! Hook %s can't be applied.\n", patch.name);
  }

  fclose(log);

  _endthreadex(0);
  return 0;
}

// OBSE
extern "C"
{
  OBSEPluginVersionData OBSEPlugin_Version{
      OBSEPluginVersionData::kVersion,
      13,
      "Configurable Enchantment Charge Cost",
      "rootBrz",
      OBSEPluginVersionData::kAddressIndependence_Signatures,
      OBSEPluginVersionData::kStructureIndependence_NoStructs,
      {},
      {},
      {},
      {},
      {}};

  bool OBSEPlugin_Load(const OBSEInterface *obse)
  {
    PLUGIN_HANDLE = obse->GetPluginHandle();
    OBSE_MESSAGE = (OBSEMessagingInterface *)obse->QueryInterface(kInterface_Messaging);

    return true;
  }
};
