#include <windows.h>
#include <strsafe.h>
#include <dxgi1_3.h>
#include <d3d11.h>

#include <wrl.h>
#include <string>
#include <stdio.h>
#include <assert.h>

#include <exception>

using namespace Microsoft::WRL;

class wexception
{
public:
    wexception (PCWSTR Message, HRESULT Hr = S_OK) noexcept : m_hr(Hr)
    {
        try {
            m_message = Message;
        } catch (const std::exception&) {
            // string assignment operator can throw
        }
    }

    virtual ~wexception () {}

    virtual PCWSTR wwhat () const { return m_message.c_str(); }

    HRESULT HResult () const { return m_hr; }

private:
    std::wstring m_message;
    HRESULT m_hr;
};

class test_exception : public wexception
{
public:
    test_exception (PCWSTR Message, HRESULT Hr = S_OK) : wexception(Message, Hr) {}
};

bool FindAdapter(PCWSTR inDescription, _COM_Outptr_ IDXGIAdapter2 ** outAdapter2)
{
    assert(inDescription != NULL);
    assert(outAdapter2 != NULL);

    bool found = false;
    ComPtr<IDXGIFactory2> factory;
    HRESULT hr = CreateDXGIFactory2(
            0,
            __uuidof(IDXGIFactory2),
            reinterpret_cast<void**>(factory.GetAddressOf()));
    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_GEN_FAILURE)) {
            throw wexception(
                L"Failed to create instance of IDXGIFactory2. "
                L"The system must be rebooted before running rostest.exe.",
                hr);
        }

        throw wexception(
            L"Failed to create instance of IDXGIFactory2.",
            hr);
    }

    UINT adapterIndex = 0;
    bool done = false;

    while (!done && !found) {

        ComPtr<IDXGIAdapter1> adapter;

        hr = factory->EnumAdapters1(adapterIndex, &adapter);

        if (hr == S_OK)
        {
            ComPtr<IDXGIAdapter2> adapter2;
            hr = adapter.As(&adapter2);
            if (hr == S_OK)
            {
                DXGI_ADAPTER_DESC2 desc;
                adapter2->GetDesc2(&desc);

                found = (wcscmp(inDescription, desc.Description) == 0);

                if (found)
                {
                    *outAdapter2 = adapter2.Detach();
                }
            }

            adapterIndex++;
        }
        else
        {
            if (hr != DXGI_ERROR_NOT_FOUND)
            {
                throw wexception(
                    L"Unexpected error enumerating adapters.",
                    hr);
            }

            done = true;
        }
    }

    return found;
}

bool CreateDevice(IDXGIAdapter2 * pAdapter2, ID3D11Device ** outDevice)
{
    bool deviceCreated = false;
    D3D_FEATURE_LEVEL level;
    ID3D11DeviceContext * context = NULL;
    D3D_FEATURE_LEVEL requestedLevel = D3D_FEATURE_LEVEL_11_1;

    HRESULT hr = D3D11CreateDevice(pAdapter2, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &requestedLevel, 1, D3D11_SDK_VERSION, outDevice, &level, &context);

    deviceCreated = (hr == S_OK);

    if (!deviceCreated)
    {
        fwprintf(stderr, L"ERROR: Unable to create device (hr = %lx)\n", hr);
    }

    return deviceCreated;
}

bool TestShaderConstantBuffer(ID3D11Device * pDevice)
{
    bool testPassed = false;

    UINT32 data[256];

    for (int i = 0; i < 256; i++) data[i] = i;

    D3D11_SUBRESOURCE_DATA subresourceData;

    subresourceData.pSysMem = data;
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC desc;

    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = sizeof(data);
    desc.CPUAccessFlags = 0; // no CPU access
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;   // read and write access by GPU

    ComPtr<ID3D11Buffer> pBufferA;
    HRESULT hr = pDevice->CreateBuffer(&desc, &subresourceData, &pBufferA);
    if (FAILED(hr))
    {
        throw wexception(
            L"Failed to create shader constant buffer.",
            hr);
    }

    ComPtr<ID3D11Buffer> pBufferB;
    hr = pDevice->CreateBuffer(&desc, NULL, &pBufferB);
    if (FAILED(hr))
    {
        throw wexception(
            L"Failed to create shader constant buffer.",
            hr);
    }


    ComPtr<ID3D11DeviceContext> pDeviceContext;
    pDevice->GetImmediateContext(&pDeviceContext);

    pDeviceContext->CopyResource(pBufferB.Get(), pBufferA.Get());
    pDeviceContext->Flush();

    testPassed = true;

    return testPassed;
}

