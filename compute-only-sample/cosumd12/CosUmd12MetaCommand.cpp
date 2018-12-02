////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Meta Command implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

void
TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, UINT, UINT, MetaCommandIdentity, UINT, UINT>::Compile()
{
}

void
TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, UINT, UINT, MetaCommandIdentity, UINT, UINT>::BindHwIoTableAndReadyHwMetaCommand()
{
}

const UINT CosUmd12MetaCommandIdentity::m_numCreationParameters = 1;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandIdentity::m_creationParametersDesc[] =
{
    { L"Size", D3D12DDI_META_COMMAND_PARAMETER_TYPE_UINT64, D3D12DDI_META_COMMAND_PARAMETER_FLAG_INPUT, D3D12DDI_RESOURCE_STATE_COMMON }
};

const UINT CosUmd12MetaCommandIdentity::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandIdentity::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandIdentity::m_numExecutionParameters = 2;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandIdentity::m_executionParametersDesc[] =
{
    { L"InputBuffer",  D3D12DDI_META_COMMAND_PARAMETER_TYPE_GPU_VIRTUAL_ADDRESS, D3D12DDI_META_COMMAND_PARAMETER_FLAG_INPUT,  D3D12DDI_RESOURCE_STATE_UNORDERED_ACCESS },
    { L"OutputBuffer", D3D12DDI_META_COMMAND_PARAMETER_TYPE_GPU_VIRTUAL_ADDRESS, D3D12DDI_META_COMMAND_PARAMETER_FLAG_OUTPUT, D3D12DDI_RESOURCE_STATE_UNORDERED_ACCESS }
};

const UINT CosUmd12MetaCommandIdentity::m_numTensorDescriptor = 0;
const UINT CosUmd12MetaCommandIdentity::m_tensorDescriptorOffsets[] = { 0 };
const UINT CosUmd12MetaCommandIdentity::m_tensorDescriptorSizes[] = { 0 };

void
CosUmd12MetaCommandIdentity::GetRequiredParameterInfo(
    D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
    UINT parameterIndex,
    _Out_ D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo)
{
    pInfo->ResourceSize = m_createDesc.BufferSize;
}

void
CosUmd12MetaCommandIdentity::Initialize(
    CONST void *pvInitializeDesc,
    SIZE_T initializeDescSize)
{
}

void
CosUmd12MetaCommandIdentity::Compile()
{
}

void
CosUmd12MetaCommandIdentity::BindHwIoTableAndReadyHwMetaCommand()
{
}

void
CosUmd12MetaCommandIdentity::Execute(
    CosUmd12CommandList * pCommandList,
    CONST void *pvExecuteDesc,
    SIZE_T executeDescSize)
{
    IdentityMetaCommandExecutionParameters * pExecutionParameters = (IdentityMetaCommandExecutionParameters *)pvExecuteDesc;

    pCommandList->GpuMemoryCopy(
        pExecutionParameters->Output,
        pExecutionParameters->Input,
        (UINT)m_createDesc.BufferSize);
}

#if COS_MLMC_RS5_SUPPORT

const UINT CosUmd12MetaCommandNormalization::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandNormalization::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandNormalization::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandNormalization::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandNormalization::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandNormalization::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandNormalization::m_numTensorDescriptor = 6;

const UINT CosUmd12MetaCommandNormalization::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_NORMALIZATION_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_NORMALIZATION_DESC, DescMean),
    FIELD_OFFSET(META_COMMAND_CREATE_NORMALIZATION_DESC, DescVariance),
    FIELD_OFFSET(META_COMMAND_CREATE_NORMALIZATION_DESC, DescScale),
    FIELD_OFFSET(META_COMMAND_CREATE_NORMALIZATION_DESC, DescBias),
    FIELD_OFFSET(META_COMMAND_CREATE_NORMALIZATION_DESC, DescOut)
};

const UINT CosUmd12MetaCommandNormalization::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_NORMALIZATION_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_NORMALIZATION_DESC::DescMean),
    sizeof(META_COMMAND_CREATE_NORMALIZATION_DESC::DescVariance),
    sizeof(META_COMMAND_CREATE_NORMALIZATION_DESC::DescScale),
    sizeof(META_COMMAND_CREATE_NORMALIZATION_DESC::DescBias),
    sizeof(META_COMMAND_CREATE_NORMALIZATION_DESC::DescOut)
};

void
CosUmd12MetaCommandNormalization::Compile()
{
}

