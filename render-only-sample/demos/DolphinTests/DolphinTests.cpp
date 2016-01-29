// DolphinTests.cpp : Defines the entry point for the console application.
//

#include <SDKDDKVer.h>

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "BitmapDecode.h"
#include "SDKmesh.h"
#include "resource.h"
#include "xTimer.h"

#include <math.h>
#include <directxmath.h>

#include <DolphinRender.h>

static void * MyLoadResource(INT Name, DWORD *pdwSize)
{
    HRSRC hRsrc = FindResourceExW(NULL, RT_RCDATA, MAKEINTRESOURCE(Name), 0);
    if (hRsrc == NULL)
    {
        __debugbreak();
        return NULL;
    }
    HGLOBAL hRes = LoadResource(NULL, hRsrc);
    if (hRes == NULL)
    {
        __debugbreak();
        return NULL;
    }
    *pdwSize = SizeofResource(NULL, hRsrc);
    return (LockResource(hRes));
}

int main(int argc, char *argv[])
{
    BOOL            bPerfMode = false;
    bool            useRosDriver = false;
    bool            useTweenedNormal = true;

#if defined(_M_ARM) && defined(VC4)
    useRosDriver = true;
#endif

    UINT            rtWidth = 800;
    UINT            rtHeight = 600;
    UINT            frames = 3;

    LARGE_INTEGER   framesStart;
    LARGE_INTEGER   framesEnd;

    LARGE_INTEGER   frequenceStart;
    LARGE_INTEGER   frequenceEnd;

    if (argc >= 3)
    {
        bPerfMode = true;
    }

    if (bPerfMode)
    {
        sscanf_s(argv[1], "%d", &rtWidth);
        sscanf_s(argv[2], "%d", &rtHeight);

        if (argc > 3)
        {
            sscanf_s(argv[3], "%d", &frames);

            if (frames < 20)
            {
                frames = 20;
            }
        }
        else
        {
            frames = 20;
        }
    }

    InitD3D(useRosDriver, rtWidth, rtHeight);

    InitDolphin(useTweenedNormal, MyLoadResource);

    if (bPerfMode)
    {
        InitPerf();

        QueryPerformanceCounter(&framesStart);
        QueryPerformanceFrequency(&frequenceStart);
    }

    for (UINT i = 0; i < frames; i++)
    {
        // Skip the 1st frame for shader compilation time
        if (bPerfMode && (i == 1))
        {
            QueryPerformanceCounter(&framesStart);
            QueryPerformanceFrequency(&frequenceStart);
        }

        UpdateDolphin(useTweenedNormal);
        RenderDolphin(useTweenedNormal);

        if (!bPerfMode)
        {
            SaveDolphin(i);
        }
        else
        {
            if (i == 0)
            {
                //
                // Wait for the 1st frame to finish to account for the GPU paging cost
                //

                EnqueueRenderEvent();
                WaitForRenderEvent();
                ResetRenderEvent();
            }
            else if (i < (frames - 1))
            {
                FlushRender();
            }
            else
            {
                EnqueueRenderEvent();
            }
        }
    }

    if (bPerfMode)
    {
        DWORD dwWaitResult = WaitForRenderEvent();
        
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            QueryPerformanceCounter(&framesEnd);
            QueryPerformanceFrequency(&frequenceEnd);

            UINT measuredFrames = frames - 1;

            if (frequenceStart.QuadPart != frequenceEnd.QuadPart)
            {
                printf("Perf frequence changed during %d frames of rendering", measuredFrames);
            }

            printf(
                "Average rendering time for (%d x %d) from %d frames: %I64d ms\n",
                rtWidth,
                rtHeight,
                measuredFrames,
                ((framesEnd.QuadPart - framesStart.QuadPart) * 1000) / (measuredFrames*frequenceEnd.QuadPart));
        }

        UninitPerf();
    }

    UninitDolphin();
    UninitD3D();

    return 0;
}
