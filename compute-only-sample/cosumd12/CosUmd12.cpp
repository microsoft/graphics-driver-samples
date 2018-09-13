
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

enum CosUmdDebugFlags
{
    CosUmdDebugBreak   = 1,
    CosUmdDebugMessage = 2
};

#if _DEBUG

UINT g_traceFlags = CosUmdDebugMessage;
UINT g_assertFlags = CosUmdDebugMessage | CosUmdDebugBreak;
UINT g_unexpectedDdiFlags = 0;

#else

UINT g_traceFlags = 0;
UINT g_assertFlags = 0;
UINT g_unexpectedDdiFlags = 0;

#endif

void TraceFunction(const char * function, const char * file, int line)
{
    if (g_traceFlags & CosUmdDebugMessage)
    {
        char output[256];

        snprintf(output, sizeof(output), "Trace %s (%s:%d)\n", function, file, line);

        OutputDebugStringA(output);
    }

    if (g_traceFlags & CosUmdDebugBreak)
    {
        DebugBreak();
    }
}

void AssertFunction(const char * function, const char * file, int line)
{
    if (g_assertFlags & CosUmdDebugMessage)
    {
        char output[256];

        snprintf(output, sizeof(output), "Assertion failed %s (%s:%d)\n", function, file, line);

        OutputDebugStringA(output);
    }

    if (g_assertFlags & CosUmdDebugBreak)
    {
        DebugBreak();
    }
}

void UnexpectedDdi(const char * function, const char * file, int line)
{
    if (g_unexpectedDdiFlags & CosUmdDebugMessage)
    {
        char output[256];

        snprintf(output, sizeof(output), "Unexpected DDI %s (%s:%d)\n", function, file, line);

        OutputDebugStringA(output);
    }

    if (g_unexpectedDdiFlags & CosUmdDebugBreak)
    {
        DebugBreak();
    }
}

