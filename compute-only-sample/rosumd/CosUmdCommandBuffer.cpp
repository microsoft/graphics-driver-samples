#include "precomp.h"

#include "RosUmdLogging.h"
#include "RosUmdCommandBuffer.tmh"

#include "RosUmdCommandBuffer.h"
#include "RosUmdResource.h"
#include "RosUmdDevice.h"
#include "RosUmdDebug.h"


RosUmdCommandBuffer::RosUmdCommandBuffer()
{
    m_pRosUmdDevice = NULL;
}

RosUmdCommandBuffer::~RosUmdCommandBuffer()
{
    // do nothing
}

void RosUmdCommandBuffer::Standup(RosUmdDevice * pRosUmdDevice, D3DDDICB_CREATECONTEXT * pCreateContext)
{
    m_pRosUmdDevice = pRosUmdDevice;

    m_pCommandBuffer = (BYTE *)pCreateContext->pCommandBuffer;
    m_commandBufferSize = pCreateContext->CommandBufferSize;

    m_pAllocationList = pCreateContext->pAllocationList;
    m_allocationListSize = pCreateContext->AllocationListSize;

    m_pPatchLocationList = pCreateContext->pPatchLocationList;
    m_patchLocationListSize = pCreateContext->PatchLocationListSize;

    m_submissionFence = s_firstFence;

    m_commandBufferPos = sizeof(GpuCommand);
    m_allocationListPos = 0;
    m_patchLocationListPos = 0;

    m_pCmdBufHeader = (GpuCommand *)m_pCommandBuffer;
    m_pCmdBufHeader->m_commandId = Header;
    m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer = 1;
}

bool RosUmdCommandBuffer::IsCommandBufferEmpty()
{
    return (m_commandBufferPos <= sizeof(GpuCommand));
}

bool RosUmdCommandBuffer::IsSwCommandBuffer()
{
    assert(m_pCmdBufHeader->m_commandId == Header);
    return m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer;
}

// TODO(bhouse) The implementation of copy resource (and write resource below) should not be here.  RosUmdCommandBuffer should
//              offer methods for using command buffer but it should not include the code that
//              is responsible for emitting the necessary commands into the command buffer to handle
//              a GPU operation (like copy resource).
void RosUmdCommandBuffer::CopyResource(RosUmdResource * pDstResource, RosUmdResource * pSrcResource)
{
    assert(m_pRosUmdDevice != NULL);

    BYTE *  pCommandBuffer;
    UINT    curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocationList;

    GpuCommand * command;

    ReserveCommandBufferSpace(
        true,                           // SW command
        sizeof(*command),
        &pCommandBuffer,
        2,
        2,
        &curCommandOffset,
        &pPatchLocationList);

    assert(pCommandBuffer != NULL);
    assert(pPatchLocationList != NULL);

    command = reinterpret_cast<GpuCommand *>(pCommandBuffer);

    command->m_commandId = GpuCommandId::ResourceCopy;
    command->m_resourceCopy.m_srcGpuAddress.QuadPart = 0;
    command->m_resourceCopy.m_dstGpuAddress.QuadPart = 0;
    command->m_resourceCopy.m_sizeBytes = pDstResource->m_hwSizeBytes;

    UINT dstAllocIndex = UseResource(pDstResource, true);
    UINT srcAllocIndex = UseResource(pSrcResource, false);

    SetPatchLocation(pPatchLocationList, dstAllocIndex, curCommandOffset + offsetof(GpuCommand, m_resourceCopy.m_dstGpuAddress));
    SetPatchLocation(pPatchLocationList, srcAllocIndex, curCommandOffset + offsetof(GpuCommand, m_resourceCopy.m_srcGpuAddress));

    CommitCommandBufferSpace(sizeof(*command), 2);
}

void RosUmdCommandBuffer::WriteResource(RosUmdResource * pResource, void * pData)
{
    assert(m_pRosUmdDevice != NULL);

    BYTE *  pCommandBuffer;
    UINT    curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocationList;

    __debugbreak();

    ReserveCommandBufferSpace(
        true,                   // SW command
        100,
        &pCommandBuffer,
        2,
        2,
        &curCommandOffset,
        &pPatchLocationList);

    UNREFERENCED_PARAMETER(pData);

    assert(pCommandBuffer != NULL);
    assert(pPatchLocationList != NULL);

    memset(pCommandBuffer, 0, 100);

    UINT allocIndex = UseResource(pResource, true);

    SetPatchLocation(pPatchLocationList, allocIndex, curCommandOffset + 10);

    // Commit the reserved command buffer space and patch location list
    CommitCommandBufferSpace(100, 1);
}

void RosUmdCommandBuffer::FlushIfMatching(ULONGLONG fence)
{
    // If the resource is used in the current command buffer
    if (fence == m_submissionFence)
    {
        Flush(0);
    }
}

