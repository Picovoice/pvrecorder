#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <delayimp.h>
#include <string.h>

static FARPROC WINAPI delayHook(unsigned int dliNotify, DelayLoadInfo* dli) {
  HMODULE m;
  if (dliNotify != dliNotePreLoadLibrary)
    return NULL;

  if (_stricmp(dli->szDll, "node.exe") != 0)
    return NULL;

  m = GetModuleHandle(NULL);
  return (FARPROC) m;
}

PfnDliHook __pfnDliNotifyHook2 = delayHook;