#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Adapter
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "CosAdapter.h"

//==================================================================================================================================
//
// CosUmdAdapter
//
//==================================================================================================================================

class CosUmd12Adapter
{
public:

    CosUmd12Adapter();
    ~CosUmd12Adapter();

public:
    HRESULT Open( D3D12DDIARG_OPENADAPTER* pArgs );
    void Close();
    static CosUmd12Adapter* CastFrom( D3D10DDI_HADAPTER );
    D3D10DDI_HADAPTER CastTo() const;

public: // DDI entry points for the DDI function table:
    static SIZE_T APIENTRY CalcPrivateDeviceSize( D3D12DDI_HADAPTER hAdapter, const D3D12DDIARG_CALCPRIVATEDEVICESIZE* pArgs);
    static HRESULT APIENTRY CreateDevice(D3D12DDI_HADAPTER hAdapter, const D3D12DDIARG_CREATEDEVICE_0003* pArgs);
    static HRESULT APIENTRY CloseAdapter( D3D12DDI_HADAPTER );
    static HRESULT APIENTRY GetSupportedVersions( D3D12DDI_HADAPTER hAdapter, UINT32* puEntries, UINT64* pSupportedDDIInterfaceVersions );
    static HRESULT APIENTRY GetCaps( D3D12DDI_HADAPTER hAdapter, __in const D3D12DDIARG_GETCAPS* pCaps );
    static HRESULT APIENTRY GetOptionalDdiTables(D3D12DDI_HADAPTER hAdapter, UINT32* puEntries, D3D12DDI_TABLE_REQUEST* pRequests);
    static HRESULT APIENTRY FillDdiTable(D3D12DDI_HADAPTER hAdapter, D3D12DDI_TABLE_TYPE tableType, void * pTable, SIZE_T tableSize, UINT uTableNum, D3D12DDI_HRTTABLE hTable);
    static VOID APIENTRY DestroyDevice(D3D12DDI_HDEVICE hDevice);

public:
    D3D10DDI_HRTADAPTER m_hRTAdapter;
    UINT m_Interface;
    UINT m_Version;
    const D3DDDI_ADAPTERCALLBACKS* m_pMSCallbacks;

    COSADAPTERINFO  m_cosAdapterInfo;

    enum TableType { Compute, Render };
    D3D12DDI_HRTTABLE m_hRTTable[2];
};

//----------------------------------------------------------------------------------------------------------------------------------
inline CosUmd12Adapter* CosUmd12Adapter::CastFrom( D3D10DDI_HADAPTER hAdapter )
{
    return static_cast< CosUmd12Adapter* >( hAdapter.pDrvPrivate );
}

//----------------------------------------------------------------------------------------------------------------------------------
inline D3D10DDI_HADAPTER CosUmd12Adapter::CastTo() const
{
    return MAKE_D3D10DDI_HADAPTER( const_cast< CosUmd12Adapter* >( this ) );
}

// Forward declaration
class CosUmdDevice;

