#pragma once

#include "RosKmd.h"

#if VC4

#include "Vc4Hw.h"
#include "VC4Ddi.h"

#endif

#include "RosKmdAllocation.h"
#include "RosKmdGlobal.h"

typedef struct __ROSKMERRORCONDITION
{
    union
    {
        struct
        {
            UINT    m_UnSupportedPagingOp           : 1;
            UINT    m_NotifyDmaBufCompletion        : 1;
            UINT    m_NotifyPreemptionCompletion    : 1;
            UINT    m_NotifyDmaBufFault             : 1;
            UINT    m_PreparationError              : 1;
            UINT    m_PagingFailure                 : 1;
        };

        UINT        m_Value;
    };
} ROSKMERRORCONDITION;

typedef struct _ROSDMABUFSTATE
{
    union
    {
        struct
        {
            UINT    m_bRender           : 1;
#if VC4

            UINT    m_bRenderTargetRef  : 1;
            UINT    m_bTileAllocMemRef  : 1;
            UINT    m_bTileStateDataRef : 1;
            UINT    m_NumDmaBufSelfRef  : 5;    // Up to 32 DMA buffer self reference
            UINT    m_HasVC4ClearColors : 1;

#endif
            UINT    m_bPresent          : 1;
            UINT    m_bPaging           : 1;
            UINT    m_bSwCommandBuffer  : 1;
            UINT    m_bPatched          : 1;
            UINT    m_bSubmittedOnce    : 1;
            UINT    m_bRun              : 1;
            UINT    m_bPreempted        : 1;
            UINT    m_bReset            : 1;
            UINT    m_bCompleted        : 1;
        };

        UINT        m_Value;
    };
} ROSDMABUFSTATE;

typedef struct _ROSDMABUFINFO
{
    PBYTE                       m_pDmaBuffer;
    LARGE_INTEGER               m_DmaBufferPhysicalAddress;
    UINT                        m_DmaBufferSize;
    ROSDMABUFSTATE              m_DmaBufState;

#if VC4

    RosKmdAllocation           *m_pRenderTarget;
    UINT                        m_RenderTargetPhysicalAddress;

    D3DDDI_PATCHLOCATIONLIST    m_DmaBufSelfRef[VC4_MAX_DMA_BUFFER_SELF_REF];

    VC4ClearColors              m_VC4ClearColors;

#endif
} ROSDMABUFINFO;

typedef struct _ROSDMABUFSUBMISSION
{
    LIST_ENTRY      m_QueueEntry;
    ROSDMABUFINFO * m_pDmaBufInfo;
    UINT            m_StartOffset;
    UINT            m_EndOffset;
    UINT            m_SubmissionFenceId;
} ROSDMABUFSUBMISSION;

typedef union _RosKmAdapterFlags
{
    struct
    {
        UINT    m_isVC4     : 1;
    };

    UINT        m_value;
} RosKmAdapterFlags;

class RosKmAdapter
{
public:

    static NTSTATUS __stdcall DdiAddAdapter(
        IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
        OUT_PPVOID                  MiniportDeviceContext);

    static NTSTATUS __stdcall DdiStartAdapter(
        IN_CONST_PVOID          MiniportDeviceContext,
        IN_PDXGK_START_INFO     DxgkStartInfo,
        IN_PDXGKRNL_INTERFACE   DxgkInterface,
        OUT_PULONG              NumberOfVideoPresentSources,
        OUT_PULONG              NumberOfChildren);

    static NTSTATUS __stdcall DdiStopAdapter(
        IN_CONST_PVOID  MiniportDeviceContext);

    static NTSTATUS __stdcall DdiRemoveAdapter(
        IN_CONST_PVOID  MiniportDeviceContext);

    static NTSTATUS __stdcall DdiBuildPagingBuffer(
        IN_CONST_HANDLE                 hAdapter,
        IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs);

    static void __stdcall DdiDpcRoutine(
        IN_CONST_PVOID  MiniportDeviceContext);

    static NTSTATUS __stdcall DdiSubmitCommand(
        IN_CONST_HANDLE                     hAdapter,
        IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand);

