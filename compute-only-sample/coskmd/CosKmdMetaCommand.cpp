#include "CosKmd.h"
#include "CosKmdMetaCommand.h"
#include "CosKmdGlobal.h"

#if MLMC

static void
CosKmExecuteMetaCommandNormalization(
    META_COMMAND_CREATE_NORMALIZATION_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_NORMALIZATION_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandNormalization;
}

static void
CosKmExecuteMetaCommandConvolution(
    META_COMMAND_CREATE_CONVOLUTION_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_CONVOLUTION_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandConvolution;
}

static void
CosKmExecuteMetaCommandGEMM(
    META_COMMAND_CREATE_GEMM_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_GEMM_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandGEMM;
}

static void
CosKmExecuteMetaCommandGRU(
    META_COMMAND_CREATE_GRU_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_GRU_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandGRU;
}

static void
CosKmExecuteMetaCommandLSTM(
    META_COMMAND_CREATE_LSTM_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_LSTM_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandLSTM;
}

static void
CosKmExecuteMetaCommandMVN(
    META_COMMAND_CREATE_MVN_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_MVN_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandMVN;
}

static void
CosKmExecuteMetaCommandPooling(
    META_COMMAND_CREATE_POOLING_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_POOLING_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandPooling;
}

static void
CosKmExecuteMetaCommandReduction(
    META_COMMAND_CREATE_REDUCTION_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_REDUCTION_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandReduction;
}

static void
CosKmExecuteMetaCommandRNN(
    META_COMMAND_CREATE_RNN_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_RNN_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandRNN;
}

static void
CosKmExecuteMetaCommandRoiPooling(
    META_COMMAND_CREATE_ROI_POOLING_DESC *  pCreateDesc,
    META_COMMAND_EXECUTE_ROI_POOLING_DESC * pExecuteDesc)
{
    UNREFERENCED_PARAMETER(pCreateDesc);
    UNREFERENCED_PARAMETER(pExecuteDesc);

    MetaCommandId   metaCommandId;

    metaCommandId = MetaCommandRoiPooling;
}

void
CosKmFixupResourceCpuAddress(
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
CosKmExecuteMetaCommand(
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

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandNormalization(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandConvolution:
        {
            META_COMMAND_CREATE_CONVOLUTION_DESC *  pCreateDesc = (META_COMMAND_CREATE_CONVOLUTION_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_CONVOLUTION_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_CONVOLUTION_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandConvolution(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandGEMM:
        {
            META_COMMAND_CREATE_GEMM_DESC *  pCreateDesc = (META_COMMAND_CREATE_GEMM_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_GEMM_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_GEMM_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandGEMM(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandGRU:
        {
            META_COMMAND_CREATE_GRU_DESC *  pCreateDesc = (META_COMMAND_CREATE_GRU_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_GRU_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_GRU_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandGRU(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandLSTM:
        {
            META_COMMAND_CREATE_LSTM_DESC *  pCreateDesc = (META_COMMAND_CREATE_LSTM_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_LSTM_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_LSTM_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandLSTM(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandMVN:
        {
            META_COMMAND_CREATE_MVN_DESC *  pCreateDesc = (META_COMMAND_CREATE_MVN_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_MVN_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_MVN_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandMVN(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandPooling:
        {
            META_COMMAND_CREATE_POOLING_DESC *  pCreateDesc = (META_COMMAND_CREATE_POOLING_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_POOLING_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_POOLING_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandPooling(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandReduction:
        {
            META_COMMAND_CREATE_REDUCTION_DESC *  pCreateDesc = (META_COMMAND_CREATE_REDUCTION_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_REDUCTION_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_REDUCTION_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandReduction(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandRNN:
        {
            META_COMMAND_CREATE_RNN_DESC *  pCreateDesc = (META_COMMAND_CREATE_RNN_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_RNN_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_RNN_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandRNN(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandRoiPooling:
        {
            META_COMMAND_CREATE_ROI_POOLING_DESC *  pCreateDesc = (META_COMMAND_CREATE_ROI_POOLING_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_ROI_POOLING_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_ROI_POOLING_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((GpuHWDescriptor *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandRoiPooling(pCreateDesc, pExecuteDesc);
        }
        break;
    default:
        break;
    }

    KeRestoreFloatingPointState(&floatingSave);
}

#else

void
CosKmExecuteMetaCommand(
    GpuHwMetaCommand *)
{
}

#endif

