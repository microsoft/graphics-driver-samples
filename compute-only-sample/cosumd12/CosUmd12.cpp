
#include "CosUmd12.h"

#include <stdio.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

bool g_doNotStop = true;

void StopInFunction(const char * function, const char * file, int line)
{
    char output[128];

    snprintf(output, sizeof(output), "Stopped in %s (%s:%d)", function, file, line);

    OutputDebugStringA(output);
    if (!g_doNotStop) DebugBreak();
}
