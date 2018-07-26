////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Meta Command implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandIdentity::m_identityMetaCommandCreationParametersDesc[] =
{
    { L"Size", D3D12DDI_META_COMMAND_PARAMETER_TYPE_UINT64, D3D12DDI_META_COMMAND_PARAMETER_FLAG_INPUT, D3D12DDI_RESOURCE_STATE_COMMON }
};

D3D12DDIARG_META_COMMAND_PARAMETER_DESC CosUmd12MetaCommandIdentity::m_identityMetaCommandExecutionParametersDesc[] =
{
    { L"InputBuffer",  D3D12DDI_META_COMMAND_PARAMETER_TYPE_GPU_VIRTUAL_ADDRESS, D3D12DDI_META_COMMAND_PARAMETER_FLAG_INPUT,  D3D12DDI_RESOURCE_STATE_UNORDERED_ACCESS },
    { L"OutputBuffer", D3D12DDI_META_COMMAND_PARAMETER_TYPE_GPU_VIRTUAL_ADDRESS, D3D12DDI_META_COMMAND_PARAMETER_FLAG_OUTPUT, D3D12DDI_RESOURCE_STATE_UNORDERED_ACCESS }
};


HRESULT CosUmd12MetaCommandIdentity::EnumerateMetaCommandParameters(
    D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
    _Inout_ UINT* pParameterCount,
    _Out_writes_opt_(*pParameterCount) D3D12DDIARG_META_COMMAND_PARAMETER_DESC* pParameterDescs)
{
    switch (stage)
    {
    case D3D12DDI_META_COMMAND_PARAMETER_STAGE_CREATION:
        *pParameterCount = _countof(m_identityMetaCommandCreationParametersDesc);
        if (pParameterDescs)
        {
            memcpy(pParameterDescs, m_identityMetaCommandCreationParametersDesc, sizeof(m_identityMetaCommandCreationParametersDesc));
        }
        break;
    case D3D12DDI_META_COMMAND_PARAMETER_STAGE_INITIALIZATION:
        *pParameterCount = 0;
        break;
    case D3D12DDI_META_COMMAND_PARAMETER_STAGE_EXECUTION:
        *pParameterCount = _countof(m_identityMetaCommandExecutionParametersDesc);
        if (pParameterDescs)
        {
            memcpy(pParameterDescs, m_identityMetaCommandExecutionParametersDesc, sizeof(m_identityMetaCommandExecutionParametersDesc));
        }
        break;
    }

    return S_OK;
}

void
CosUmd12MetaCommandIdentity::GetRequiredParameterInfo(
    D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
    UINT parameterIndex,
    _Out_ D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo)
{
    pInfo->ResourceSize = m_identityMetaCommandCreationParameters.BufferSize;
}

void
CosUmd12MetaCommandIdentity::Initialize(
    CONST void *pInitializationParameters,
    SIZE_T initializationParametersSize)
{
}

void
CosUmd12MetaCommandIdentity::Execute(
    CosUmd12CommandList * pCommandList,
    CONST void *pExecutionParameters,
    SIZE_T executionParametersSize)
{
    IdentityMetaCommandExecutionParameters * pIdentityMetaCommandExecutionParameters = (IdentityMetaCommandExecutionParameters *)pExecutionParameters;

    pCommandList->GpuMemoryCopy(
        pIdentityMetaCommandExecutionParameters->Output,
        pIdentityMetaCommandExecutionParameters->Input,
        (UINT)m_identityMetaCommandCreationParameters.BufferSize);
}

