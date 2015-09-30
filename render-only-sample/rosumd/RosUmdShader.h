#pragma once

#include "d3dumddi_.h"
//#include "D3d10tokenizedprogramformat.hpp"

class RosUmdShader
{
public:

    RosUmdShader(const UINT * pCode, D3D10DDI_HRTSHADER hRT) :
        m_hRTShader(hRT)
    {
        UINT codeSize = pCode[1];
        m_pCode = new UINT[codeSize];
        memcpy(m_pCode, pCode, codeSize);
    }

    virtual ~RosUmdShader()
    {
        delete[] m_pCode;
    }

    static RosUmdShader* CastFrom(D3D10DDI_HSHADER);
    D3D10DDI_HSHADER CastTo() const;

protected:

    UINT *                          m_pCode;
    D3D10DDI_HRTSHADER              m_hRTShader;
};

inline RosUmdShader* RosUmdShader::CastFrom(D3D10DDI_HSHADER hShader)
{
    return static_cast< RosUmdShader* >(hShader.pDrvPrivate);
}

inline D3D10DDI_HSHADER RosUmdShader::CastTo() const
{
    return MAKE_D3D10DDI_HSHADER(const_cast< RosUmdShader* >(this));
}

class RosUmdPipelineShader : public RosUmdShader
{
public:

    RosUmdPipelineShader(const UINT * pCode, D3D10DDI_HRTSHADER hRT, const D3D11_1DDIARG_STAGE_IO_SIGNATURES * pSignatures) :
        RosUmdShader(pCode, hRT)
    {
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

    virtual ~RosUmdPipelineShader()
    {
        delete[] m_pInputSignatureEntries;
        delete[] m_pOutputSignatureEntries;
    }

    static RosUmdPipelineShader* CastFrom(D3D10DDI_HSHADER);
    D3D10DDI_HSHADER CastTo() const;

private:

    UINT                            m_numInputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY * m_pInputSignatureEntries;
    UINT                            m_numOutputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY * m_pOutputSignatureEntries;

};

inline RosUmdPipelineShader* RosUmdPipelineShader::CastFrom(D3D10DDI_HSHADER hShader)
{
    return dynamic_cast<RosUmdPipelineShader*>(static_cast< RosUmdShader* >(hShader.pDrvPrivate));
}

inline D3D10DDI_HSHADER RosUmdPipelineShader::CastTo() const
{
    return MAKE_D3D10DDI_HSHADER(const_cast< RosUmdPipelineShader* >(this));
}

class RosUmdTesselationShader : public RosUmdShader
{
public:

    RosUmdTesselationShader(const UINT * pCode, D3D10DDI_HRTSHADER hRT, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES * pSignatures) :
        RosUmdShader(pCode, hRT)
    {
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

    virtual ~RosUmdTesselationShader()
    {
        delete[] m_pInputSignatureEntries;
        delete[] m_pOutputSignatureEntries;
        delete[] m_pPatchConstantSignatureEntries;
    }

    static RosUmdTesselationShader* CastFrom(D3D10DDI_HSHADER);
    D3D10DDI_HSHADER CastTo() const;

private:

    UINT                                m_numInputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *     m_pInputSignatureEntries;
    UINT                                m_numOutputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *     m_pOutputSignatureEntries;
    UINT                                m_numPatchConstantSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *     m_pPatchConstantSignatureEntries;

};

inline RosUmdTesselationShader* RosUmdTesselationShader::CastFrom(D3D10DDI_HSHADER hShader)
{
    return dynamic_cast<RosUmdTesselationShader*>(static_cast< RosUmdShader* >(hShader.pDrvPrivate));
}

inline D3D10DDI_HSHADER RosUmdTesselationShader::CastTo() const
{
    return MAKE_D3D10DDI_HSHADER(const_cast< RosUmdTesselationShader* >(this));
}
