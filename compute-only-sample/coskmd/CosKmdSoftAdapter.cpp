#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdSoftAdapter.tmh"

#include "CosKmdSoftAdapter.h"
#include "CosGpuCommand.h"
#include "CosKmdMetaCommand.h"

void * CosKmdSoftAdapter::operator new(size_t size)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, size, 'COSD');
}

void CosKmdSoftAdapter::operator delete(void * ptr)
{
    ExFreePool(ptr);
}

NTSTATUS
CosKmdSoftAdapter::Start(
    IN_PDXGK_START_INFO     DxgkStartInfo,
    IN_PDXGKRNL_INTERFACE   DxgkInterface,
    OUT_PULONG              NumberOfVideoPresentSources,
    OUT_PULONG              NumberOfChildren)
{
    return CosKmAdapter::Start(DxgkStartInfo, DxgkInterface, NumberOfVideoPresentSources, NumberOfChildren);
}

void
CosKmdSoftAdapter::ProcessRenderBuffer(
    COSDMABUFSUBMISSION * pDmaBufSubmission)
{
    COSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    NT_ASSERT(pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer);

    NT_ASSERT(0 == (pDmaBufSubmission->m_EndOffset - pDmaBufSubmission->m_StartOffset) % sizeof(GpuCommand));

    GpuCommand * pGpuCommand = (GpuCommand *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_StartOffset);
    GpuCommand * pEndofCommand = (GpuCommand *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_EndOffset);

    for (; pGpuCommand < pEndofCommand; pGpuCommand++)
    {
        switch (pGpuCommand->m_commandId)
        {
        case Header:
        case Nop:
            break;
        case ResourceCopy:
        {
            RtlCopyMemory(
                ((BYTE *)CosKmdGlobal::s_pVideoMemory) + pGpuCommand->m_resourceCopy.m_dstGpuAddress.QuadPart,
                ((BYTE *)CosKmdGlobal::s_pVideoMemory) + pGpuCommand->m_resourceCopy.m_srcGpuAddress.QuadPart,
                pGpuCommand->m_resourceCopy.m_sizeBytes);
        }
        break;
        default:
            break;
        }
    }
}

#if COS_GPUVA_SUPPORT

void
CosKmdSoftAdapter::ProcessHWRenderBuffer(
    COSDMABUFSUBMISSION * pDmaBufSubmission)
{
    UNREFERENCED_PARAMETER(pDmaBufSubmission);
}

#else

#if RS_2LEVEL

