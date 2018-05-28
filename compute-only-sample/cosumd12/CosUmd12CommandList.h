#pragma once

#include "CosUmd12.h"

class CosUmd12CommandList
{
public:
    explicit CosUmd12CommandList(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_COMMAND_LIST_0001* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12CommandList()
    {
    }

    void Reset()
    {
        // do nothing
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_COMMAND_LIST_0001 * pArgs)
    {
        return sizeof(CosUmd12CommandList);
    }

    static CosUmd12CommandList* CastFrom(D3D12DDI_HCOMMANDLIST);
    D3D12DDI_HCOMMANDLIST CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_COMMAND_LIST_0001 m_args;

};

inline CosUmd12CommandList* CosUmd12CommandList::CastFrom(D3D12DDI_HCOMMANDLIST hRootSignature)
{
    return static_cast< CosUmd12CommandList* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HCOMMANDLIST CosUmd12CommandList::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D12DDI_HCOMMANDLIST(const_cast< CosUmd12CommandList* >(this));
}
