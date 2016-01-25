#pragma once

HRESULT LoadBMP(BYTE* pFile, ULONG *pRetWidth, ULONG *pRetHeight, PBYTE *pRetData);
HRESULT LoadTGA(BYTE* pFile, ULONG *pRetWidth, ULONG *pRetHeight, PBYTE *pRetData);

HRESULT SaveBMP(const char* pFileName, ID3D11Device *pDevice, ID3D11Texture2D *pTexture);


