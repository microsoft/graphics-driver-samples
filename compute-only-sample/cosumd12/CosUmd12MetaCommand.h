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
        CONST void *pvInitializeDesc,
        SIZE_T initializeDescSize) = NULL;

    virtual void
    Compile() = NULL;

    virtual void
    Execute(
        CosUmd12CommandList * pCommandList,
        CONST void *pvExecuteDesc,
        SIZE_T executeDescSize) = NULL;

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

template <typename TCreateDesc, typename TInitializeDesc, typename TExecuteDesc, MetaCommandId TMetaCommandId>
class TCosUmd12MetaCommand : public CosUmd12MetaCommand
{
public:
    explicit
    TCosUmd12MetaCommand(
        CosUmd12Device* pDevice,
        UINT nodeMask,
        CONST void* pvCreateDesc,
        SIZE_T createDescSizeInBytes,
        D3D12DDI_HRTMETACOMMAND_0052 rtMetaCommand)
    {
        m_pDevice = pDevice;
        m_rtMetaCommand = rtMetaCommand;

        if (sizeof(TCreateDesc) != createDescSizeInBytes)
        {
            m_pDevice->m_pUMCallbacks->pfnSetErrorCb(m_pDevice->m_hRTDevice, E_INVALIDARG);
            return;
        }

        memcpy(&m_createDesc, pvCreateDesc, createDescSizeInBytes);

        memset(&m_initializeDesc, 0, sizeof(m_initializeDesc));
        memset(&m_executeDesc, 0, sizeof(m_executeDesc));
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
        D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo)
    {
        pInfo->ResourceSize = 0;
    }

    virtual void
    Initialize(
        CONST void *pvInitializeDesc,
        SIZE_T initializeDescSize)
    {
        if (0 == initializeDescSize)
        {
            return;
        }

        if (sizeof(m_initializeDesc) != initializeDescSize)
        {
            m_pDevice->m_pUMCallbacks->pfnSetErrorCb(m_pDevice->m_hRTDevice, E_INVALIDARG);
            return;
        }

        memcpy(&m_initializeDesc, pvInitializeDesc, initializeDescSize);
    }

    //
    // HW driver can compile meta command code at creation or intialization time
    //
    virtual void
    Compile();

    virtual void
    Execute(
        CosUmd12CommandList * pCommandList,
        CONST void *pvExecuteDesc,
        SIZE_T executeDescSize)
    {
        //
        // Record the latest execute desc
        //
        assert(sizeof(m_executeDesc) == executeDescSize);
        memcpy(&m_executeDesc, pvExecuteDesc, sizeof(m_executeDesc));

        pCommandList->ExecuteMlMetaCommand(&m_createDesc, &m_executeDesc, TMetaCommandId);
    }

protected:
    CosUmd12Device * m_pDevice;
    D3D12DDI_HRTMETACOMMAND_0052 m_rtMetaCommand;
    TCreateDesc m_createDesc;
    TInitializeDesc m_initializeDesc;
    TExecuteDesc m_executeDesc;
};

class CosUmd12MetaCommandIdentity : public TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, UINT, UINT, MetaCommandIdentity>
{
public:
    CosUmd12MetaCommandIdentity(
        CosUmd12Device* pDevice,
        UINT nodeMask,
        CONST void* pvCreateDesc,
        SIZE_T createDescSizeInBytes,
        D3D12DDI_HRTMETACOMMAND_0052 rtMetaCommand) :
        TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, UINT, UINT, MetaCommandIdentity>(
            pDevice,
            nodeMask,
            pvCreateDesc,
            createDescSizeInBytes,
            rtMetaCommand)
    {
    };

    virtual void
    GetRequiredParameterInfo(
        D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
        UINT parameterIndex,
        D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo);

    virtual void
    Initialize(
        CONST void *pvInitializeDesc,
        SIZE_T initializeDescSize);

    virtual void
    Compile();

    virtual void
    Execute(
        CosUmd12CommandList * pCommandList,
        CONST void *pvExecuteDesc,
        SIZE_T executeDescSize);
};

#if MLMC

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_NORMALIZATION_DESC, META_COMMAND_INITIALIZE_NORMALIZATION_DESC, META_COMMAND_EXECUTE_NORMALIZATION_DESC, MetaCommandNormalization> CosUmd12MetaCommandNormalization;
typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_CONVOLUTION_DESC, META_COMMAND_INITIALIZE_CONVOLUTION_DESC, META_COMMAND_EXECUTE_CONVOLUTION_DESC, MetaCommandConvolution> CosUmd12MetaCommandConvolution;

#endif
