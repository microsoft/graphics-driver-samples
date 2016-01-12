//
// Copyright (C) Microsoft. All rights reserved.
//

#include "precomp.h"

#include "RosKmdLogging.h"

namespace { // static

    bool rosLogIsDebuggerPresent (
        bool Refresh = false
        ) throw ()
    {
        //
        // NOTE: we do not care about possible multithreading issues here as in
        // the worst case we will refresh debuggers status more than once. Such
        // negligible side-effect doesn't warrant complexity and performance "tax"
        // associated with "proper" synchronization...
        //
        static bool debuggerPresenceInitialized = false;

        if (!debuggerPresenceInitialized) {
            KdRefreshDebuggerNotPresent();
            debuggerPresenceInitialized = true;
        } else if (Refresh != false) {
            KdRefreshDebuggerNotPresent();
        } // iff

        return (KD_DEBUGGER_ENABLED && !KD_DEBUGGER_NOT_PRESENT);
    } // rosLogIsDebuggerPresent (...)

} // namespace static

int _RosLogBugcheck (
    ULONG Level
    )
{
    volatile void* returnAddress = _ReturnAddress();
#pragma prefast(suppress:__WARNING_USE_OTHER_FUNCTION, "We really-really want to bugcheck here...)")
    KeBugCheckEx(
        BUGCODE_ID_DRIVER,
        ULONG_PTR(returnAddress),
        Level,
        0,
        0);

    //return 1;
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
    DbgPrintEx(
        DPFLTR_DEFAULT_ID,
        DPFLTR_ERROR_LEVEL,
        "\n*** ROSDOD %s detected @%p.\n",
        levelDescriptionSz,
        returnAddress);

    if (!rosLogIsDebuggerPresent(false)) return 1;

    for (;;) {
        char response[2] = {0};
        DbgPrompt(
            "Break to debug, Ignore, ignore All (bi)? ",
            response,
            sizeof(response));

        if ((response[0] == 'B') || (response[0] == 'b')) {
            DbgBreakPoint();
            break;
        } else if ((response[0] == 'I') || (response[0] == 'i')) {
            break;
        } // iff
    } // for (;;)

    return 1;
} // _RosLogDebug (...)

