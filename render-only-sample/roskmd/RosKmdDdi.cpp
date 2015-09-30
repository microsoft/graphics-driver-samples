#include "RosKmd.h"

#include "RosKmdAdapter.h"
#include "RosKmdDevice.h"
#include "RosKmdAllocation.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"

NTSTATUS
RosKmdPnpDispatch(
    __in struct _DEVICE_OBJECT *DeviceObject,
    __inout struct _IRP *pIrp)
{
    DeviceObject;
    pIrp;

    return STATUS_NOT_SUPPORTED;
}


