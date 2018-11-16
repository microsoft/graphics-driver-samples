#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdContext.tmh"

#include "CosKmdAdapter.h"
#include "CosKmdDevice.h"
#include "CosKmdContext.h"
#include "CosKmdAllocation.h"
#include "CosGpuCommand.h"
#include "CosKmdGlobal.h"
#include "CosKmdUtil.h"

NTSTATUS
__stdcall
CosKmContext::DdiRender(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_RENDER   pRender)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "CosKmdRender hAdapter=%lx\n", hContext);

    CosKmContext  *pCosKmContext = CosKmContext::Cast(hContext);
    CosKmAdapter  *pCosKmAdapter = pCosKmContext->m_pDevice->m_pCosKmAdapter;

    pRender->MultipassOffset;
    pRender->DmaBufferSegmentId;
    pRender->DmaBufferPhysicalAddress;
    pRender->pDmaBufferPrivateData;
    pRender->DmaBufferPrivateDataSize;

    // Copy command buffer
    NT_ASSERT(pRender->DmaSize >= pRender->CommandLength);

#ifdef SSR_END_DMA
    __try {
        memcpy(pRender->pDmaBuffer, pRender->pCommand, COS_COMMAND_BUFFER_SIZE);
    }
#else
    __try {
        memcpy(pRender->pDmaBuffer, pRender->pCommand, pRender->CommandLength);
    }
#endif
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        COS_LOG_ERROR("Caught structured exception while copying usermode buffer.");
        return STATUS_INVALID_PARAMETER;
    }

    // Check to command buffer header
    GpuCommand *    pCmdBufHeader = (GpuCommand *)pRender->pDmaBuffer;
    if (pCmdBufHeader->m_commandId != Header)
    {
        COS_LOG_ERROR(
            "Invalid command buffer header. (pCmdBufHeader->m_commandId=0x%x, Header=0x%x)",
            pCmdBufHeader->m_commandId,
            Header);
        return STATUS_ILLEGAL_INSTRUCTION;
    }

    // Copy path list
    NT_ASSERT(pRender->PatchLocationListOutSize >= pRender->PatchLocationListInSize);

    D3DDDI_PATCHLOCATIONLIST * pPatchLocationList = pRender->pPatchLocationListOut;

    __try {
        for (UINT i = 0; i < pRender->PatchLocationListInSize; i++)
        {
            pPatchLocationList[i] = pRender->pPatchLocationListIn[i];
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        COS_LOG_ERROR("Caught structured exception while copying usermode buffer.");
        return STATUS_INVALID_PARAMETER;
    }

    // Must update pPatchLocationListOut to reflect what space was used
    pRender->pPatchLocationListOut += pRender->PatchLocationListInSize;

    // Record DMA buffer information
    COSDMABUFINFO * pDmaBufInfo = (COSDMABUFINFO *)pRender->pDmaBufferPrivateData;

    pDmaBufInfo->m_DmaBufState.m_Value = 0;
    pDmaBufInfo->m_DmaBufState.m_bRender = 1;
    pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer = pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer;

    pDmaBufInfo->m_pDmaBuffer = (PBYTE)pRender->pDmaBuffer;
#if COS_PHYSICAL_SUPPORT
    pDmaBufInfo->m_DmaBufferPhysicalAddress.QuadPart = 0;
#else
    pDmaBufInfo->m_DmaBufferGpuVa = 0;
#endif
    pDmaBufInfo->m_DmaBufferSize = pRender->DmaSize;

    // Validate DMA buffer
    bool isValidDmaBuffer;

    isValidDmaBuffer = pCosKmAdapter->ValidateDmaBuffer(
        pDmaBufInfo,
        pRender->pAllocationList,
        pRender->AllocationListSize,
        pPatchLocationList,
        pRender->PatchLocationListInSize);

    if (! isValidDmaBuffer)
    {
        COS_LOG_ERROR("DMA buffer is not valid. (pDmaBufInfo=0x%p)", pDmaBufInfo);
        return STATUS_INVALID_PARAMETER;
    }

#if COS_PHYSICAL_SUPPORT
    // Perform pre-patch
    pCosKmAdapter->PatchDmaBuffer(
        pDmaBufInfo,
        pRender->pAllocationList,
        pRender->AllocationListSize,
        pPatchLocationList,
        pRender->PatchLocationListInSize);
#endif

    // Must update pDmaBuffer to reflect what space we used
    pRender->pDmaBuffer = (char *)pRender->pDmaBuffer + pRender->CommandLength;

    return STATUS_SUCCESS;
}

