#include "CosUmd12.h"

#include "CosUmd12Adapter.h"
#include "CosUmd12Device.h"

CosUmd12Adapter::CosUmd12Adapter()
{
    // do nothing
}

CosUmd12Adapter::~CosUmd12Adapter()
{
    // do nothing
}

HRESULT CosUmd12Adapter::Open( D3D12DDIARG_OPENADAPTER* pArgs )
{

    m_hRTAdapter = pArgs->hRTAdapter;
    m_pMSCallbacks = pArgs->pAdapterCallbacks;

    D3DDDICB_QUERYADAPTERINFO   queryAdapterInfo = { 0 };

    queryAdapterInfo.pPrivateDriverData = &m_cosAdapterInfo;
    queryAdapterInfo.PrivateDriverDataSize = sizeof(m_cosAdapterInfo);

    HRESULT hr = m_pMSCallbacks->pfnQueryAdapterInfoCb(
        m_hRTAdapter.handle,
        &queryAdapterInfo);

    if (FAILED(hr)) goto cleanup;

    *pArgs->pAdapterFuncs = {
        CalcPrivateDeviceSize,
        CreateDevice,
        CloseAdapter,
        GetSupportedVersions,
        GetCaps,
        GetOptionalDdiTables,
        FillDdiTable,
        DestroyDevice
    };

    pArgs->hAdapter = CastTo();

cleanup:

    return hr;
}

void CosUmd12Adapter::Close()
{
    // do nothing
}

HRESULT APIENTRY CosUmd12Adapter::CreateDevice(
    D3D12DDI_HADAPTER hAdapter,
    CONST D3D12DDIARG_CREATEDEVICE_0003* pArgs)
{
    CosUmd12Adapter* pThis = CosUmd12Adapter::CastFrom( hAdapter );

    CosUmd12Device * pCosUmdDevice = new (pArgs->hDrvDevice.pDrvPrivate) CosUmd12Device(pThis, pArgs);

    pCosUmdDevice->Standup();

    return S_OK;
}

HRESULT APIENTRY CosUmd12Adapter::CloseAdapter( D3D12DDI_HADAPTER hAdapter )
{
    CosUmd12Adapter* pAdapter = CosUmd12Adapter::CastFrom( hAdapter );
    pAdapter->Close();
    delete pAdapter;

    return S_OK;
}

SIZE_T APIENTRY CosUmd12Adapter::CalcPrivateDeviceSize(
    D3D12DDI_HADAPTER hAdapter,
    const D3D12DDIARG_CALCPRIVATEDEVICESIZE* pArgs )
{
    pArgs;

    CosUmd12Adapter* pThis = CosUmd12Adapter::CastFrom( hAdapter );
    pThis;

    return sizeof( CosUmd12Device );
}

// List of DDIs ref is compatible with.
const UINT64 c_aSupportedVersions[] =
{
    D3DWDDM2_3_DDI_SUPPORTED
};

