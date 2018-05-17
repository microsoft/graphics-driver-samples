#pragma once

#include <wingdi.h>
#include <d3d10umddi.h>

const UINT MAX_DEVICE_ID_LENGTH = 32;

//
// Structure for exchange adapter info between UMD and KMD
//
typedef struct _COSADAPTERINFO
{
    UINT                m_version;
    DXGK_WDDMVERSION    m_wddmVersion;
    BOOL                m_isSoftwareDevice;
    CHAR                m_deviceId[MAX_DEVICE_ID_LENGTH];
} COSADAPTERINFO;