void
CosKmdSoftAdapter::ProcessHWRenderBuffer(
    COSDMABUFSUBMISSION * pDmaBufSubmission)
{
    COSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    NT_ASSERT(false == pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer);

    BYTE * pGpuCommand = pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_StartOffset;
    BYTE * pEndofCommand = pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_EndOffset;
    UINT64 commandSize;

    BOOL bDescriptorTableSet = false;
    GpuHWDescriptor * pDescriptorTable = NULL;
    BOOL bRootSignatureSet = false;
    BYTE * pRootValues = NULL;

    for (; pGpuCommand < pEndofCommand; pGpuCommand += commandSize)
    {
        switch (*((GpuCommandId *)pGpuCommand))
        {
        case Header:
        case Nop:
            commandSize = sizeof(GpuCommand);
            break;
        case QwordWrite:
            {
                GpuHwQwordWrite * pQwordWrite = (GpuHwQwordWrite *)pGpuCommand;

                ULONGLONG * pGpuAddress = (ULONGLONG *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) +
                                                        (pQwordWrite->m_gpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));
                *pGpuAddress = pQwordWrite->m_data;

                commandSize = sizeof(GpuHwQwordWrite);
            }
            break;
        case DescriptorHeapSet:
            {
                GpuHwDescriptorHeapSet * pDescriptorHeapSet = (GpuHwDescriptorHeapSet *)pGpuCommand;

                bDescriptorTableSet = true;

                pDescriptorTable = (GpuHWDescriptor *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) +
                                                       (pDescriptorHeapSet->m_descriptorHeapGpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                commandSize = sizeof(GpuHwDescriptorHeapSet);
            }
            break;
        case RootSignature2LevelSet:
            {
                GpuHWRootSignature2LSet * pRootSignatureSet = (GpuHWRootSignature2LSet *)pGpuCommand;

                bRootSignatureSet = true;

                pRootValues = pRootSignatureSet->m_rootValues;

                commandSize = pRootSignatureSet->m_commandSize;
            }
            break;
        case ComputeShaderDispatch:
            {
                GpuHwComputeShaderDisptch * pCSDispatch = (GpuHwComputeShaderDisptch *)pGpuCommand;

                if (bRootSignatureSet)
                {
                    KFLOATING_SAVE floatingSave;

                    KeSaveFloatingPointState(&floatingSave);

                    //
                    // Shader compiler generates code according to Root Signature passed in at 
                    // compilation time, so that shader code can access the root values with 
                    // offset at the runtime.
                    //

#if ENABLE_FOR_COSTEST

                    UINT numElements =  pCSDispatch->m_numThreadPerGroup*
                                        pCSDispatch->m_threadGroupCountX*
                                        pCSDispatch->m_threadGroupCountY*
                                        pCSDispatch->m_threadGroupCountZ;

                    UINT * pIntIn1 = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              ((*(LONGLONG *)(pRootValues + 0)) - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    UINT * pIntIn2 = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              ((*(LONGLONG *)(pRootValues + 8)) - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    UINT * pIntOut = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) +
                                              ((*(LONGLONG *)(pRootValues + 0x10)) - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    for (UINT i = 0; i < numElements; i++)
                    {
                        *pIntOut++ = *pIntIn1++ + *pIntIn2++;
                        *((FLOAT *)pIntOut++) = *((FLOAT *)pIntIn1++) + *((FLOAT *)pIntIn2++);
                    }

#elif ENABLE_FOR_COSTEST2

                    UINT numElements =  pCSDispatch->m_numThreadPerGroup*
                                        pCSDispatch->m_threadGroupCountX*
                                        pCSDispatch->m_threadGroupCountY*
                                        pCSDispatch->m_threadGroupCountZ;

                    UINT * pIntIn1 = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              ((*(LONGLONG *)(pRootValues + 0)) - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    UINT * pIntIn2 = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              ((*(LONGLONG *)(pRootValues + 8)) - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    UINT * pIntOut = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              (pDescriptorTable[1 + 1].m_resourceGpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    for (UINT i = 0; i < numElements; i++)
                    {
                        *pIntOut++ = *pIntIn1++ + *pIntIn2++;
                        *((FLOAT *)pIntOut++) = *((FLOAT *)pIntIn1++) + *((FLOAT *)pIntIn2++);
                    }

#endif

                    KeRestoreFloatingPointState(&floatingSave);
                }

                commandSize = pCSDispatch->m_commandSize;
            }
            break;
        case MetaCommandExecute:
            {
                GpuHwMetaCommand *  pMetaCommand = (GpuHwMetaCommand *)pGpuCommand;

                CosKmExecuteMetaCommand(pMetaCommand);

                commandSize = pMetaCommand->m_commandSize;
            }
            break;
        default:
            {
                NT_ASSERT(false);
                commandSize = pEndofCommand - pGpuCommand;
            }
            break;
        }
    }
}

#else

void
CosKmdSoftAdapter::ProcessHWRenderBuffer(
    COSDMABUFSUBMISSION * pDmaBufSubmission)
{
    COSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    NT_ASSERT(false == pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer);

    BYTE * pGpuCommand = pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_StartOffset;
    BYTE * pEndofCommand = pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_EndOffset;
    UINT64 commandSize;

    BOOL bRootSignatureSet = false;
    GpuHWConstantDescriptor * pCbvTable = NULL;
    GpuHWDescriptor * pSrvTable = NULL;
    GpuHWDescriptor * pUavTable = NULL;

    for (; pGpuCommand < pEndofCommand; pGpuCommand += commandSize)
    {
        switch (*((GpuCommandId *)pGpuCommand))
        {
        case Header:
        case Nop:
            commandSize = sizeof(GpuCommand);
            break;
        case RootSignatureSet:
            {
                GpuHWRootSignatureSet * pRootSignatureSet = (GpuHWRootSignatureSet *)pGpuCommand;

                bRootSignatureSet = true;

                BYTE * pRSData = (BYTE *)(pRootSignatureSet + 1);

                if (pRootSignatureSet->m_numCbvRegisters)
                {
                    pCbvTable = (GpuHWConstantDescriptor *)pRSData;

                    pRSData += (sizeof(GpuHWConstantDescriptor)*pRootSignatureSet->m_numCbvRegisters);
                }
                if (pRootSignatureSet->m_numSrvRegisters)
                {
                    pSrvTable = (GpuHWDescriptor *)pRSData;

                    pRSData += (sizeof(GpuHWDescriptor)*pRootSignatureSet->m_numSrvRegisters);
                }
                if (pRootSignatureSet->m_numUavRegisters)
                {
                    pUavTable = (GpuHWDescriptor *)pRSData;
                }

                commandSize = pRootSignatureSet->m_commandSize;
            }
            break;
        case ComputeShaderDispatch:
            {
                GpuHwComputeShaderDisptch * pCSDispatch = (GpuHwComputeShaderDisptch *)pGpuCommand;

                if (bRootSignatureSet)
                {
#if ENABLE_FOR_COSTEST
                    KFLOATING_SAVE floatingSave;

                    KeSaveFloatingPointState(&floatingSave);

                    UINT numElements =  pCSDispatch->m_numThreadPerGroup*
                                        pCSDispatch->m_threadGroupCountX*
                                        pCSDispatch->m_threadGroupCountY*
                                        pCSDispatch->m_threadGroupCountZ;

                    UINT * pIntIn1 = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              (pUavTable[0].m_resourceGpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    UINT * pIntIn2 = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              (pUavTable[1].m_resourceGpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    UINT * pIntOut = (UINT *)(((PBYTE)CosKmdGlobal::s_pVideoMemory) + 
                                              (pUavTable[2].m_resourceGpuAddress.QuadPart - CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart));

                    for (UINT i = 0; i < numElements; i++)
                    {
                        *pIntOut++ = *pIntIn1++ + *pIntIn2++;
                        *((FLOAT *)pIntOut++) = *((FLOAT *)pIntIn1++) + *((FLOAT *)pIntIn2++);
                    }

                    KeRestoreFloatingPointState(&floatingSave);
#endif
                }

                commandSize = pCSDispatch->m_commandSize;
            }
            break;
        case MetaCommandExecute:
            {
                GpuHwMetaCommand *  pMetaCommand = (GpuHwMetaCommand *)pGpuCommand;

                CosKmExecuteMetaCommand(pMetaCommand);

                commandSize = pMetaCommand->m_commandSize;
            }
            break;
        default:
            {
                NT_ASSERT(false);
                commandSize = pEndofCommand - pGpuCommand;
            }
            break;
        }
    }
}

#endif  // RS_2LEVEL

#endif  // GPUVA

#if COS_GPUVA_SUPPORT

void
CosKmdSoftAdapter::ProcessGpuVaRenderBuffer(
    COSDMABUFSUBMISSION *   pDmaBufSubmission)
{
    UNREFERENCED_PARAMETER(pDmaBufSubmission);

    // TODO: emulate command buffer submitted with GPU VA from UMD
}

#endif

BOOLEAN CosKmdSoftAdapter::InterruptRoutine(
    IN_ULONG        MessageNumber)
{
    MessageNumber;

    return false;
}

