#include "CosKmd.h"
#include "CosKmdMetaCommand.h"
#include "CosKmdGlobal.h"

#if MLMC

static void
CosKmdExecuteMetaCommandNormalization(
    META_COMMAND_CREATE_NORMALIZATION_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_NORMALIZATION_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandNormalization;
}

static void
CosKmdExecuteMetaCommandConvolution(
    META_COMMAND_CREATE_CONVOLUTION_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_CONVOLUTION_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandConvolution;
}

void
CosKmdFixupResourceCpuAddress(
    GpuHWDescriptor *   pHwDescriptor,
    UINT                numDescriptors)
{
    for (UINT i = 0; i < numDescriptors; i++, pHwDescriptor++)
    {
        if (0 != pHwDescriptor->m_resourceGpuAddress.QuadPart)
        {
            PBYTE pResource = (((PBYTE)CosKmdGlobal::s_pVideoMemory) +
                               (pHwDescriptor->m_resourceGpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

            pHwDescriptor->m_resourceGpuAddress.QuadPart = (LONGLONG)pResource;
        }
    }
}

void
CosKmdExecuteMetaCommand(
    GpuHwMetaCommand *  pMetaCommand)
{
    KFLOATING_SAVE floatingSave;

    KeSaveFloatingPointState(&floatingSave);

    switch (pMetaCommand->m_metaCommandId)
    {
    case MetaCommandNormalization:
        {
            META_COMMAND_CREATE_NORMALIZATION_DESC *  pCreateDesc = (META_COMMAND_CREATE_NORMALIZATION_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_NORMALIZATION_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_NORMALIZATION_DESC *)(pCreateDesc + 1);

            CosKmdFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmdExecuteMetaCommandNormalization(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandConvolution:
        {
            META_COMMAND_CREATE_CONVOLUTION_DESC *  pCreateDesc = (META_COMMAND_CREATE_CONVOLUTION_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_CONVOLUTION_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_CONVOLUTION_DESC *)(pCreateDesc + 1);

            CosKmdFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmdExecuteMetaCommandConvolution(pCreateDesc, pExecuteDesc);
        }
        break;
    default:
        break;
    }

    KeRestoreFloatingPointState(&floatingSave);
}

#else

void
CosKmdExecuteMetaCommand(
    GpuHwMetaCommand *)
{
}

#endif

