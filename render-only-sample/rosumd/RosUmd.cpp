#include <windows.h>

#include <RosUmdLogging.h>

// TODO[bhouse] Turn ApiValidator back on

HINSTANCE g_hDLL;

BOOL WINAPI DllMain(
    HINSTANCE hmod,
    UINT dwReason,
    LPVOID lpvReserved )
{
	lpvReserved; // unused

    RosUmdLogging::Entry(__FUNCTION__);

    // Warning, do not call outside of this module, except for functions located in kernel32.dll. BUT, do not call LoadLibrary nor
    // FreeLibrary, either. Nor, call malloc nor new; use HeapAlloc directly.

    switch( dwReason )
    {
    case( DLL_PROCESS_ATTACH ):
        {
            g_hDLL = hmod;
        } break;

    case( DLL_PROCESS_DETACH ):
        {
            g_hDLL = NULL;
        } break;

    default: break;
    }

    RosUmdLogging::Exit(__FUNCTION__);

    return TRUE;
}