    static NTSTATUS __stdcall DdiPatch(
        IN_CONST_HANDLE             hAdapter,
        IN_CONST_PDXGKARG_PATCH     pPatch);

    static NTSTATUS __stdcall DdiCreateAllocation(
        IN_CONST_HANDLE                     hAdapter,
        INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation);

    static NTSTATUS __stdcall DdiDestroyAllocation(
        IN_CONST_HANDLE                         hAdapter,
        IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation);

    static NTSTATUS __stdcall DdiQueryAdapterInfo(
        IN_CONST_HANDLE                         hAdapter,
        IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo);

    static NTSTATUS __stdcall DdiDescribeAllocation(
        IN_CONST_HANDLE                         hAdapter,
        INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation);

    static NTSTATUS __stdcall DdiGetNodeMetadata(
        IN_CONST_HANDLE                 hAdapter,
        UINT                            NodeOrdinal,
        OUT_PDXGKARG_GETNODEMETADATA    pGetNodeMetadata);

    static NTSTATUS __stdcall DdiGetStandardAllocationDriverData(
        IN_CONST_HANDLE                                 hAdapter,
        INOUT_PDXGKARG_GETSTANDARDALLOCATIONDRIVERDATA  pGetStandardAllocationDriverData);

    static NTSTATUS
        __stdcall
        DdiSubmitCommandVirtual(
            IN_CONST_HANDLE                         hAdapter,
            IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  pSubmitCommandVirtual);

    static NTSTATUS
        __stdcall
        DdiPreemptCommand(
            IN_CONST_HANDLE                     hAdapter,
            IN_CONST_PDXGKARG_PREEMPTCOMMAND    pPreemptCommand);

    static NTSTATUS
        __stdcall CALLBACK
        DdiRestartFromTimeout(
            IN_CONST_HANDLE     hAdapter);

    static NTSTATUS
        __stdcall
        DdiCancelCommand(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_PDXGKARG_CANCELCOMMAND pCancelCommand);

    static NTSTATUS
        __stdcall
        DdiQueryCurrentFence(
            IN_CONST_HANDLE                    hAdapter,
            INOUT_PDXGKARG_QUERYCURRENTFENCE   pCurrentFence);

    static NTSTATUS
        __stdcall
        DdiResetEngine(
            IN_CONST_HANDLE             hAdapter,
            INOUT_PDXGKARG_RESETENGINE  pResetEngine);

    static NTSTATUS
        __stdcall
        DdiQueryEngineStatus(
            IN_CONST_HANDLE                     hAdapter,
            INOUT_PDXGKARG_QUERYENGINESTATUS    pQueryEngineStatus);

    static NTSTATUS
        __stdcall
        DdiQueryDependentEngineGroup(
            IN_CONST_HANDLE                             hAdapter,
            INOUT_DXGKARG_QUERYDEPENDENTENGINEGROUP     pQueryDependentEngineGroup);

    static NTSTATUS
        __stdcall
        DdiGetScanLine(
            IN_CONST_HANDLE             hAdapter,
            INOUT_PDXGKARG_GETSCANLINE  /*pGetScanLine*/);

    static NTSTATUS
        __stdcall
        DdiControlInterrupt(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_DXGK_INTERRUPT_TYPE    InterruptType,
            IN_BOOLEAN                      EnableInterrupt);

    static NTSTATUS
        __stdcall
        DdiCollectDbgInfo(
            IN_CONST_HANDLE                         hAdapter,
            IN_CONST_PDXGKARG_COLLECTDBGINFO        /*pCollectDbgInfo*/);

    static NTSTATUS
        __stdcall
        DdiPresent(
            IN_CONST_HANDLE         hContext,
            INOUT_PDXGKARG_PRESENT  pPresent);

    static NTSTATUS
        __stdcall
        DdiCreateProcess(
            IN_PVOID  pMiniportDeviceContext,
            IN DXGKARG_CREATEPROCESS* pArgs);

    static NTSTATUS
        __stdcall
        DdiDestroyProcess(
            IN_PVOID pMiniportDeviceContext,
            IN HANDLE KmdProcessHandle);

    static void
        __stdcall
        DdiSetStablePowerState(
            IN_CONST_HANDLE                        hAdapter,
            IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs);