void
CosUmd12MetaCommandNormalization::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandConvolution::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandConvolution::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandConvolution::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandConvolution::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandConvolution::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandConvolution::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandConvolution::m_numTensorDescriptor = 4;

const UINT CosUmd12MetaCommandConvolution::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_CONVOLUTION_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_CONVOLUTION_DESC, DescFilter),
    FIELD_OFFSET(META_COMMAND_CREATE_CONVOLUTION_DESC, DescBias),
    FIELD_OFFSET(META_COMMAND_CREATE_CONVOLUTION_DESC, DescOut)
};

const UINT CosUmd12MetaCommandConvolution::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_CONVOLUTION_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_CONVOLUTION_DESC::DescFilter),
    sizeof(META_COMMAND_CREATE_CONVOLUTION_DESC::DescBias),
    sizeof(META_COMMAND_CREATE_CONVOLUTION_DESC::DescOut)
};

void
CosUmd12MetaCommandConvolution::Compile()
{
}

void
CosUmd12MetaCommandConvolution::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandGEMM::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGEMM::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandGEMM::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGEMM::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandGEMM::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGEMM::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandGEMM::m_numTensorDescriptor = 4;

const UINT CosUmd12MetaCommandGEMM::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_GEMM_DESC, DescA),
    FIELD_OFFSET(META_COMMAND_CREATE_GEMM_DESC, DescB),
    FIELD_OFFSET(META_COMMAND_CREATE_GEMM_DESC, DescC),
    FIELD_OFFSET(META_COMMAND_CREATE_GEMM_DESC, DescOut),
};

const UINT CosUmd12MetaCommandGEMM::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_GEMM_DESC::DescA),
    sizeof(META_COMMAND_CREATE_GEMM_DESC::DescB),
    sizeof(META_COMMAND_CREATE_GEMM_DESC::DescC),
    sizeof(META_COMMAND_CREATE_GEMM_DESC::DescOut)
};

void
CosUmd12MetaCommandGEMM::Compile()
{
}

void
CosUmd12MetaCommandGEMM::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandGRU::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGRU::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandGRU::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGRU::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandGRU::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGRU::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandGRU::m_numTensorDescriptor = 8;

const UINT CosUmd12MetaCommandGRU::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescWeight),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescRecurrence),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescBias),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescHiddenInit),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescSeqLengths),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescOutSingle),
    FIELD_OFFSET(META_COMMAND_CREATE_GRU_DESC, DescOutSequence)
};

const UINT CosUmd12MetaCommandGRU::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescWeight),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescRecurrence),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescBias),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescHiddenInit),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescSeqLengths),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescOutSingle),
    sizeof(META_COMMAND_CREATE_GRU_DESC::DescOutSequence)
};

void
CosUmd12MetaCommandGRU::Compile()
{
}

void
CosUmd12MetaCommandGRU::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandLSTM::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandLSTM::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandLSTM::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandLSTM::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandLSTM::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandLSTM::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandLSTM::m_numTensorDescriptor = 11;

const UINT CosUmd12MetaCommandLSTM::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescWeight),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescRecurrence),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescBias),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescHiddenInit),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescCellMemInit),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescSeqLengths),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescPeephole),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescOutSingle),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescOutSequence),
    FIELD_OFFSET(META_COMMAND_CREATE_LSTM_DESC, DescOutCellSingle),
};

const UINT CosUmd12MetaCommandLSTM::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescWeight),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescRecurrence),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescBias),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescHiddenInit),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescCellMemInit),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescSeqLengths),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescPeephole),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescOutSingle),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescOutSequence),
    sizeof(META_COMMAND_CREATE_LSTM_DESC::DescOutCellSingle),
};

void
CosUmd12MetaCommandLSTM::Compile()
{
}

void
CosUmd12MetaCommandLSTM::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandMVN::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandMVN::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandMVN::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandMVN::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandMVN::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandMVN::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandMVN::m_numTensorDescriptor = 4;

const UINT CosUmd12MetaCommandMVN::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_MVN_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_MVN_DESC, DescScale),
    FIELD_OFFSET(META_COMMAND_CREATE_MVN_DESC, DescBias),
    FIELD_OFFSET(META_COMMAND_CREATE_MVN_DESC, DescOut)
};

const UINT CosUmd12MetaCommandMVN::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_MVN_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_MVN_DESC::DescScale),
    sizeof(META_COMMAND_CREATE_MVN_DESC::DescBias),
    sizeof(META_COMMAND_CREATE_MVN_DESC::DescOut)
};

