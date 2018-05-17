// coscon.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include <stdio.h>

#include <exception>
#include <memory>
#include <list>

template<class T> class D3DPointer
{
public:

    D3DPointer() : m_p(NULL) {}
    ~D3DPointer() { if (m_p) m_p->Release(); }

    T * operator->() { return m_p; }

    operator bool() { return m_p != NULL; }
    operator T *() { return m_p; }

    D3DPointer<T> & operator=(T * p) { if (m_p) m_p->Release();  m_p = p; return *this; }

private:

    T * m_p;

};


class D3DDevice
{
public:

    D3DDevice(std::wstring & driverString)
    {
        if (!FindAdapter(driverString)) throw std::exception("Failed to find adapter");
        if (!CreateDevice()) throw std::exception("Failed to create device");
    }

    ~D3DDevice()
    {
        // do nothing
    }

    ID3D11Device * GetDevice() { return m_pDevice; }
    ID3D11DeviceContext * GetContext() { return m_pContext; }

private:

    D3DPointer<ID3D11Device>        m_pDevice;
    D3DPointer<ID3D11DeviceContext> m_pContext;
    D3DPointer<IDXGIAdapter2>       m_pAdapter;

    bool CreateDevice()
    {
        bool deviceCreated = false;
        D3D_FEATURE_LEVEL level;
        ID3D11DeviceContext * context = NULL;
        D3D_FEATURE_LEVEL requestedLevel = D3D_FEATURE_LEVEL_11_1;

        ID3D11Device * pDevice;
        ID3D11DeviceContext * pContext;

        HRESULT hr = D3D11CreateDevice(m_pAdapter, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &requestedLevel, 1, D3D11_SDK_VERSION, &pDevice, &level, &pContext);

        deviceCreated = (hr == S_OK);

        if (deviceCreated)
        {
            m_pDevice = pDevice;
            m_pContext = pContext;
        }

        return deviceCreated;
    }

    bool FindAdapter(std::wstring & driverString)
    {
        bool found = false;
        IDXGIFactory2 * factory = NULL;
        HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), ((void **)&factory));
        if (FAILED(hr)) throw std::exception("Unable to create DXGIFactor2");

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

                    found = (wcscmp(driverString.c_str(), desc.Description) == 0);

                    if (found)
                    {
                        m_pAdapter = adapter2;
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
                done = true;
            }
        }

        factory->Release();

        return found;
    }

};


class D3DAdapter
{
public:

    static void GetAdapterList(std::list<std::wstring> & list)
    {
        IDXGIFactory2 * factory = NULL;
        HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), ((void **)&factory));
        if (FAILED(hr)) throw std::exception("Unable to create DXGIFactor2");

        UINT adapterIndex = 0;
        bool done = false;

        list.clear();

        while (!done) {

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

                    list.push_back(desc.Description);

                    adapter2->Release();
                }

                adapter->Release();

                adapterIndex++;
            }
            else
            {
                done = true;
            }
        }

        factory->Release();
    }
};

class D3DShader
{
public:

protected:

    void CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3DBlob* pErrorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
            dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

        if (FAILED(hr) && pErrorBlob)
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));

        if (pErrorBlob)
            pErrorBlob->Release();

        if (FAILED(hr)) throw std::exception("Failed to compile shader from file");

    }

};

class D3DComputeShader : public D3DShader
{
public:

    D3DComputeShader(D3DDevice & inDevice)
    {
        ID3DBlob* pCSBlob = nullptr;

        CompileShaderFromFile(L"ComputeShader.hlsl", "main", "cs_4_0", &pCSBlob);

        D3DPointer<ID3DBlob> csBlob;

        csBlob = pCSBlob;

        // Create the vertex shader
        ID3D11ComputeShader * pComputeShader;
        HRESULT hr = inDevice.GetDevice()->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &pComputeShader);
        if (FAILED(hr)) throw std::exception("Failed to create compute shader");

        m_pComputeShader = pComputeShader;
    }

    ~D3DComputeShader()
    {

    }

    ID3D11ComputeShader * GetComputeShader() { return m_pComputeShader; }

private:

    D3DPointer<ID3D11ComputeShader> m_pComputeShader;

};

class D3DStructuredBuffer
{
public:

