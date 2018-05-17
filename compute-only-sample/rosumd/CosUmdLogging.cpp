//
// Copyright (C) Microsoft. All rights reserved.
//

#include "precomp.h"

#include <RosLogging.h>

int _RosLogBugcheck (
    ULONG /*Level*/
    )
{
    // Raise a noncontinuable exception
    RaiseException(
        DWORD(E_FAIL),              // dwExceptionCode
        EXCEPTION_NONCONTINUABLE,
        0,
        nullptr);
    
    return 1;
} // _RosLogBugcheck (...)

int _RosLogDebug (
    ULONG Level
    )
{
    static PCSTR levelDescriptions[] = {
        "[%s]",              // TRACE_LEVEL_NONE
        "critical error",    // TRACE_LEVEL_CRITICAL
        "noncritical error", // TRACE_LEVEL_ERROR
        "warning",          // TRACE_LEVEL_WARNING
        "information",       // TRACE_LEVEL_INFORMATION
        "trace"              // TRACE_LEVEL_VERBOSE
    }; // levelDescriptions

    volatile void* returnAddress = _ReturnAddress();
    volatile PCSTR levelDescriptionSz =
        levelDescriptions[(Level < ARRAYSIZE(levelDescriptions)) ? Level : 0];
    char buf[64];
    HRESULT hr = StringCchPrintfA(
        buf,
        ARRAYSIZE(buf),
        "\n*** ROSUMD %s detected @%p.\n",
        levelDescriptionSz,
        returnAddress);

    if (SUCCEEDED(hr)) {
        OutputDebugStringA(buf);
    }

    DbgRaiseAssertionFailure();
    
    return 1;
} // _RosLogDebug (...)

