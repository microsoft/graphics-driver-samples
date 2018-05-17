#pragma once

#include "CosUmdDevice.h"
#include <coscompiler.h>

class CosUmdShader
{
friend CosUmdDevice;

public:

    CosUmdShader(CosUmdDevice * pDevice, D3D10_SB_TOKENIZED_PROGRAM_TYPE Type)
        : m_pDevice(pDevice),
          m_ProgramType(Type),
          m_pCompiler(NULL)
    {
    }

    virtual ~CosUmdShader()
    {
    }

    static CosUmdShader* CastFrom(D3D10DDI_HSHADER);
    D3D10DDI_HSHADER CastTo() const;

    void Standup(const UINT * pCode, D3D10DDI_HRTSHADER hRTShader);
    virtual void Teardown();
    virtual void Update();

    CosUmdResource * GetCodeResource()
    {
        return &m_hwShaderCode;
    }

    UINT * GetHLSLCode()
    {
        return m_pCode;
    }

    UINT GetShaderInputCount()
    {
        return m_pCompiler->GetShaderInputCount();
    }

    UINT GetShaderOutputCount()
    {
        return m_pCompiler->GetShaderOutputCount();
    }

#if VC4

    VC4_UNIFORM_FORMAT * GetShaderUniformFormat(UINT Type, UINT *pUniformFormatEntries);
    
#endif

protected:

    D3D10_SB_TOKENIZED_PROGRAM_TYPE m_ProgramType;

    UINT *                          m_pCode;
    D3D10DDI_HRTSHADER              m_hRTShader;

    CosUmdDevice *                  m_pDevice;
    CosUmdResource                  m_hwShaderCode;
    UINT                            m_hwShaderCodeSize;
    UINT                            m_vc4CoordinateShaderOffset;

    CosCompiler *                   m_pCompiler;
};

inline CosUmdShader* CosUmdShader::CastFrom(D3D10DDI_HSHADER hShader)
{
    return static_cast< CosUmdShader* >(hShader.pDrvPrivate);
}

inline D3D10DDI_HSHADER CosUmdShader::CastTo() const
{
    return MAKE_D3D10DDI_HSHADER(const_cast< CosUmdShader* >(this));
}

class CosUmdPipelineShader : public CosUmdShader
{
public:

    CosUmdPipelineShader(CosUmdDevice *pDevice, D3D10_SB_TOKENIZED_PROGRAM_TYPE Type)
        : CosUmdShader(pDevice, Type)
    {
    }

    virtual ~CosUmdPipelineShader()
    {
    }

    static CosUmdPipelineShader* CastFrom(D3D10DDI_HSHADER);
    D3D10DDI_HSHADER CastTo() const;

    void Standup(const UINT * pCode, D3D10DDI_HRTSHADER hRT, const D3D11_1DDIARG_STAGE_IO_SIGNATURES * pSignatures);
    void Teardown();
    void Update();

private:

    UINT                            m_numInputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY * m_pInputSignatureEntries;
    UINT                            m_numOutputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY * m_pOutputSignatureEntries;

};

inline CosUmdPipelineShader* CosUmdPipelineShader::CastFrom(D3D10DDI_HSHADER hShader)
{
    return dynamic_cast<CosUmdPipelineShader*>(static_cast< CosUmdShader* >(hShader.pDrvPrivate));
}

inline D3D10DDI_HSHADER CosUmdPipelineShader::CastTo() const
{
    return MAKE_D3D10DDI_HSHADER(const_cast< CosUmdPipelineShader* >(this));
}

class CosUmdTesselationShader : public CosUmdShader
{
public:

    CosUmdTesselationShader(CosUmdDevice *pDevice, D3D10_SB_TOKENIZED_PROGRAM_TYPE Type)
        : CosUmdShader(pDevice, Type)
    {
    }

    virtual ~CosUmdTesselationShader()
    {
    }

    static CosUmdTesselationShader* CastFrom(D3D10DDI_HSHADER);
    D3D10DDI_HSHADER CastTo() const;

    void Standup(const UINT * pCode, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES * pSignatures);
    void Teardown();
    void Update();

private:

    UINT                                m_numInputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *     m_pInputSignatureEntries;
    UINT                                m_numOutputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *     m_pOutputSignatureEntries;
    UINT                                m_numPatchConstantSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *     m_pPatchConstantSignatureEntries;

};

inline CosUmdTesselationShader* CosUmdTesselationShader::CastFrom(D3D10DDI_HSHADER hShader)
{
    return dynamic_cast<CosUmdTesselationShader*>(static_cast< CosUmdShader* >(hShader.pDrvPrivate));
}

inline D3D10DDI_HSHADER CosUmdTesselationShader::CastTo() const
{
    return MAKE_D3D10DDI_HSHADER(const_cast< CosUmdTesselationShader* >(this));
}
