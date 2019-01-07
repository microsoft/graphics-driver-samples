// coscon.cpp : Defines the entry point for the console application.
//

#include <initguid.h>
#include <windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <dxcore.h>
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

    D3DDevice(std::string & driverString)
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

    D3DPointer<IDXCoreAdapter>              m_pAdapter;
    D3DPointer<ID3D12Device>                m_pDevice;
    D3DPointer<ID3D12CommandAllocator>      m_pCommandAllocator;
    D3DPointer<ID3D12CommandQueue>          m_pCommandQueue;
    D3DPointer<ID3D12GraphicsCommandList>   m_pCommandList;
    D3DPointer<ID3D12RootSignature>         m_pRootSignature;

    bool CreateDevice()
    {
        ID3D12Device * pDevice;

        HRESULT hr = D3D12CreateDevice(m_pAdapter, D3D_FEATURE_LEVEL_1_0_CORE, IID_PPV_ARGS(&pDevice));

        bool success = (hr == S_OK);

        if (success)
            m_pDevice = pDevice;

        if (success) {
            ID3D12CommandQueue * pCommandQueue;
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc;

            commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            commandQueueDesc.Priority = 0;
            commandQueueDesc.NodeMask = 0;

            hr = pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&pCommandQueue));

            success = (hr == S_OK);

            if (success)
                m_pCommandQueue = pCommandQueue;
        }

        if (success) {
            ID3D12CommandAllocator * pCommandAllocator;

            hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator));

            success = (hr == S_OK);

            if (success)
                m_pCommandAllocator = pCommandAllocator;
        }

        if (success) {
            ID3D12GraphicsCommandList * pCommandList;

            hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_pCommandAllocator, NULL, IID_PPV_ARGS(&pCommandList));

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

    bool FindAdapter(std::string & driverString)
    {
        IDXCoreAdapterFactory * factory = NULL;
        HRESULT hr = DXCoreCreateAdapterFactory(__uuidof(IDXCoreAdapterFactory), ((void **)&factory));
        if (FAILED(hr)) throw std::exception("Unable to create IDXCoreAdapterFactory");

        const GUID guids[] = { DXCORE_ADAPTER_ATTRIBUTE_D3D_CORE_COMPUTE};
        IDXCoreAdapterList * adapterList = NULL;
        hr = factory->GetAdapterList(guids, 1, &adapterList);
        if (FAILED(hr)) throw std::exception("Unable to create IDXCorAdapterList");

        bool found = false;
        UINT adapterIndex = 0;
        bool done = false;

        while (!done && !found) {

            IDXCoreAdapter * adapter = NULL;

            hr = adapterList->GetItem(adapterIndex, &adapter);

            if (hr == S_OK)
            {
                size_t descriptionSize;
                hr = adapter->QueryPropertySize(DXCoreProperty::DriverDescription, &descriptionSize);
                if (FAILED(hr)) throw std::exception("Unable get get adapter description size");

                char * description = new char[descriptionSize];
                if (description == nullptr) throw std::exception("Unable to allocate description storage");

                hr = adapter->QueryProperty(DXCoreProperty::DriverDescription, descriptionSize, description);
                if (FAILED(hr)) throw std::exception("Unable get get adapter description");

                found = (strcmp(driverString.c_str(), description) == 0);

                if (found)
                {
                    m_pAdapter = adapter;
                }
                else
                {
                    adapter->Release();
                }

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

    static void GetAdapterList(std::list<std::string> & list)
    {
        IDXCoreAdapterFactory * factory = NULL;
        HRESULT hr = DXCoreCreateAdapterFactory(__uuidof(IDXCoreAdapterFactory), ((void **)&factory));
        if (FAILED(hr)) throw std::exception("Unable to create IDXCoreAdapterFactory");

        const GUID guids[] = { DXCORE_ADAPTER_ATTRIBUTE_D3D_CORE_COMPUTE};
        IDXCoreAdapterList * adapterList = NULL;
        hr = factory->GetAdapterList(guids, 1, &adapterList);
        if (FAILED(hr)) throw std::exception("Unable to create IDXCorAdapterList");

        UINT adapterIndex = 0;
        bool done = false;

        list.clear();

        while (!done) {

            IDXCoreAdapter * adapter = NULL;

            hr = adapterList->GetItem(adapterIndex, &adapter);

            if (hr == S_OK)
            {
                size_t descriptionSize;
                hr = adapter->QueryPropertySize(DXCoreProperty::DriverDescription, &descriptionSize);
                if (FAILED(hr)) throw std::exception("Unable get get adapter description size");

                char * description = new char[descriptionSize];
                if (description == nullptr) throw std::exception("Unable to allocate description storage");

                hr = adapter->QueryProperty(DXCoreProperty::DriverDescription, descriptionSize, description);
                if (FAILED(hr)) throw std::exception("Unable get get adapter description");

                list.push_back(description);
                delete [descriptionSize] description;

                adapter->Release();

                adapterIndex++;
            }
            else
            {
                done = true;
            }
        }

        adapterList->Release();
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
		HRESULT hr;
        ID3DBlob* pCSBlob = nullptr;

		hr = D3DReadFileToBlob(L"ComputeShader.cso", &pCSBlob);
		assert(hr == S_OK);
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

        hr = inDevice.GetDevice()->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pPipelineState));

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

class D3DInputBuffer;
class D3DOutputBuffer;

class D3DBuffer {

public:
    D3DBuffer(D3DDevice & inDevice, UINT uSize, D3D12_HEAP_TYPE inHeapType = D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATES inInitialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_FLAGS inResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    {
        D3D12_HEAP_PROPERTIES heapProperties;

        heapProperties.Type = inHeapType;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 0;
        heapProperties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resourceDesc;

        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = uSize;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = inResourceFlags;

        ID3D12Resource * pBuffer;

        OutputDebugStringA("creating structured buffer\n");

        HRESULT hr = inDevice.GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            inInitialState, NULL, IID_PPV_ARGS(&pBuffer));

        if (FAILED(hr)) throw std::exception("Unable to create structed buffer");

        m_pBuffer = pBuffer;
        m_initialState = inInitialState;
        m_currentState = inInitialState;
    }

    void CopyFrom(ID3D12GraphicsCommandList * inCommandList, D3DInputBuffer & inBuffer);
    void CopyTo(ID3D12GraphicsCommandList * inCommandList, D3DOutputBuffer & outBuffer);

    void StartStateTracking()
    {
        m_currentState = m_initialState;
    }

    void SetState(ID3D12GraphicsCommandList * pCommandList, D3D12_RESOURCE_STATES inNewState)
    {
        if (inNewState != m_currentState)
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barrier.Transition.pResource = m_pBuffer;

            barrier.Transition.StateBefore = m_currentState;
            barrier.Transition.StateAfter = inNewState;

            pCommandList->ResourceBarrier(1, &barrier);

            m_currentState = inNewState;
        }
    }

    ID3D12Resource * Get() { return m_pBuffer; }

protected:

    D3DPointer<ID3D12Resource>  m_pBuffer;
    D3D12_RESOURCE_STATES       m_initialState;
    D3D12_RESOURCE_STATES       m_currentState;
};


class D3DInputBuffer : public D3DBuffer
{
public:
    D3DInputBuffer(D3DDevice & inDevice, UINT inSize, void * inData) :
        D3DBuffer(inDevice, inSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE)
    {
        void * pDst;
        D3D12_RANGE range = { 0, 0 };
        m_pBuffer->Map(0, &range, &pDst);
        memcpy(pDst, inData, inSize);
        m_pBuffer->Unmap(0, NULL);
    }
};

class D3DOutputBuffer : public D3DBuffer
{
public:
    D3DOutputBuffer(D3DDevice & inDevice, UINT inSize, void * inData = NULL) :
        D3DBuffer(inDevice, inSize,  D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_NONE)
    {
        // do nothing
    }

    void Read( UINT inSize, void * pDst)
    {
        D3D12_RANGE range = { 0, inSize };
        void * pSrc;
        m_pBuffer->Map(0, &range, &pSrc);
        memcpy(pDst, pSrc, inSize);

        range.End = 0;
        m_pBuffer->Unmap(0, &range);
    }

};

void D3DBuffer::CopyFrom(ID3D12GraphicsCommandList * inCommandList, D3DInputBuffer & inBuffer)
{
    SetState(inCommandList, D3D12_RESOURCE_STATE_COPY_DEST);
    inCommandList->CopyResource(Get(), inBuffer.Get());
    SetState(inCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void D3DBuffer::CopyTo(ID3D12GraphicsCommandList * inCommandList, D3DOutputBuffer & outBuffer)
{
    SetState(inCommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
    inCommandList->CopyResource(outBuffer.Get(), Get());
    SetState(inCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}



struct BufType
{
    int i;
    float f;
};

#define NUM_ELEMENTS     1024
#define THREADS_IN_GROUP 4      // Expressed in the shader

int main()
{
    int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;

    _CrtSetDbgFlag(dbgFlags);

    HRESULT hr = D3D12EnableExperimentalFeatures(1, &D3D12ComputeOnlyDevices, NULL, 0);
    if (hr != S_OK) {
        printf("unable to turn on experimental feature 'D3D12ComputeOnlyDevices'\n");
        return 0;
    }

    try {

        std::string cosDriverString = "Compute Only Sample Driver";
        std::string brdDriverString = "Microsoft Basic Render Driver";
        std::string amdDriverString = "Radeon (TM) RX 480 Graphics";
        std::string intelDriverString = "Intel(R) HD Graphics 620";

        std::list<std::string> adapterList;

        D3DAdapter::GetAdapterList(adapterList);

        printf("Found adapters:\n");
        for (auto description : adapterList)
            printf("  %s\n", description.c_str());

        auto findIter = std::find(adapterList.begin(), adapterList.end(), brdDriverString);

        if (findIter == adapterList.end()) {
            printf("%s was not found\n", cosDriverString.c_str());
            return 0;
        }

        std::string driverString = cosDriverString;

        findIter = std::find(adapterList.begin(), adapterList.end(), driverString);

        if (findIter == adapterList.end())
            driverString = brdDriverString;

        printf("Creating device on %s ... ", driverString.c_str());
        D3DDevice computeDevice(driverString);
        printf("done.\n");

        printf("Creating compute shader ... ");
        D3DComputeShader computeShader(computeDevice);
        printf("done.\n");

        printf("Creating buffers ... ");

        BufType initialData[NUM_ELEMENTS];

        for (int i = 0; i < NUM_ELEMENTS; i++) {
            initialData[i].i = i;
            initialData[i].f = (float)i;
        }

        UINT uSize = sizeof(BufType) * NUM_ELEMENTS;

        D3DInputBuffer input(computeDevice, uSize, initialData);

        D3DBuffer a(computeDevice, uSize);
        D3DBuffer b(computeDevice, uSize);
        D3DBuffer result(computeDevice, uSize);

        D3DOutputBuffer output(computeDevice, uSize);
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
		
		printf("a UAV handld = %08x allocation offset = %08x", (UINT32) (a.Get()->GetGPUVirtualAddress() >> 32), (UINT32) a.Get()->GetGPUVirtualAddress());
		printf("b UAV handld = %08x allocation offset = %08x", (UINT32)(b.Get()->GetGPUVirtualAddress() >> 32), (UINT32) b.Get()->GetGPUVirtualAddress());
		printf("result UAV handld = %08x allocation offset = %08x", (UINT32)(result.Get()->GetGPUVirtualAddress() >> 32), (UINT32) result.Get()->GetGPUVirtualAddress());

        pCommandList->SetComputeRootUnorderedAccessView(0, a.Get()->GetGPUVirtualAddress());
        pCommandList->SetComputeRootUnorderedAccessView(1, b.Get()->GetGPUVirtualAddress());
        pCommandList->SetComputeRootUnorderedAccessView(2, result.Get()->GetGPUVirtualAddress());

        a.StartStateTracking();
        b.StartStateTracking();

        a.CopyFrom(pCommandList, input);
        b.CopyFrom(pCommandList, input);

        pCommandList->Dispatch(NUM_ELEMENTS/THREADS_IN_GROUP, 1, 1);

        result.CopyTo(pCommandList, output);

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

        printf("done\n");

        BufType results[NUM_ELEMENTS];
        output.Read(uSize, results);

        printf("checking results ... ");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            if (results[i].i != i * 2) {
                printf("Bad integer result %d at index %d\n", results[i].i, i);
                break;
            }

            if (results[i].f != (float) (i * 2)) {
                printf("Bad float result %f at index %d\n", results[i].f, i);
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