    static NTSTATUS
        __stdcall
        DdiCalibrateGpuClock(
            IN_CONST_HANDLE                             hAdapter,
            IN UINT32                                   NodeOrdinal,
            IN UINT32                                   EngineOrdinal,
            OUT_PDXGKARG_CALIBRATEGPUCLOCK              pClockCalibration
            );

    static NTSTATUS
        __stdcall
        DdiRenderKm(
            IN_CONST_HANDLE         hContext,
            INOUT_PDXGKARG_RENDER   pRender);

    static NTSTATUS
        __stdcall
        DdiEscape(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_PDXGKARG_ESCAPE        pEscape);

    static NTSTATUS
        __stdcall
        DdiSetPalette(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_PDXGKARG_SETPALETTE    /*pSetPalette*/);

    static NTSTATUS
        __stdcall
        DdiSetPointerPosition(
            IN_CONST_HANDLE                         hAdapter,
            IN_CONST_PDXGKARG_SETPOINTERPOSITION    pSetPointerPosition);

    static NTSTATUS
        __stdcall
        DdiSetPointerShape(
            IN_CONST_HANDLE                     hAdapter,
            IN_CONST_PDXGKARG_SETPOINTERSHAPE   pSetPointerShape);

    static NTSTATUS
        __stdcall
        DdiResetFromTimeout(
            IN_CONST_HANDLE     hAdapter);

    static NTSTATUS
        DdiQueryInterface(
            IN_CONST_PVOID          MiniportDeviceContext,
            IN_PQUERY_INTERFACE     QueryInterface);

    static NTSTATUS
        DdiDispatchIoRequest(
            IN_CONST_PVOID              MiniportDeviceContext,
            IN_ULONG                    VidPnSourceId,
            IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket);

    static BOOLEAN
        DdiInterruptRoutine(
            IN_CONST_PVOID  MiniportDeviceContext,
            IN_ULONG        MessageNumber);

    static NTSTATUS
        DdiQueryChildRelations(
            IN_CONST_PVOID                  MiniportDeviceContext,
            INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
            IN_ULONG                        ChildRelationsSize);

    static NTSTATUS
        DdiQueryChildStatus(
            IN_CONST_PVOID          MiniportDeviceContext,
            IN_PDXGK_CHILD_STATUS   ChildStatus,
            IN_BOOLEAN              NonDestructiveOnly);
    static NTSTATUS
        DdiQueryDeviceDescriptor(
            IN_CONST_PVOID                  MiniportDeviceContext,
            IN_ULONG                        ChildUid,
            INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor);

    static NTSTATUS
        DdiSetPowerState(
            IN_CONST_PVOID          MiniportDeviceContext,
            IN_ULONG                DeviceUid,
            IN_DEVICE_POWER_STATE   DevicePowerState,
            IN_POWER_ACTION         ActionType);

    static NTSTATUS
        DdiSetPowerComponentFState(
            IN_CONST_PVOID       MiniportDeviceContext,
            IN UINT              ComponentIndex,
            IN UINT              FState);

    static NTSTATUS
        DdiPowerRuntimeControlRequest(
            IN_CONST_PVOID       MiniportDeviceContext,
            IN LPCGUID           PowerControlCode,
            IN OPTIONAL PVOID    InBuffer,
            IN SIZE_T            InBufferSize,
            OUT OPTIONAL PVOID   OutBuffer,
            IN SIZE_T            OutBufferSize,
            OUT OPTIONAL PSIZE_T BytesReturned);

    static NTSTATUS
        DdiNotifyAcpiEvent(
            IN_CONST_PVOID      MiniportDeviceContext,
            IN_DXGK_EVENT_TYPE  EventType,
            IN_ULONG            Event,
            IN_PVOID            Argument,
            OUT_PULONG          AcpiFlags);

    static void
        DdiResetDevice(
            IN_CONST_PVOID  MiniportDeviceContext);

private:

    RosKmAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext);
    ~RosKmAdapter();

    void * operator new(size_t  size);
    void operator delete(void * ptr);

