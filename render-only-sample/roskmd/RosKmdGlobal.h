#pragma once

#include "RosKmd.h"

class RosKmdGlobal
{
public:

    static void DdiUnload(void);

    static void DdiControlEtwLogging(
        IN_BOOLEAN  Enable,
        IN_ULONG    Flags,
        IN_UCHAR    Level);

    static NTSTATUS DriverEntry(__in IN DRIVER_OBJECT* pDriverObject, __in IN UNICODE_STRING* pRegistryPath);

    __forceinline static bool IsRenderOnly () { return s_bRenderOnly; }

    static const size_t kMaxVideoMemorySize = 128 * 1024 * 1024;

    static DRIVER_OBJECT* s_pDriverObject;
    static size_t s_videoMemorySize;
    static void * s_pVideoMemory;
    static PHYSICAL_ADDRESS s_videoMemoryPhysicalAddress;

private:

    static bool s_bDoNotInstall;
    static bool s_bRenderOnly;

};