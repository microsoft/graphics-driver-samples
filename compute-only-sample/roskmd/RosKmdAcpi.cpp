#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdAcpi.tmh"

#include "RosKmdAcpi.h"

RosKmAcpiReader::RosKmAcpiReader(RosKmAdapter* pAdapter, ULONG DeviceUid) :
    m_pRosKmAdapter(pAdapter),
    m_DeviceUid(DeviceUid),
    m_pOutputBuffer(NULL)
{
    Reset();
}

RosKmAcpiReader::~RosKmAcpiReader()
{
    Reset();
}

NTSTATUS RosKmAcpiReader::EvalAcpiMethod()
{
    NT_ASSERT(m_pOutputBuffer);
    NT_ASSERT(m_OutputBufferSize);

    NTSTATUS Status;
    for (;;)
    {
        Status = m_pRosKmAdapter->GetDxgkInterface()->DxgkCbEvalAcpiMethod(
                     m_pRosKmAdapter->GetDxgkInterface()->DeviceHandle,
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

            m_pOutputBuffer = (ACPI_EVAL_OUTPUT_BUFFER*)ExAllocatePoolWithTag(PagedPool, RequiredSize, 'ROSD');
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

RosKmAcpiArgumentParser::RosKmAcpiArgumentParser(RosKmAcpiReader* pReader, UNALIGNED ACPI_METHOD_ARGUMENT* pParentArgument) :
    m_pReader(pReader),
    m_pParentArgument(pParentArgument)
{
    Reset();
}

RosKmAcpiArgumentParser::~RosKmAcpiArgumentParser()
{
    Reset();
}
