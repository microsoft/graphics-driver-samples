#ifndef _COSKMDLOGGING_H_
#define _COSKMDLOGGING_H_ 1


//
// Copyright (C) Microsoft. All rights reserved.
//
// WPP tracing configuration file. Logs can be dumped from windbg with
// the following command:
//
//   !rcdrkd.rcdrlogdump roskmd
//

#define COS_ASSERTION(x, ...) { \
    DbgPrintEx( DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, x, __VA_ARGS__); \
    DbgBreakPoint(); \
    }


//
// Tracing GUID - B5B486C1-F57B-4993-8ED7-E3C2F5E4E65A
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(COSKMD, (B5B486C1,F57B,4993,8ED7,E3C2F5E4E65A), \
        WPP_DEFINE_BIT(COS_TRACING_DEFAULT) \
        WPP_DEFINE_BIT(COS_TRACING_PRESENT) \
        WPP_DEFINE_BIT(COS_TRACING_VIDPN) \
        WPP_DEFINE_BIT(COS_TRACING_DEBUG) \
        WPP_DEFINE_BIT(COS_TRACING_BUGCHECK) \
    )

#include <CosLogging.h>

#endif // _COSKMDLOGGING_H_
