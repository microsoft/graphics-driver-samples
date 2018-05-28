////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "CosUmd12.h"

#include "CosUmd12Device.h"

//==================================================================================================================================
//
// CosUmdDevice
//
//==================================================================================================================================

CosUmd12Device::CosUmd12Device(
    CosUmd12Adapter* pAdapter,
    const D3D12DDIARG_CREATEDEVICE_0003* pArgs) :
    m_pAdapter(pAdapter),
    m_Interface(pArgs->Interface),
    m_hRTDevice(pArgs->hRTDevice),
    m_pUMCallbacks(pArgs->p12UMCallbacks),
    m_pKMCallbacks(pArgs->pKTCallbacks)
{
    assert(pArgs->hDrvDevice.pDrvPrivate == (void *) this);

    assert(m_Interface == D3D12DDI_INTERFACE_VERSION_R3);

    STOP_IN_FUNCTION();
}

void CosUmd12Device::Standup()
{
}

//----------------------------------------------------------------------------------------------------------------------------------
void CosUmd12Device::Teardown()
{
}

//----------------------------------------------------------------------------------------------------------------------------------
CosUmd12Device::~CosUmd12Device()
{
    // do nothing
}
