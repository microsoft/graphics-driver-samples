#include "precomp.h"

#include "RosUmdLogging.h"
#include "RosUmdAdapter.tmh"

#include "RosUmdAdapter.h"
#include "RosUmdDevice.h"
#include "RosAdapter.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Adapter implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RosUmdAdapter::RosUmdAdapter()
{
    RosUmdLogging::Call(__FUNCTION__);

    // do nothing
}

RosUmdAdapter::~RosUmdAdapter()
{
    RosUmdLogging::Call(__FUNCTION__);

    // do nothing
}

//----------------------------------------------------------------------------------------------------------------------------------
void RosUmdAdapter::Open( D3D10DDIARG_OPENADAPTER* pArgs )
{

    m_hRTAdapter = pArgs->hRTAdapter;
    m_Interface = pArgs->Interface;
    m_Version = pArgs->Version;
    m_pMSCallbacks = pArgs->pAdapterCallbacks;

    D3DDDICB_QUERYADAPTERINFO   queryAdapterInfo = { 0 };
    HRESULT hr;

    queryAdapterInfo.pPrivateDriverData = &m_rosAdapterInfo;
    queryAdapterInfo.PrivateDriverDataSize = sizeof(m_rosAdapterInfo);

    hr = m_pMSCallbacks->pfnQueryAdapterInfoCb(
        m_hRTAdapter.handle,
        &queryAdapterInfo);
    if (FAILED(hr))
    {
        throw RosUmdException(hr);
    }

    const D3D10_2DDI_ADAPTERFUNCS AdapterFuncs =
    {
        CalcPrivateDeviceSize,
        CreateDevice,
        CloseAdapter,
        GetSupportedVersions,
        GetCaps,
    };

    *pArgs->pAdapterFuncs_2 = AdapterFuncs;

    pArgs->hAdapter = CastTo();
}

//----------------------------------------------------------------------------------------------------------------------------------
void RosUmdAdapter::Close()
{
    RosUmdLogging::Call(__FUNCTION__);

    // do nothing
}

