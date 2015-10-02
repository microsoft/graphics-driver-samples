#pragma once

#include <wingdi.h>
#include <d3d10umddi.h>

//
// Structure for exchange adapter info between UMD and KMD
//
typedef struct _ROSADAPTERINFO
{
    UINT                m_version;
    DXGK_WDDMVERSION    m_wddmVersion;
    BOOL                m_isSoftwareDevice;
} ROSADAPTERINFO;

