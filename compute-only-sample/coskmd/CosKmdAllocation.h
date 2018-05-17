#pragma once

struct CosKmdAllocation : public CosAllocationExchange
{
};

struct CosKmdDeviceAllocation
{
    D3DKMT_HANDLE       m_hKMAllocation;
    CosKmdAllocation   *m_pCosKmdAllocation;
};
