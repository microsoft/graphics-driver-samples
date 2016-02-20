#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdContext.tmh"

#include "RosKmdAdapter.h"
#include "RosKmdDevice.h"
#include "RosKmdContext.h"
#include "RosKmdAllocation.h"
#include "RosGpuCommand.h"
#include "RosKmdGlobal.h"
#include "RosKmdUtil.h"

NTSTATUS
__stdcall
RosKmContext::DdiRender(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_RENDER   pRender)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdRender hAdapter=%lx\n", hContext);

    RosKmContext  *pRosKmContext = RosKmContext::Cast(hContext);
    RosKmAdapter  *pRosKmAdapter = pRosKmContext->m_pDevice->m_pRosKmAdapter;

    pRender->MultipassOffset;
    pRender->DmaBufferSegmentId;
    pRender->DmaBufferPhysicalAddress;
    pRender->pDmaBufferPrivateData;
    pRender->DmaBufferPrivateDataSize;

    // Copy command buffer
    NT_ASSERT(pRender->DmaSize >= pRender->CommandLength);

#ifdef SSR_END_DMA
    __try {
        memcpy(pRender->pDmaBuffer, pRender->pCommand, ROSD_COMMAND_BUFFER_SIZE);
    }
#else
    __try {
        memcpy(pRender->pDmaBuffer, pRender->pCommand, pRender->CommandLength);
    }
#endif
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ROS_LOG_ERROR("Caught structured exception while copying usermode buffer.");
        return STATUS_INVALID_PARAMETER;
    }

    // Check to command buffer header
    GpuCommand *    pCmdBufHeader = (GpuCommand *)pRender->pDmaBuffer;
    if (pCmdBufHeader->m_commandId != Header)
    {
        ROS_LOG_ERROR(
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
        ROS_LOG_ERROR("Caught structured exception while copying usermode buffer.");
        return STATUS_INVALID_PARAMETER;
    }

    // Must update pPatchLocationListOut to reflect what space was used
    pRender->pPatchLocationListOut += pRender->PatchLocationListInSize;

    // Record DMA buffer information
    ROSDMABUFINFO * pDmaBufInfo = (ROSDMABUFINFO *)pRender->pDmaBufferPrivateData;

    pDmaBufInfo->m_DmaBufState.m_Value = 0;
    pDmaBufInfo->m_DmaBufState.m_bRender = 1;
    pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer = pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer;

    pDmaBufInfo->m_pDmaBuffer = (PBYTE)pRender->pDmaBuffer;
    pDmaBufInfo->m_DmaBufferPhysicalAddress.QuadPart = 0;
    pDmaBufInfo->m_DmaBufferSize = pRender->DmaSize;

    pDmaBufInfo->m_pRenderTarget = NULL;

    // Validate DMA buffer
    bool isValidDmaBuffer;

    isValidDmaBuffer = pRosKmAdapter->ValidateDmaBuffer(
        pDmaBufInfo,
        pRender->pAllocationList,
        pRender->AllocationListSize,
        pPatchLocationList,
        pRender->PatchLocationListInSize);

    if (! isValidDmaBuffer)
    {
        ROS_LOG_ERROR("DMA buffer is not valid. (pDmaBufInfo=0x%p)", pDmaBufInfo);
        return STATUS_INVALID_PARAMETER;
    }

    if (pCmdBufHeader->m_commandBufferHeader.m_hasVC4ClearColors)
    {
        pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors = 1;
        pDmaBufInfo->m_VC4ClearColors = pCmdBufHeader->m_commandBufferHeader.m_vc4ClearColors;
    }

    // Perform pre-patch
    pRosKmAdapter->PatchDmaBuffer(
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
RosKmContext::DdiCreateContext(
    IN_CONST_HANDLE                 hDevice,
    INOUT_PDXGKARG_CREATECONTEXT    pCreateContext)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hDevice=%lx\n",
        __FUNCTION__, hDevice);

    RosKmContext  *pRosKmContext;

    pRosKmContext = (RosKmContext *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(RosKmContext), 'ROSD');
    if (!pRosKmContext)
    {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(pRosKmContext, sizeof(RosKmContext));

    pRosKmContext->m_magic = kMagic;
    pRosKmContext->m_pDevice = (RosKmDevice *)hDevice;
    pRosKmContext->m_Node = pCreateContext->NodeOrdinal;
    pRosKmContext->m_hRTContext = pCreateContext->hContext;
    pRosKmContext->m_Flags = pCreateContext->Flags;

    //
    // Set up info returned to runtime
    //
    DXGK_CONTEXTINFO   *pContextInfo = &pCreateContext->ContextInfo;

    pContextInfo->DmaBufferSegmentSet = 1 << (ROSD_SEGMENT_APERTURE - 1);
    pContextInfo->DmaBufferSize = ROSD_COMMAND_BUFFER_SIZE;
    pContextInfo->DmaBufferPrivateDataSize = sizeof(ROSDMABUFINFO);

    if (pCreateContext->Flags.GdiContext)
    {
        pContextInfo->AllocationListSize = DXGK_ALLOCATION_LIST_SIZE_GDICONTEXT;
        pContextInfo->PatchLocationListSize = DXGK_ALLOCATION_LIST_SIZE_GDICONTEXT;
    }
    else
    {
        pContextInfo->AllocationListSize = C_ROSD_ALLOCATION_LIST_SIZE;
        pContextInfo->PatchLocationListSize = C_ROSD_PATCH_LOCATION_LIST_SIZE;
    }

    pCreateContext->hContext = pRosKmContext;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmContext::DdiDestroyContext(
    IN_CONST_HANDLE     hContext)
{
    RosKmContext  *pRosKmContext = RosKmContext::Cast(hContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdDestroyContext hContext=%lx\n",
        hContext);

    pRosKmContext->~RosKmContext();

    ExFreePool(pRosKmContext);

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmContext::RenderKm(
    INOUT_PDXGKARG_RENDER   pRender)
{
    pRender;

    NT_ASSERT(FALSE);
    return STATUS_SUCCESS;
}

ROS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS RosKmContext::Present (DXGKARG_PRESENT* Args)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_WARNING(
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

ROS_PAGED_SEGMENT_END; //=====================================================
