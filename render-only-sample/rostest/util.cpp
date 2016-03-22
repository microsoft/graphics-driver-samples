#include "precomp.h"
#include "util.h"

using namespace Microsoft::WRL;
using namespace WEX::TestExecution;

PCWSTR StringFromFeatureLevel (D3D_FEATURE_LEVEL FeatureLevel)
{
    switch (FeatureLevel) {
    case D3D_FEATURE_LEVEL_12_1: return L"12.1";
    case D3D_FEATURE_LEVEL_12_0: return L"12.0";
    case D3D_FEATURE_LEVEL_11_1: return L"11.1";
    case D3D_FEATURE_LEVEL_11_0: return L"11.0";
    case D3D_FEATURE_LEVEL_10_1: return L"10.1";
    case D3D_FEATURE_LEVEL_10_0: return L"10.0";
    case D3D_FEATURE_LEVEL_9_3: return L"9.3";
    case D3D_FEATURE_LEVEL_9_2: return L"9.2";
    case D3D_FEATURE_LEVEL_9_1: return L"9.1";
    default:
        throw MyException::Make(
            E_INVALIDARG,
            L"Invalid feature level: %d",
            FeatureLevel);
    }
}

ComPtr<IDXGIAdapter2> FindAdapter (PCWSTR Description)
{
    ComPtr<IDXGIFactory2> factory;
    VERIFY_SUCCEEDED(
        CreateDXGIFactory2(
            0,
            __uuidof(factory),
            reinterpret_cast<void**>(factory.GetAddressOf())),
        L"Verifying that CreateDXGIFactory2() succeeded");


    for (UINT adapterIndex = 0; ; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> adapter1;
        HRESULT hr = factory->EnumAdapters1(adapterIndex, &adapter1);
        if (FAILED(hr)) {
            if (hr == DXGI_ERROR_NOT_FOUND)
                break;

            VERIFY_SUCCEEDED(
                hr,
                L"Verifying that factory->EnumAdapters1(...) succeeded");
        }

        ComPtr<IDXGIAdapter2> adapter2;
        VERIFY_SUCCEEDED(
            adapter1.As(&adapter2),
            L"Doing QI for IDXGIAdapter2");

        DXGI_ADAPTER_DESC2 desc;
        VERIFY_SUCCEEDED(
            adapter2->GetDesc2(&desc),
            L"Getting DXGI_ADAPTER_DESC2 from adapter");

        if (wcscmp(Description, desc.Description) == 0) {
            LogComment(L"Found adapter: %s", Description);
            return adapter2;
        }
    }

    throw MyException::Make(
        DXGI_ERROR_NOT_FOUND,
        L"The specified adapter could not be found: %s",
        Description);
}

