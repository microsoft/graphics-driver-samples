#pragma once

#include "RosKmd.h"

class RosKmAdapter;

class RosKmDevice
{
public: // NONPAGED

    __forceinline static RosKmDevice* Cast (HANDLE hDevice)
    {
        return static_cast<RosKmDevice*>(hDevice);
    }
    
public:

    static NTSTATUS __stdcall DdiCreateDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice);
    static NTSTATUS __stdcall DdiDestroyDevice(IN_CONST_HANDLE hDevice);

    static NTSTATUS
        __stdcall
        DdiCloseAllocation(
            IN_CONST_HANDLE                     hDevice,
            IN_CONST_PDXGKARG_CLOSEALLOCATION   pCloseAllocation);

private:

    RosKmDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice);
    ~RosKmDevice();

private:

    DXGK_CREATEDEVICEFLAGS  m_Flags;
    HANDLE                  m_hRTDevice;

public:

    RosKmAdapter *          m_pRosKmAdapter;

public: // PAGED

    _Check_return_
    _Function_class_DXGK_(DXGKDDI_OPENALLOCATIONINFO)
    _IRQL_requires_(PASSIVE_LEVEL)
    NTSTATUS OpenAllocation (IN_CONST_PDXGKARG_OPENALLOCATION);
};
