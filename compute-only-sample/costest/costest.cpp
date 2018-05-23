// coscon.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <d3d12.h>

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

    ID3D12Device * GetDevice() { return m_pDevice; }
    ID3D12CommandAllocator * GetCommandAllocator() { return m_pCommandAllocator; }
    ID3D12CommandQueue * GetCommandQueue() { return m_pCommandQueue; }
    ID3D12GraphicsCommandList * GetCommandList() { return m_pCommandList; }
    ID3D12RootSignature * GetRootSignature() { return m_pRootSignature; }


private:

    D3DPointer<IDXGIAdapter2>               m_pAdapter;
    D3DPointer<ID3D12Device>                m_pDevice;
    D3DPointer<ID3D12CommandAllocator>      m_pCommandAllocator;
    D3DPointer<ID3D12CommandQueue>          m_pCommandQueue;
    D3DPointer<ID3D12GraphicsCommandList>   m_pCommandList;
    D3DPointer<ID3D12RootSignature>         m_pRootSignature;

    bool CreateDevice()
    {
        ID3D12Device * pDevice;

        HRESULT hr = D3D12CreateDevice(m_pAdapter, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&pDevice));

        bool success = (hr == S_OK);

        if (success)
            m_pDevice = pDevice;

        if (success) {
            ID3D12CommandQueue * pCommandQueue;
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc;

            commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            commandQueueDesc.Priority = 0;
            commandQueueDesc.NodeMask = 0;

            hr = pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&pCommandQueue));

            success = (hr == S_OK);

            if (success)
                m_pCommandQueue = pCommandQueue;
        }

        if (success) {
            ID3D12CommandAllocator * pCommandAllocator;

            hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator));

            success = (hr == S_OK);

            if (success)
                m_pCommandAllocator = pCommandAllocator;
        }

        if (success) {
            ID3D12GraphicsCommandList * pCommandList;

            hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, NULL, IID_PPV_ARGS(&pCommandList));

            success = (hr == S_OK);

            if (success)
                m_pCommandList = pCommandList;
        }

        if (success) {
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            D3D12_ROOT_PARAMETER rootParameters[3];
            int index = 0;

            rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParameters[index].Descriptor.RegisterSpace = 0;
            rootParameters[index].Descriptor.ShaderRegister = 0;
            rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            index++;
            
            rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParameters[index].Descriptor.RegisterSpace = 0;
            rootParameters[index].Descriptor.ShaderRegister = 1;
            rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            index++;

            rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParameters[index].Descriptor.RegisterSpace = 0;
            rootParameters[index].Descriptor.ShaderRegister = 2;
            rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            index++;

            rootSignatureDesc.NumParameters = index;
            rootSignatureDesc.pParameters = rootParameters;
            rootSignatureDesc.NumStaticSamplers = 0;
            rootSignatureDesc.pStaticSamplers = NULL;
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

            ID3DBlob * pBlob;
            ID3DBlob * pErrorBlob;
            hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, &pErrorBlob);

            success = (hr == S_OK);

            if (success) {
                ID3D12RootSignature * pRootSignature;

                hr = pDevice->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));

                success = (hr == S_OK);

                if (success)
                    m_pRootSignature = pRootSignature;
            }
        }

        return success;
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

        CompileShaderFromFile(L"ComputeShader.hlsl", "main", "cs_5_0", &pCSBlob);

        D3DPointer<ID3DBlob> csBlob;

        csBlob = pCSBlob;

        ID3D12PipelineState * pPipelineState;
        D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc;

        pipelineStateDesc.pRootSignature = inDevice.GetRootSignature();
        pipelineStateDesc.CS.pShaderBytecode = csBlob->GetBufferPointer();
        pipelineStateDesc.CS.BytecodeLength = csBlob->GetBufferSize();
        pipelineStateDesc.CachedPSO.pCachedBlob = NULL;
        pipelineStateDesc.CachedPSO.CachedBlobSizeInBytes = 0;
        pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        pipelineStateDesc.NodeMask = 0;

        HRESULT hr = inDevice.GetDevice()->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pPipelineState));

        if (FAILED(hr)) throw std::exception("Failed to create compute pipeline state");
        
        m_pPipelineState = pPipelineState;

    }

    ~D3DComputeShader()
    {

    }

    ID3D12PipelineState * GetComputePipelineState() { return m_pPipelineState; }

