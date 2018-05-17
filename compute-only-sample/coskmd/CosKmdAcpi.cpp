#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdAcpi.tmh"

#include "CosKmdAcpi.h"

CosKmAcpiReader::CosKmAcpiReader(CosKmAdapter* pAdapter, ULONG DeviceUid) :
    m_pCosKmAdapter(pAdapter),
    m_DeviceUid(DeviceUid),
    m_pOutputBuffer(NULL)
{
    Reset();
}

CosKmAcpiReader::~CosKmAcpiReader()
{
    Reset();
}

NTSTATUS CosKmAcpiReader::EvalAcpiMethod()
{
    NT_ASSERT(m_pOutputBuffer);
    NT_ASSERT(m_OutputBufferSize);

    NTSTATUS Status;
    for (;;)
    {
        Status = m_pCosKmAdapter->GetDxgkInterface()->DxgkCbEvalAcpiMethod(
                     m_pCosKmAdapter->GetDxgkInterface()->DeviceHandle,
                     m_DeviceUid,
                     &m_InputBuffer,
                     sizeof(m_InputBuffer),
                     m_pOutputBuffer,
                     m_OutputBufferSize);
        if (Status == STATUS_BUFFER_OVERFLOW)
        {
            ULONG RequiredSize = m_pOutputBuffer->Length;

            if (m_pOutputBuffer != (ACPI_EVAL_OUTPUT_BUFFER*)&m_ScratchBuffer[0])
            {
                ExFreePool(m_pOutputBuffer);
            }

            m_pOutputBuffer = (ACPI_EVAL_OUTPUT_BUFFER*)ExAllocatePoolWithTag(PagedPool, RequiredSize, 'COSD');
            if (NULL == m_pOutputBuffer)
            {
                Reset();
                Status = STATUS_NO_MEMORY;
                break;
            }
            m_OutputBufferSize = RequiredSize;
        }
        else
        {
            break;
        }
    }

    return Status;
}

CosKmAcpiArgumentParser::CosKmAcpiArgumentParser(CosKmAcpiReader* pReader, UNALIGNED ACPI_METHOD_ARGUMENT* pParentArgument) :
    m_pReader(pReader),
    m_pParentArgument(pParentArgument)
{
    Reset();
}

CosKmAcpiArgumentParser::~CosKmAcpiArgumentParser()
{
    Reset();
}
