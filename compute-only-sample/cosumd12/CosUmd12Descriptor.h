#pragma once

#include "CosUmd12.h"

#if COS_PHYSICAL_SUPPORT

#if COS_GPUVA_SUPPORT
#error COS_GPUVA_SUPPORT should not be defined
#endif

#if COS_RS_2LEVEL_SUPPORT
#error COS_RS_2LEVEL_SUPPORT should not be defined
#endif

class CosUmd12Descriptor
{
public:

    enum Type 
    {
        COS_CBV = 1,
        COS_SRV = 2,
        COS_UAV = 3
    };

    CosUmd12Descriptor(const D3D12DDI_CONSTANT_BUFFER_VIEW_DESC * pDesc)
    {
        m_type = COS_CBV;
        m_cbv = *pDesc;
    }

    CosUmd12Descriptor(const D3D12DDIARG_CREATE_SHADER_RESOURCE_VIEW_0002 * pDesc)
    {
        m_type = COS_SRV;
        m_srv = *pDesc;
    }

    CosUmd12Descriptor(const D3D12DDIARG_CREATE_UNORDERED_ACCESS_VIEW_0002 * pDesc)
    {
        m_type = COS_UAV;
        m_uav = *pDesc;
    }

#if GPUVA_SYSMEM_DH

    void WriteHWDescriptor(
        D3D12DDI_GPU_VIRTUAL_ADDRESS * pResourceGpuVa) const;

#endif

    void WriteHWDescriptor(
        CosUmd12CommandBuffer * pCurCommandBuffer,
        UINT hwDescriptorOffset,
        D3DDDI_PATCHLOCATIONLIST * &pPatchLocations) const;

private:
    Type    m_type;
    union
    {
        D3D12DDI_CONSTANT_BUFFER_VIEW_DESC m_cbv;
        D3D12DDIARG_CREATE_SHADER_RESOURCE_VIEW_0002 m_srv;
        D3D12DDIARG_CREATE_UNORDERED_ACCESS_VIEW_0002 m_uav;
    };
};

#endif // COS_PHYSICAL_SUPPORT