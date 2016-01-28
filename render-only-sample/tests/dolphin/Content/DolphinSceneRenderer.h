#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace dolphin
{
    using namespace DirectX;

    struct DOLPHIN_VS_CONSTANT_BUFFER
    {
        XMVECTOR vZero;
        XMVECTOR vConstants;
        XMVECTOR vWeight;

        XMMATRIX matTranspose;
        XMMATRIX matCameraTranspose;
        XMMATRIX matViewTranspose;
        XMMATRIX matProjTranspose;

        XMVECTOR vLight;
        XMVECTOR vLightDolphinSpace;
        XMVECTOR vDiffuse;
        XMVECTOR vAmbient;
        XMVECTOR vFog;
        XMVECTOR vCaustics;
    };

    struct DOLPHIN_PS_CONSTANT_BUFFER
    {
        FLOAT fAmbient[4];
        FLOAT fFogColor[4];
    };

    // This sample renderer instantiates a basic rendering pipeline.
    class DolphinSceneRenderer
    {
    public:
        DolphinSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void Render();
        void StartTracking();
        void TrackingUpdate(float positionX);
        void StopTracking();
        bool IsTracking() { return m_tracking; }


    private:
        void Rotate(float radians);

    private:
        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        // Direct3D resources for cube geometry.
        Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

        Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_dolphinInputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_dolphinVertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_dolphinPixelShader;

        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_dolphinPsConstantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_dolphinVsConstantBuffer;

        // System resources for cube geometry.
        ModelViewProjectionConstantBuffer	m_constantBufferData;
        uint32	m_indexCount;

        // Variables used with the rendering loop.
        bool	m_loadingComplete;
        float	m_degreesPerSecond;
        bool	m_tracking;
    };
}