// GPUVA_INIT_DDI_8
NTSTATUS
__stdcall
CosKmContext::DdiCreateContext(
    IN_CONST_HANDLE                 hDevice,
    INOUT_PDXGKARG_CREATECONTEXT    pCreateContext)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hDevice=%lx\n",
        __FUNCTION__, hDevice);

    CosKmContext  *pCosKmContext;

    pCosKmContext = (CosKmContext *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(CosKmContext), 'COSD');
    if (!pCosKmContext)
    {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(pCosKmContext, sizeof(CosKmContext));

    pCosKmContext->m_magic = kMagic;
    pCosKmContext->m_pDevice = (CosKmDevice *)hDevice;
    pCosKmContext->m_Node = pCreateContext->NodeOrdinal;
    pCosKmContext->m_hRTContext = pCreateContext->hContext;
    pCosKmContext->m_Flags = pCreateContext->Flags;

    //
    // Set up info returned to runtime
    //
    DXGK_CONTEXTINFO   *pContextInfo = &pCreateContext->ContextInfo;

#if COS_GPUVA_SUPPORT

    if (pCreateContext->Flags.VirtualAddressing)
    {
        pContextInfo->Caps.DriverManagesResidency = 1;

        //
        // For SubmitCommandCb from UMD, the private data must be the beginning
        // portion of the KM DMA buffer private data. That implies that the UMD
        // private data size must not exceed the DmaBufferPrivateDataSize here.
        //

        pContextInfo->DmaBufferPrivateDataSize = sizeof(COSDMABUFINFO);
    }
    else
#endif
    {
#if 1
        pContextInfo->DmaBufferSegmentSet = 0;  // Use physical contiguos memory
#else
        pContextInfo->DmaBufferSegmentSet = 1 << (COS_SEGMENT_APERTURE - 1);
#endif
        pContextInfo->DmaBufferSize = COS_COMMAND_BUFFER_SIZE;
        pContextInfo->DmaBufferPrivateDataSize = sizeof(COSDMABUFINFO);

        NT_ASSERT(!pCreateContext->Flags.GdiContext);

        pContextInfo->AllocationListSize = C_COS_ALLOCATION_LIST_SIZE;
        pContextInfo->PatchLocationListSize = C_COS_PATCH_LOCATION_LIST_SIZE;
    }

    pCreateContext->hContext = pCosKmContext;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
CosKmContext::DdiDestroyContext(
    IN_CONST_HANDLE     hContext)
{
    CosKmContext  *pCosKmContext = CosKmContext::Cast(hContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "CosKmdDestroyContext hContext=%lx\n",
        hContext);

    pCosKmContext->~CosKmContext();

    ExFreePool(pCosKmContext);

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
CosKmContext::DdiFormatHistoryBuffer(
    IN_CONST_HANDLE                 hContext,
    IN DXGKARG_FORMATHISTORYBUFFER* pFormatData)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hContext=%lx\n", __FUNCTION__, hContext);

    pFormatData;

    return STATUS_NOT_IMPLEMENTED;
}

#if COS_GPUVA_SUPPORT

VOID
CosKmContext::SetRootPageTable(
    IN_CONST_PDXGKARG_SETROOTPAGETABLE  pSetPageTable)
{
    m_rootPageTableAddress = pSetPageTable->Address;
    m_numRootPageTableEntries = pSetPageTable->NumEntries;
}

#endif

