#pragma once

#include <Windows.h>
#include <d3dcommon.h>

extern "C"  __declspec(dllimport) bool vpu_compiler(ID3DBlob * inDxilByteCode, ID3DBlob ** outVpuImage);
