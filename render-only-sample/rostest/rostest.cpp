#include <windows.h>
#include <dxgi1_3.h>
#include <d3d11.h>

#include <stdio.h>
#include <assert.h>

#include <exception>

class test_exception
{
public:

    test_exception(const char * inMessage, HRESULT inHr = S_OK) : m_message(inMessage), m_hr(inHr) { }

    const char * m_message;
    HRESULT m_hr;
};

bool FindAdapter(WCHAR * inDescription, IDXGIAdapter2 ** outAdapter2)
{
    assert(inDescription != NULL);
    assert(outAdapter2 != NULL);

    bool found = false;
    IDXGIFactory2 * factory = NULL;
    HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), ((void **)&factory));

    if (hr == S_OK)
    {
        UINT adapterIndex = 0;
        bool done = false;

        while (!done && !found) {

            IDXGIAdapter1 * adapter = NULL;

            hr = factory->EnumAdapters1(adapterIndex, &adapter);

            if (hr == S_OK)
            {
                IDXGIAdapter2 * adapter2 = NULL;

                hr = adapter->QueryInterface(__uuidof(IDXGIAdapter2), (void **)&adapter2);

                if (hr == S_OK)
                {
                    DXGI_ADAPTER_DESC2 desc;
                    adapter2->GetDesc2(&desc);

                    found = (wcscmp(inDescription, desc.Description) == 0);

                    if (found)
                    {
                        *outAdapter2 = adapter2;
                    }
                    else
                    {
                        adapter2->Release();
                    }
                }

                adapter->Release();

                adapterIndex++;
            }
            else
            {
                if (hr != DXGI_ERROR_NOT_FOUND)
                {
                    fprintf(stderr, "ERROR: Unexpected error enumerating adapters (hr = %lx)\n", hr);
                }

                done = true;
            }
        }

        factory->Release();
    }
    else
    {
        fprintf(stderr, "Unable to create DXGIFactory2\n");
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
        fprintf(stderr, "ERROR: Unable to create device (hr = %lx)\n", hr);
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

    ID3D11Buffer * pBufferA;

    HRESULT hr = pDevice->CreateBuffer(&desc, &subresourceData, &pBufferA);

    if (hr == S_OK)
    {
        ID3D11Buffer * pBufferB;

        hr = pDevice->CreateBuffer(&desc, NULL, &pBufferB);

        if (hr == S_OK)
        {
            ID3D11DeviceContext * pDeviceContext;

            pDevice->GetImmediateContext(&pDeviceContext);

            pDeviceContext->CopyResource(pBufferB, pBufferA);
            pDeviceContext->Flush();
            pDeviceContext->Release();

            testPassed = true;
            pBufferB->Release();
        }

        pBufferA->Release();
    }
    else
    {
        fprintf(stderr, "ERROR: failed to create shader constant buffer (hr = %lx)\n", hr);
    }

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

    ID3D11Texture2D * pTextureDefaultA = NULL;
    ID3D11Texture2D * pTextureDefaultB = NULL;
    ID3D11Texture2D * pTextureStagingA = NULL;
    ID3D11Texture2D * pTextureStagingB = NULL;
    ID3D11Texture2D * pTextureStagingC = NULL;
    ID3D11DeviceContext * pDeviceContext = NULL;

    try
    {
        data[0] = 'defA';
        HRESULT hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureDefaultA);

        if (hr != S_OK) throw test_exception("Failed to create default texture A", hr);

        data[0] = 'defB';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureDefaultB);

        if (hr != S_OK) throw test_exception("Failed to create default texture B", hr);

        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;    // read by CPU

        data[0] = 'stgA';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureStagingA);

        if (hr != S_OK) throw test_exception("Failed to create staging texture A", hr);

        data[0] = 'stgB';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureStagingB);

        if (hr != S_OK) throw test_exception("Failed to create staging texture B", hr);

        data[0] = 'stgC';
        hr = pDevice->CreateTexture2D(&desc, &subresourceData, &pTextureStagingC);

        if (hr != S_OK) throw test_exception("Failed to create staging texture C", hr);

        pDevice->GetImmediateContext(&pDeviceContext);

        pDeviceContext->CopyResource(pTextureDefaultA, pTextureStagingA);
        pDeviceContext->CopyResource(pTextureDefaultB, pTextureDefaultA);
        pDeviceContext->CopyResource(pTextureStagingB, pTextureDefaultB);
        pDeviceContext->CopyResource(pTextureStagingC, pTextureStagingB);

        D3D11_MAPPED_SUBRESOURCE  mappedSubRes;

        hr = pDeviceContext->Map(pTextureStagingA, 0, D3D11_MAP_READ, 0, &mappedSubRes);

        if (hr != S_OK) throw test_exception("Failed to map staging texture A", hr);

        DWORD * pMappedData = static_cast<DWORD *>(mappedSubRes.pData);

        if (pMappedData[0] != 'stgA') throw test_exception("Staging A data mis-match");

        pDeviceContext->Unmap(pTextureStagingA, 0);

        hr = pDeviceContext->Map(pTextureStagingB, 0, D3D11_MAP_READ, 0, &mappedSubRes);

        if (hr != S_OK) throw test_exception("Failed to map staging texture B", hr);

        pMappedData = static_cast<DWORD *>(mappedSubRes.pData);

        if (pMappedData[0] != 'stgA') throw test_exception("copies failed, staging B incorrect");

        pDeviceContext->Unmap(pTextureStagingB, 0);

        hr = pDeviceContext->Map(pTextureStagingC, 0, D3D11_MAP_READ, 0, &mappedSubRes);

        if (hr != S_OK) throw test_exception("Failed to map staging texture C", hr);

        pMappedData = static_cast<DWORD *>(mappedSubRes.pData);

        if (pMappedData[0] != 'stgA') throw test_exception("staging to staging copy failed");

        pDeviceContext->Unmap(pTextureStagingC, 0);

        testPassed = true;
    }

    catch (test_exception & e)
    {
        if (e.m_hr != S_OK)
            fprintf(stderr, "ERROR: %s (hr = 0x%08x)\n", e.m_message, e.m_hr);
        else
            fprintf(stderr, "ERROR: %s\n", e.m_message);
    }

    if (pTextureDefaultA != NULL) pTextureDefaultA->Release();
    if (pTextureDefaultB != NULL) pTextureDefaultB->Release();
    if (pTextureStagingA != NULL) pTextureStagingA->Release();
    if (pTextureStagingB != NULL) pTextureStagingB->Release();
    if (pTextureStagingC != NULL) pTextureStagingC->Release();
    if (pDeviceContext != NULL) pDeviceContext->Release();

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

    IDXGIAdapter2 * pRosAdapter2 = NULL;

    if (FindAdapter(inAdapterDescription, &pRosAdapter2))
    {
        ID3D11Device * pRosDevice = NULL;

        if (CreateDevice(pRosAdapter2, &pRosDevice))
        {
            passedAllTests = RunTestsOnDevice(pRosDevice);

            pRosDevice->Release();
        }

        pRosAdapter2->Release();
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to to find adapter %S\n", inAdapterDescription);
    }

    return passedAllTests;
}

int main(int argc, char ** argv)
{
    if (!RunTestsOnAdapter(L"Microsoft Basic Render Driver"))
    {
        fprintf(stderr, "ERROR: Basic Render Driver failed the tests\n");
    }

    if (RunTestsOnAdapter(L"Render Only Sample Driver"))
    {
        printf("Success\n");
    }
}
