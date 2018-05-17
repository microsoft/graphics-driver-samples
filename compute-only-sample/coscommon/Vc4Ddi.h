#pragma once

//
// VC4 DDI definitions
//

typedef enum _VC4ResourceSlotId
{
    VC4_SLOT_TILE_ALLOCATION_MEMORY = 0x80, // For code 112, Tile Binning Mode Config
    VC4_SLOT_TILE_STATE_DATA_ARRAY  = 0x81,

    VC4_SLOT_RT_BINNING_CONFIG      = 0xC0,

    VC4_SLOT_NV_SHADER_STATE        = 0xE0, // For code 65, NV Shader State
    VC4_SLOT_BRANCH                 = 0xE1, // For code 16, Branch

    VC4_SLOT_GL_SHADER_STATE        = 0xE2, // For code 64, GL Shader State
    VC4_SLOT_FS_UNIFORM_ADDRESS     = 0xE3,
    VC4_SLOT_VS_UNIFORM_ADDRESS     = 0xE4,
    VC4_SLOT_CS_UNIFORM_ADDRESS     = 0xE5
} VC4ResourceSlotId;

//
// Shader Record are in DMA buffer and reference by Shader State
//

const UINT  VC4_MAX_DMA_BUFFER_SELF_REF = 31;

//
// TODO[indyz]: Decide the proper size of the needed memory for binning
//              and handle binning memory usage spill over
//
// For now, reserve at the end of allocated contiguous memory:
//   1. 64KB for Rendering Control List, 
//   2. 1MB for Tile Allocation 
//   3. 1MB for Tile State Data Array
//
// The default value used by UMD specifies that binning process generates
// a 32 bytes control list and uses 48 bytes for state for each tile.
//

const UINT  VC4_RENDERING_CTRL_LIST_POOL_SIZE = 1024 * 1024;
const UINT  VC4_TILE_ALLOCATION_MEMORY_SIZE = 1024 * 1024;
const UINT  VC4_TILE_STATE_DATA_ARRAY_SIZE = 1024 * 1024;

//
// TODO[indyz]: Choose proper size for VC4TileBinningModeConfig::TileAllocationBlockSize
//
// Each Allocation Block is a Control List and they are chained together with 
// branch command of 4 bytes. So 32 bytes is probably too small, this is an 
// efficiency tuning issue.
//

const UINT  VC4_TILE_ALLOCATION_BLOCK_SIZE  = 32;