public:

    static RosKmAdapter * Cast(void * ptr)
    {
        RosKmAdapter * rosKmAdapter = reinterpret_cast<RosKmAdapter *>(ptr);

        NT_ASSERT(rosKmAdapter->m_magic == RosKmAdapter::kMagic);

        return rosKmAdapter;
    }

	DXGKRNL_INTERFACE* GetDxgkInterface()
	{
		return &m_DxgkInterface;
	}

    void QueueDmaBuffer(IN_CONST_PDXGKARG_SUBMITCOMMAND pSubmitCommand);

    bool
    ValidateDmaBuffer(
        ROSDMABUFINFO*                  pDmaBufInfo,
        CONST DXGK_ALLOCATIONLIST*      pAllocationList,
        UINT                            allocationListSize,
        CONST D3DDDI_PATCHLOCATIONLIST* pPatchLocationList,
        UINT                            patchAllocationList);

    void
    PatchDmaBuffer(
        ROSDMABUFINFO*                  pDmaBufInfo,
        CONST DXGK_ALLOCATIONLIST*      pAllocationList,
        UINT                            allocationListSize,
        CONST D3DDDI_PATCHLOCATIONLIST* pPatchLocationList,
        UINT                            patchAllocationList);

private:

    NTSTATUS Start(
        IN_PDXGK_START_INFO     DxgkStartInfo,
        IN_PDXGKRNL_INTERFACE   DxgkInterface,
        OUT_PULONG              NumberOfVideoPresentSources,
        OUT_PULONG              NumberOfChildren);

    NTSTATUS Stop();

    NTSTATUS BuildPagingBuffer(
        IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs);

private:

    static void WorkerThread(void * StartContext);
    void DoWork(void);
    void DpcRoutine(void);
    void NotifyDmaBufCompletion(ROSDMABUFSUBMISSION * pDmaBufSubmission);
    static BOOLEAN SynchronizeNotifyInterrupt(PVOID SynchronizeContext);
    BOOLEAN SynchronizeNotifyInterrupt();
    ROSDMABUFSUBMISSION * DequeueDmaBuffer(KSPIN_LOCK * pDmaBufQueueLock);
    void ProcessPagingBuffer(ROSDMABUFSUBMISSION * pDmaBufSubmission);
    void ProcessRenderBuffer(ROSDMABUFSUBMISSION * pDmaBufSubmission);
    static void HwDmaBufCompletionDpcRoutine(KDPC *, PVOID, PVOID, PVOID);

    void SubmitControlList(bool bBinningControlist, UINT startAddress, UINT endAddress);
    UINT GetAperturePhysicalAddress(UINT apertureAddress)
    {
        UINT pageIndex = (apertureAddress - ROSD_SEGMENT_APERTURE_BASE_ADDRESS) / kPageSize;

        return ((UINT)m_aperturePageTable[pageIndex])*kPageSize + (apertureAddress & (kPageSize - 1));
    };

    UINT GenerateRenderingControlList(ROSDMABUFINFO *pDmaBufInf);

    void MoveToNextBinnerRenderMemChunk(UINT controlListLength)
    {
        controlListLength = (controlListLength + (kPageSize - 1)) & (~(kPageSize - 1));

#if BINNER_DBG

        m_pRenderingControlList               += controlListLength;
        m_renderingControlListPhysicalAddress += controlListLength;

        if ((m_renderingControlListPhysicalAddress + 64*kPageSize) > (m_controlListPoolPhysicalAddress + VC4_RENDERING_CTRL_LIST_POOL_SIZE))
        {
            m_pRenderingControlList               = m_pControlListPool;
            m_renderingControlListPhysicalAddress = m_controlListPoolPhysicalAddress;
        }

        m_tileAllocationMemoryPhysicalAddress += 64*kPageSize;

        if ((m_tileAllocationMemoryPhysicalAddress + 64*kPageSize) >= m_tileStatePoolPhysicalAddress)
        {
            m_tileAllocationMemoryPhysicalAddress = m_tileAllocPoolPhysicalAddress;
        }

        m_tileStateDataArrayPhysicalAddress += 64 * kPageSize;

        if ((m_tileStateDataArrayPhysicalAddress + 64*kPageSize) >= (RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart + RosKmdGlobal::s_videoMemorySize))
        {
            m_tileStateDataArrayPhysicalAddress = m_tileStatePoolPhysicalAddress;
        }

#endif
    }

