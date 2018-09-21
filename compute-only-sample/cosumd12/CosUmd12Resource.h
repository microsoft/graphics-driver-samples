#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12Resource
{
public:

    explicit CosUmd12Resource(CosUmd12Device* pDevice, D3D12DDI_HRTRESOURCE hRTResource)
    {
        m_pDevice = pDevice;
        m_hRTResource = hRTResource;

        m_dataSize = 0;
        m_textureLayout = D3D12DDI_TL_UNDEFINED;

        m_magic = kMagic;

        memset(&m_desc, 0, sizeof(m_desc));
    }

    explicit CosUmd12Resource(CosUmd12Device* pDevice, const D3D12DDIARG_CREATERESOURCE_0003 *pDesc)
    {
        TRACE_FUNCTION();

        m_pDevice = pDevice;
        m_hRTResource.handle = NULL;

        m_dataSize = 0;
        m_textureLayout = pDesc->Layout;

        m_magic = kMagic;

        m_desc = *pDesc;
    }
    

    ~CosUmd12Resource()
    {
    }

    static int CalculateSize()
    {
        return sizeof(CosUmd12Resource);
    }

    void Initialize(CosAllocationExchange * pAllocation)
    {
        ASSERT(pAllocation->m_magic == CosAllocationExchange::kMagic);
        m_dataSize = pAllocation->m_dataSize;
        m_textureLayout = pAllocation->m_textureLayout;
    }

    UINT64 GetDataSize()
    {
        ASSERT(m_dataSize != 0);
        return m_dataSize;
    }

    D3D12DDI_TEXTURE_LAYOUT GetTextureLayout()
    {
        ASSERT(m_textureLayout != D3D12DDI_TL_UNDEFINED);
        return m_textureLayout;
    }

    static CosUmd12Resource* CastFrom(D3D12DDI_HRESOURCE);
    D3D12DDI_HRESOURCE CastTo() const;

    HRESULT Standup(CosUmd12Heap *);
    void Teardown();

    D3D12DDI_GPU_VIRTUAL_ADDRESS GetUniqueAddress();

    UINT GetHeapOffset()
    {
        ASSERT(m_dataSize < 0x100000000L);
        return (UINT)m_desc.ReuseBufferGPUVA.BaseAddress.UMD.Offset;
    }

    D3DKMT_HANDLE GetHeapAllocationHandle()
    {
        return m_pHeap->m_hKMAllocation;
    }

    D3D12DDI_GPU_VIRTUAL_ADDRESS GetGpuVa();

private:

    static const int kMagic = 'rsrc';

    UINT m_magic;

    CosUmd12Device * m_pDevice;
    D3D12DDI_HRTRESOURCE m_hRTResource;

    UINT64 m_dataSize;
    D3D12DDI_TEXTURE_LAYOUT m_textureLayout;

    D3D12DDIARG_CREATERESOURCE_0003 m_desc;

    CosUmd12Heap * m_pHeap;
};

inline CosUmd12Resource* CosUmd12Resource::CastFrom(D3D12DDI_HRESOURCE hResource)
{
    CosUmd12Resource* pResource = static_cast< CosUmd12Resource* >(hResource.pDrvPrivate);
    ASSERT(pResource->m_magic == kMagic);
    return pResource;
}

inline D3D12DDI_HRESOURCE CosUmd12Resource::CastTo() const
{
    return MAKE_D3D10DDI_HRESOURCE(const_cast< CosUmd12Resource* >(this));
}
