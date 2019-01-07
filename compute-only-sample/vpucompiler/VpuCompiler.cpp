#include "llvm/Support/Error.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/COFF.h"
#include "llvm/Object/ELF.h"

#include <Windows.h>
#include <d3dcommon.h>

#include "LlvmLinker.h"
#include "LlvmCompiler.h"
#include "VpuImage.h"

#include "LlvmObject.h"

#include <dxcapi.h>
#include <d3dcompiler.h>

#include <assert.h>

using namespace llvm;
using namespace object;

#define TLS_SYMBOL "g_tls"

extern "C" __declspec(dllexport) bool vpu_compiler(ID3DBlob * inDxilByteCode, ID3DBlob ** outVpuImage)
{
	IDxcCompiler * pCompiler;
	HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void **)& pCompiler);
	assert(hr == S_OK);

	IDxcBlobEncoding * pDisassembly;
	hr = pCompiler->Disassemble((IDxcBlob *)inDxilByteCode, &pDisassembly);
	assert(hr == S_OK);

	char * assembly = (char *) pDisassembly->GetBufferPointer();
	uint64_t assemblyLength = pDisassembly->GetBufferSize();

	if (assembly == nullptr || assemblyLength == 0)
		return false;

	hr = D3DWriteBlobToFile((ID3DBlob *)pDisassembly, L"temp.dxil", TRUE);
	assert(hr == S_OK);

	pDisassembly->Release();
	pCompiler->Release();

	llvm_linker("vpu_compiler",
		"VpuShaderLib.bc",
		"DxilToVpu.ll",
		"temp.dxil",
		"VpuShader.bc");

	llvm_compile("vpu_compiler", "VpuShader.bc", "VpuShader.obj");

	bool success = true;
	VpuObject obj;

	VpuImageHeader header;

	success = obj.Open("VpuShader.obj") &&
		obj.GetSymbolValue("main", header.m_entryOffset) &&
		obj.GetSymbolValue(TLS_SYMBOL, header.m_tlsSize);

	if (success) {
		header.m_codeSize = obj.GetCodeSize();
		header.m_relocationCount = obj.GetRelocations().size();

		hr = D3DCreateBlob(header.GetSerializationSize(), outVpuImage);
		assert(hr == S_OK);

		VpuImageHeader * image = (VpuImageHeader*) (*outVpuImage)->GetBufferPointer();

		memcpy(image, &header, sizeof(header));

		const std::vector<VpuObjRelocation> & objRelocations = obj.GetRelocations();

		VpuRelocation * relocations = (VpuRelocation *)(image + 1);

		int relocationCount = 0;
		for (auto & objr : objRelocations) {
			VpuRelocation & r = relocations[relocationCount++];

			r.m_fixupOffset = objr.m_fixupOffset;
			r.m_type = objr.m_type;

			if (objr.m_symbolName == TLS_SYMBOL)
				r.m_referenceOffset = header.GetTlsOffset();
			else {

				std::string name = objr.m_symbolName;

				if (!obj.GetSymbolValue(name.c_str(), r.m_referenceOffset)) {
					success = false;
					break;
				}
			}
		}

		if (success) {
			assert(relocationCount == header.m_relocationCount);
			uint8_t * code = (uint8_t *)&relocations[header.m_relocationCount];
			obj.GetCode(code, header.m_codeSize);
		}
	}

	obj.Close();

	return true;
}
