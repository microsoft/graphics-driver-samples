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

template <typename TCreationParameters, typename TInitializationParameters, typename TExecutionParameters>
class TCosUmd12MetaCommand : public CosUmd12MetaCommand
{
public:
    explicit
        TCosUmd12MetaCommand(
            CosUmd12Device* pDevice,
            UINT nodeMask,
            CONST void* pCreationParameters,
            SIZE_T creationParametersDataSizeInBytes,
            D3D12DDI_HRTMETACOMMAND_0052 rtMetaCommand)
    {
        m_pDevice = pDevice;
        m_rtMetaCommand = rtMetaCommand;

        if (sizeof(TCreationParameters) != creationParametersDataSizeInBytes)
        {
            m_pDevice->m_pUMCallbacks->pfnSetErrorCb(m_pDevice->m_hRTDevice, E_INVALIDARG);
            return;
        }

        memcpy(&m_creationParameters, pCreationParameters, creationParametersDataSizeInBytes);
    }

    ~TCosUmd12MetaCommand()
    {
    }

    static int CalculateSize(GUID& commandId)
    {
        return sizeof(TCosUmd12MetaCommand);
    }

    static const UINT m_numCreationParameters;
    static const D3D12DDIARG_META_COMMAND_PARAMETER_DESC m_creationParametersDesc[];
    static const UINT m_numInitializationParameters;
    static const D3D12DDIARG_META_COMMAND_PARAMETER_DESC m_initializationParametersDesc[];
    static const UINT m_numExecutionParameters;
    static const D3D12DDIARG_META_COMMAND_PARAMETER_DESC m_executionParametersDesc[];

    static HRESULT EnumerateMetaCommandParameters(
        D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
        UINT* pParameterCount,
        D3D12DDIARG_META_COMMAND_PARAMETER_DESC* pParameterDescs)
    {
        switch (stage)
        {
        case D3D12DDI_META_COMMAND_PARAMETER_STAGE_CREATION:
            *pParameterCount = m_numCreationParameters;
            if (pParameterDescs)
            {
                memcpy(pParameterDescs, m_creationParametersDesc, m_numCreationParameters * sizeof(D3D12DDIARG_META_COMMAND_PARAMETER_DESC));
            }
            break;
        case D3D12DDI_META_COMMAND_PARAMETER_STAGE_INITIALIZATION:
            *pParameterCount = m_numInitializationParameters;
            if (pParameterDescs)
            {
                memcpy(pParameterDescs, m_initializationParametersDesc, m_numInitializationParameters * sizeof(D3D12DDIARG_META_COMMAND_PARAMETER_DESC));
            }
            break;
        case D3D12DDI_META_COMMAND_PARAMETER_STAGE_EXECUTION:
            *pParameterCount = m_numExecutionParameters;
            if (pParameterDescs)
            {
                memcpy(pParameterDescs, m_executionParametersDesc, m_numExecutionParameters * sizeof(D3D12DDIARG_META_COMMAND_PARAMETER_DESC));
            }
            break;
        }

        return S_OK;
    }

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
    D3D12DDI_HRTMETACOMMAND_0052 m_rtMetaCommand;
    TCreationParameters m_creationParameters;
    TInitializationParameters m_initializationParameters;
    TExecutionParameters m_executionParameters;
};

typedef TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, UINT, UINT> CosUmd12MetaCommandIdentity;

