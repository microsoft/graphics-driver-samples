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

template <typename TCreateDesc,
          typename TInitializeDesc,
          typename TExecuteDesc,
          MetaCommandId TMetaCommandId,
          typename THWMetaCommand,
          typename THWIoTable>
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

        memset(&m_hwMetaCommand, 0, sizeof(m_hwMetaCommand));
        memset(&m_hwIoTable, 0, sizeof(m_hwIoTable));

        //
        // If m_hwMetaCommand contains code for the GPU, it is preferred to happen here
        //
        Compile();
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

    static const UINT m_numTensorDescriptor;
    static const UINT m_tensorDescriptorOffsets[];
    static const UINT m_tensorDescriptorSizes[];

    virtual void
    GetRequiredParameterInfo(
        D3D12DDI_META_COMMAND_PARAMETER_STAGE stage,
        UINT parameterIndex,
        D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo)
    {
        ASSERT(D3D12DDI_META_COMMAND_PARAMETER_STAGE_EXECUTION == stage);

#if COS_MLMC_RS5_SUPPORT

        //
        // When CopyTensor Meta Command is implemented, tensor resource's size
        // must be calculated based on a particular tensor's chosen HW layout.
        //
        // parameterIndex is the index for tensor resources in META_COMMAND_EXECUTE_*_DESC
        //
        // Except for PersistentResource and TemporaryResource which are internal to the 
        // driver, META_COMMAND_TENSOR_DESC or META_COMMAND_OPTIONAL_TENSOR_DESC for other
        // tensor resources can be found by parameterIndex in META_COMMAND_CREATE_*_DESC
        //

        if (parameterIndex >= m_numTensorDescriptor)
        {
            pInfo->ResourceSize = 0;

            return;
        }
        
        META_COMMAND_TENSOR_DESC * pTensorDesc = NULL;

        if (sizeof(META_COMMAND_OPTIONAL_TENSOR_DESC) == m_tensorDescriptorSizes[parameterIndex])
        {
            META_COMMAND_OPTIONAL_TENSOR_DESC * pOptinalTensorDesc = (META_COMMAND_OPTIONAL_TENSOR_DESC *)(((PBYTE)&m_createDesc) + m_tensorDescriptorOffsets[parameterIndex]);

            if (pOptinalTensorDesc->IsNull)
            {
                pInfo->ResourceSize = 0;

                return;
            }
            else
            {
                pTensorDesc = (META_COMMAND_TENSOR_DESC *)pOptinalTensorDesc;
            }
        }
        else
        {
            pTensorDesc = (META_COMMAND_TENSOR_DESC *)(((PBYTE)&m_createDesc) + m_tensorDescriptorOffsets[parameterIndex]);
        }
        
        UINT    elementSize;

        switch (pTensorDesc->DataType)
        {
        case META_COMMAND_TENSOR_DATA_TYPE_FLOAT16:
            elementSize = 2;
            break;
        case META_COMMAND_TENSOR_DATA_TYPE_FLOAT32:
        case META_COMMAND_TENSOR_DATA_TYPE_UINT32:
            elementSize = 4;
            break;
        default:
            ASSERT(false);
            elementSize = 0;
            break;
        }

        if (pTensorDesc->Stride[0])
        {
            pInfo->ResourceSize = pTensorDesc->Stride[0] * pTensorDesc->Size[0] * elementSize;
        }
        else
        {
            //
            // Stride of 0 indicates there is only 1 element that is replicated
            //

            pInfo->ResourceSize = elementSize;
        }

        //
        // Align the size to DWORD
        //

        pInfo->ResourceSize = (pInfo->ResourceSize + sizeof(UINT) - 1) & (~(sizeof(UINT) - 1));
#else

        pInfo->ResourceSize = 0;

#endif
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
    BindHwIoTableAndReadyHwMetaCommand();

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

        BindHwIoTableAndReadyHwMetaCommand();

        pCommandList->ExecuteMlMetaCommand(&m_hwMetaCommand, &m_hwIoTable, TMetaCommandId);
    }

protected:
    CosUmd12Device * m_pDevice;
    D3D12DDI_HRTMETACOMMAND_0052 m_rtMetaCommand;
    
    TCreateDesc m_createDesc;
    TInitializeDesc m_initializeDesc;
    TExecuteDesc m_executeDesc;

    THWMetaCommand m_hwMetaCommand;
    THWIoTable m_hwIoTable;
};

