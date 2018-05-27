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
    D3D12DDI_SUPPORTED_0034
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
    HRESULT hr = S_OK;

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
#endif

    case (D3D12DDICAPS_TYPE_3DPIPELINESUPPORT):
    {
        assert(pCaps->DataSize == sizeof(D3D12DDI_3DPIPELINELEVEL));
        D3D12DDI_3DPIPELINELEVEL* pPipelineCaps = (D3D12DDI_3DPIPELINELEVEL*)pCaps->pData;
        *pPipelineCaps = D3D12DDI_3DPIPELINELEVEL_12_1;
        break;
    }

    case (D3D12DDICAPS_TYPE_SHADER):
    {
        if (pCaps->DataSize != sizeof(D3D12DDI_SHADER_CAPS_0015))
        {
            assert(0);
            return E_UNEXPECTED;
        }

        // TODO: Research these caps
        D3D12DDI_SHADER_CAPS_0015* pShaderCaps = (D3D12DDI_SHADER_CAPS_0015*)pCaps->pData;
        pShaderCaps->MinPrecision = D3D12DDI_SHADER_MIN_PRECISION_NONE;
        pShaderCaps->DoubleOps = FALSE;
        pShaderCaps->ShaderSpecifiedStencilRef = FALSE;
        pShaderCaps->TypedUAVLoadAdditionalFormats = FALSE;
        pShaderCaps->ROVs = FALSE;
        pShaderCaps->WaveOps = FALSE;
        pShaderCaps->WaveLaneCountMin = 1;
        pShaderCaps->WaveLaneCountMax = 1;
        pShaderCaps->TotalLaneCount = 1;
        pShaderCaps->Int64Ops = FALSE;

    } return S_OK;

    case D3D12DDICAPS_TYPE_D3D12_OPTIONS:
    {
        // TODO: Review these options with Amar/D3D Team in the context of compute only

        assert(pCaps->DataSize == sizeof(D3D12DDI_D3D12_OPTIONS_DATA_0033));
        D3D12DDI_D3D12_OPTIONS_DATA_0033* pOptions = (D3D12DDI_D3D12_OPTIONS_DATA_0033*)pCaps->pData;

        // TODO: Find out what is required to meet D3D12DDI_RESOURCE_BINDING_TIER_1
        pOptions->ResourceBindingTier = D3D12DDI_RESOURCE_BINDING_TIER_1;

        pOptions->TiledResourcesTier = D3D12DDI_TILED_RESOURCES_TIER_NOT_SUPPORTED;

        pOptions->CrossNodeSharingTier = D3D12DDI_CROSS_NODE_SHARING_TIER_NOT_SUPPORTED;

        pOptions->VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation = FALSE;

        pOptions->OutputMergerLogicOp = FALSE;

        // TODO: Find out what is required to meet D3D12DDI_RESOURCE_HEAP_TIER_1
        pOptions->ResourceHeapTier = D3D12DDI_RESOURCE_HEAP_TIER_1;

        pOptions->DepthBoundsTestSupported = FALSE;

        pOptions->ProgrammableSamplePositionsTier = D3D12DDI_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED;

        pOptions->CopyQueueTimestampQueriesSupported = false;

        // TODO: What are these flags?
        pOptions->WriteBufferImmediateQueueFlags = D3D12DDI_COMMAND_QUEUE_FLAG_3D | D3D12DDI_COMMAND_QUEUE_FLAG_COMPUTE; //TODO: Copy Support

        pOptions->ViewInstancingTier = D3D12DDI_VIEW_INSTANCING_TIER_NOT_SUPPORTED; //TODO: Support

        pOptions->BarycentricsSupported = false;

        pOptions->ConservativeRasterizationTier = D3D12DDI_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;

        break;
    }

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