void
RosUmdCommandBuffer::Flush(
    UINT /*flushFlags*/)
{
    assert(m_pRosUmdDevice != NULL);

    if (false == m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer)
    {
        m_pRosUmdDevice->WriteEpilog();
    }

    D3DDDICB_RENDER render;

    memset(&render, 0, sizeof(render));

    render.hContext = m_pRosUmdDevice->m_hContext;

    render.BroadcastContextCount = 0;

    render.CommandLength = m_commandBufferPos;
    render.NumAllocations = m_allocationListPos;
    render.NumPatchLocations = m_patchLocationListPos;

    m_pRosUmdDevice->Render(&render);

    // Increase the submission fence
    m_submissionFence++;

    // Reset the comamnd buffer, allocation list and patch location list positions
    m_commandBufferPos = sizeof(GpuCommand);
    m_allocationListPos = 0;
    m_patchLocationListPos = 0;

    // Reset type of command buffer to software
    m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer   = 1;
#if VC4

    m_pCmdBufHeader->m_commandBufferHeader.m_hasVC4ClearColors = 0;
    m_pCmdBufHeader->m_commandBufferHeader.m_vc4ClearColors = vc4ClearColors;

#endif

    render.QueuedBufferCount; // unused
}

void
RosUmdCommandBuffer::ReserveCommandBufferSpace(
    bool                        bSwCommand,
    UINT                        commandSize,
    BYTE **                     ppCommandBuffer,
    UINT                        allocationListSize,
    UINT                        patchLocationSize,
    UINT *                      pCurCommandOffset,
    D3DDDI_PATCHLOCATIONLIST ** ppPatchLocationList)
{
    assert(m_pRosUmdDevice != NULL);

    if (IsSwCommandBuffer() != bSwCommand)
    {
        if (IsCommandBufferEmpty() == false)
        {
            Flush(0);
        }

        // Command buffer contains either SW or HW commands
        m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer = bSwCommand;
    }

    if (((m_commandBufferPos + commandSize + COMMAND_BUFFER_FLUSH_THRESHOLD) > m_commandBufferSize) ||
        ((m_allocationListPos + allocationListSize + ALLOCATION_LIST_FLUSH_THRESHOLD) > m_allocationListSize) ||
        ((m_patchLocationListPos + patchLocationSize + PACTH_LOCATION_LIST_FLUSH_THRESHOLD) > m_patchLocationListSize))
    {
        Flush(0);
    }

    *ppCommandBuffer = m_pCommandBuffer + m_commandBufferPos;

    if (allocationListSize)
    {
        *pCurCommandOffset = m_commandBufferPos;
        *ppPatchLocationList = m_pPatchLocationList + m_patchLocationListPos;
    }
}

void
RosUmdCommandBuffer::CommitCommandBufferSpace(
    UINT    commandSize,
    UINT    patchLocationSize)
{
    assert(m_pRosUmdDevice != NULL);

    // Update the command buffer position
    m_commandBufferPos += commandSize;
    assert((m_commandBufferPos + COMMAND_BUFFER_FLUSH_THRESHOLD) < m_commandBufferSize);

    // Update the patch location list position
    m_patchLocationListPos += patchLocationSize;
    assert((m_patchLocationListPos + PACTH_LOCATION_LIST_FLUSH_THRESHOLD) < m_patchLocationListSize);

    assert((m_allocationListPos + ALLOCATION_LIST_FLUSH_THRESHOLD) < m_allocationListSize);
}

UINT
RosUmdCommandBuffer::UseResource(
    RosUmdResource *    pResource,
    BOOL                bWriteOperation)
{
    assert(m_pRosUmdDevice != NULL);

    if (pResource->m_mostRecentFence < m_submissionFence)
    {
        auto pAllocationEntry = m_pAllocationList + m_allocationListPos;

        pAllocationEntry->hAllocation = pResource->m_hKMAllocation;
        pAllocationEntry->Value = 0;
        pAllocationEntry->WriteOperation = bWriteOperation;

        // Update the resource's reference fence
        pResource->m_mostRecentFence = m_submissionFence;
        pResource->m_allocationListIndex = m_allocationListPos;

        // Update the allocation list position
        m_allocationListPos++;
        assert((m_allocationListPos + ALLOCATION_LIST_FLUSH_THRESHOLD) < m_allocationListSize);
    }
    else
    {
        assert(pResource->m_mostRecentFence == m_submissionFence);
    }

    return pResource->m_allocationListIndex;
}

bool
RosUmdCommandBuffer::IsResourceUsed(
    RosUmdResource *    pResource)
{
    return (pResource->m_mostRecentFence == m_submissionFence);
}

#if VC4

void RosUmdCommandBuffer::UpdateClearColor(
    UINT clearColor)
{
    m_pCmdBufHeader->m_commandBufferHeader.m_hasVC4ClearColors = 1;

    VC4ClearColors *    pVC4ClearColor = &m_pCmdBufHeader->m_commandBufferHeader.m_vc4ClearColors;

    pVC4ClearColor->CommandCode    = VC4_CMD_CLEAR_COLOR;

    pVC4ClearColor->ClearColor8    = clearColor;
    pVC4ClearColor->ClearColor8Dup = clearColor;
}

void RosUmdCommandBuffer::UpdateClearDepthStencil(
    FLOAT depthValue,
    UINT8 stencilValue)
{
    m_pCmdBufHeader->m_commandBufferHeader.m_hasVC4ClearColors = 1;

    VC4ClearColors *    pVC4ClearColor = &m_pCmdBufHeader->m_commandBufferHeader.m_vc4ClearColors;

    pVC4ClearColor->CommandCode = VC4_CMD_CLEAR_COLOR;

    UINT    max24BitDepthValue = 0xFFFFFF;
    pVC4ClearColor->ClearZ = (UINT)round(max24BitDepthValue*depthValue);

    pVC4ClearColor->ClearStencil = stencilValue;
}

#endif