class CosUmd12MetaCommandIdentity : public TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters,
                                                                UINT,
                                                                UINT,
                                                                MetaCommandIdentity,
                                                                UINT,
                                                                UINT>
{
public:
    CosUmd12MetaCommandIdentity(
        CosUmd12Device* pDevice,
        UINT nodeMask,
        CONST void* pvCreateDesc,
        SIZE_T createDescSizeInBytes,
        D3D12DDI_HRTMETACOMMAND_0052 rtMetaCommand) :
        TCosUmd12MetaCommand<IdentityMetaCommandCreationParameters, 
                             UINT,
                             UINT,
                             MetaCommandIdentity,
                             UINT,
                             UINT>(
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
    BindHwIoTableAndReadyHwMetaCommand();

    virtual void
    Execute(
        CosUmd12CommandList * pCommandList,
        CONST void *pvExecuteDesc,
        SIZE_T executeDescSize);
};

#if COS_MLMC_RS5_SUPPORT

//
// COSD reuses :
//
//     1. META_COMMAND_CREATE_*_DESC for THWMetaCommand
//     2. META_COMMAND_EXECUTE_*_DESC for THWIoTable
//
// for showing how meta command info flows from UMD to KMD
//

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_NORMALIZATION_DESC,
                             META_COMMAND_INITIALIZE_NORMALIZATION_DESC,
                             META_COMMAND_EXECUTE_NORMALIZATION_DESC,
                             MetaCommandNormalization,
                             META_COMMAND_CREATE_NORMALIZATION_DESC,
                             META_COMMAND_EXECUTE_NORMALIZATION_DESC> CosUmd12MetaCommandNormalization;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_CONVOLUTION_DESC,
                             META_COMMAND_INITIALIZE_CONVOLUTION_DESC,
                             META_COMMAND_EXECUTE_CONVOLUTION_DESC,
                             MetaCommandConvolution,
                             META_COMMAND_CREATE_CONVOLUTION_DESC,
                             META_COMMAND_EXECUTE_CONVOLUTION_DESC> CosUmd12MetaCommandConvolution;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_GEMM_DESC,
                             META_COMMAND_INITIALIZE_GEMM_DESC,
                             META_COMMAND_EXECUTE_GEMM_DESC,
                             MetaCommandGEMM, 
                             META_COMMAND_CREATE_GEMM_DESC,
                             META_COMMAND_EXECUTE_GEMM_DESC> CosUmd12MetaCommandGEMM;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_GRU_DESC,
                             META_COMMAND_INITIALIZE_GRU_DESC,
                             META_COMMAND_EXECUTE_GRU_DESC,
                             MetaCommandGRU,
                             META_COMMAND_CREATE_GRU_DESC,
                             META_COMMAND_EXECUTE_GRU_DESC> CosUmd12MetaCommandGRU;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_LSTM_DESC,
                             META_COMMAND_INITIALIZE_LSTM_DESC,
                             META_COMMAND_EXECUTE_LSTM_DESC,
                             MetaCommandLSTM,
                             META_COMMAND_CREATE_LSTM_DESC,
                             META_COMMAND_EXECUTE_LSTM_DESC> CosUmd12MetaCommandLSTM;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_MVN_DESC,
                             META_COMMAND_INITIALIZE_MVN_DESC,
                             META_COMMAND_EXECUTE_MVN_DESC,
                             MetaCommandMVN,
                             META_COMMAND_CREATE_MVN_DESC,
                             META_COMMAND_EXECUTE_MVN_DESC> CosUmd12MetaCommandMVN;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_POOLING_DESC,
                             META_COMMAND_INITIALIZE_POOLING_DESC,
                             META_COMMAND_EXECUTE_POOLING_DESC,
                             MetaCommandPooling,
                             META_COMMAND_CREATE_POOLING_DESC,
                             META_COMMAND_EXECUTE_POOLING_DESC> CosUmd12MetaCommandPooling;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_REDUCTION_DESC,
                             META_COMMAND_INITIALIZE_REDUCTION_DESC,
                             META_COMMAND_EXECUTE_REDUCTION_DESC,
                             MetaCommandReduction,
                             META_COMMAND_CREATE_REDUCTION_DESC,
                             META_COMMAND_EXECUTE_REDUCTION_DESC> CosUmd12MetaCommandReduction;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_RNN_DESC,
                             META_COMMAND_INITIALIZE_RNN_DESC,
                             META_COMMAND_EXECUTE_RNN_DESC,
                             MetaCommandRNN,
                             META_COMMAND_CREATE_RNN_DESC,
                             META_COMMAND_EXECUTE_RNN_DESC> CosUmd12MetaCommandRNN;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_ROI_POOLING_DESC,
                             META_COMMAND_INITIALIZE_ROI_POOLING_DESC,
                             META_COMMAND_EXECUTE_ROI_POOLING_DESC,
                             MetaCommandRoiPooling,
                             META_COMMAND_CREATE_ROI_POOLING_DESC,
                             META_COMMAND_EXECUTE_ROI_POOLING_DESC> CosUmd12MetaCommandRoiPooling;

typedef TCosUmd12MetaCommand<META_COMMAND_CREATE_COPY_TENSOR_DESC,
                             META_COMMAND_INITIALIZE_COPY_TENSOR_DESC,
                             META_COMMAND_EXECUTE_COPY_TENSOR_DESC,
                             MetaCommandCopyTensor,
                             HW_META_COMMAND_COPY_TENSOR,
                             HW_IO_TABLE_COPY_TENSOR> CosUmd12MetaCommandCopyTensor;

#endif
