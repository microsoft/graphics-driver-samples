#pragma once

#include "RosKmdAdapter.h"

class RosKmdSoftAdapter : public RosKmAdapter
{
private:

    friend class RosKmAdapter;

    RosKmdSoftAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext) :
        RosKmAdapter(PhysicalDeviceObject, MiniportDeviceContext)
    {
        // do nothing
    }

    virtual ~RosKmdSoftAdapter()
    {
        // do nothing
    }

    void * operator new(size_t  size);
    void operator delete(void * ptr);

protected:

    virtual void ProcessRenderBuffer(ROSDMABUFSUBMISSION * pDmaBufSubmission);

    virtual NTSTATUS Start(
        IN_PDXGK_START_INFO     DxgkStartInfo,
        IN_PDXGKRNL_INTERFACE   DxgkInterface,
        OUT_PULONG              NumberOfVideoPresentSources,
        OUT_PULONG              NumberOfChildren);

    virtual BOOLEAN InterruptRoutine(
        IN_ULONG        MessageNumber);
};