private:

    D3DPointer<ID3D12PipelineState> m_pPipelineState;

};

class D3DStructuredBuffer
{
public:

    D3DStructuredBuffer(D3DDevice & inDevice, UINT uElementSize, UINT uCount, void * pSrc = NULL)
    {
        UINT64 bufferSize = uElementSize * uCount;

        D3D12_HEAP_PROPERTIES heapProperties;

        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 0;
        heapProperties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resourceDesc;

        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = bufferSize;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        ID3D12Resource * pBuffer;

        OutputDebugStringA("creating structured buffer\n");

        HRESULT hr = inDevice.GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, IID_PPV_ARGS(&pBuffer));

        if (FAILED(hr)) throw std::exception("Unable to create structed buffer");

        m_pBuffer = pBuffer;

        ID3D12Resource * pInputBuffer;

        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        OutputDebugStringA("creating structured input buffer\n");

        hr = inDevice.GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&pInputBuffer));

        if (FAILED(hr)) throw std::exception("Unable to create structed input buffer");

        m_pInputBuffer = pInputBuffer;

        ID3D12Resource * pOutputBuffer;

        heapProperties.Type = D3D12_HEAP_TYPE_READBACK;

        OutputDebugStringA("creating structured output buffer\n");

        hr = inDevice.GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&pOutputBuffer));

        if (FAILED(hr)) throw std::exception("Unable to create structed output buffer");

        m_pOutputBuffer = pOutputBuffer;

        if (pSrc != NULL) {
            void * pDst;
            D3D12_RANGE range = { 0, 0 };
            m_pInputBuffer->Map(0, &range, &pDst);
            memcpy(pDst, pSrc, bufferSize);
            m_pInputBuffer->Unmap(0, NULL);
        }
    }

    void GetOutput( UINT uElementSize, UINT uCount, void * pDst)
    {
        UINT64 bufferSize = uElementSize * uCount;
        D3D12_RANGE range = { 0, bufferSize };
        void * pSrc;
        m_pOutputBuffer->Map(0, &range, &pSrc);
        memcpy(pDst, pSrc, bufferSize);

        range.End = 0;
        m_pOutputBuffer->Unmap(0, &range);
    }

    ~D3DStructuredBuffer()
    {
        // do nothing
    }

    ID3D12Resource * GetBuffer() { return m_pBuffer; }
    ID3D12Resource * GetInputBuffer() { return m_pInputBuffer; }
    ID3D12Resource * GetOutputBuffer() { return m_pOutputBuffer; }

private:

    D3DPointer<ID3D12Resource> m_pBuffer;
    D3DPointer<ID3D12Resource> m_pInputBuffer;
    D3DPointer<ID3D12Resource> m_pOutputBuffer;

};


struct BufType
{
    int i;
    float f;
};

#define NUM_ELEMENTS     1024
#define THREADS_IN_GROUP 4      // Expressed in the shader

BufType g_vBuf[2][NUM_ELEMENTS];

