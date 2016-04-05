#ifndef _ROSKMDLOGGING_H_
#define _ROSKMDLOGGING_H_ 1

//
// Copyright (C) Microsoft. All rights reserved.
//
// WPP tracing configuration file. Logs can be dumped from windbg with
// the following command:
//
//   !rcdrkd.rcdrlogdump roskmd
//

//
// Tracing GUID - B5B486C1-F57B-4993-8ED7-E3C2F5E4E65A
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ROSKMD, (B5B486C1,F57B,4993,8ED7,E3C2F5E4E65A), \
        WPP_DEFINE_BIT(ROS_TRACING_DEFAULT) \
        WPP_DEFINE_BIT(ROS_TRACING_PRESENT) \
        WPP_DEFINE_BIT(ROS_TRACING_VIDPN) \
        WPP_DEFINE_BIT(ROS_TRACING_DEBUG) \
        WPP_DEFINE_BIT(ROS_TRACING_BUGCHECK) \
    )

#include <RosLogging.h>

#endif // _ROSKMDLOGGING_H_
