#pragma once

#include "CosUmd12.h"

#if COS_GPUVA_SUPPORT

#if COS_PHYSICAL_SUPPORT
#error COS_PHYSICAL_SUPPORT should not be defined
#endif

#if COS_RS_2LEVEL_SUPPORT
#error COS_RS_2LEVEL_SUPPORT should not be defined
#endif

class CosUmd12Descriptor : public GpuHWDescriptor
{
public:

    CosUmd12Descriptor(const D3D12DDI_CONSTANT_BUFFER_VIEW_DESC * pDesc)
    {
        m_type = COS_CBV;
        
        m_cbv.m_sizeInBytes = pDesc->SizeInBytes;
        m_cbv.m_padding = pDesc->Padding;

        m_resourceGpuAddress.QuadPart = pDesc->BufferLocation;
    }

    CosUmd12Descriptor(const D3D12DDIARG_CREATE_UNORDERED_ACCESS_VIEW_0002 * pDesc)
    {
        m_type = COS_UAV;

        m_uav.m_format = pDesc->Format;
        m_uav.m_resourceDimension = pDesc->ResourceDimension;
        m_uav.m_buffer = pDesc->Buffer;

        CosUmd12Resource * pResource = CosUmd12Resource::CastFrom(pDesc->hDrvResource);
        m_resourceGpuAddress.QuadPart = pResource->GetGpuVa();
    }

#if COS_GPUVA_MLMC_NO_DESCRIPTOR_HEAP

    void WriteHWDescriptor(
        D3D12DDI_GPU_VIRTUAL_ADDRESS * pResourceGpuVa) const
    {
        *pResourceGpuVa = m_resourceGpuAddress.QuadPart;
    }

#endif

private:
    friend class CosUmd12RootSignature;
    friend class CosUmd12CommandList;
};

C_ASSERT(sizeof(CosUmd12Descriptor) == sizeof(GpuHWDescriptor));

#endif // COS_GPUVA_SUPPORT