private:

    static const size_t kPageSize = 4096;
    static const size_t kPageShift = 12;

    static const size_t kApertureSegmentId = 1;
    static const size_t kApertureSegmentPageCount = 1024;
    static const size_t kApertureSegmentSize = kApertureSegmentPageCount * kPageSize;

    PFN_NUMBER  m_aperturePageTable[kApertureSegmentPageCount];

    static const size_t kVideoMemorySegmentId = 2;

private:

    static const UINT32 kMagic = 'ADPT';

    UINT32                      m_magic;

    RosKmAdapterFlags           m_flags;

    DEVICE_OBJECT              *m_pPhysicalDevice;
    DXGKRNL_INTERFACE           m_DxgkInterface;
    DXGK_START_INFO             m_DxgkStartInfo;

    ROSKMERRORCONDITION         m_ErrorHit;

    PKTHREAD                    m_pWorkerThread;
    KEVENT                      m_workerThreadEvent;
    bool                        m_workerExit;

    // TODO[indyz]: Switch to use the m_DxgkStartInfo::RequiredDmaQueueEntry
    const static UINT           m_maxDmaBufQueueLength = 32;
    ROSDMABUFSUBMISSION         m_dmaBufSubssions[m_maxDmaBufQueueLength];

    LIST_ENTRY                  m_dmaBufSubmissionFree;
    LIST_ENTRY                  m_dmaBufQueue;
    KSPIN_LOCK                  m_dmaBufQueueLock;

    KDPC                        m_hwDmaBufCompletionDpc;
    KEVENT                      m_hwDmaBufCompletionEvent;

    DXGKARGCB_NOTIFY_INTERRUPT_DATA m_interruptData;

    DXGKARG_RESETENGINE        *m_pResetEngine;

    BOOL                        m_bReadyToHandleInterrupt;

    DXGK_DEVICE_INFO            m_deviceInfo;

    BYTE                        m_deviceId[MAX_DEVICE_ID_LENGTH];
    ULONG                       m_deviceIdLength;

#if VC4

    VC4_REGISTER_FILE          *m_pVC4RegFile;
    UINT                        m_localVidMemSegmentSize;

    BYTE                       *m_pRenderingControlList;
    UINT                        m_renderingControlListPhysicalAddress;
    UINT                        m_tileAllocationMemoryPhysicalAddress;
    UINT                        m_tileStateDataArrayPhysicalAddress;

    BYTE                       *m_pControlListPool;
    UINT                        m_controlListPoolPhysicalAddress;
    UINT                        m_tileAllocPoolPhysicalAddress;
    UINT                        m_tileStatePoolPhysicalAddress;

#endif

public:

    DEVICE_POWER_STATE          m_AdapterPowerDState;
    BOOLEAN                     m_PowerManagementStarted;
	UINT                        m_NumPowerComponents;
	DXGK_POWER_RUNTIME_COMPONENT m_PowerComponents[C_ROSD_GPU_ENGINE_COUNT];
    UINT                         m_EnginePowerFState[C_ROSD_GPU_ENGINE_COUNT];
	
    UINT                        m_NumNodes;
    DXGK_WDDMVERSION            m_WDDMVersion;

public:

    NTSTATUS
    ResetFromTdr();

    NTSTATUS
    QueryEngineStatus(
        DXGKARG_QUERYENGINESTATUS  *pQueryEngineStatus);

	NTSTATUS
		InitializePowerComponentInfo();
		
    NTSTATUS 
        GetNumPowerComponents();

    NTSTATUS
        GetPowerComponentInfo(
            IN UINT ComponentIndex,
            OUT DXGK_POWER_RUNTIME_COMPONENT* pPowerComponent);

    NTSTATUS
    SetPowerComponentFState(
        IN UINT ComponentIndex,
        IN UINT FState);

};

template<typename TypeCur, typename TypeNext>
void MoveToNextCommand(TypeCur pCurCommand, TypeNext &pNextCommand)
{
    pNextCommand = (TypeNext)(pCurCommand + 1);
}

