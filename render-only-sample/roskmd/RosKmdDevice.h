#pragma once

#include "RosKmd.h"

class RosKmAdapter;

class RosKmDevice
{
public:

    static NTSTATUS __stdcall DdiCreateDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice);
    static NTSTATUS __stdcall DdiDestroyDevice(IN_CONST_HANDLE hDevice);

    static NTSTATUS
        __stdcall
        DdiOpenAllocation(
            IN_CONST_HANDLE                         hDevice,
            IN_CONST_PDXGKARG_OPENALLOCATION        pOpenAllocation);

    static NTSTATUS
        __stdcall
        DdiCloseAllocation(
            IN_CONST_HANDLE                     hDevice,
            IN_CONST_PDXGKARG_CLOSEALLOCATION   pCloseAllocation);

private:

    void * operator new(size_t  size);
    void operator delete(void * ptr);

    RosKmDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice);
    ~RosKmDevice();

private:

    DXGK_CREATEDEVICEFLAGS  m_Flags;
    HANDLE                  m_hRTDevice;

public:

    RosKmAdapter *          m_pRosKmAdapter;

};
