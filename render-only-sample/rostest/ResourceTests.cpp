#include "precomp.h"

#include "util.h"
#include "ResourceTests.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace WEX::TestExecution;

bool ResourceTests::ClassSetup ()
{
    CreateDevice(&m_device, &m_context);
    return true;
}

// TODO: this doesn't seem like a very good test ...
void ResourceTests::TestShaderConstantBuffer ()
{
    UINT32 data[1024];
    std::iota(std::begin(data), std::end(data), 0);

    D3D11_BUFFER_DESC desc = {};
    {
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.ByteWidth = sizeof(data);
        desc.CPUAccessFlags = 0; // no CPU access
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;   // read and write access by GPU
    }

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    {
        subresourceData.pSysMem = data;
        subresourceData.SysMemPitch = 0;
        subresourceData.SysMemSlicePitch = 0;
    }

    ComPtr<ID3D11Buffer> bufferA;
    VERIFY_SUCCEEDED(
        m_device->CreateBuffer(&desc, &subresourceData, &bufferA),
        L"Creating constant buffer A");

    ComPtr<ID3D11Buffer> bufferB;
    VERIFY_SUCCEEDED(
        m_device->CreateBuffer(&desc, &subresourceData, &bufferB),
        L"Creating constant buffer B");

    LogComment(L"Copying buffer");
    m_context->CopyResource(bufferB.Get(), bufferA.Get());

    LogComment(L"Flushing device context");
    m_context->Flush();
}