//
// TAEF runtime parameters affecting this method:
//
//   Adapter=Ros|Warp|Default
//      Specify whether to use the default adapter, ROS, or WARP.
//      The default is to use ROS.
//
//   NoDebugLayer=true|false
//      Specify whether to create the device with the debug layer.
//      The default is to create the device with the debug layer.
//
//   MaxFeatureLevel=12.1|12.0|11.1|11.0|10.1|10.0|9.3|9.2|9.1
//      Specify the highest feature level to be used. For example,
//      if you specify a maximum feature level of 10.1, the device will be
//      created with feature level 10.1, 10.0, 9.3, etc.
//      The default is 12.1.
//
void CreateDevice (
    _COM_Outptr_ ID3D11Device3** DevicePPtr,
    _COM_Outptr_ ID3D11DeviceContext3** ContextPPtr
    )
{
    *DevicePPtr = nullptr;
    *ContextPPtr = nullptr;

    HRESULT hr;

    ComPtr<IDXGIAdapter2> adapter;
    D3D_DRIVER_TYPE driverType;
    {
        WEX::Common::String adapterStr;
        hr = RuntimeParameters::TryGetValue(L"Adapter", adapterStr);
        if (SUCCEEDED(hr)) {
            if (_wcsicmp((const wchar_t*)adapterStr, L"Default") == 0) {
                driverType = D3D_DRIVER_TYPE_HARDWARE;
                adapter = nullptr;
            } else if (_wcsicmp((const wchar_t*)adapterStr, L"Ros") == 0) {
                driverType = D3D_DRIVER_TYPE_UNKNOWN;
                adapter = FindAdapter(ROS_DRIVER_NAME_WSZ);
            } else if (_wcsicmp((const wchar_t*)adapterStr, L"Warp") == 0) {
                driverType = D3D_DRIVER_TYPE_WARP;
                adapter = nullptr;
            } else {
                throw MyException::Make(
                    E_INVALIDARG,
                    L"Invalid Adapter value: %s. "
                    L"Valid values are: Default, Ros, Warp",
                    (const wchar_t*)adapterStr);
            }
        } else {
            // default to using ROS
            driverType = D3D_DRIVER_TYPE_UNKNOWN;
            adapter = FindAdapter(ROS_DRIVER_NAME_WSZ);
        }
    }

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    {
        bool noDebugLayer;
        hr = RuntimeParameters::TryGetValue(L"NoDebugLayer", noDebugLayer);
        if (SUCCEEDED(hr) && noDebugLayer) {
            LogComment(L"Creating device WITHOUT debug layer.");
        } else {
            LogComment(L"Creating device with debug layer.");
            creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }
    }

    const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

    PCWSTR const featureLevelStrings[] = {
        L"12.1",
        L"12.0",
        L"11.1",
        L"11.0",
        L"10.1",
        L"10.0",
        L"9.3",
        L"9.2",
        L"9.1"
    };

    static_assert(
        ARRAYSIZE(featureLevels) == ARRAYSIZE(featureLevelStrings),
        "featureLevels and featureLevelStrings must have same number of elements");

    int featureLevelIndex;
    {
        WEX::Common::String featureLevelStr;
        hr = RuntimeParameters::TryGetValue(L"MaxFeatureLevel", featureLevelStr);
        if (SUCCEEDED(hr)) {
            int i;
            for (i = 0; i < ARRAYSIZE(featureLevelStrings); ++i) {
                if (featureLevelStr == featureLevelStrings[i]) {
                    LogComment(
                        L"Will create device with feature level no higher than %s",
                        featureLevelStr.operator const wchar_t*());
                    break;
                }
            }

            if (i == ARRAYSIZE(featureLevelStrings)) {
                throw MyException::Make(
                    E_INVALIDARG,
                    L"Invalid feature level: %s. Valid feature levels are "
                    L"12.1, 12.0, 11.1, 11.0, 10.1, 10.0, 9.3, 9.2, 9.1",
                    featureLevelStr.operator const wchar_t*());
            }

            featureLevelIndex = i;
        } else {
            featureLevelIndex = 0;
        }
    }

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL selectedFeatureLevel;
    hr = D3D11CreateDevice(
            adapter.Get(),
            driverType,
            0,
            creationFlags,
            featureLevels + featureLevelIndex,
            ARRAYSIZE(featureLevels) - featureLevelIndex,
            D3D11_SDK_VERSION,
            &device,
            &selectedFeatureLevel,
            &context);
    if (FAILED(hr)) {
        throw MyException::Make(hr, L"D3D11CreateDevice(...) failed.");
    }

    LogComment(
        L"Successfully created device. Selected feature level: %s",
        StringFromFeatureLevel(selectedFeatureLevel));

    ComPtr<ID3D11Device3> device3;
    VERIFY_SUCCEEDED(
        device.As(&device3),
        L"Doing QI for ID3DDevice3");

    ComPtr<ID3D11DeviceContext3> context3;
    VERIFY_SUCCEEDED(
        context.As(&context3),
        L"Doing QI for ID3D11DeviceContext3");

    *DevicePPtr = device3.Detach();
    *ContextPPtr = context3.Detach();
}