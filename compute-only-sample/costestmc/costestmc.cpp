// costestmc.cpp : Defines the entry point for the console application.
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

#include "CosMetaCommand.h"

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

    ID3D12Device5 * GetDevice() { return m_pDevice; }
    ID3D12CommandAllocator * GetCommandAllocator() { return m_pCommandAllocator; }
    ID3D12CommandQueue * GetCommandQueue() { return m_pCommandQueue; }
    ID3D12GraphicsCommandList4 * GetCommandList() { return m_pCommandList; }

private:

    D3DPointer<IDXGIAdapter2>               m_pAdapter;
    D3DPointer<ID3D12Device5>               m_pDevice;
    D3DPointer<ID3D12CommandAllocator>      m_pCommandAllocator;
    D3DPointer<ID3D12CommandQueue>          m_pCommandQueue;
    D3DPointer<ID3D12GraphicsCommandList4>  m_pCommandList;

    bool CreateDevice()
    {
        D3D12EnableExperimentalFeatures(1, &D3D12ComputeOnlyDevices, NULL, 0);

        ID3D12Device5 * pDevice;

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
            ID3D12GraphicsCommandList4 * pCommandList;

            hr = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_pCommandAllocator, NULL, IID_PPV_ARGS(&pCommandList));

            success = (hr == S_OK);

            if (success)
                m_pCommandList = pCommandList;
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
    std::wstring cosDriverString = L"Compute Only Sample Driver";
    std::wstring brdDriverString = L"Microsoft Basic Render Driver";
    std::wstring amdDriverString = L"Radeon (TM) RX 480 Graphics";
    std::wstring intelDriverString = L"Intel(R) HD Graphics 620";

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

        printf("Creating buffers ... ");

        BufType initialData[NUM_ELEMENTS];

        for (int i = 0; i < NUM_ELEMENTS; i++) {
            initialData[i].i = i;
            initialData[i].f = (float)i;
        }

        UINT uSize = sizeof(BufType) * NUM_ELEMENTS;

        D3DInputBuffer input(computeDevice, uSize, initialData);

        D3DBuffer a(computeDevice, uSize);
        D3DBuffer result(computeDevice, uSize);

        D3DOutputBuffer output(computeDevice, uSize);
        printf("done.\n");

        printf("Running shader ... ");

        ID3D12Device5 * pDevice = computeDevice.GetDevice();
        ID3D12GraphicsCommandList4 * pCommandList = computeDevice.GetCommandList();

        ID3D12Fence * pFence;
        HRESULT hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
        if (FAILED(hr)) throw std::exception("Failed to create fence");

        UINT metaCommandCount = 0;
        D3D12_META_COMMAND_DESC metaCommandDescriptions[16] = { 0 };

        metaCommandCount = _countof(metaCommandDescriptions);

        //
        // For metaCommandCount, 
        //     in  : number of elmements in metaCommandDescriptions
        //     out : number of metaCommandDescriptions filled in by driver
        //

        hr = pDevice->EnumerateMetaCommands(&metaCommandCount, metaCommandDescriptions);
        if (FAILED(hr))
        {
            throw std::exception("EnumerateMetaCommands failed");
        }

        //
        // Check GUID_IDENTITY meta command is supported by the driver
        //

        UINT i;

        for (i = 0; i < metaCommandCount; i++)
        {
            if (IsEqualGUID(GUID_IDENTITY, metaCommandDescriptions[i].Id))
            {
                break;
            }
        }

        if (i == metaCommandCount)
        {
            throw std::exception("Driver doesn't support meta command copy resource");
        }

        //
        // Enumerate the meta command parameters
        //
        // Parameters are passed to meta command creation/initialization/execution calls in structures
        // totalStructureSizeInBytes is the byte size of meta command parameter structure
        //
        // This is mostly used to ensure API driver consistency about a meta command
        //

        UINT totalStructureSizeInBytes, parameterCount;
        D3D12_META_COMMAND_PARAMETER_DESC metaCommandParamterDescriptions[2];

        totalStructureSizeInBytes = 0;
        parameterCount = _countof(metaCommandParamterDescriptions);

        hr = pDevice->EnumerateMetaCommandParameters(
                        GUID_IDENTITY,
                        D3D12_META_COMMAND_PARAMETER_STAGE_CREATION,
                        &totalStructureSizeInBytes,
                        &parameterCount,
                        metaCommandParamterDescriptions);
        if (FAILED(hr))
        {
            throw std::exception("EnumerateMetaCommandParameters() for creation stage failed");
        }

        totalStructureSizeInBytes = 0;
        parameterCount = _countof(metaCommandParamterDescriptions);

        hr = pDevice->EnumerateMetaCommandParameters(
                        GUID_IDENTITY,
                        D3D12_META_COMMAND_PARAMETER_STAGE_INITIALIZATION,
                        &totalStructureSizeInBytes,
                        &parameterCount,
                        metaCommandParamterDescriptions);
        if (FAILED(hr))
        {
            throw std::exception("EnumerateMetaCommandParameters() for initialization stage failed");
        }

        totalStructureSizeInBytes = 0;
        parameterCount = _countof(metaCommandParamterDescriptions);

        hr = pDevice->EnumerateMetaCommandParameters(
                        GUID_IDENTITY,
                        D3D12_META_COMMAND_PARAMETER_STAGE_EXECUTION,
                        &totalStructureSizeInBytes,
                        &parameterCount,
                        metaCommandParamterDescriptions);
        if (FAILED(hr))
        {
            throw std::exception("EnumerateMetaCommandParameters() for execution stage failed");
        }

        //
        // Create Identity meta command
        //

        IdentityMetaCommandCreationParameters identityMetaCommandCreationParameters = { uSize };
        ID3D12MetaCommand * pMetaCommand;

        hr = pDevice->CreateMetaCommand(
                        GUID_IDENTITY,
                        0,
                        &identityMetaCommandCreationParameters,
                        sizeof(identityMetaCommandCreationParameters),
                        IID_PPV_ARGS(&pMetaCommand));
        if (FAILED(hr))
        {
            throw std::exception("CreateMetaCommand() failed");
        }

        a.StartStateTracking();

        a.CopyFrom(pCommandList, input);

        //
        // Initialize the meta command
        //

        pCommandList->InitializeMetaCommand(pMetaCommand, nullptr, 0);

        //
        // Transit the resource to UVA
        //

        a.SetState(pCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        result.SetState(pCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        //
        // Execute the meta command
        //

        IdentityMetaCommandExecutionParameters identityMetaCommandExecutionParameters = { 0 };

        identityMetaCommandExecutionParameters.Input = a.Get()->GetGPUVirtualAddress();
        identityMetaCommandExecutionParameters.Output = result.Get()->GetGPUVirtualAddress();

        pCommandList->ExecuteMetaCommand(
                        pMetaCommand,
                        &identityMetaCommandExecutionParameters,
                        sizeof(identityMetaCommandExecutionParameters));

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
            if (results[i].i != i) {
                printf("Bad integer result %d at index %d\n", results[i].i, i);
                break;
            }

            if (results[i].f != (float) (i)) {
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

