#include "VpuCompiler.h"
#include "VpuImage.h"

//#include <dxcapi.h>
#include <d3dcompiler.h>

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <memory.h>
#include <sstream>

int main(int argc, char ** argv)
{
	ID3DBlob *pNullByteCode;
	HRESULT hr = D3DReadFileToBlob(L"nulltest.cso", &pNullByteCode);
	assert(hr == S_OK);

	ID3DBlob *pNullImage;
	if (!vpu_compiler(pNullByteCode, &pNullImage)) {
		printf("failed to compile null byte code\n");
		exit(1);
	}

	ID3DBlob *pByteCode;
	hr = D3DReadFileToBlob(L"Test.cso", &pByteCode);
	assert(hr == S_OK);

	ID3DBlob *pImage;
	if (!vpu_compiler(pByteCode, &pImage)) {
		printf("failed to compile byte code\n");
		exit(1);
	}

	typedef struct {
		int32_t i;
		float f;
	} uavElement;

	uavElement uavData[3][4] = {
		{ { 4, 4.0 }, { 2, 2.0 }, { 7, 7.0 }, { 10, 10.0} },
		{ { 2, 2.0 }, { 8, 8.0 }, { 3, 3.0 }, { 2, 2.0 } },
		{ { 0, 0.0 }, { 0, 0.0 }, { 0, 0.0 }, { 0, 0.0 } }
	};

	VpuImageHeader * header = (VpuImageHeader *)pImage->GetBufferPointer();

	uint64_t imageSize = header->GetImageSize();
	uint8_t * imageBase = (uint8_t *)VirtualAlloc(NULL, imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!header->Load(imageBase, imageSize)) {
		printf("error loading binary\n");
		exit(1);
	}

	uint64_t tlsOffset = header->GetTlsOffset();
	uint64_t entryOffset = header->GetEntryOffset();

	VpuThreadLocalStorage * tls = (VpuThreadLocalStorage *)(imageBase + tlsOffset);
	void(*shader_main)() = (void(*)(void)) (imageBase + entryOffset);

	printf("running shader\n");

	for (int i = 0; i < 3; i++) {
		tls->m_uavs[i].m_base = (int8_t *)&uavData[i][0];
		tls->m_uavs[i].m_elementSize = sizeof(uavElement);
	}

	for (int threadId = 0; threadId < 4; threadId++)
	{
		tls->m_id = threadId;
		shader_main();
	}

	VirtualFree(imageBase, 0, MEM_RELEASE);

	printf("results\n");

	for (int i = 0; i < 4; i++)
		printf("{ %d %f } ", uavData[2][i].i, uavData[2][i].f);
	printf("\n");

	printf("done \n");

}
