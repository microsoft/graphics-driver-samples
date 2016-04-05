#ifndef _ROSUMDLOGGING_H_
#define _ROSUMDLOGGING_H_

//
// Tracing GUID - D124564D-F51F-402D-A86E-7E2247D30AF3
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ROSUMD, (D124564D,F51F,402D,A86E,7E2247D30AF3), \
        WPP_DEFINE_BIT(ROS_TRACING_DEFAULT) \
        WPP_DEFINE_BIT(ROS_TRACING_PRESENT) \
        WPP_DEFINE_BIT(ROS_TRACING_VIDPN) \
        WPP_DEFINE_BIT(ROS_TRACING_DEBUG) \
        WPP_DEFINE_BIT(ROS_TRACING_BUGCHECK) \
    )

#define WPP_LEVEL_FLAGS_LOGGER(level,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(level, flags) \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)

#include <RosLogging.h>

class RosUmdLogging
{
public:

    static void Entry(const char * inFunctionName)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "ENTRY: %s\r\n", inFunctionName);
        OutputDebugStringA(buffer);
    }

    static void Exit(const char * inFunctionName)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "EXIT: %s\r\n", inFunctionName);
        OutputDebugStringA(buffer);
    }

    static void Call(const char * inFunctionName)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "CALL: %s\r\n", inFunctionName);
        OutputDebugStringA(buffer);
    }

};

#endif // _ROSUMDLOGGING_H_
