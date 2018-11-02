#pragma once

#include "CosKmd.h"

class CosKmAdapter;
struct CosKmdProcess;

class CosKmDevice
{
public: // NONPAGED

    __forceinline static CosKmDevice* Cast (HANDLE hDevice)
    {
        return static_cast<CosKmDevice*>(hDevice);
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

    CosKmDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice);
    ~CosKmDevice();

private:

    DXGK_CREATEDEVICEFLAGS  m_Flags;
    HANDLE                  m_hRTDevice;

#if COS_GPUVA_SUPPORT

    CosKmdProcess *         m_pKmdProcess;

#endif

public:

    CosKmAdapter *          m_pCosKmAdapter;

public: // PAGED

    _Check_return_
    _Function_class_DXGK_(DXGKDDI_OPENALLOCATIONINFO)
    _IRQL_requires_(PASSIVE_LEVEL)
    NTSTATUS OpenAllocation (IN_CONST_PDXGKARG_OPENALLOCATION);
};
