#pragma once

typedef void * (*LoadResourceFunc)(int Name, unsigned long * pdwSize);

void UninitD3D();
bool InitD3D(bool useRosDriver, unsigned int rtWidth, unsigned int rtHeight, IDXGIAdapter* pAdapter, ID3D11Device* pDevice, ID3D11DeviceContext * pContext);
void UninitDolphin();
bool InitDolphin(bool useTweenedNormal, LoadResourceFunc loadResourceFunc, ID3D11Device* pDevice, ID3D11DeviceContext * pContext);
void UpdateDolphin(bool useTweenedNormal, ID3D11DeviceContext * pContext);
void RenderDolphin(bool useTweenedNormal, ID3D11DeviceContext * pContext);
void SaveDolphin(int iFrame, ID3D11Device* pDevice, ID3D11DeviceContext * pContext);

bool LoadDeviceDependentDolphinResources(bool useTweenedNormal, LoadResourceFunc loadResourceFunc, ID3D11Device * inDevice, ID3D11DeviceContext * inContext);
