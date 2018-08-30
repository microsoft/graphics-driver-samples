////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Meta Command implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

void
TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, UINT, UINT, MetaCommandIdentity>::Compile()
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

#if MLMC

const UINT CosUmd12MetaCommandNormalization::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandNormalization::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandNormalization::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandNormalization::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandNormalization::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandNormalization::m_executionParametersDesc[];

void
CosUmd12MetaCommandNormalization::Compile()
{
}

const UINT CosUmd12MetaCommandConvolution::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandConvolution::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandConvolution::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandConvolution::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandConvolution::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandConvolution::m_executionParametersDesc[];

void
CosUmd12MetaCommandConvolution::Compile()
{
}

const UINT CosUmd12MetaCommandGEMM::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGEMM::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandGEMM::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGEMM::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandGEMM::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGEMM::m_executionParametersDesc[];

void
CosUmd12MetaCommandGEMM::Compile()
{
}

const UINT CosUmd12MetaCommandGRU::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGRU::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandGRU::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGRU::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandGRU::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandGRU::m_executionParametersDesc[];

void
CosUmd12MetaCommandGRU::Compile()
{
}

const UINT CosUmd12MetaCommandLSTM::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandLSTM::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandLSTM::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandLSTM::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandLSTM::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandLSTM::m_executionParametersDesc[];

void
CosUmd12MetaCommandLSTM::Compile()
{
}

const UINT CosUmd12MetaCommandMVN::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandMVN::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandMVN::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandMVN::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandMVN::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandMVN::m_executionParametersDesc[];

void
CosUmd12MetaCommandMVN::Compile()
{
}

const UINT CosUmd12MetaCommandPooling::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandPooling::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandPooling::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandPooling::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandPooling::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandPooling::m_executionParametersDesc[];

void
CosUmd12MetaCommandPooling::Compile()
{
}

const UINT CosUmd12MetaCommandReduction::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandReduction::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandReduction::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandReduction::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandReduction::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandReduction::m_executionParametersDesc[];

void
CosUmd12MetaCommandReduction::Compile()
{
}

const UINT CosUmd12MetaCommandRNN::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRNN::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandRNN::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRNN::m_executionParametersDesc[];

const UINT CosUmd12MetaCommandRNN::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRNN::m_initializationParametersDesc[];

void
CosUmd12MetaCommandRNN::Compile()
{
}

const UINT CosUmd12MetaCommandRoiPooling::m_numCreationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRoiPooling::m_creationParametersDesc[];

const UINT CosUmd12MetaCommandRoiPooling::m_numInitializationParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRoiPooling::m_initializationParametersDesc[];

const UINT CosUmd12MetaCommandRoiPooling::m_numExecutionParameters = 0;
const D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandRoiPooling::m_executionParametersDesc[];

void
CosUmd12MetaCommandRoiPooling::Compile()
{
}

#endif
