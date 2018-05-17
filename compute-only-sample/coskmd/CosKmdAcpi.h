#pragma once

#include "CosKmd.h"
#include "CosKmdAdapter.h"

#define COS_KMD_TEMP_ACPI_BUFFER_SIZE (256 - sizeof(ACPI_EVAL_OUTPUT_BUFFER))

class CosKmAcpiReader
{
public:

    CosKmAcpiReader(CosKmAdapter* pCosKmAdapter, ULONG DeviceUid);
    ~CosKmAcpiReader();

    void Reset()
    {
        RtlZeroMemory(&m_InputBuffer, sizeof(m_InputBuffer));
        m_InputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        // TODO:[hideyukn] no input arguments support at this point.
        // m_InputBuffer.Size = 0;
        // m_InputBuffer.ArgumentCount = 0;

        if ((m_pOutputBuffer != NULL) &&
            (m_pOutputBuffer != (ACPI_EVAL_OUTPUT_BUFFER*)&m_ScratchBuffer[0]))
        {
            ExFreePool(m_pOutputBuffer);
        }

        RtlZeroMemory(&m_ScratchBuffer[0], sizeof(m_ScratchBuffer));
        m_pOutputBuffer = (ACPI_EVAL_OUTPUT_BUFFER*)&m_ScratchBuffer[0];
        m_OutputBufferSize = sizeof(m_ScratchBuffer);
    }

    NTSTATUS Read(ULONG MethodName)
    {
        SetMethod(MethodName);
        return EvalAcpiMethod();
    }

    UNALIGNED ACPI_METHOD_ARGUMENT* GetOutputArgument()
    {
        return &(m_pOutputBuffer->Argument[0]);
    }

    ULONG GetOutputArgumentCount()
    {
        return m_pOutputBuffer->Count;
    }

private:

    void SetMethod(ULONG MethodName)
    {
        m_InputBuffer.MethodNameAsUlong = MethodName;
    }

    NTSTATUS EvalAcpiMethod();

private:

    CosKmAdapter *m_pCosKmAdapter;
    ULONG  m_DeviceUid;

    ACPI_EVAL_INPUT_BUFFER_COMPLEX m_InputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER *m_pOutputBuffer;
    ULONG m_OutputBufferSize;

    BYTE m_ScratchBuffer[sizeof(ACPI_EVAL_OUTPUT_BUFFER) + COS_KMD_TEMP_ACPI_BUFFER_SIZE];
};

class CosKmAcpiArgumentParser
{ 
public:

    CosKmAcpiArgumentParser(CosKmAcpiReader* pReader, UNALIGNED ACPI_METHOD_ARGUMENT* pParentArgument);
    ~CosKmAcpiArgumentParser();

    void Reset()
    {
        m_CurrentArgumentIndex = 0;
        if (m_pParentArgument)
        {
            m_pCurrentArgument = (UNALIGNED ACPI_METHOD_ARGUMENT*)(&m_pParentArgument->Data[0]);
        }
        else
        {
            m_pCurrentArgument = m_pReader->GetOutputArgument();
        }
    }

    BOOLEAN IsCurrentArgumentValid()
    {
        if (m_pParentArgument)
        {
            return m_pParentArgument->DataLength > ((PBYTE)m_pCurrentArgument - (PBYTE)(&m_pParentArgument->Data[0]));
        }
        else
        {
            return m_pReader->GetOutputArgumentCount() > m_CurrentArgumentIndex;
        }
    }

    UNALIGNED ACPI_METHOD_ARGUMENT *NextArgument()
    {
        if (IsCurrentArgumentValid())
        {
            m_CurrentArgumentIndex++;
            m_pCurrentArgument = ACPI_METHOD_NEXT_ARGUMENT(m_pCurrentArgument);
            NT_ASSERT((PBYTE)m_pCurrentArgument > (PBYTE)(&m_pParentArgument->Data[0]));
            if (IsCurrentArgumentValid())
            {
                return m_pCurrentArgument;
            }
        }
        return NULL;
    }

    UNALIGNED ACPI_METHOD_ARGUMENT *GetCurrentArgument()
    {
        return m_pCurrentArgument;
    }

    UNALIGNED ACPI_METHOD_ARGUMENT *GetPackage()
    {
        if (IsCurrentArgumentValid() && (GetCurrentArgument()->Type == ACPI_METHOD_ARGUMENT_PACKAGE))
        {
            UNALIGNED ACPI_METHOD_ARGUMENT *currentArgument = GetCurrentArgument();
            NextArgument();
            return currentArgument;
        }
        return NULL;
    }
    
    NTSTATUS GetAnsiString(char** ppString, ULONG *pLength)
    {
        NT_ASSERT(ppString);
        NT_ASSERT(pLength);

        NTSTATUS Status = STATUS_SUCCESS;
        if (IsCurrentArgumentValid() && (GetCurrentArgument()->Type == ACPI_METHOD_ARGUMENT_STRING))
        {
            *ppString = reinterpret_cast<char *>(&m_pCurrentArgument->Data[0]);
            *pLength = m_pCurrentArgument->DataLength;
            NextArgument();
        }
        else
        {
            *ppString = NULL;
            *pLength = 0;
            Status = STATUS_ACPI_INVALID_DATA;
        }
        return Status;
    }

    NTSTATUS GetBuffer(BYTE **ppData, ULONG *pLength)
    {
        NT_ASSERT(ppData);
        NT_ASSERT(pLength);

        NTSTATUS Status = STATUS_SUCCESS;
        if (IsCurrentArgumentValid() && (GetCurrentArgument()->Type == ACPI_METHOD_ARGUMENT_BUFFER))
        {
            *ppData = reinterpret_cast<BYTE *>(&m_pCurrentArgument->Data[0]);
            *pLength = m_pCurrentArgument->DataLength;
            NextArgument();
        }
        else
        {
            *ppData = NULL;
            *pLength = 0;
            Status = STATUS_ACPI_INVALID_DATA;
        }
        return Status;
    }

    template <typename _Ty>
    NTSTATUS GetValue(_Ty* pValue)
    {
        NT_ASSERT(pValue);
        NTSTATUS Status = STATUS_SUCCESS;
        if (IsCurrentArgumentValid() && (GetCurrentArgument()->Type == ACPI_METHOD_ARGUMENT_INTEGER))
        {
            if (GetCurrentArgument()->DataLength >= 8)
            {
                *pValue = (_Ty)(*((reinterpret_cast<UNALIGNED ULONGLONG*>(&m_pCurrentArgument->Data[0]))));
            }
            else if (GetCurrentArgument()->DataLength >= 4)
            {
                *pValue = (_Ty)(*((reinterpret_cast<UNALIGNED ULONG*>(&m_pCurrentArgument->Data[0]))));
            }
            else if (GetCurrentArgument()->DataLength >= 2)
            {
                *pValue = (_Ty)(*((reinterpret_cast<UNALIGNED USHORT*>(&m_pCurrentArgument->Data[0]))));
            }
            else if (GetCurrentArgument()->DataLength >= 1)
            {
                *pValue = (_Ty)(*((reinterpret_cast<BYTE*>(&m_pCurrentArgument->Data[0]))));
            }
            else
            {
                Status = STATUS_ACPI_INVALID_DATA;
            }
            NextArgument();
        }
        return Status;
    }
 
private:

    CosKmAcpiReader* m_pReader;
    UNALIGNED ACPI_METHOD_ARGUMENT* m_pParentArgument;

    ULONG m_CurrentArgumentIndex;
    UNALIGNED ACPI_METHOD_ARGUMENT* m_pCurrentArgument;
};