#pragma once

#include "RosKmdAdapter.h"

class RosKmdRapAdapter : public RosKmAdapter
{
private:

    friend class RosKmAdapter;

    RosKmdRapAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext);
    virtual ~RosKmdRapAdapter();

    void * operator new(size_t  size);
    void operator delete(void * ptr);

protected:

    virtual void ProcessRenderBuffer(ROSDMABUFSUBMISSION * pDmaBufSubmission);

    virtual NTSTATUS Start(
        IN_PDXGK_START_INFO     DxgkStartInfo,
        IN_PDXGKRNL_INTERFACE   DxgkInterface,
        OUT_PULONG              NumberOfVideoPresentSources,
        OUT_PULONG              NumberOfChildren
        ) override;

    virtual NTSTATUS Stop () override;

    _Check_return_
    _IRQL_requires_(HIGH_LEVEL)
    virtual BOOLEAN InterruptRoutine(
        IN_ULONG MessageNumber
        ) override;

private:

    VC4_REGISTER_FILE          *m_pVC4RegFile;

    void SubmitControlList(bool bBinningControlist, UINT startAddress, UINT endAddress);
    UINT GenerateRenderingControlList(ROSDMABUFINFO *pDmaBufInf);

    NTSTATUS SetVC4Power(bool bOn);

    void MoveToNextBinnerRenderMemChunk(UINT controlListLength)
    {
        controlListLength = (controlListLength + (kPageSize - 1)) & (~(kPageSize - 1));

#if BINNER_DBG

        m_pRenderingControlList += controlListLength;
        m_renderingControlListPhysicalAddress += controlListLength;

        if ((m_renderingControlListPhysicalAddress + 64 * kPageSize) > (m_controlListPoolPhysicalAddress + VC4_RENDERING_CTRL_LIST_POOL_SIZE))
        {
            m_pRenderingControlList = m_pControlListPool;
            m_renderingControlListPhysicalAddress = m_controlListPoolPhysicalAddress;
        }

        m_tileAllocationMemoryPhysicalAddress += 64 * kPageSize;

        if ((m_tileAllocationMemoryPhysicalAddress + 64 * kPageSize) >= m_tileStatePoolPhysicalAddress)
        {
            m_tileAllocationMemoryPhysicalAddress = m_tileAllocPoolPhysicalAddress;
        }

        m_tileStateDataArrayPhysicalAddress += 64 * kPageSize;

        if ((m_tileStateDataArrayPhysicalAddress + 64 * kPageSize) >= (RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart + RosKmdGlobal::s_videoMemorySize))
        {
            m_tileStateDataArrayPhysicalAddress = m_tileStatePoolPhysicalAddress;
        }

#endif
    }
    
private: // NONPAGED
    
    _Check_return_
    _IRQL_requires_(HIGH_LEVEL)
    BOOLEAN RendererInterruptRoutine (IN_ULONG MessageNumber);
    
};