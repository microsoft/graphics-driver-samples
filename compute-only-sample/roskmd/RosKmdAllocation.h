#pragma once

struct RosKmdAllocation : public RosAllocationExchange
{
};

struct RosKmdDeviceAllocation
{
    D3DKMT_HANDLE       m_hKMAllocation;
    RosKmdAllocation   *m_pRosKmdAllocation;
};