int main()
{
    int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;

    _CrtSetDbgFlag(dbgFlags);
    std::wstring cosDriverString = L"Compute Only Sample Driver";
    std::wstring brdDriverString = L"Microsoft Basic Render Driver";
    std::wstring amdDriverString = L"Radeon (TM) RX 480 Graphics";
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
//        std::wstring driverString = amdDriverString;

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

        D3DStructuredBuffer inputA(computeDevice, sizeof(BufType), NUM_ELEMENTS, g_vBuf[0]);
        D3DStructuredBuffer inputB(computeDevice, sizeof(BufType), NUM_ELEMENTS, g_vBuf[1]);
        D3DStructuredBuffer output(computeDevice, sizeof(BufType), NUM_ELEMENTS);
        printf("done.\n");


        printf("Running shader ... ");

        ID3D12Device * pDevice = computeDevice.GetDevice();
        ID3D12GraphicsCommandList * pCommandList = computeDevice.GetCommandList();
        ID3D12PipelineState * pPipelineState = computeShader.GetComputePipelineState();
        ID3D12RootSignature * pRootSignature = computeDevice.GetRootSignature();

        ID3D12Fence * pFence;
        HRESULT hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
        if (FAILED(hr)) throw std::exception("Failed to create fence");

        pCommandList->SetPipelineState(pPipelineState);
        pCommandList->SetComputeRootSignature(pRootSignature);

        pCommandList->SetComputeRootUnorderedAccessView(0, inputA.GetBuffer()->GetGPUVirtualAddress());
        pCommandList->SetComputeRootUnorderedAccessView(1, inputB.GetBuffer()->GetGPUVirtualAddress());
        pCommandList->SetComputeRootUnorderedAccessView(2, output.GetBuffer()->GetGPUVirtualAddress());

#if 1
        D3D12_RESOURCE_BARRIER copyBarrier;
        copyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        copyBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        copyBarrier.Transition.pResource = inputA.GetBuffer();
        copyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        copyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        copyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        D3D12_RESOURCE_BARRIER uaBarrier = copyBarrier;
        uaBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        uaBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCommandList->ResourceBarrier(1, &copyBarrier);
        pCommandList->CopyResource(inputA.GetBuffer(), inputA.GetInputBuffer());
        pCommandList->ResourceBarrier(1, &uaBarrier);

        copyBarrier.Transition.pResource = inputB.GetBuffer();
        uaBarrier.Transition.pResource = inputB.GetBuffer();

        pCommandList->ResourceBarrier(1, &copyBarrier);
        pCommandList->CopyResource(inputB.GetBuffer(), inputB.GetInputBuffer());
        pCommandList->ResourceBarrier(1, &uaBarrier);

        pCommandList->Dispatch(NUM_ELEMENTS/THREADS_IN_GROUP, 1, 1);

        copyBarrier.Transition.pResource = output.GetBuffer();
        copyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        uaBarrier.Transition.pResource = output.GetBuffer();
        uaBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;

        pCommandList->ResourceBarrier(1, &copyBarrier);
        pCommandList->CopyResource(output.GetOutputBuffer(), output.GetBuffer());
        pCommandList->ResourceBarrier(1, &uaBarrier);
#else

        // inputA::inputBuffer GENERIC_READ
        // inputA::outputBuffer COPY_DEST
        // inputA::buffer UNORDERED_ACCESS

        // output::inputBuffer GENERIC_READ
        // output::outputBuffer COPY_DEST
        // output::buffer UNORDERED_ACCESS

        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        barrier.Transition.pResource = inputA.GetBuffer();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

        // inputA::buffer UNORDERED_ACCESS -> COPY_DEST
        pCommandList->ResourceBarrier(1, &barrier);

        pCommandList->CopyResource(inputA.GetBuffer(), inputA.GetInputBuffer());

        barrier.Transition.pResource = inputB.GetBuffer();

        // inputA::buffer UNORDERED_ACCESS -> COPY_DEST
        pCommandList->ResourceBarrier(1, &barrier);

        pCommandList->CopyResource(inputA.GetBuffer(), inputA.GetInputBuffer());

        // inputA::buffer COPY_DEST -> UNORDERED_ACCESS

        barrier.Transition.pResource = inputA.GetBuffer();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

        // inputA::buffer UNORDERED_ACCESS -> COPY_SOURCE
        pCommandList->ResourceBarrier(1, &barrier);

        // inputA::buffer COPY_SOURCE
        // output::outputBuffer COPY_DEST
        pCommandList->CopyResource(output.GetOutputBuffer(), inputA.GetBuffer());

#endif

        pCommandList->Close();

        ID3D12CommandQueue * pCommandQueue = computeDevice.GetCommandQueue();

        ID3D12CommandList * ppCommandLists[] = { pCommandList };
        pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        hr = pCommandQueue->Signal(pFence, 1);
        if (FAILED(hr)) throw std::exception("Failed to signal");

        if (pFence->GetCompletedValue() < 1) {
            HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            hr = pFence->SetEventOnCompletion(1, hEvent);
            if (FAILED(hr)) throw std::exception("Failed to set event on completion");

            WaitForSingleObject(hEvent, INFINITE);
        }

        BufType result[NUM_ELEMENTS];
        output.GetOutput(sizeof(BufType), NUM_ELEMENTS, result);

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

