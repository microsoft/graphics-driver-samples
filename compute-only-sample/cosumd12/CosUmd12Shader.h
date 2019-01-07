#pragma once

#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12Shader
{
public:
	explicit CosUmd12Shader(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_SHADER_0026* pArgs);

	~CosUmd12Shader();

	static int CalculateSize(const D3D12DDIARG_CREATE_SHADER_0026 * pArgs);

    static CosUmd12Shader* CastFrom(D3D12DDI_HSHADER);
    D3D12DDI_HSHADER CastTo() const;

private:

    friend class CosUmd12CommandList;

    CosUmd12Device * m_pDevice;

	D3D12DDI_HROOTSIGNATURE m_hRootSignature;
	ID3DBlob * m_byteCode;
	D3D12DDIARG_STAGE_IO_SIGNATURES m_ioSignatures;
	D3D12DDI_CREATE_SHADER_FLAGS m_createFlags;
	D3D12DDI_LIBRARY_REFERENCE_0010 m_libraryReference;
	D3D12DDI_SHADERCACHE_HASH m_shaderCodeHash;

	ID3DBlob * m_image;

};

inline CosUmd12Shader* CosUmd12Shader::CastFrom(D3D12DDI_HSHADER hRootSignature)
{
    return static_cast< CosUmd12Shader* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HSHADER CosUmd12Shader::CastTo() const
{
    // TODO: Why does MAKE_D3D12DD_HSHADER not exist?
    return MAKE_D3D10DDI_HSHADER(const_cast< CosUmd12Shader* >(this));
}
