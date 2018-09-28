#pragma once

#include "CosUmd12.h"

class CosUmd12CommandBuffer
{
public:
    static CosUmd12CommandBuffer * Create(
        CosUmd12Device *    pDevice);

    CosUmd12CommandBuffer(
        CosUmd12Device *                    pDevice,
        D3D12DDI_HRTRESOURCE                hRTHeap,
        const D3D12DDIARG_CREATEHEAP_0001 * pHeapDesc);

    ~CosUmd12CommandBuffer();

    HRESULT Standup();
    void Teardown();

    void
    ReserveCommandBufferSpace(
        UINT                        commandSize,
        BYTE **                     ppCommandBuffer);

    void
    CommitCommandBufferSpace(
        UINT    commandSize);

    bool IsCommandBufferEmpty();

    // Interface for Command Queue
    HRESULT Execute(CosUmd12CommandQueue * pCommandQueue);

private:

    CosUmd12Heap                        m_commandHeap;

    BYTE *                              m_pCommandBuffer;
    UINT                                m_commandBufferSize;
    UINT                                m_commandBufferPos;

    GpuCommand *                        m_pCmdBufHeader;

    CONST UINT  COMMAND_BUFFER_FLUSH_THRESHOLD = 512;
};
