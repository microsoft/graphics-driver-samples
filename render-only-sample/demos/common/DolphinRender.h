#pragma once

typedef void * (*LoadResourceFunc)(int Name, unsigned long * pdwSize);

void UninitTargetSizeDependentDolphinResources();
bool InitTargetSizeDependentDolphinResources(bool useRosDriver, UINT rtWidth, UINT rtHeight, IDXGIAdapter* pAdapter, ID3D11Device* pDevice, ID3D11DeviceContext * pContext,
    ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView* pDepthStencilView);

void UninitDeviceDependentDolphinResources();
bool InitDeviceDependentDolphinResources(bool useTweenedNormal, LoadResourceFunc loadResourceFunc, ID3D11Device* pDevice, ID3D11DeviceContext * pContext);

void UpdateDolphin(bool useTweenedNormal, ID3D11DeviceContext * pContext);
void RenderDolphin(bool useTweenedNormal, ID3D11DeviceContext * pContext, ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView* pDepthStencilView);

bool LoadDeviceDependentDolphinResources(bool useTweenedNormal, LoadResourceFunc loadResourceFunc, ID3D11Device * inDevice, ID3D11DeviceContext * inContext);
