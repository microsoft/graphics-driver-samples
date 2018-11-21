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

    memset(m_hRTTable, 0, sizeof(m_hRTTable));

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
    D3D12DDI_SUPPORTED_0052
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

    case D3D12DDICAPS_TYPE_TEXTURE_LAYOUT:
    case D3D12DDICAPS_TYPE_0022_TEXTURE_LAYOUT:
    {
        if (pCaps->DataSize != sizeof(D3D12DDI_TEXTURE_LAYOUT_CAPS_0026)) {
            hr = E_UNEXPECTED;
        } else {
            D3D12DDI_TEXTURE_LAYOUT_CAPS_0026* pLayoutCaps = (D3D12DDI_TEXTURE_LAYOUT_CAPS_0026*)pCaps->pData;
            pLayoutCaps->DeviceDependentLayoutCount = 0;
            pLayoutCaps->DeviceDependentSwizzleCount = 0;
            pLayoutCaps->Supports64KStandardSwizzle = FALSE;
            pLayoutCaps->SupportsRowMajorTexture = TRUE;
            pLayoutCaps->IndexableSwizzlePatterns = FALSE;
        }
        break;
    }

#if 0
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

        //
        // Pipeline Level maps to API Feature Level
        //
        // Each Pipeline Level has a set of minimal requirements for features
        // or capabilities that HW and driver are required to support
        //

        *pPipelineCaps = D3D12DDI_3DPIPELINELEVEL_1_0_CORE;
        break;
    }

    case (D3D12DDICAPS_TYPE_SHADER):
    {
        if (pCaps->DataSize != sizeof(D3D12DDI_SHADER_CAPS_0042))
        {
            hr = E_UNEXPECTED;
        } else {
            // TODO: Research these caps
            D3D12DDI_SHADER_CAPS_0042* pShaderCaps = (D3D12DDI_SHADER_CAPS_0042*)pCaps->pData;
            pShaderCaps->MinPrecision = D3D12DDI_SHADER_MIN_PRECISION_NONE;
            pShaderCaps->DoubleOps = FALSE;
            pShaderCaps->ShaderSpecifiedStencilRef = FALSE;
            pShaderCaps->TypedUAVLoadAdditionalFormats = TRUE; // FL 12.0+ must support
            pShaderCaps->ROVs = TRUE; // FL 12.1+ must support
            pShaderCaps->WaveOps = FALSE;
            pShaderCaps->WaveLaneCountMin = 4;  // min required support is 4
            pShaderCaps->WaveLaneCountMax = 4;
            pShaderCaps->TotalLaneCount = 4;
            pShaderCaps->Int64Ops = FALSE;
            pShaderCaps->Native16BitOps = FALSE;
        }

        break;
    }

    case D3D12DDICAPS_TYPE_D3D12_OPTIONS:
    {
        // TODO: Review these options with Amar/D3D Team in the context of compute only

        assert(pCaps->DataSize == sizeof(D3D12DDI_D3D12_OPTIONS_DATA_0052));
        D3D12DDI_D3D12_OPTIONS_DATA_0052* pOptions = (D3D12DDI_D3D12_OPTIONS_DATA_0052*)pCaps->pData;

#if COS_USE_RESOURCE_BINDING_TIER_1
        
        //
        // D3D12DDI_RESOURCE_BINDING_TIER_1 is the mininal requirement for
        // resource binding on D3D12DDI_3DPIPELINELEVEL_11_0 HW/driver
        //

        pOptions->ResourceBindingTier = D3D12DDI_RESOURCE_BINDING_TIER_1;

        pOptions->TiledResourcesTier = D3D12DDI_TILED_RESOURCES_TIER_NOT_SUPPORTED;

#else

        pOptions->ResourceBindingTier = D3D12DDI_RESOURCE_BINDING_TIER_2; // FL 12.0+  must report tier+

        pOptions->TiledResourcesTier = D3D12DDI_TILED_RESOURCES_TIER_2; // FL 12.0+ must report tier 2+

#endif
        pOptions->CrossNodeSharingTier = D3D12DDI_CROSS_NODE_SHARING_TIER_NOT_SUPPORTED;

        pOptions->VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation = FALSE;

        pOptions->OutputMergerLogicOp = TRUE;   //  feature level 11.1+ must report support

        // TODO: Find out what is required to meet D3D12DDI_RESOURCE_HEAP_TIER_1
        pOptions->ResourceHeapTier = D3D12DDI_RESOURCE_HEAP_TIER_1;

        pOptions->DepthBoundsTestSupported = FALSE;

        pOptions->ProgrammableSamplePositionsTier = D3D12DDI_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED;

        pOptions->CopyQueueTimestampQueriesSupported = false;

        // TODO: What are these flags?
        pOptions->WriteBufferImmediateQueueFlags = D3D12DDI_COMMAND_QUEUE_FLAG_3D | D3D12DDI_COMMAND_QUEUE_FLAG_COMPUTE; //TODO: Copy Support

        pOptions->ViewInstancingTier = D3D12DDI_VIEW_INSTANCING_TIER_NOT_SUPPORTED; //TODO: Support

        pOptions->BarycentricsSupported = false;

        pOptions->ConservativeRasterizationTier = D3D12DDI_CONSERVATIVE_RASTERIZATION_TIER_1; // FL12.1+ must support tier1+

        pOptions->ReservedBufferPlacementSupported = false;
        pOptions->Deterministic64KBUndefinedSwizzle = false;
        pOptions->SRVOnlyTiledResourceTier3 = false;

        break;
    }

