#include "RosKmdAdapter.h"
#include "RosKmdContext.h"
#include "RosKmdAllocation.h"
#include "RosGpuCommand.h"

NTSTATUS
__stdcall
RosKmContext::DdiRender(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_RENDER   pRender)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdRender hAdapter=%lx\n", hContext);

    RosKmContext  *pRosKmContext = (RosKmContext *)hContext;
    pRosKmContext;

    pRender->MultipassOffset;
    pRender->DmaBufferSegmentId;
    pRender->DmaBufferPhysicalAddress;
    pRender->pDmaBufferPrivateData;
    pRender->DmaBufferPrivateDataSize;

    // Copy command buffer
    NT_ASSERT(pRender->DmaSize >= pRender->CommandLength);

    __try {
        memcpy(pRender->pDmaBuffer, pRender->pCommand, pRender->CommandLength);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Check to command buffer header
    GpuCommand *    pCmdBufHeader = (GpuCommand *)pRender->pDmaBuffer;
    if (pCmdBufHeader->m_commandId != Header)
    {
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
        return STATUS_INVALID_PARAMETER;
    }

    // Must update pPatchLocationListOut to reflect what space was used
    pRender->pPatchLocationListOut += pRender->PatchLocationListInSize;

    // Perform pre-patch
    PBYTE   pDmaBuf = (BYTE *)pCmdBufHeader;
    for (UINT i = 0; i < pRender->PatchLocationListInSize; i++)
    {
        auto patch = &pPatchLocationList[i];
        NT_ASSERT(patch->AllocationIndex < pRender->AllocationListSize);
        auto allocation = &pRender->pAllocationList[patch->AllocationIndex];

        if (allocation->SegmentId != 0)
        {
            RosKmdDeviceAllocation * pRosKmdDeviceAllocation = (RosKmdDeviceAllocation *)allocation->hDeviceSpecificAllocation;

            DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Pre-patch RosKmdDeviceAllocation %lx at %lx\n", pRosKmdDeviceAllocation, allocation->PhysicalAddress);
            DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Pre-patch buffer offset %lx allocation offset %lx\n", patch->PatchOffset, patch->AllocationOffset);

            // Patch in dma buffer
            NT_ASSERT(allocation->SegmentId == ROSD_SEGMENT_VIDEO_MEMORY);
            if (pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer)
            {
                *((PHYSICAL_ADDRESS *)(pDmaBuf + patch->PatchOffset)) = allocation->PhysicalAddress;
            }
            else
            {
                // Patch HW command buffer
            }
        }

    }

    // Record DMA buffer information
    ROSDMABUFINFO * pDmaBufInfo = (ROSDMABUFINFO *)pRender->pDmaBufferPrivateData;

    pDmaBufInfo->m_DmaBufState.m_Value = 0;
    pDmaBufInfo->m_DmaBufState.m_Render = 1;
    pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer = pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer;

    pDmaBufInfo->m_pDmaBuffer = (PBYTE)pRender->pDmaBuffer;
    pDmaBufInfo->m_DmaBufferSize = pRender->DmaSize;

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
    pContextInfo->DmaBufferPrivateDataSize = sizeof(ROSUMDDMAPRIVATEDATA2);

    if (pCreateContext->Flags.GdiContext)
    {
        pContextInfo->AllocationListSize = DXGK_ALLOCATION_LIST_SIZE_GDICONTEXT;
        pContextInfo->PatchLocationListSize = DXGK_ALLOCATION_LIST_SIZE_GDICONTEXT;
    }
    else
    {
        pContextInfo->AllocationListSize = C_ROSD_ALLOCATION_LIST_SIZE;
        pContextInfo->PatchLocationListSize = C_ROSD_ALLOCATION_LIST_SIZE;
    }

    pCreateContext->hContext = pRosKmContext;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmContext::DdiDestroyContext(
    IN_CONST_HANDLE     hContext)
{
    RosKmContext  *pRosKmContext = (RosKmContext *)hContext;

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdDestroyContext hContext=%lx\n",
        hContext);

    pRosKmContext->~RosKmContext();

    ExFreePool(pRosKmContext);

    return STATUS_SUCCESS;
}

