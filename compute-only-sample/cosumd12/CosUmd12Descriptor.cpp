////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Descriptor implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if GPUVA

void CosUmd12Descriptor::WriteHWDescriptor(
    GpuHWDescriptor *   pHwDescriptor) const
{
    D3D12DDI_GPU_VIRTUAL_ADDRESS resourceGpuVa;
    UINT allocationOffset = 0;
    CosUmd12Resource * pResource = NULL;

    switch (m_type)
    {
    case COS_CBV:
        resourceGpuVa = m_cbv.BufferLocation;
        break;
    case COS_SRV:
        ASSERT(false);
        break;
    case COS_UAV:
        //
        // TODO : Support Counter resource for Append structured buffer
        //
        pResource = CosUmd12Resource::CastFrom(m_uav.hDrvResource);
        resourceGpuVa = pResource->GetGpuVa();
        switch (m_uav.ResourceDimension)
        {
        case D3D12DDI_RD_BUFFER:
            resourceGpuVa += (m_uav.Buffer.FirstElement * m_uav.Buffer.StructureByteStride);
            break;
        case D3D12DDI_RD_TEXTURE1D:
        case D3D12DDI_RD_TEXTURE2D:
        case D3D12DDI_RD_TEXTURE3D:
            ASSERT(false);
            break;
        }
        break;
    }

    pHwDescriptor->m_resourceGpuAddress.QuadPart = resourceGpuVa;
}

#endif

void CosUmd12Descriptor::WriteHWDescriptor(
    CosUmd12CommandBuffer * pCurCommandBuffer,
    UINT hwDescriptorOffset,
    D3DDDI_PATCHLOCATIONLIST * &pPatchLocations) const
{
    D3D12DDI_GPU_VIRTUAL_ADDRESS resourceGpuVA;
    UINT allocationOffset = 0;
    CosUmd12Resource * pResource = NULL;

    switch (m_type)
    {
    case COS_CBV:
        resourceGpuVA = m_cbv.BufferLocation;
        break;
    case COS_SRV:
        pResource = CosUmd12Resource::CastFrom(m_srv.hDrvResource);
        resourceGpuVA = pResource->GetUniqueAddress();
        switch (m_srv.ResourceDimension)
        {
        case D3D12DDI_RD_BUFFER:
            allocationOffset = (UINT)m_srv.Buffer.FirstElement * m_srv.Buffer.StructureByteStride;
            break;
        case D3D12DDI_RD_TEXTURE1D:
        case D3D12DDI_RD_TEXTURE2D:
        case D3D12DDI_RD_TEXTURE3D:
        case D3D12DDI_RD_TEXTURECUBE:
            //
            // TODO : 
            //
            ASSERT(false);
            break;
        }
        break;
    case COS_UAV:
        //
        // TODO : Support Counter resource for Append structured buffer
        //
        pResource = CosUmd12Resource::CastFrom(m_uav.hDrvResource);
        resourceGpuVA = pResource->GetUniqueAddress();
        switch (m_uav.ResourceDimension)
        {
        case D3D12DDI_RD_BUFFER:
            allocationOffset = (UINT)m_uav.Buffer.FirstElement * m_uav.Buffer.StructureByteStride;
            break;
        case D3D12DDI_RD_TEXTURE1D:
        case D3D12DDI_RD_TEXTURE2D:
        case D3D12DDI_RD_TEXTURE3D:
            //
            // TODO : 
            //
            ASSERT(false);
            break;

        }
        break;
    }

#if !GPUVA

    D3DKMT_HANDLE hAllocation = (D3DKMT_HANDLE)(resourceGpuVA >> 32);
    allocationOffset += (UINT)(resourceGpuVA & 0xFFFFFFFF);

    UINT allocIndex = pCurCommandBuffer->UseAllocation(hAllocation, true);

    pCurCommandBuffer->SetPatchLocation(
                        pPatchLocations,
                        allocIndex,
                        hwDescriptorOffset,
                        0,
                        allocationOffset);

#endif
}