#if 0
    case (D3D11_1DDICAPS_D3D11_OPTIONS):
    {
        if (pCaps->DataSize != sizeof(D3D11_1DDI_D3D11_OPTIONS_DATA))
        {
            hr = E_UNEXPECTED;
        } else {
            D3D11_1DDI_D3D11_OPTIONS_DATA* pData = static_cast<D3D11_1DDI_D3D11_OPTIONS_DATA*>(pCaps->pData);
            pData->AssignDebugBinarySupport = TRUE;
            pData->OutputMergerLogicOp = TRUE;
        }
    } return S_OK;
#endif

    case (D3D12DDICAPS_TYPE_ARCHITECTURE_INFO):
    {
        if (pCaps->DataSize != sizeof(D3D12DDI_ARCHITECTURE_INFO_DATA))
        {
            hr = E_UNEXPECTED;
        } else {
            D3D12DDI_ARCHITECTURE_INFO_DATA* pData = static_cast<D3D12DDI_ARCHITECTURE_INFO_DATA*>(pCaps->pData);
            pData->TileBasedDeferredRenderer = FALSE;
        }
        break;
    }

#if 0
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

        case D3D12DDICAPS_TYPE_0011_SHADER_MODELS:
        {
            if (pCaps->DataSize < sizeof(D3D12DDI_D3D12_SHADER_MODELS_DATA_0011)) {
                hr = E_UNEXPECTED;
            } else {
                D3D12DDI_D3D12_SHADER_MODELS_DATA_0011* pModelCaps = (D3D12DDI_D3D12_SHADER_MODELS_DATA_0011*)pCaps->pData;

                static const D3D12DDI_SHADER_MODEL supportedModels[] =
                {
                    D3D12DDI_SHADER_MODEL_5_1_RELEASE_0011,
                    D3D12DDI_SHADER_MODEL_6_0_RELEASE_0011,
                };

                if (pModelCaps->pShaderModelsSupported != nullptr)
                {
                    CopyMemory(pModelCaps->pShaderModelsSupported, supportedModels, sizeof(supportedModels));
                }
                *pModelCaps->pNumShaderModelsSupported = _countof(supportedModels);
            }
            break;
        }

        case D3D12DDICAPS_TYPE_0023_UMD_BASED_COMMAND_QUEUE_PRIORITY:
        {
            if (pCaps->DataSize < sizeof(D3D12DDICAPS_UMD_BASED_COMMAND_QUEUE_PRIORITY_DATA_0023)) {
                hr = E_UNEXPECTED;
            } else {
                D3D12DDICAPS_UMD_BASED_COMMAND_QUEUE_PRIORITY_DATA_0023* pData = 
                    (D3D12DDICAPS_UMD_BASED_COMMAND_QUEUE_PRIORITY_DATA_0023*)pCaps->pData;
                pData->SupportedQueueFlagsForGlobalRealtimeQueues = D3D12DDI_COMMAND_QUEUE_FLAG_NONE;
            }
            break;
        }

        case D3D12DDICAPS_TYPE_MEMORY_ARCHITECTURE:
        {
            ASSERT(pCaps->DataSize == sizeof(D3D12DDI_MEMORY_ARCHITECTURE_CAPS_0041));
            D3D12DDI_MEMORY_ARCHITECTURE_CAPS_0041* pMemoryCaps = (D3D12DDI_MEMORY_ARCHITECTURE_CAPS_0041*)pCaps->pData;
            pMemoryCaps->UMA = TRUE;
            pMemoryCaps->IOCoherent = TRUE;
            pMemoryCaps->CacheCoherent = TRUE;
            pMemoryCaps->HeapSerializationTier = D3D12DDI_HEAP_SERIALIZATION_TIER_0041_0;
            pMemoryCaps->ResourceSerializationTier = D3D12DDI_RESOURCE_SERIALIZATION_TIER_0041_0;
            break;
        }

        case D3D12DDICAPS_TYPE_TEXTURE_LAYOUT_SETS:
        {
            ASSERT(pCaps->pInfo != nullptr);
            const UINT* pInfo = (UINT*)pCaps->pInfo;
            if (pInfo[0] == D3D12DDI_TL_ROW_MAJOR &&
                pInfo[1] == D3D12DDI_FUNCUNIT_COMBINED)
            {
                ASSERT(pCaps->DataSize == sizeof(D3D12DDI_ROW_MAJOR_LAYOUT_CAPS));
                const UINT textureAlignment = 1;
                D3D12DDI_ROW_MAJOR_LAYOUT_CAPS* pLayoutCaps = (D3D12DDI_ROW_MAJOR_LAYOUT_CAPS*)pCaps->pData;
                pLayoutCaps->SubCaps[0].MaxElementSize = 0xFFFF;
                pLayoutCaps->SubCaps[0].BaseOffsetAlignment = textureAlignment;
                pLayoutCaps->SubCaps[0].PitchAlignment = textureAlignment;
                pLayoutCaps->SubCaps[0].DepthPitchAlignment = textureAlignment;
                ZeroMemory(&pLayoutCaps->SubCaps[1], sizeof(D3D12DDI_ROW_MAJOR_LAYOUT_SUB_CAPS));
                pLayoutCaps->Flags = D3D12DDI_ROW_MAJOR_LAYOUT_FLAG_NONE;
            }
            else
            {
                hr = E_NOTIMPL;
            }
            break;
        }

        case D3D12DDICAPS_TYPE_0022_CPU_PAGE_TABLE_FALSE_POSITIVES:
        {
            // TODO: What is this?
            ASSERT(pCaps->DataSize == sizeof(D3D12DDI_COMMAND_QUEUE_FLAGS));
            D3D12DDI_COMMAND_QUEUE_FLAGS* pFlags = (D3D12DDI_COMMAND_QUEUE_FLAGS*)pCaps->pData;
            *pFlags = D3D12DDI_COMMAND_QUEUE_FLAG_COMPUTE;
            break;
        }

        case D3D12DDICAPS_TYPE_GPUVA_CAPS:
        {
            // TODO: Learn about this caps call - do we have to support it?
            ASSERT(pCaps->DataSize == sizeof(D3D12DDI_GPUVA_CAPS_0004));
            D3D12DDI_GPUVA_CAPS_0004* pCpuVaCaps = (D3D12DDI_GPUVA_CAPS_0004*)pCaps->pData;
            pCpuVaCaps->MaxGPUVirtualAddressBitsPerResource = 20;   // TOOD: Should come from kernel - we currently allocate 1Meg (2^20) for video memory
            break;
        }

        case D3D12DDICAPS_TYPE_0050_HARDWARE_SCHEDULING_CAPS:
        {
            D3D12DDICAPS_HARDWARE_SCHEDULING_CAPS_0050* pHwSchedulingCaps = (D3D12DDICAPS_HARDWARE_SCHEDULING_CAPS_0050*)pCaps->pData;
            pHwSchedulingCaps->ComputeQueuesPer3DQueue = 0;
            break;
        }

        default:
        {
            TRACE_FUNCTION();
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
            if (tableSize < sizeof(g_CosUmd12Device_Ddi_0052))
            {
                assert(0);
                hr = E_INVALIDARG;
            }
            else
            {
                memcpy(pTable, (void *) &g_CosUmd12Device_Ddi_0052, sizeof(g_CosUmd12Device_Ddi_0052));
            }
            break;
        }
        case D3D12DDI_TABLE_TYPE_COMMAND_LIST_3D:
        {
            // TODO: Talk with Jesse about where we indicate how many tables will get filled out and what they correspond to?  Is this documented somewhere?

            assert(uTableNum <= Render);

            memcpy(pTable, (void*)&g_CosUmd12ComputeCommandList_Ddi_0052, sizeof(g_CosUmd12ComputeCommandList_Ddi_0052));
            pAdapter->m_hRTTable[uTableNum] = hTable;

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
            TRACE_FUNCTION();
            hr = E_UNEXPECTED;
        }

        default:
        {
            TRACE_FUNCTION();
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
