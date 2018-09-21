#pragma once

class CosUmd12Device;

class CosUmd12CommandQueue
{
public:
    explicit CosUmd12CommandQueue(CosUmd12Device* pDevice, D3D12DDI_HRTCOMMANDQUEUE hRTCommandQueue, const D3D12DDIARG_CREATECOMMANDQUEUE_0050* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
        m_hRTCommandQueue = hRTCommandQueue;
    }

    ~CosUmd12CommandQueue()
    {
    }

    static int CalculateSize(const D3D12DDIARG_CREATECOMMANDQUEUE_0050 * pArgs)
    {
        return sizeof(CosUmd12CommandQueue);
    }

    static CosUmd12CommandQueue* CastFrom(D3D12DDI_HCOMMANDQUEUE);
    D3D12DDI_HCOMMANDQUEUE CastTo() const;

    HRESULT Standup();
    void Teardown();

    // Ddi ExecuteCommandLists support
    void ExecuteCommandLists(UINT Count, const D3D12DDI_HCOMMANDLIST* pCommandLists);

    // Interface for Command List
    HRESULT
    ExecuteCommandBuffer(
        D3DGPU_VIRTUAL_ADDRESS  commandBufferGpuVa,
        UINT                    commandBufferLength,
        BYTE *                  pCommandBuffer);

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATECOMMANDQUEUE_0050 m_args;
    D3D12DDI_HRTCOMMANDQUEUE m_hRTCommandQueue;

    D3DDDICB_CREATECONTEXTVIRTUAL m_createContext;
};

inline CosUmd12CommandQueue* CosUmd12CommandQueue::CastFrom(D3D12DDI_HCOMMANDQUEUE hRootSignature)
{
    return static_cast< CosUmd12CommandQueue* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HCOMMANDQUEUE CosUmd12CommandQueue::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D12DDI_HCOMMANDQUEUE(const_cast< CosUmd12CommandQueue* >(this));
}

