///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Resource implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "precomp.h"

#include "RosUmdLogging.h"
#include "RosUmdShader.tmh"

#include "RosUmdDevice.h"
#include "RosUmdShader.h"

void
RosUmdShader::Standup(
    const UINT * pCode, D3D10DDI_HRTSHADER hRTShader)
{
    UINT codeSize = pCode[1];

    m_pCode = new UINT[codeSize];
    
    memcpy(m_pCode, pCode, codeSize * sizeof(UINT));

    m_hRTShader = hRTShader;
}

void
RosUmdShader::Teardown()
{
    delete m_pCompiler;
    delete[] m_pCode;

    m_hwShaderCode.Teardown();
}

void
RosUmdShader::Update()
{

}

#if VC4

VC4_UNIFORM_FORMAT *
RosUmdShader::GetShaderUniformFormat(
    UINT Type, UINT *pUniformFormatEntries)
{
    return m_pCompiler->GetShaderUniformFormat(Type, pUniformFormatEntries);
}

#endif

void
RosUmdPipelineShader::Standup(
    const UINT * pCode, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES * pSignatures)
{
    RosUmdShader::Standup(pCode, hRTShader);

    if (pSignatures)
    {
        m_numInputSignatureEntries = pSignatures->NumInputSignatureEntries;
        m_pInputSignatureEntries = new D3D11_1DDIARG_SIGNATURE_ENTRY[m_numInputSignatureEntries];
        memcpy(m_pInputSignatureEntries, pSignatures->pInputSignature, m_numInputSignatureEntries * sizeof(D3D11_1DDIARG_SIGNATURE_ENTRY));

        m_numOutputSignatureEntries = pSignatures->NumOutputSignatureEntries;
        m_pOutputSignatureEntries = new D3D11_1DDIARG_SIGNATURE_ENTRY[m_numOutputSignatureEntries];
        memcpy(m_pOutputSignatureEntries, pSignatures->pOutputSignature, m_numOutputSignatureEntries * sizeof(D3D11_1DDIARG_SIGNATURE_ENTRY));
    }
    else
    {
        m_numInputSignatureEntries = 0;
        m_numOutputSignatureEntries = 0;
        m_pInputSignatureEntries = nullptr;
        m_pOutputSignatureEntries = nullptr;
    }
}

void
RosUmdPipelineShader::Update()
{
    // TODO: state dirtiness check.
    if (m_pCompiler)
    {
        return;
    }

    assert(m_pCode != NULL);

    const UINT *ShaderLinkage[2] = { NULL, NULL }; // Downstream, Upstream.

    switch (m_ProgramType)
    {
    case D3D10_SB_VERTEX_SHADER:
        assert(m_pDevice->m_pixelShader);
        ShaderLinkage[0] = m_pDevice->m_pixelShader->GetHLSLCode();
        break;
    case D3D10_SB_PIXEL_SHADER:
        assert(m_pDevice->m_vertexShader);
        ShaderLinkage[1] = m_pDevice->m_vertexShader->GetHLSLCode();
        break;
    default:
        assert(false);
    };

    m_pCompiler = RosCompilerCreate(m_ProgramType,
                                    m_pCode,
                                    ShaderLinkage[0], // Downstream
                                    ShaderLinkage[1], // Upstream
                                    m_pDevice->m_blendState->GetDesc(),
                                    m_pDevice->m_depthStencilState->GetDesc(),
                                    m_pDevice->m_rasterizerState->GetDesc(),
                                    (const RosUmdRenderTargetView **)&m_pDevice->m_renderTargetViews[0],
                                    (const RosUmdShaderResourceView **)&m_pDevice->m_psResourceViews[0],
                                    m_numInputSignatureEntries,
                                    m_pInputSignatureEntries,
                                    m_numOutputSignatureEntries,
                                    m_pOutputSignatureEntries,
                                    0,
                                    NULL);
    if (m_pCompiler == NULL)
    {
        throw RosUmdException(E_OUTOFMEMORY);
    }

    HRESULT hr;
    if (FAILED(hr = m_pCompiler->Compile()))
    {
        throw RosUmdException(hr);
    }

    {
        m_hwShaderCodeSize = m_pCompiler->GetShaderCodeSize();
        assert(m_hwShaderCodeSize != 0);
           
        m_pDevice->CreateInternalBuffer(
            &m_hwShaderCode,
            ROUND_TO_PAGES(m_hwShaderCodeSize)); // TODO: for now, for easier debugging, round allocation size to PAGE size aligned.

        {
            D3D10DDI_MAPPED_SUBRESOURCE mappedSubRes = { 0 };

            m_hwShaderCode.Map(
                m_pDevice,
                0,
                D3D10_DDI_MAP_WRITE,
                0,
                &mappedSubRes);

            if (mappedSubRes.pData)
            {
                m_pCompiler->GetShaderCode(
                    mappedSubRes.pData, 
                    &m_vc4CoordinateShaderOffset);

                m_hwShaderCode.Unmap(
                    m_pDevice,
                    0);

                return;
            }
        }
    }

    throw RosUmdException(E_FAIL);
}

void
RosUmdPipelineShader::Teardown()
{
    delete[] m_pInputSignatureEntries;
    delete[] m_pOutputSignatureEntries;

    RosUmdShader::Teardown();
}

void
RosUmdTesselationShader::Standup(
    const UINT * pCode, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES * pSignatures)
{
    RosUmdShader::Standup(pCode, hRTShader);

    m_numInputSignatureEntries = pSignatures->NumInputSignatureEntries;
    m_pInputSignatureEntries = new D3D11_1DDIARG_SIGNATURE_ENTRY[m_numInputSignatureEntries];
    memcpy(m_pInputSignatureEntries, pSignatures->pInputSignature, m_numInputSignatureEntries * sizeof(D3D11_1DDIARG_SIGNATURE_ENTRY));

    m_numOutputSignatureEntries = pSignatures->NumOutputSignatureEntries;
    m_pOutputSignatureEntries = new D3D11_1DDIARG_SIGNATURE_ENTRY[m_numOutputSignatureEntries];
    memcpy(m_pOutputSignatureEntries, pSignatures->pOutputSignature, m_numOutputSignatureEntries * sizeof(D3D11_1DDIARG_SIGNATURE_ENTRY));

    m_numPatchConstantSignatureEntries = pSignatures->NumPatchConstantSignatureEntries;
    m_pPatchConstantSignatureEntries = new D3D11_1DDIARG_SIGNATURE_ENTRY[m_numPatchConstantSignatureEntries];
    memcpy(m_pPatchConstantSignatureEntries, pSignatures->pPatchConstantSignature, m_numPatchConstantSignatureEntries * sizeof(D3D11_1DDIARG_SIGNATURE_ENTRY));
}

void
RosUmdTesselationShader::Teardown()
{
    delete[] m_pInputSignatureEntries;
    delete[] m_pOutputSignatureEntries;
    delete[] m_pPatchConstantSignatureEntries;

    RosUmdShader::Teardown();
}

void
RosUmdTesselationShader::Update()
{
    //TODO:
}


