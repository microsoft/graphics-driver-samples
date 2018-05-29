#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12Heap
{
public:
    explicit CosUmd12Heap(CosUmd12Device* pDevice)
    {
        m_pDevice = pDevice;
        memset(&m_desc, 0, sizeof(m_desc));
    }

    explicit CosUmd12Heap(CosUmd12Device * pDevice, D3D12DDI_HRTRESOURCE hRTHeap, const D3D12DDIARG_CREATEHEAP_0001 * pDesc)
    {
        m_pDevice = pDevice;
        m_hRTHeap = hRTHeap;
        m_desc = *pDesc;
    }

    ~CosUmd12Heap()
    {
    }

    static int CalculateSize()
    {
        return sizeof(CosUmd12Heap);
    }

    static CosUmd12Heap* CastFrom(D3D12DDI_HHEAP);
    D3D12DDI_HHEAP CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATEHEAP_0001 m_desc;
    D3D12DDI_HRTRESOURCE m_hRTHeap;
};

inline CosUmd12Heap* CosUmd12Heap::CastFrom(D3D12DDI_HHEAP hHeap)
{
    return static_cast< CosUmd12Heap* >(hHeap.pDrvPrivate);
}

inline D3D12DDI_HHEAP CosUmd12Heap::CastTo() const
{
    return MAKE_D3D12DDI_HHEAP(const_cast< CosUmd12Heap* >(this));
}

