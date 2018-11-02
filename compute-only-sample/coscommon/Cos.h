#pragma once

#define DXGKDDI_INTERFACE_VERSION 0xA00A    // WDDM 2.5

// Defines needed due to problems in video.h when we turned on warning 4668
#define DXGKDDI_INTERFACE_VERSION_WDDM1_3       0x4002
#define DXGKDDI_INTERFACE_VERSION_WDDM1_3_M1    0x4000
#define DXGKDDI_INTERFACE_VERSION_WDDM_2_0      0x5023
#define DXGKDDI_INTERFACE_VERSION_WDDM_1_3      0x4002

// Defines needed due to problems in ntverp.h when we turned on warning 4668
#define BETA 0
#define OFFICIAL_BUILD 0

// Global configuration settings

#define COS_GPUVA_SUPPORT 0
#define COS_PHYSICAL_SUPPORT !(COS_GPUVA_SUPPORT)


