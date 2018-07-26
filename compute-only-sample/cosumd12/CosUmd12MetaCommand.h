#pragma once

#include "CosUmd12.h"

class CosUmd12MetaCommand
{
public:
    virtual void
    GetRequiredParameterInfo(
        D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
        UINT parameterIndex,
        _Out_ D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo) = NULL;

    virtual void
    Initialize(
        CONST void *pInitializationParameters,
        SIZE_T initializationParametersSize) = NULL;

    virtual void
    Execute(
        CosUmd12CommandList * pCommandList,
        CONST void *pExecutionParameters,
        SIZE_T executionParametersSize) = NULL;

    static CosUmd12MetaCommand* CastFrom(D3D12DDI_HMETACOMMAND_0052);
    D3D12DDI_HMETACOMMAND_0052 CastTo() const;
};

inline CosUmd12MetaCommand* CosUmd12MetaCommand::CastFrom(D3D12DDI_HMETACOMMAND_0052 hMetaCommand)
{
    return static_cast< CosUmd12MetaCommand* >(hMetaCommand.pDrvPrivate);
}

inline D3D12DDI_HMETACOMMAND_0052 CosUmd12MetaCommand::CastTo() const
{
    return MAKE_D3D12DDI_HMETACOMMAND_0052(const_cast< CosUmd12MetaCommand* >(this));
}

class CosUmd12MetaCommandIdentity
{
public:
    explicit 
    CosUmd12MetaCommandIdentity(
        CosUmd12Device* pDevice,
        UINT nodeMask,
        CONST void* pCreationParameters,
        SIZE_T creationParametersDataSizeInBytes,
        D3D12DDI_HRTMETACOMMAND_0052 rtMetaCommand)
    {
        m_pDevice = pDevice;
        memcpy(&m_identityMetaCommandCreationParameters, pCreationParameters, creationParametersDataSizeInBytes);
        m_rtMetaCommand = rtMetaCommand;
    }

    ~CosUmd12MetaCommandIdentity()
    {
    }

    static int CalculateSize(GUID& commandId)
    {
        return sizeof(CosUmd12MetaCommandIdentity);
    }

    static D3D12DDIARG_META_COMMAND_PARAMETER_DESC m_identityMetaCommandCreationParametersDesc[];
    static D3D12DDIARG_META_COMMAND_PARAMETER_DESC m_identityMetaCommandExecutionParametersDesc[];

    static HRESULT EnumerateMetaCommandParameters(
        D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
        UINT* pParameterCount,
        D3D12DDIARG_META_COMMAND_PARAMETER_DESC* pParameterDescs);

    virtual void
    GetRequiredParameterInfo(
        D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
        UINT parameterIndex,
        D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo);

    virtual void
    Initialize(
        CONST void *pInitializationParameters,
        SIZE_T initializationParametersSize);

    virtual void
    Execute(
        CosUmd12CommandList * pCommandList,
        CONST void *pExecutionParameters,
        SIZE_T executionParametersSize);

private:
    CosUmd12Device * m_pDevice;
    IdentityMetaCommandCreationParameters m_identityMetaCommandCreationParameters;
    D3D12DDI_HRTMETACOMMAND_0052 m_rtMetaCommand;
};

