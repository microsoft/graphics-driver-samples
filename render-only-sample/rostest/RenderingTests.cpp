#include "precomp.h"

#include "util.h"
#include "RenderingTests.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace WEX::TestExecution;
using namespace DirectX;

bool RenderingTests::ClassSetup ()
{
    CreateDevice(&m_device, &m_context);
    return true;
}

ComPtr<ID3D11Texture2D1> RenderingTests::RenderToTexture (const RENDER_DESC& Desc)
{
    HRESULT hr;
    
    UINT width;
    if (SUCCEEDED(RuntimeParameters::TryGetValue(L"Width", width))) {
        LogComment(
            L"Overriding default width (%d) with runtime parameter: %d",
            Desc.Width,
            width);
    } else {
        width = Desc.Width;
    }
    
    UINT height;
    if (SUCCEEDED(RuntimeParameters::TryGetValue(L"Height", height))) {
        LogComment(
            L"Overriding default height (%d) with runtime parameter: %d",
            Desc.Height,
            height);
    } else {
        height = Desc.Height;
    }

    // Input data:
    //   device - reasonable default
    //   render target format - reasonable default
    //   render target width/height - reasonable default
    //   shaders - required
    //   input layout descriptor - required
    //   vertex buffer - required
    //   primitive topology - required

    // Set up per-scene stuff
        // 5. Output Merger
    //   Per scene:
    //    create a 2D texture (BGRA) to use as the render target (CreateTexture2D1) D3D11_BIND_RENDER_TARGET
    //    create a render target view of the texture (CreateRenderTargetView)
    //    create a 2D texture to use as depth stencil (CreateTexture2D1) DXGI_FORMAT_D24_UNORM_S8_UINT D3D11_BIND_DEPTH_STENCIL
    //    create depth stencil view of the texture (CreateDepthStencilView)
    //
    //  Per object?
    //    create depth stencil state (CreateDepthStencilState) (optional)
    //    set depth stencil state (OMSetDepthStencilState) (optional)
    ComPtr<ID3D11Texture2D1> d3dRenderTargetTexture;
    ComPtr<ID3D11RenderTargetView1> d3dRenderTargetView;
    {
        D3D11_TEXTURE2D_DESC1 desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = Desc.RenderTargetFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        VERIFY_SUCCEEDED(
            m_device->CreateTexture2D1(
                &desc,
                nullptr,
                &d3dRenderTargetTexture),
            L"Creating texture for render target");

        VERIFY_SUCCEEDED(
            m_device->CreateRenderTargetView1(
                d3dRenderTargetTexture.Get(),
                nullptr,    // desc
                &d3dRenderTargetView),
            L"Creating render target view");
    }

    ComPtr<ID3D11Texture2D1> d3dDepthTexture;
    ComPtr<ID3D11DepthStencilView> d3dDepthStencilView;
    {
        D3D11_TEXTURE2D_DESC1 desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        VERIFY_SUCCEEDED(
            m_device->CreateTexture2D1(
                &desc,
                nullptr,
                &d3dDepthTexture),
            L"Creating texture for depth stencil");

        VERIFY_SUCCEEDED(
            m_device->CreateDepthStencilView(
                d3dDepthTexture.Get(),
                nullptr,    // desc
                &d3dDepthStencilView),
            L"Creating depth stencil view");
    }

    // Set viewport
    {
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = D3D11_MIN_DEPTH;
        viewport.MaxDepth = D3D11_MAX_DEPTH;

        LogComment(L"Setting viewport");
        m_context->RSSetViewports(1, &viewport);
    }

    //     Set render target and depth stencil (OMSetRenderTargets)
    LogComment(L"Setting render target");
    ID3D11RenderTargetView *const targets[1] = { d3dRenderTargetView.Get() };
    m_context->OMSetRenderTargets(
        ARRAYSIZE(targets),
        targets,
        d3dDepthStencilView.Get());

    //     Clear render target view (ClearRenderTargetView)
    LogComment(L"Clearing render target view");
    const float clearColor[] = {1.0f, 1.0f, 1.0f, 1.0f};     // white
    m_context->ClearRenderTargetView(d3dRenderTargetView.Get(), Desc.ClearColor);

    //     Clear deptch stencil view (ClearDepthStencilView)
    LogComment(L"Clearing depth stencil view");
    m_context->ClearDepthStencilView(
        d3dDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,       // Depth
        0);         // Stencil

    // Set up the per-object stuff

    // 2. Vertex Shader
    //   Per object:
    //     create vertex shader (CreateVertexShader)
    //     create constant buffer (CreateBuffer), D3D11_BIND_CONSTANT_BUFFER
    ComPtr<ID3D11VertexShader> d3dVertexShader;
    ComPtr<ID3DBlob> d3dVertexShaderBlob;
    {
        // GetBufferPointer, GetBufferSize
        LogComment(L"Compiling vertex shader:\n%S", Desc.VertexShader);
        ComPtr<ID3DBlob> d3dErrorMsgBlob;
        hr = D3DCompile(
                Desc.VertexShader,
                strlen(Desc.VertexShader) + 1,
                nullptr,        // pSourceName
                nullptr,        // pDefines
                nullptr,        // pInclude
                "main",
                "vs_4_0_level_9_1",
                D3DCOMPILE_WARNINGS_ARE_ERRORS,
                0,              // compile effect constants (not used)
                &d3dVertexShaderBlob,
                &d3dErrorMsgBlob);

        if (FAILED(hr)) {
            LogError(
                L"Failed to compile shader:\n%S",
                d3dErrorMsgBlob->GetBufferPointer());
            throw MyException::Make(hr, L"Failed to compile shader");
        }

        VERIFY_SUCCEEDED(
            m_device->CreateVertexShader(
                d3dVertexShaderBlob->GetBufferPointer(),
                d3dVertexShaderBlob->GetBufferSize(),
                nullptr,
                &d3dVertexShader),
            L"Verifying that vertex shader was created successfully");
    }

    // struct MY_VERTEX {
        // XMFLOAT3 Position;
        // XMFLOAT3 Color;
    // };

    // // describe a triangle
    // const MY_VERTEX vertexData[] = {
        // { XMFLOAT3(0.0, 0.5f, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        // { XMFLOAT3(0.45f, -0.5f, 0), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        // { XMFLOAT3(-0.45f, -0.5f, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) }
    // };

    // 1. Input Assembler
    //   Per object:
    //     create input layout (CreateInputLayout)
    //     create vertex buffer (CreateBuffer)
    //     create index buffer ()
    ComPtr<ID3D11InputLayout> d3dInputLayout;
    ComPtr<ID3D11Buffer> d3dVertexBuffer;
    {
        // const D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
            // { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // };

        VERIFY_SUCCEEDED(
            m_device->CreateInputLayout(
                Desc.InputLayout,
                Desc.InputDescriptorCount,
                d3dVertexShaderBlob->GetBufferPointer(),
                d3dVertexShaderBlob->GetBufferSize(),
                &d3dInputLayout),
            L"Verifying that input layout was created successfully");

        CD3D11_BUFFER_DESC desc(Desc.VertexBufferSize, D3D11_BIND_VERTEX_BUFFER);
        D3D11_SUBRESOURCE_DATA subResData = {
            Desc.VertexBuffer,    // pSysMem
            0,                    // SysMemPitch
            0                     // SysMemSlicePitch
        };

        // create vertex buffer for a triangle
        VERIFY_SUCCEEDED(
            m_device->CreateBuffer(
                &desc,
                &subResData,
                &d3dVertexBuffer),
            L"Creating vertex buffer");
    }

    // 3. Rasterizer
    //   create rasterizer state (CreateRasterizerState) (optional)
    //   set rasterizer state (RSSetState) (optional)
    // ComPtr<ID3D11RasterizerState> d3dRasterState;
    // {
        // D3D11_RASTERIZER_DESC desc = {};
        // desc.FillMode = D3D11_FILL_SOLID;
        // desc.CullMode = D3D11_CULL_NONE;
        // desc.FrontCounterClockwise = FALSE;
        // desc.DepthBias = 0;
        // desc.DepthBiasClamp = 0.0f;
        // desc.SlopeScaledDepthBias = 0.0f;
        // desc.DepthClipEnable = FALSE;
        // desc.ScissorEnable = FALSE;
        // desc.MultisampleEnable = FALSE;
        // desc.AntialiasedLineEnable = FALSE;

        // VERIFY_SUCCEEDED(
            // m_device->CreateRasterizerState(&desc, &d3dRasterState),
            // L"Creating rasterizier state");
    // }

    // 4. Pixel Shader
    //   Per object:
    //     create pixel shader (CreatePixelShader)
    //     create shader resource view of texture (CreateShaderResourceView) (only if shader needs them)
    ComPtr<ID3D11PixelShader> d3dPixelShader;
    {
        // GetBufferPointer, GetBufferSize
        LogComment(L"Compiling pixel shader:\n%S", Desc.PixelShader);
        ComPtr<ID3DBlob> d3dPixelShaderBlob;
        ComPtr<ID3DBlob> d3dErrorMsgBlob;
        hr = D3DCompile(
                Desc.PixelShader,
                strlen(Desc.PixelShader) + 1,
                nullptr,        // pSourceName
                nullptr,        // pDefines
                nullptr,        // pInclude
                "main",
                "ps_4_0_level_9_1",
                D3DCOMPILE_WARNINGS_ARE_ERRORS,
                0,              // compile effect constants (not used)
                &d3dPixelShaderBlob,
                &d3dErrorMsgBlob);

        if (FAILED(hr)) {
            LogError(
                L"Failed to compile shader:\n%S",
                d3dErrorMsgBlob->GetBufferPointer());
            throw MyException::Make(hr, L"Failed to compile shader");
        }

        VERIFY_SUCCEEDED(
            m_device->CreatePixelShader(
                d3dPixelShaderBlob->GetBufferPointer(),
                d3dPixelShaderBlob->GetBufferSize(),
                nullptr,
                &d3dPixelShader),
            L"Verifying that pixel shader was created successfully");
    }

    // Render
    //   Set up scene
    //     Set viewport (RSSetViewports)
    //     Set render target and depth stencil (OMSetRenderTargets)
    //     Clear render target view (ClearRenderTargetView)
    //     Clear depth stencil view (ClearDepthStencilView)



    // m_context->RSSetState(d3dRasterState.Get());



    //   Draw the object
    //     set input layout (IASetInputLayout)
    LogComment(L"Setting input layout");
    m_context->IASetInputLayout(d3dInputLayout.Get());

    //     set vertex buffer (IASetVertexBuffers)
    {
        LogComment(L"Setting vertex buffer");

        ID3D11Buffer* const vertexBuffers[] = { d3dVertexBuffer.Get() };
        const UINT strides[] = { Desc.VertexBufferStride };
        const UINT offsets[] = { 0 };
        m_context->IASetVertexBuffers(
            0,                          // StartSlot
            ARRAYSIZE(vertexBuffers),   // NumBuffers
            vertexBuffers,
            strides,
            offsets);
    }

    //     set index buffers (IASetIndexBuffer) (only need if using DrawIndexed)
    //     set primitive topology (IASetPrimitiveTopology)
    LogComment(L"Setting primitive topology");
    m_context->IASetPrimitiveTopology(Desc.Topology);

    //
    //     set vertex shader (VSSetShader)
    LogComment(L"Setting the vertex shader");
    m_context->VSSetShader(
        d3dVertexShader.Get(),
        nullptr,    // ppClasInstances
        0);         // NumClassInstances

    //     Load the transformation matrix into the constant buffer (UpdateSubresource1)
    //     set constant buffer (VSSetConstantBuffers1)
    //
    //     set the pixel shader (PSSetShader)
    LogComment(L"Setting pixel shader");
    m_context->PSSetShader(
        d3dPixelShader.Get(),
        nullptr,        // ppClasInstances
        0);             // NumClassInstances

    //     set shader resources (PSSetShaderResources)
    //
    //     Draw or DrawIndexed
    LogComment(L"Performing draw");
    //static_assert(ARRAYSIZE(vertexData) == 3, "Verifying size of vertex Data");
    m_context->Draw(
        Desc.VertexBufferSize / Desc.VertexBufferStride,
        0 /*StartVertexLocation*/);

    m_context->Flush();

    return d3dRenderTargetTexture;
}