//----------------------------------------------------------------------------------------------------------------------------------
HRESULT APIENTRY RosUmdAdapter::CreateDevice(
    D3D10DDI_HADAPTER hAdapter,
    D3D10DDIARG_CREATEDEVICE* pArgs )
{
    RosUmdAdapter* pThis = RosUmdAdapter::CastFrom( hAdapter );

    RosUmdDevice * pRosUmdDevice = new (pArgs->hDrvDevice.pDrvPrivate) RosUmdDevice(pThis, pArgs);

    try
    {
        pRosUmdDevice->Standup();
    }

    catch( RosUmdException & e )
    {
        pRosUmdDevice->~RosUmdDevice();
        return e.m_hr;
    }

    return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------------------
HRESULT APIENTRY RosUmdAdapter::CloseAdapter( D3D10DDI_HADAPTER hAdapter )
{
    RosUmdAdapter* pAdapter = RosUmdAdapter::CastFrom( hAdapter );
    pAdapter->Close();
    delete pAdapter;

    return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------------------
SIZE_T APIENTRY RosUmdAdapter::CalcPrivateDeviceSize(
    D3D10DDI_HADAPTER hAdapter,
    const D3D10DDIARG_CALCPRIVATEDEVICESIZE* pArgs )
{
    pArgs;

    RosUmdAdapter* pThis = RosUmdAdapter::CastFrom( hAdapter );
    pThis;

    return sizeof( RosUmdDevice );
}

//----------------------------------------------------------------------------------------------------------------------------------
// List of DDIs ref is compatible with.
const UINT64 c_aSupportedVersions[] =
{
    D3DWDDM1_3_DDI_SUPPORTED
};

HRESULT APIENTRY RosUmdAdapter::GetSupportedVersions(
    D3D10DDI_HADAPTER hAdapter,
    __inout UINT32* puEntries,
    __out_ecount_part_opt( *puEntries, *puEntries ) UINT64* pSupportedDDIInterfaceVersions )
{
    RosUmdAdapter* pAdapter = RosUmdAdapter::CastFrom( hAdapter );
    pAdapter;

    UINT32 uEntries = ARRAYSIZE( c_aSupportedVersions );
    const UINT64* pSupportedVersions = c_aSupportedVersions;

    if (pSupportedDDIInterfaceVersions &&
        *puEntries < uEntries)
    {
        return HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    *puEntries = uEntries;
    if (pSupportedDDIInterfaceVersions)
    {
        UINT64* pCurEntry = pSupportedDDIInterfaceVersions;
        memcpy( pCurEntry, pSupportedVersions, uEntries * sizeof( *pSupportedVersions ) );
    }
    return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------------------
HRESULT APIENTRY RosUmdAdapter::GetCaps(
    D3D10DDI_HADAPTER hAdapter,
    __in const D3D10_2DDIARG_GETCAPS* pCaps )
{
    hAdapter;

    switch (pCaps->Type)
    {
    case (D3D11DDICAPS_THREADING):
        {
            if (pCaps->DataSize != sizeof( D3D11DDI_THREADING_CAPS ))
            {
                return E_UNEXPECTED;
            }

            D3D11DDI_THREADING_CAPS* pData = static_cast< D3D11DDI_THREADING_CAPS* >( pCaps->pData );
            pData->Caps = 0;
        } return S_OK;

    case (D3D11DDICAPS_3DPIPELINESUPPORT):
        {
            if (pCaps->DataSize != sizeof( D3D11DDI_3DPIPELINESUPPORT_CAPS ))
            {
                return E_UNEXPECTED;
            }

            D3D11DDI_3DPIPELINESUPPORT_CAPS* pData = static_cast< D3D11DDI_3DPIPELINESUPPORT_CAPS* >( pCaps->pData );
            // Ref11 supports pipeline levels 9.1, 9.2, 9.3, 10, 10.1, 11, 11.1
            pData->Caps =
#if !VC4 // VC4 only supports up to FL9_3.
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11_1DDI_3DPIPELINELEVEL_11_1 ) |
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11DDI_3DPIPELINELEVEL_11_0 ) |
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11DDI_3DPIPELINELEVEL_10_1 ) |
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11DDI_3DPIPELINELEVEL_10_0 ) |
#endif // !VC4
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11_1DDI_3DPIPELINELEVEL_9_3 ) | // 9_x are not interesting for IHVs implementing this DDI.
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11_1DDI_3DPIPELINELEVEL_9_2 ) | // For hardware, these levels go through the D3D9 DDI.
                D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP( D3D11_1DDI_3DPIPELINELEVEL_9_1 )
                ;
        } return S_OK;

    case (D3D11DDICAPS_SHADER):
        {
            if (pCaps->DataSize != sizeof( D3D11DDI_SHADER_CAPS ))
            {
                return E_UNEXPECTED;
            }

            D3D11DDI_SHADER_CAPS* pData = static_cast< D3D11DDI_SHADER_CAPS* >( pCaps->pData );
            pData->Caps = D3D11DDICAPS_SHADER_DOUBLES | D3D11DDICAPS_SHADER_COMPUTE_PLUS_RAW_AND_STRUCTURED_BUFFERS_IN_SHADER_4_X;
            pData->Caps |= D3D11DDICAPS_SHADER_DEBUGGABLE;
        } return S_OK;
    case (D3D11_1DDICAPS_D3D11_OPTIONS):
        {
            if (pCaps->DataSize != sizeof( D3D11_1DDI_D3D11_OPTIONS_DATA ))
            {
                return E_UNEXPECTED;
            }
            D3D11_1DDI_D3D11_OPTIONS_DATA* pData = static_cast< D3D11_1DDI_D3D11_OPTIONS_DATA* >( pCaps->pData );
            pData->AssignDebugBinarySupport = TRUE;
            pData->OutputMergerLogicOp = TRUE;
        } return S_OK;
    case (D3D11_1DDICAPS_ARCHITECTURE_INFO):
        {
            if (pCaps->DataSize != sizeof( D3D11_1DDI_ARCHITECTURE_INFO_DATA ))
            {
                return E_UNEXPECTED;
            }
            D3D11_1DDI_ARCHITECTURE_INFO_DATA* pData = static_cast< D3D11_1DDI_ARCHITECTURE_INFO_DATA* >( pCaps->pData );
            pData->TileBasedDeferredRenderer = FALSE;
        } return S_OK;
    case (D3D11_1DDICAPS_SHADER_MIN_PRECISION_SUPPORT):
        {
            if (pCaps->DataSize != sizeof( D3D11_DDI_SHADER_MIN_PRECISION_SUPPORT_DATA ))
            {
                return E_UNEXPECTED;
            }
            D3D11_DDI_SHADER_MIN_PRECISION_SUPPORT_DATA* pData = static_cast< D3D11_DDI_SHADER_MIN_PRECISION_SUPPORT_DATA* >( pCaps->pData );
            pData->PixelShaderMinPrecision = D3D11_DDI_SHADER_MIN_PRECISION_10_BIT | D3D11_DDI_SHADER_MIN_PRECISION_16_BIT;
            pData->AllOtherStagesMinPrecision = D3D11_DDI_SHADER_MIN_PRECISION_10_BIT | D3D11_DDI_SHADER_MIN_PRECISION_16_BIT;
        } return S_OK;
    default: return E_NOTIMPL;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
HRESULT APIENTRY OpenAdapter10_2( D3D10DDIARG_OPENADAPTER* pArgs )
{
    RosUmdAdapter* pAdapter = new RosUmdAdapter;
    if( NULL == pAdapter )
    {
        return E_OUTOFMEMORY;
    }

    try
    {
        pAdapter->Open(pArgs);
    }

    catch (RosUmdException & e)
    {
        pAdapter->Close();
        delete pAdapter;
        return e.m_hr;
    }

    return S_OK;
}