void
CosUmd12MetaCommandMVN::Compile()
{
}

void
CosUmd12MetaCommandMVN::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandPooling::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandPooling::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandPooling::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandPooling::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandPooling::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandPooling::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandPooling::m_numTensorDescriptor = 2;

const UINT CosUmd12MetaCommandPooling::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_POOLING_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_POOLING_DESC, DescOut)
};

const UINT CosUmd12MetaCommandPooling::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_POOLING_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_POOLING_DESC::DescOut)
};

void
CosUmd12MetaCommandPooling::Compile()
{
}

void
CosUmd12MetaCommandPooling::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandReduction::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandReduction::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandReduction::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandReduction::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandReduction::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandReduction::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandReduction::m_numTensorDescriptor = 2;

const UINT CosUmd12MetaCommandReduction::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_REDUCTION_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_REDUCTION_DESC, DescOut)
};

const UINT CosUmd12MetaCommandReduction::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_REDUCTION_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_REDUCTION_DESC::DescOut)
};

void
CosUmd12MetaCommandReduction::Compile()
{
}

void
CosUmd12MetaCommandReduction::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandRNN::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRNN::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandRNN::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRNN::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandRNN::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRNN::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandRNN::m_numTensorDescriptor = 8;

const UINT CosUmd12MetaCommandRNN::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescWeight),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescRecurrence),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescBias),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescHiddenInit),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescSeqLengths),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescOutSingle),
    FIELD_OFFSET(META_COMMAND_CREATE_RNN_DESC, DescOutSequence)
};

const UINT CosUmd12MetaCommandRNN::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescWeight),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescRecurrence),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescBias),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescHiddenInit),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescSeqLengths),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescOutSingle),
    sizeof(META_COMMAND_CREATE_RNN_DESC::DescOutSequence)
};

void
CosUmd12MetaCommandRNN::Compile()
{
}

void
CosUmd12MetaCommandRNN::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandRoiPooling::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRoiPooling::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandRoiPooling::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRoiPooling::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandRoiPooling::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRoiPooling::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandRoiPooling::m_numTensorDescriptor = 3;

const UINT CosUmd12MetaCommandRoiPooling::m_tensorDescriptorOffsets[] =
{
    FIELD_OFFSET(META_COMMAND_CREATE_ROI_POOLING_DESC, DescIn),
    FIELD_OFFSET(META_COMMAND_CREATE_ROI_POOLING_DESC, DescRoi),
    FIELD_OFFSET(META_COMMAND_CREATE_ROI_POOLING_DESC, DescOut)
};

const UINT CosUmd12MetaCommandRoiPooling::m_tensorDescriptorSizes[] =
{
    sizeof(META_COMMAND_CREATE_ROI_POOLING_DESC::DescIn),
    sizeof(META_COMMAND_CREATE_ROI_POOLING_DESC::DescRoi),
    sizeof(META_COMMAND_CREATE_ROI_POOLING_DESC::DescOut)
};

void
CosUmd12MetaCommandRoiPooling::Compile()
{
}

void
CosUmd12MetaCommandRoiPooling::BindHwIoTableAndReadyHwMetaCommand()
{
    memcpy(&m_hwIoTable, &m_executeDesc, sizeof(m_hwIoTable));

    memcpy(&m_hwMetaCommand, &m_createDesc, sizeof(m_hwMetaCommand));
}

const UINT CosUmd12MetaCommandCopyTensor::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandCopyTensor::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandCopyTensor::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandCopyTensor::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandCopyTensor::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandCopyTensor::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandCopyTensor::m_numTensorDescriptor = 0;

const UINT CosUmd12MetaCommandCopyTensor::m_tensorDescriptorOffsets[] = { 0 };

const UINT CosUmd12MetaCommandCopyTensor::m_tensorDescriptorSizes[] = { 0 };

void
CosUmd12MetaCommandCopyTensor::Compile()
{
}

void
CosUmd12MetaCommandCopyTensor::BindHwIoTableAndReadyHwMetaCommand()
{
    m_hwIoTable.DstResource = m_executeDesc.DstResource;
    m_hwIoTable.SrcResource = m_executeDesc.SrcResource;
    m_hwIoTable.TemporaryResource = m_executeDesc.TemporaryResource;

    m_hwMetaCommand.DstDesc = m_executeDesc.DstDesc;
    m_hwMetaCommand.SrcDesc = m_executeDesc.SrcDesc;
    m_hwMetaCommand.BindFlags = m_createDesc.BindFlags;
}

#endif
