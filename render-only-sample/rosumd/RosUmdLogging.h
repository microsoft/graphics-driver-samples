#pragma once

#include <debugapi.h>
#include <stdio.h>

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
