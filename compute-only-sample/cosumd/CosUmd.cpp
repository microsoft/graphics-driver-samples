
#include "CosUmd.h"

#undef WPP_MACRO_USE_KM_VERSION_FOR_UM
#include "CosUmdLogging.h"
#include "CosUmd.tmh"

#include "coscompiler.h"

// TODO[bhouse] Turn ApiValidator back on

HINSTANCE g_hDLL;

BOOL WINAPI DllMain(
    HINSTANCE hmod,
    UINT dwReason,
    LPVOID lpvReserved )
{
    lpvReserved; // unused

    // Warning, do not call outside of this module, except for functions located in kernel32.dll. BUT, do not call LoadLibrary nor
    // FreeLibrary, either. Nor, call malloc nor new; use HeapAlloc directly.

    switch( dwReason )
    {
    case( DLL_PROCESS_ATTACH ):
        {
            WPP_INIT_TRACING(L"CosUmd");
                        
            InitializeShaderCompilerLibrary();
            g_hDLL = hmod;
        } break;

    case( DLL_PROCESS_DETACH ):
        {
            g_hDLL = NULL;
            WPP_CLEANUP();
            return TRUE;
        }

    default: break;
    }

    return TRUE;
}
