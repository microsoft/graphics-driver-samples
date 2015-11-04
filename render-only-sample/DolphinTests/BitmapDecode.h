#pragma once

HRESULT LoadBMP(PBYTE pFile, ULONG *pRetWidth, ULONG *pRetHeight, PBYTE *pRetData);
HRESULT LoadTGA(PBYTE pFile, ULONG *pRetWidth, ULONG *pRetHeight, PBYTE *pRetData);

void SaveBMP(const char* inFilePath, ID3D11Device *pDevice, ID3D11Texture2D *pTexture);