#if 0
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

        default:
        {
            DebugBreak();
            hr = E_NOTIMPL;
            break;
        }
    }

    return hr;

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
        case D3D12DDI_TABLE_TYPE_DEVICE_CORE:
        {
            if (tableSize < sizeof(g_CosUmd12Device_Ddi_0033))
            {
                assert(0);
                hr = E_INVALIDARG;
            }
            else
            {
                memcpy(pTable, (void *) &g_CosUmd12Device_Ddi_0033, sizeof(g_CosUmd12Device_Ddi_0033));
            }
            break;
        }
        case D3D12DDI_TABLE_TYPE_COMMAND_LIST_3D:
        {
            if (uTableNum == D3D12_COMMAND_LIST_TYPE_COMPUTE)
            {
                memcpy(pTable, (void*) &g_CosUmd12ComputeCommandList_Ddi_0033, sizeof(g_CosUmd12ComputeCommandList_Ddi_0033));
            }
            else if (uTableNum == D3D12_COMMAND_LIST_TYPE_DIRECT || uTableNum == D3D12_COMMAND_LIST_TYPE_BUNDLE)
            {
                memcpy(pTable, (void*) &g_CosUmd12CommandList_Ddi_0033, sizeof(g_CosUmd12CommandList_Ddi_0033));
            }
            else
            {
                DebugBreak();
            }
            break;
        }
        case D3D12DDI_TABLE_TYPE_COMMAND_QUEUE_3D:
        {
            if (tableSize < sizeof(g_CosUmd12CommandQueue_Ddi_0001))
            {
                assert(0);
                hr = E_INVALIDARG;
            }
            else
            {
                memcpy(pTable, (void *) &g_CosUmd12CommandQueue_Ddi_0001, sizeof(g_CosUmd12CommandQueue_Ddi_0001));
            }
            break;            
        }
        case D3D12DDI_TABLE_TYPE_DXGI:
        {
            if (tableSize < sizeof(g_CosUmd12Dxgi_Ddi))
            {
                assert(0);
                hr = E_INVALIDARG;
            }
            else
            {
                memcpy(pTable, (void *) &g_CosUmd12Dxgi_Ddi, sizeof(g_CosUmd12Dxgi_Ddi));
            }
            break;            
        }
        case D3D12DDI_TABLE_TYPE_0020_EXTENDED_FEATURES:
        {
            if (tableSize < sizeof(g_CosUmd12ExtendedFeatures_Ddi_0020))
            {
                assert(0);
                hr = E_INVALIDARG;
            }
            else
            {
                memcpy(pTable, (void *) &g_CosUmd12ExtendedFeatures_Ddi_0020, sizeof(g_CosUmd12ExtendedFeatures_Ddi_0020));
            }
            break;            
        }

        case D3D12DDI_TABLE_TYPE_0020_DEVICE_VIDEO:
        case D3D12DDI_TABLE_TYPE_0020_DEVICE_CORE_VIDEO:
        case D3D12DDI_TABLE_TYPE_0020_PASS_EXPERIMENT:
        case D3D12DDI_TABLE_TYPE_0021_SHADERCACHE_CALLBACKS:
        case D3D12DDI_TABLE_TYPE_0022_COMMAND_QUEUE_VIDEO_DECODE:
        case D3D12DDI_TABLE_TYPE_0022_COMMAND_LIST_VIDEO_DECODE:
        case D3D12DDI_TABLE_TYPE_0022_COMMAND_QUEUE_VIDEO_PROCESS:
        case D3D12DDI_TABLE_TYPE_0022_COMMAND_LIST_VIDEO_PROCESS:
        case D3D12DDI_TABLE_TYPE_0030_DEVICE_CONTENT_PROTECTION_RESOURCES:
        case D3D12DDI_TABLE_TYPE_0030_CONTENT_PROTECTION_CALLBACKS:
        case D3D12DDI_TABLE_TYPE_0030_DEVICE_CONTENT_PROTECTION_STREAMING:
        {
            DebugBreak();
            hr = E_UNEXPECTED;
        }

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

extern "C"
{
    __declspec(dllexport) HRESULT APIENTRY OpenAdapter12(D3D12DDIARG_OPENADAPTER* pArgs)
    {
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
}
