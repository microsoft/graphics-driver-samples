//
//    Copyright (C) Microsoft.  All rights reserved.
//

#include <initguid.h>
#include <ntddk.h>
#include <ntintsafe.h>
#include <ntstrsafe.h>

#define ENABLE_DXGK_SAL

extern "C" {
#include <dispmprt.h>
#include <dxgiformat.h>
#include <WppRecorder.h>
} // extern "C"
