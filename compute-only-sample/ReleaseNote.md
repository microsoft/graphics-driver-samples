# Release note for Compute Only Sample Driver

## Release 0.2
- UMD 12 has support to pass all ML meta commands to KMD
- KMD has stubs (CosKmExecuteMetaCommand*) for emulating HW meta command execution
- CosUmd12CommandListDddi.cpp is removed and DIRECT and COMPUTE commmand list are supported with the same DDI table
- STOP_IN_FUNCTION is split into STOP_IN_FUNCTION into TRACE_FUNCTION, ASSERT_FUNCTION and UNEXPECTED_DDI.
- UNEXPECTED_DDI is used to mark out DDIs UMD 12 doesn't expect to implement with some of them cleaned up.
- UMD 12 implements CopyBufferRegion DDI
- Mark CreateSRV, CreateSampler, CopyTextureRegion as unexpected and fail creation of Texture2D resource properly

---
## Release 0.1
- KMD exposes the proper PNP device through DdiAddDevice, DdiStartAdapter, DdiStopAdapter
- KMD supports a local video memory segment without emulation of aperture segment through DdiQueryAdapterInfo and DdiBuildPagingBuffer
- KMD creates KM Device for UMD 12 Device and KM Context for UMD 12 Command Queue.
- KMD supports DdiCreateAllocation for UMD 12 to support creation of Heap and Resource (1D Buffer)
- KMD supports the execution of DMA buffer through DdiRender, DdiPatch, DdiSubmitCommandBuffer and usage of DxgkCbNotifyInterrupt so that UMD 12 can implement the Command Queue related Ddis particularly Ddi_CommandQueue_ExecuteCommandLists
- UMD 12 implements Device Ddis for creation of UM Ddi objects (Shader, Root Signature, Descriptor Heap/Table, UAV, CBV besides aforementioned Heap and Resource)
- UMD 12 implements D3D12 Shader Resource Binding Model through creation of Descriptor and setting Root Constant, View and Descriptor Table on the Root Signature.
- UMD 12 implements Command List related Ddis to fill "_GPU_" commands into the command buffer for later execution. The most important Ddi here is the Dispatch for running Compute Shader.
- CopyResource is implement end-to-end by cooperation of UMD 12 and KMD
- UMD 12 implements Identity Meta Command by reusing KMD's support for CopyResource.
- UMD 12 supports WDDM 2.5 Resource Residency Model with MakeResident and Evict Ddis.