//
// Feed in a single triangle
//
void RenderingTests::TestRenderTriangle ()
{
    struct MY_VERTEX {
        XMFLOAT3 Position;
        XMFLOAT3 Color;
    };

    const D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // describe a triangle
    const MY_VERTEX vertexData[] = {
        { XMFLOAT3(0.0, 0.5f, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.45f, -0.5f, 0), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-0.45f, -0.5f, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) }
    };

    const char vertexShader[] = R"SHADER(
        struct VertexShaderInput {
            float3 pos : POSITION;
            // float3 color : COLOR0;
        };

        struct PixelShaderInput {
            float4 pos : SV_POSITION;
            // float3 color : COLOR0;
        };

        PixelShaderInput main (VertexShaderInput input)
        {
            PixelShaderInput output;
            output.pos = float4(input.pos, 1.0f);
            // output.color = input.color;
            return output;
        }
    )SHADER";

    const char pixelShader[] = R"SHADER(

        struct PixelShaderInput {
            float4 pos : SV_POSITION;
            // float3 color : COLOR0;
        };

        float4 main (PixelShaderInput input) : SV_TARGET
        {
            return float4(0.0f, 0.0f, 1.0f, 1.0f); // blue
        }

    )SHADER";

    RENDER_DESC desc;
    desc.VertexShader = vertexShader;
    desc.PixelShader = pixelShader;
    desc.InputLayout = inputDesc;
    desc.InputDescriptorCount = ARRAYSIZE(inputDesc);
    desc.VertexBuffer = vertexData;
    desc.VertexBufferSize = sizeof(vertexData);
    desc.VertexBufferStride = sizeof(MY_VERTEX);
    desc.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    auto texture = RenderToTexture(desc);

    // Save the render target to a bitmap
    LogComment(L"Saving texture to bitmap");
    SaveTextureToBmp(L"triangle.bmp", texture.Get());
}
