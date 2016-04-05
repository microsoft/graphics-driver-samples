#ifndef _RENDERING_TESTS_H_
#define _RENDERING_TESTS_H_

//
// Tests related to rendering
//
class RenderingTests {
    BEGIN_TEST_CLASS(RenderingTests)
        TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
    END_TEST_CLASS()

    TEST_CLASS_SETUP(ClassSetup)

    BEGIN_TEST_METHOD(TestRenderTriangle)
        TEST_METHOD_PROPERTY(
            L"Description",
            L"Render a triangle.")
    END_TEST_METHOD()

    struct RENDER_DESC {
        UINT Width = 512;
        UINT Height = 512;
        DXGI_FORMAT RenderTargetFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        float ClearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        PCSTR VertexShader;
        PCSTR PixelShader;
        _Field_size_(InputDescriptorCount) const D3D11_INPUT_ELEMENT_DESC* InputLayout;
        UINT InputDescriptorCount;
        _Field_size_bytes_(VertexBufferSize) const void* VertexBuffer;
        UINT VertexBufferSize;
        UINT VertexBufferStride;
        D3D11_PRIMITIVE_TOPOLOGY Topology;
    };

    Microsoft::WRL::ComPtr<ID3D11Texture2D1> RenderToTexture (
        const RENDER_DESC& Desc
        );

    Microsoft::WRL::ComPtr<ID3D11Device3> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_context;
};

#endif // _RENDERING_TESTS_H_
