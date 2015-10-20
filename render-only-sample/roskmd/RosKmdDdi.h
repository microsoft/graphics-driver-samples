#pragma once

#include "RosKmd.h"

class RosKmdDdi
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

    static void __stdcall DdiDpcRoutine(
        IN_CONST_PVOID  MiniportDeviceContext);

    static NTSTATUS
        DdiDispatchIoRequest(
            IN_CONST_PVOID              MiniportDeviceContext,
            IN_ULONG                    VidPnSourceId,
            IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket);

    static BOOLEAN
        DdiInterruptRoutine(
            IN_CONST_PVOID  MiniportDeviceContext,
            IN_ULONG        MessageNumber);

    static NTSTATUS __stdcall DdiBuildPagingBuffer(
        IN_CONST_HANDLE                 hAdapter,
        IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs);


};

