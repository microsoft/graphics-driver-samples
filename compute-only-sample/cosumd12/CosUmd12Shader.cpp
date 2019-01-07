#include "CosUmd12Shader.h"
#include "d3dcompiler.h"
#include "VpuCompiler.h"

CosUmd12Shader::CosUmd12Shader(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_SHADER_0026* pArgs)
{
	m_pDevice = pDevice;
	m_createFlags = pArgs->Flags;
	m_hRootSignature = pArgs->hRootSignature;
	m_libraryReference = pArgs->LibraryReference;
	m_shaderCodeHash = pArgs->ShaderCodeHash;

	size_t size = pArgs->pShaderCode[1] * sizeof(UINT);
	if (D3DCreateBlob(size, &m_byteCode) != S_OK)
		m_byteCode = nullptr;
	else
		memcpy(m_byteCode->GetBufferPointer(), pArgs->pShaderCode, size);

	m_ioSignatures.NumInputSignatureEntries = pArgs->IOSignatures.Standard->NumInputSignatureEntries;
	m_ioSignatures.NumOutputSignatureEntries = pArgs->IOSignatures.Standard->NumOutputSignatureEntries;

	size = m_ioSignatures.NumInputSignatureEntries * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012);
	m_ioSignatures.pInputSignature = (D3D12DDIARG_SIGNATURE_ENTRY_0012 *) malloc(size);
	if (m_ioSignatures.pInputSignature)
		memcpy(m_ioSignatures.pInputSignature, pArgs->IOSignatures.Standard->pInputSignature, size);

	size = m_ioSignatures.NumOutputSignatureEntries * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012);
	m_ioSignatures.pOutputSignature = (D3D12DDIARG_SIGNATURE_ENTRY_0012 *)malloc(size);
	if (m_ioSignatures.pOutputSignature)
		memcpy(m_ioSignatures.pOutputSignature, pArgs->IOSignatures.Standard->pOutputSignature, size);

	if (!vpu_compiler(m_byteCode, &m_image))
		m_image = nullptr;

	// TODO: How should we deal with errors here?  Put the device in an error state?

}

CosUmd12Shader::~CosUmd12Shader()
{
	if (m_byteCode != nullptr) m_byteCode->Release();
	if (m_ioSignatures.pInputSignature != nullptr) free(m_ioSignatures.pInputSignature);
	if (m_ioSignatures.pOutputSignature != nullptr) free(m_ioSignatures.pOutputSignature);
}

int CosUmd12Shader::CalculateSize(const D3D12DDIARG_CREATE_SHADER_0026 * pArgs)
{
	return sizeof(CosUmd12Shader);
}
