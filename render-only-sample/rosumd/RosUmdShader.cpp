///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Resource implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RosUmdDevice.h"
#include "RosUmdShader.h"
#include "d3d11tokenizedprogramformat.hpp"

void
RosUmdShader::Standup(
    const UINT * pCode, D3D10DDI_HRTSHADER hRTShader)
{
    UINT codeSize = pCode[1];

    m_pCode = new UINT[codeSize];
    
    memcpy(m_pCode, pCode, codeSize);

    m_hRTShader = hRTShader;
}

void
RosUmdShader::Teardown()
{
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

    //
    // TODO: Calculate the size of VC4 shader instead using PAGE_SIZE
    //
    m_pDevice->CreateInternalBuffer(
        &m_hwShaderCode,
        PAGE_SIZE);

    UINT versionToken = pCode[0];
    UINT programType = (versionToken & D3D10_SB_TOKENIZED_PROGRAM_TYPE_MASK) >> D3D10_SB_TOKENIZED_PROGRAM_TYPE_SHIFT;

    if (D3D10_SB_PIXEL_SHADER == programType)
    {
        D3D10DDI_MAPPED_SUBRESOURCE mappedSubRes;

        m_hwShaderCode.Map(
            m_pDevice,
            0,
            D3D10_DDI_MAP_WRITE,
            0,
            &mappedSubRes);

        UINT *  pShaderCode = (UINT *)mappedSubRes.pData;

        //
        // TODO: Before shader compiler is online, use a hard-coded shader for testing
        //

        *pShaderCode++ = 0x958e0dbf;
        *pShaderCode++ = 0xd1724823;    // mov r0, vary; mov r3.8d, 1.0
        *pShaderCode++ = 0x818e7176;
        *pShaderCode++ = 0x40024821;    // fadd r0, r0, r5; mov r1, vary
        *pShaderCode++ = 0x818e7376;
        *pShaderCode++ = 0x10024862;    // fadd r1, r1, r5; mov r2, vary
        *pShaderCode++ = 0x819e7540;
        *pShaderCode++ = 0x114248a3;    // fadd r2, r2, r5; mov r3.8a, r0
        *pShaderCode++ = 0x809e7009;
        *pShaderCode++ = 0x115049e3;    // nop; mov r3.8b, r1
        *pShaderCode++ = 0x809e7012;
        *pShaderCode++ = 0x116049e3;    // nop; mov r3.8c, r2
        *pShaderCode++ = 0x159e76c0;
        *pShaderCode++ = 0x30020ba7;    // mov tlbc, r3; nop; thrend
        *pShaderCode++ = 0x009e7000;
        *pShaderCode++ = 0x100009e7;    // nop; nop; nop
        *pShaderCode++ = 0x009e7000;
        *pShaderCode++ = 0x500009e7;    // nop; nop; sbdone

        m_hwShaderCode.Unmap(
            m_pDevice,
            0);
    }
    else if (D3D10_SB_PIXEL_SHADER == programType)
    {
        // Implement vertex shader compiling
        __debugbreak();
    }
    else
    {
        // Other type of shader
    }
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