HRESULT APIENTRY CosUmd12Adapter::GetSupportedVersions(
    D3D12DDI_HADAPTER hAdapter,
    UINT32* puEntries,
    UINT64* pSupportedDDIInterfaceVersions )
{
    CosUmd12Adapter* pAdapter = CosUmd12Adapter::CastFrom( hAdapter );
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

HRESULT APIENTRY CosUmd12Adapter::GetCaps(
    D3D12DDI_HADAPTER hAdapter,
    const D3D12DDIARG_GETCAPS* pCaps )
{
    hAdapter;

    switch (pCaps->Type)
    {

    #if 0
    case D3D12DDICAPS_TYPE_TEXTURE_LAYOUT:
        {
            if (pCaps->DataSize != sizeof(D3D12DDI_TEXTURE_LAYOUT_CAPS_0001))
                return E_UNEXPECTED;

            D3D12DDI_TEXTURE_LAYOUT_CAPS_0001* pData = static_cast<D3D12DDI_TEXTURE_LAYOUT_CAPS_0001*>(pCaps->pData);

            pData->Supports64KStandardSwizzle = FALSE;
            pData->DeviceDependentLayoutCount = 0;
            pData->DeviceDependentSwizzleCount = 0;
            pData->SupportsRowMajorTexture = FALSE;
        }
        break;

    case (D3D11DDICAPS_THREADING):
    {
        if (pCaps->DataSize != sizeof(D3D11DDI_THREADING_CAPS))
        {
            return E_UNEXPECTED;
        }

        D3D11DDI_THREADING_CAPS* pData = static_cast<D3D11DDI_THREADING_CAPS*>(pCaps->pData);
        pData->Caps = 0;
    } return S_OK;

    case (D3D11DDICAPS_3DPIPELINESUPPORT):
    {
        if (pCaps->DataSize != sizeof(D3D11DDI_3DPIPELINESUPPORT_CAPS))
        {
            return E_UNEXPECTED;
        }

        D3D11DDI_3DPIPELINESUPPORT_CAPS* pData = static_cast<D3D11DDI_3DPIPELINESUPPORT_CAPS*>(pCaps->pData);
        // Ref11 supports pipeline levels 9.1, 9.2, 9.3, 10, 10.1, 11, 11.1
        pData->Caps =
#if !VC4 // VC4 only supports up to FL9_3.
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11_1DDI_3DPIPELINELEVEL_11_1) |
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11DDI_3DPIPELINELEVEL_11_0) |
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11DDI_3DPIPELINELEVEL_10_1) |
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11DDI_3DPIPELINELEVEL_10_0) |
#endif // !VC4
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11_1DDI_3DPIPELINELEVEL_9_3) | // 9_x are not interesting for IHVs implementing this DDI.
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11_1DDI_3DPIPELINELEVEL_9_2) | // For hardware, these levels go through the D3D9 DDI.
            D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11_1DDI_3DPIPELINELEVEL_9_1)
            ;
    } return S_OK;

    case (D3D11DDICAPS_SHADER):
    {
        if (pCaps->DataSize != sizeof(D3D11DDI_SHADER_CAPS))
        {
            return E_UNEXPECTED;
        }

        D3D11DDI_SHADER_CAPS* pData = static_cast<D3D11DDI_SHADER_CAPS*>(pCaps->pData);
        pData->Caps = D3D11DDICAPS_SHADER_DOUBLES | D3D11DDICAPS_SHADER_COMPUTE_PLUS_RAW_AND_STRUCTURED_BUFFERS_IN_SHADER_4_X;
        pData->Caps |= D3D11DDICAPS_SHADER_DEBUGGABLE;
    } return S_OK;
    case (D3D11_1DDICAPS_D3D11_OPTIONS):
    {
        if (pCaps->DataSize != sizeof(D3D11_1DDI_D3D11_OPTIONS_DATA))
        {
            return E_UNEXPECTED;
        }
        D3D11_1DDI_D3D11_OPTIONS_DATA* pData = static_cast<D3D11_1DDI_D3D11_OPTIONS_DATA*>(pCaps->pData);
        pData->AssignDebugBinarySupport = TRUE;
        pData->OutputMergerLogicOp = TRUE;
    } return S_OK;
    case (D3D11_1DDICAPS_ARCHITECTURE_INFO):
    {
        if (pCaps->DataSize != sizeof(D3D11_1DDI_ARCHITECTURE_INFO_DATA))
        {
            return E_UNEXPECTED;
        }
        D3D11_1DDI_ARCHITECTURE_INFO_DATA* pData = static_cast<D3D11_1DDI_ARCHITECTURE_INFO_DATA*>(pCaps->pData);
        pData->TileBasedDeferredRenderer = FALSE;
    } return S_OK;
    case (D3D11_1DDICAPS_SHADER_MIN_PRECISION_SUPPORT):
    {
        if (pCaps->DataSize != sizeof(D3D11_DDI_SHADER_MIN_PRECISION_SUPPORT_DATA))
        {
            return E_UNEXPECTED;
        }
        D3D11_DDI_SHADER_MIN_PRECISION_SUPPORT_DATA* pData = static_cast<D3D11_DDI_SHADER_MIN_PRECISION_SUPPORT_DATA*>(pCaps->pData);
        pData->PixelShaderMinPrecision = D3D11_DDI_SHADER_MIN_PRECISION_10_BIT | D3D11_DDI_SHADER_MIN_PRECISION_16_BIT;
        pData->AllOtherStagesMinPrecision = D3D11_DDI_SHADER_MIN_PRECISION_10_BIT | D3D11_DDI_SHADER_MIN_PRECISION_16_BIT;
    } return S_OK;
#endif

        case 123213:
        default:
        {
            DebugBreak();
            return E_NOTIMPL;
        }
    }

    return S_OK;

}

HRESULT APIENTRY CosUmd12Adapter::GetOptionalDdiTables(D3D12DDI_HADAPTER hAdapter, UINT32* puEntries, D3D12DDI_TABLE_REQUEST* pRequests)
{
    CosUmd12Adapter* pAdapter = CosUmd12Adapter::CastFrom( hAdapter );
    pAdapter;

    if (puEntries == NULL)
        return E_INVALIDARG;

    *puEntries = 0;
    return S_OK;
}

HRESULT APIENTRY CosUmd12Adapter::FillDdiTable(D3D12DDI_HADAPTER hAdapter, D3D12DDI_TABLE_TYPE tableType, void * pTable, SIZE_T tableSize, UINT uTableNum, D3D12DDI_HRTTABLE hTable)
{
    CosUmd12Adapter* pAdapter = CosUmd12Adapter::CastFrom( hAdapter );
    pAdapter;

    HRESULT hr = S_OK;

    switch (tableType) {
        case 28938213:
        default:
        {
            DebugBreak();
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

VOID APIENTRY CosUmd12Adapter::DestroyDevice(D3D12DDI_HDEVICE hDevice)
{
    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(hDevice);

    pDevice->~CosUmd12Device();
}


__declspec(dllexport) HRESULT APIENTRY OpenAdapter12( D3D12DDIARG_OPENADAPTER* pArgs )
{
    DebugBreak();

    CosUmd12Adapter* pAdapter = new CosUmd12Adapter;
    if( NULL == pAdapter )
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pAdapter->Open(pArgs);

    if (hr != S_OK) {
        pAdapter->Close();
        delete pAdapter;
    }

    return hr;
}