    D3DStructuredBuffer(D3DDevice & inDevice, UINT uElementSize, UINT uCount, void* pInitData)
    {
        D3D11_BUFFER_DESC bufferDesc = {};

        bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.ByteWidth = uElementSize * uCount;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = uElementSize;

        ID3D11Buffer * pBuffer;
        D3D11_SUBRESOURCE_DATA data = {};
        D3D11_SUBRESOURCE_DATA* pData = NULL;

        if (pInitData) {
            data.pSysMem = pInitData;
            pData = &data;
        }
        
        HRESULT hr = inDevice.GetDevice()->CreateBuffer(&bufferDesc, pData, &pBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create structured buffer");

        m_pStructuredBuffer = pBuffer;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement = 0;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.BufferEx.NumElements = uCount;

        ID3D11ShaderResourceView * pShaderResourceView;
        hr = inDevice.GetDevice()->CreateShaderResourceView(m_pStructuredBuffer, &srvDesc, &pShaderResourceView);
        if (FAILED(hr)) throw std::exception("Unable to create shader resource view");

        m_pShaderResourceView = pShaderResourceView;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.Buffer.NumElements = uCount;

        ID3D11UnorderedAccessView * pUnorderedAccessView;
        hr = inDevice.GetDevice()->CreateUnorderedAccessView(m_pStructuredBuffer, &uavDesc, &pUnorderedAccessView);
        if (FAILED(hr)) throw std::exception("Unable to create unordered access view");

        m_pUnorderedAccessView = pUnorderedAccessView;

        D3D11_BUFFER_DESC cpuDesc = bufferDesc;
        cpuDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        cpuDesc.Usage = D3D11_USAGE_STAGING;
        cpuDesc.BindFlags = 0;
        cpuDesc.MiscFlags = 0;

        ID3D11Buffer * pCpuReadBuffer;
        hr = inDevice.GetDevice()->CreateBuffer(&cpuDesc, nullptr, &pCpuReadBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create cpu read buffer");

        m_pCpuReadBuffer = pCpuReadBuffer;
    }

    void Read(D3DDevice & inDevice, UINT size, void * dst)
    {
        inDevice.GetContext()->CopyResource(m_pCpuReadBuffer, m_pStructuredBuffer);

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT hr = inDevice.GetContext()->Map(m_pCpuReadBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
        if (FAILED(hr)) throw std::exception("Unable to map cpu read buffer");

        memcpy(dst, mappedResource.pData, size);

        inDevice.GetContext()->Unmap(m_pCpuReadBuffer, 0);
    }

    ~D3DStructuredBuffer()
    {
        // do nothing
    }

    ID3D11Buffer * GetStructuredBuffer() { return m_pStructuredBuffer; }
    ID3D11ShaderResourceView * GetShaderResourceView() { return m_pShaderResourceView; }
    ID3D11UnorderedAccessView * GetUnorderedAccessView() { return m_pUnorderedAccessView; }

private:

    D3DPointer<ID3D11Buffer> m_pCpuReadBuffer;

    D3DPointer<ID3D11Buffer> m_pStructuredBuffer;
    D3DPointer<ID3D11ShaderResourceView> m_pShaderResourceView;
    D3DPointer<ID3D11UnorderedAccessView> m_pUnorderedAccessView;

};


struct BufType
{
    int i;
    float f;
};

#define NUM_ELEMENTS 1024

BufType g_vBuf[2][NUM_ELEMENTS];

int main()
{
    std::wstring cosDriverString = L"Compute Only Sample Driver";
    std::wstring brdDriverString = L"Microsoft Basic Render Driver";
    std::list<std::wstring> adapterList;

    D3DAdapter::GetAdapterList(adapterList);

    printf("Found adapters:\n");
    for (std::wstring & a : adapterList)
        printf("  %S\n", a.c_str());

    auto findIter = std::find(adapterList.begin(), adapterList.end(), brdDriverString);

    if (findIter == adapterList.end()) {
        printf("%S was not found\n", cosDriverString.c_str());
        return 0;
    }

    try {

        std::wstring driverString = cosDriverString;

        findIter = std::find(adapterList.begin(), adapterList.end(), driverString);

        if (findIter == adapterList.end())
            driverString = brdDriverString;


        printf("Creating device on %S ... ", driverString.c_str());
        D3DDevice computeDevice(driverString);
        printf("done.\n");

        printf("Creating compute shader ... ");
        D3DComputeShader computeShader(computeDevice);
        printf("done.\n");

        printf("Creating buffers ... ");
        for (int j = 0; j < 2; j++) {
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                g_vBuf[j][i].i = i;
                g_vBuf[j][i].f = (float)i;
            }
        }

        D3DStructuredBuffer buf1(computeDevice, sizeof(BufType), NUM_ELEMENTS, g_vBuf[0]);
        D3DStructuredBuffer buf2(computeDevice, sizeof(BufType), NUM_ELEMENTS, g_vBuf[1]);
        D3DStructuredBuffer buf3(computeDevice, sizeof(BufType), NUM_ELEMENTS, NULL);
        printf("done.\n");


        printf("Running shader ... ");

        computeDevice.GetContext()->CSSetShader(computeShader.GetComputeShader(), NULL, 0);

        ID3D11ShaderResourceView * srvs[2];
        srvs[0] = buf1.GetShaderResourceView();
        srvs[1] = buf2.GetShaderResourceView();
        computeDevice.GetContext()->CSSetShaderResources(0, 2, srvs);

        ID3D11UnorderedAccessView * uavs[1];
        uavs[0] = buf3.GetUnorderedAccessView();
        computeDevice.GetContext()->CSSetUnorderedAccessViews(0, 1, uavs, NULL);

        computeDevice.GetContext()->Dispatch(NUM_ELEMENTS, 1, 1);

        printf("done.\n");

        BufType result[NUM_ELEMENTS];
        buf3.Read(computeDevice, sizeof(result), result);

        printf("checking results ... ");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            if (result[i].i != i * 2) {
                printf("Bad integer result %d at index %d\n", result[i].i, i);
                break;
            }

            if (result[i].f != (float) (i * 2)) {
                printf("Bad float result %f at index %d\n", result[i].f, i);
                break;
            }
        }

        printf("done.\n");

    }
    catch (std::exception & e)
    {
        printf("Hit error: %s\n", e.what());
    }

    printf("Done.\n");

    return 0;
}

