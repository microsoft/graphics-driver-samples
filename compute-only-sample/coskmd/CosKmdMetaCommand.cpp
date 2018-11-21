#include "CosKmd.h"
#include "CosKmdMetaCommand.h"
#include "CosKmdGlobal.h"

#if COS_MLMC_RS5_SUPPORT

static UINT
CosKmGetTensorElementSize(
    META_COMMAND_TENSOR_DATA_TYPE   dstDataType,
    META_COMMAND_TENSOR_DATA_TYPE   srcDataType)
{
    // No support for type conversion
    if (dstDataType != srcDataType)
    {
        return 0;
    }

    switch (dstDataType)
    {
    case META_COMMAND_TENSOR_DATA_TYPE_FLOAT16:
        return 2;
        break;
    case META_COMMAND_TENSOR_DATA_TYPE_FLOAT32:
    case META_COMMAND_TENSOR_DATA_TYPE_UINT32:
        return 4;
        break;
    default:
        return 0;
        break;
    }
}

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

static void
CosKmExecuteMetaCommandCopyTensor(
    HW_META_COMMAND_COPY_TENSOR *   pHwMetaCommand,
    HW_IO_TABLE_COPY_TENSOR *   pHwIoTable)
{
    UINT dataSize;

    if (pHwMetaCommand->DstDesc.Stride[0] != pHwMetaCommand->SrcDesc.Stride[0])
    {
        return;
    }

    dataSize = ((UINT)pHwMetaCommand->DstDesc.Stride[0])*
               CosKmGetTensorElementSize(
                pHwMetaCommand->DstDesc.DataType,
                pHwMetaCommand->SrcDesc.DataType);

    memcpy(
        (BYTE*)pHwIoTable->DstResource.ptr,
        (BYTE*)pHwIoTable->SrcResource.ptr,
        dataSize);
}

void
CosKmFixupResourceCpuAddress(
    D3D12_GPU_DESCRIPTOR_HANDLE *   pGpuAddress,
    UINT                            numGpuAddresses)
{
    for (UINT i = 0; i < numGpuAddresses; i++, pGpuAddress++)
    {
        if (0 != pGpuAddress->ptr)
        {
            PBYTE pCpuAddress = (((PBYTE)CosKmdGlobal::s_pVideoMemory) +
                                 (pGpuAddress->ptr - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

            pGpuAddress->ptr = (UINT64)pCpuAddress;
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

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandNormalization(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandConvolution:
        {
            META_COMMAND_CREATE_CONVOLUTION_DESC *  pCreateDesc = (META_COMMAND_CREATE_CONVOLUTION_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_CONVOLUTION_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_CONVOLUTION_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandConvolution(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandGEMM:
        {
            META_COMMAND_CREATE_GEMM_DESC *  pCreateDesc = (META_COMMAND_CREATE_GEMM_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_GEMM_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_GEMM_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandGEMM(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandGRU:
        {
            META_COMMAND_CREATE_GRU_DESC *  pCreateDesc = (META_COMMAND_CREATE_GRU_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_GRU_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_GRU_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandGRU(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandLSTM:
        {
            META_COMMAND_CREATE_LSTM_DESC *  pCreateDesc = (META_COMMAND_CREATE_LSTM_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_LSTM_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_LSTM_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandLSTM(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandMVN:
        {
            META_COMMAND_CREATE_MVN_DESC *  pCreateDesc = (META_COMMAND_CREATE_MVN_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_MVN_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_MVN_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandMVN(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandPooling:
        {
            META_COMMAND_CREATE_POOLING_DESC *  pCreateDesc = (META_COMMAND_CREATE_POOLING_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_POOLING_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_POOLING_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandPooling(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandReduction:
        {
            META_COMMAND_CREATE_REDUCTION_DESC *  pCreateDesc = (META_COMMAND_CREATE_REDUCTION_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_REDUCTION_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_REDUCTION_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandReduction(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandRNN:
        {
            META_COMMAND_CREATE_RNN_DESC *  pCreateDesc = (META_COMMAND_CREATE_RNN_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_RNN_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_RNN_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandRNN(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandRoiPooling:
        {
            META_COMMAND_CREATE_ROI_POOLING_DESC *  pCreateDesc = (META_COMMAND_CREATE_ROI_POOLING_DESC *)(pMetaCommand + 1);
            META_COMMAND_EXECUTE_ROI_POOLING_DESC * pExecuteDesc = (META_COMMAND_EXECUTE_ROI_POOLING_DESC *)(pCreateDesc + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pExecuteDesc, sizeof(*pExecuteDesc)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandRoiPooling(pCreateDesc, pExecuteDesc);
        }
        break;
    case MetaCommandCopyTensor:
        {
            HW_META_COMMAND_COPY_TENSOR *  pHwMetaCommand = (HW_META_COMMAND_COPY_TENSOR *)(pMetaCommand + 1);
            HW_IO_TABLE_COPY_TENSOR * pHwIoTable = (HW_IO_TABLE_COPY_TENSOR *)(pHwMetaCommand + 1);

            CosKmFixupResourceCpuAddress((D3D12_GPU_DESCRIPTOR_HANDLE *)pHwIoTable, sizeof(*pHwIoTable)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
            CosKmExecuteMetaCommandCopyTensor(pHwMetaCommand, pHwIoTable);
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

