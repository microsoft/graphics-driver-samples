#pragma once

typedef void * (*LoadResourceFunc)(int Name, unsigned long * pdwSize);

void UninitD3D();
bool InitD3D(bool useRosDriver, unsigned int rtWidth, unsigned int rtHeight);
void UninitDolphin();
bool InitDolphin(bool useTweenedNormal, LoadResourceFunc loadResourceFunc);
void UpdateDolphin(bool useTweenedNormal);
void RenderDolphin(bool useTweenedNormal);
void SaveDolphin(int iFrame);
void UninitPerf();
bool InitPerf();
void EnqueueRenderEvent();
unsigned long WaitForRenderEvent();
void ResetRenderEvent();
void FlushRender();

bool LoadDeviceDependentDolphinResources(bool useTweenedNormal, LoadResourceFunc loadResourceFunc, ID3D11Device * inDevice, ID3D11DeviceContext * inContext);