bool TestTexture2D(ID3D11Device * pDevice)
{
    bool testPassed = false;
    const UINT TEX_WIDTH = 64;
    const UINT TEX_HEIGHT = 64;
    const UINT PIXEL_SIZE = 4;

    // UINT32 automatically makes SysMemPitch DWORD aligned
    UINT32 data[TEX_WIDTH*TEX_HEIGHT];

    for (int i = 0; i < TEX_WIDTH*TEX_HEIGHT; i++)
    {
        data[i] = i;
    }

    D3D11_SUBRESOURCE_DATA subresourceData;

    subresourceData.pSysMem = data;
    subresourceData.SysMemPitch = TEX_WIDTH*PIXEL_SIZE;
    subresourceData.SysMemSlicePitch = subresourceData.SysMemPitch*TEX_HEIGHT;

    D3D11_TEXTURE2D_DESC desc;

    desc.Width = TEX_WIDTH;
    desc.Height = TEX_HEIGHT;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;   // read and write access by GPU
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;    // no CPU access
    desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> pTextureDefaultA;
    ComPtr<ID3D11Texture2D> pTextureDefaultB;
    ComPtr<ID3D11Texture2D> pTextureStagingA;
    ComPtr<ID3D11Texture2D> pTextureStagingB;
    ComPtr<ID3D11Texture2D> pTextureStagingC;
    ComPtr<ID3D11DeviceContext> pDeviceContext;

    try
    {
        data[0] = 'defA';
        HRESULT hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureDefaultA);

        if (hr != S_OK) throw test_exception(L"Failed to create default texture A", hr);

        data[0] = 'defB';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureDefaultB);

        if (hr != S_OK) throw test_exception(L"Failed to create default texture B", hr);

        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;    // read by CPU

        data[0] = 'stgA';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureStagingA);

        if (hr != S_OK) throw test_exception(L"Failed to create staging texture A", hr);

        data[0] = 'stgB';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureStagingB);

        if (hr != S_OK) throw test_exception(L"Failed to create staging texture B", hr);

        data[0] = 'stgC';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureStagingC);

        if (hr != S_OK) throw test_exception(L"Failed to create staging texture C", hr);

        pDevice->GetImmediateContext(&pDeviceContext);

        pDeviceContext->CopyResource(pTextureDefaultA.Get(), pTextureStagingA.Get());
        pDeviceContext->CopyResource(pTextureDefaultB.Get(), pTextureDefaultA.Get());
        pDeviceContext->CopyResource(pTextureStagingB.Get(), pTextureDefaultB.Get());
        pDeviceContext->CopyResource(pTextureStagingC.Get(), pTextureStagingB.Get());

        D3D11_MAPPED_SUBRESOURCE  mappedSubRes;

        hr = pDeviceContext->Map(pTextureStagingA.Get(), 0, D3D11_MAP_READ, 0, &mappedSubRes);

        if (hr != S_OK) throw test_exception(L"Failed to map staging texture A", hr);

        DWORD * pMappedData = static_cast<DWORD *>(mappedSubRes.pData);

        if (pMappedData[0] != 'stgA') throw test_exception(L"Staging A data mis-match");

        pDeviceContext->Unmap(pTextureStagingA.Get(), 0);

        hr = pDeviceContext->Map(pTextureStagingB.Get(), 0, D3D11_MAP_READ, 0, &mappedSubRes);

        if (hr != S_OK) throw test_exception(L"Failed to map staging texture B", hr);

        pMappedData = static_cast<DWORD *>(mappedSubRes.pData);

        if (pMappedData[0] != 'stgA') throw test_exception(L"copies failed, staging B incorrect");

        pDeviceContext->Unmap(pTextureStagingB.Get(), 0);

        hr = pDeviceContext->Map(pTextureStagingC.Get(), 0, D3D11_MAP_READ, 0, &mappedSubRes);

        if (hr != S_OK) throw test_exception(L"Failed to map staging texture C", hr);

        pMappedData = static_cast<DWORD *>(mappedSubRes.pData);

        if (pMappedData[0] != 'stgA') throw test_exception(L"staging to staging copy failed");

        pDeviceContext->Unmap(pTextureStagingC.Get(), 0);

        testPassed = true;
    }
    catch (const test_exception & e)
    {
        if (e.HResult() != S_OK)
            fwprintf(stderr, L"ERROR: %s (hr = 0x%08x)\n", e.wwhat(), e.HResult());
        else
            fwprintf(stderr, L"ERROR: %s\n", e.wwhat());
    }

    return testPassed;
}

bool RunTestsOnDevice(ID3D11Device * pDevice)
{
    bool passedAllTests = true;

    passedAllTests = passedAllTests && TestShaderConstantBuffer(pDevice);
    passedAllTests = passedAllTests && TestTexture2D(pDevice);

    return passedAllTests;
}

bool RunTestsOnAdapter(WCHAR * inAdapterDescription)
{
    bool passedAllTests = false;

    ComPtr<IDXGIAdapter2> pRosAdapter2;

    if (FindAdapter(inAdapterDescription, &pRosAdapter2))
    {
        ComPtr<ID3D11Device> pRosDevice;

        if (CreateDevice(pRosAdapter2.Get(), &pRosDevice))
        {
            passedAllTests = RunTestsOnDevice(pRosDevice.Get());
        }
    }
    else
    {
        fwprintf(stderr, L"ERROR: Unable to to find adapter %s\n", inAdapterDescription);
    }

    return passedAllTests;
}

int wmain (int argc, wchar_t *argv[])
{
    try
    {
        if (!RunTestsOnAdapter(L"Microsoft Basic Render Driver"))
        {
            fwprintf(stderr, L"ERROR: Basic Render Driver failed the tests\n");
        }

        if (RunTestsOnAdapter(L"Render Only Sample Driver"))
        {
            wprintf(L"Success\n");
        }
    }
    catch (const wexception& ex)
    {
        fwprintf(stderr, L"Error(0x%x): %s\n", ex.HResult(), ex.wwhat());
        return 1;
    }

    return 0;
}
