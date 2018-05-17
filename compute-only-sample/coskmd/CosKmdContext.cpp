#include "precomp.h"

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
        memcpy(pRender->pDmaBuffer, pRender->pCommand, COSD_COMMAND_BUFFER_SIZE);
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
    pDmaBufInfo->m_DmaBufferPhysicalAddress.QuadPart = 0;
    pDmaBufInfo->m_DmaBufferSize = pRender->DmaSize;

#if VC4
    pDmaBufInfo->m_pRenderTarget = NULL;
#endif

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

#if VC4
    if (pCmdBufHeader->m_commandBufferHeader.m_hasVC4ClearColors)
    {
        pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors = 1;
        pDmaBufInfo->m_VC4ClearColors = pCmdBufHeader->m_commandBufferHeader.m_vc4ClearColors;
    }
#endif

    // Perform pre-patch
    pCosKmAdapter->PatchDmaBuffer(
        pDmaBufInfo,
        pRender->pAllocationList,
        pRender->AllocationListSize,
        pPatchLocationList,
        pRender->PatchLocationListInSize);

    // Must update pDmaBuffer to reflect what space we used
    pRender->pDmaBuffer = (char *)pRender->pDmaBuffer + pRender->CommandLength;

    return STATUS_SUCCESS;
}

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

    pContextInfo->DmaBufferSegmentSet = 1 << (COSD_SEGMENT_APERTURE - 1);
    pContextInfo->DmaBufferSize = COSD_COMMAND_BUFFER_SIZE;
    pContextInfo->DmaBufferPrivateDataSize = sizeof(COSDMABUFINFO);

    if (pCreateContext->Flags.GdiContext)
    {
        pContextInfo->AllocationListSize = DXGK_ALLOCATION_LIST_SIZE_GDICONTEXT;
        pContextInfo->PatchLocationListSize = DXGK_ALLOCATION_LIST_SIZE_GDICONTEXT;
    }
    else
    {
        pContextInfo->AllocationListSize = C_COSD_ALLOCATION_LIST_SIZE;
        pContextInfo->PatchLocationListSize = C_COSD_PATCH_LOCATION_LIST_SIZE;
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
CosKmContext::RenderKm(
    INOUT_PDXGKARG_RENDER   pRender)
{
    pRender;

    NT_ASSERT(FALSE);
    return STATUS_SUCCESS;
}

COS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS CosKmContext::Present (DXGKARG_PRESENT* Args)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    COS_LOG_WARNING(
        "Kernel-mode DxgkDdiPresent should not be called in direct-flip case! (Flags=%s %s %s %s %s %s %s %s)",
        Args->Flags.Blt ? "Blt" : "",
        Args->Flags.ColorFill ? "ColorFill" : "",
        Args->Flags.Flip ? "Flip" : "",
        Args->Flags.FlipWithNoWait ? "FlipWithNoWait" : "",
        Args->Flags.SrcColorKey ? "SrcColorKey" : "",
        Args->Flags.DstColorKey ? "DstColorKey" : "",
        Args->Flags.LinearToSrgb ? "LinearToSrgb" : "",
        Args->Flags.Rotate ? "Rotate" : "");

    return STATUS_SUCCESS;
}

COS_PAGED_SEGMENT_END; //=====================================================
