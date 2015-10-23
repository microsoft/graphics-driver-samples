///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Resource implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RosUmdDevice.h"
#include "RosUmdShader.h"

void
RosUmdShader::Standup(
    const UINT * pCode, D3D10DDI_HRTSHADER hRTShader)
{
    UINT codeSize = pCode[1] * sizeof(UINT);

    m_pCode = new UINT[codeSize];
    
    memcpy(m_pCode, pCode, codeSize);

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

    m_pCompiler = RosCompilerCreate(m_pCode,
                                    m_numInputSignatureEntries,
                                    m_pInputSignatureEntries,
									m_numOutputSignatureEntries,
                                    m_pOutputSignatureEntries,
		                            0,
		                            NULL);

    if (m_pCompiler &&
        m_pCompiler->Compile(&m_hwShaderCodeSize) &&
        m_hwShaderCodeSize)
    {
        m_pDevice->CreateInternalBuffer(
            &m_hwShaderCode,
            m_hwShaderCodeSize);

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
                UINT* pShaderCode = (UINT *)mappedSubRes.pData;

                RtlCopyMemory(pShaderCode, m_pCompiler->GetShaderCode(), m_hwShaderCodeSize);

                m_hwShaderCode.Unmap(
                    m_pDevice,
                    0);

                return;
            }
        }
    }

    // TODO: error ?
    __debugbreak();
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


