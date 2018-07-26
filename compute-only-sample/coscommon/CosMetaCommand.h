#pragma once

//
// Meta command definition is shared between app and UMD
//

// {980BFE94-D9FC-4AA7-837B-ACD6798383F7}
// Identity meta command copies input resource into output resource
const GUID GUID_IDENTITY =
{ 
    0x980bfe94, 0xd9fc, 0x4aa7, { 0x83, 0x7b, 0xac, 0xd6, 0x79, 0x83, 0x83, 0xf7 }
};

#pragma pack(push,4)
struct IdentityMetaCommandCreationParameters
{
    UINT64 BufferSize;
};
struct IdentityMetaCommandExecutionParameters
{
    D3D12_GPU_VIRTUAL_ADDRESS Input;
    D3D12_GPU_VIRTUAL_ADDRESS Output;
};
#pragma pack(pop)

