#pragma once

//
// The official DDI versions are listed in d3dukmdt.h as verfied by
// IS_OFFICIAL_DDI_INTERFACE_VERSION(version)
//
// DXGKDDI_INTERFACE_VERSION by default is the highest/current version
// defined in d3dukmdt.h. If a lower version is used, it must be one of
// the official versions.
//
// Otherwise DxgkInitialize() fails and driver is unloaded.
//

#define DXGKDDI_INTERFACE_VERSION 0xB004    // WDDM 2.6

// Defines needed due to problems in video.h when we turned on warning 4668
#define DXGKDDI_INTERFACE_VERSION_WDDM1_3       0x4002
#define DXGKDDI_INTERFACE_VERSION_WDDM1_3_M1    0x4000
#define DXGKDDI_INTERFACE_VERSION_WDDM_2_0      0x5023
#define DXGKDDI_INTERFACE_VERSION_WDDM_1_3      0x4002

// Defines needed due to problems in ntverp.h when we turned on warning 4668
#define BETA 0
#define OFFICIAL_BUILD 0

// Global configuration settings

//
// COSD support RS5 Machine Learning Meta Command DDIs
// MLMC works with both GPU Memory Models (GPUVA and Physical)
//

#define COS_MLMC_RS5_SUPPORT    0

//
// GPU Memory Model configuration
//
// COSD has only 1 engine, so COS_GPUVA_SUPPORT and COS_PHYSICAL_SUPPORT are 
// mutually exclusive.
//

#define COS_GPUVA_SUPPORT       0
#define COS_PHYSICAL_SUPPORT    !(COS_GPUVA_SUPPORT)

//
// COS_RS_2LEVEL_SUPPORT is only applicable in Physical GPU Memory Model
//
// When it is enabled, GPU command is used to Patch the Descriptor Heap, which
// is 2nd level data referenced by Root Signature data.
//
// The alternative design is to copy portions of Descriptor Heap into Command 
// List/Buffer for Patching.
//

#define COS_RS_2LEVEL_SUPPORT   0

//
// ENABLE_FOR_COSTEST and ENABLE_FOR_COSTEST2 are only applicable in Physical
// GPU Memory Model.
//
// They enable hard-coded shader in KMD to allow end to end testing of driver
// bring-up tests costest.exe and costest2.exe.
//
// Other than driver develpment using costest.exe and costest2.exe, both must
// be set to 0.
//
// When COS_RS_2LEVEL_SUPPORT is 0, ENABLE_FOR_COSTEST of 1 allows both
// costest.exe and costest2.exe to finish successfully.
//
// When COS_RS_2LEVEL_SUPPORT is 1, ENABLE_FOR_COSTEST takes precedence over
// ENABLE_FOR_COSTEST2. ENABLE_FOR_COSTEST of 1 allows costest.exe to finish
// successfully and ENABLE_FOR_COSTEST2 of 1 is for costest2.exe
//

#define ENABLE_FOR_COSTEST      0
#define ENABLE_FOR_COSTEST2     0

//
// For Physical GPU Memory Model and COS_RS_2LEVEL_SUPPORT of 0, Resource
// Binding Tier 1 limits the size of the Descriptor Table, thus the amount of
// data copying from Descriptor Heap.
//
// https://docs.microsoft.com/en-us/windows/desktop/direct3d12/hardware-support
//

#define COS_USE_RESOURCE_BINDING_TIER_1 0

//
// COS_GPUVA_MLMC_NO_DESCRIPTOR_HEAP is only applicable when COS_GPUVA_SUPPORT
// and COS_MLMC_RS5_SUPPORT are both set to 1.
//
// It is meant to support HW with Meta Command unit accessing resource directly
// (without indirection through Descriptor Heap)
//

#define COS_GPUVA_MLMC_NO_DESCRIPTOR_HEAP   0

