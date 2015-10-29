#pragma once

class HLSLDisasm : public BaseDisasm
{
public:
	HLSLDisasm() { }
	~HLSLDisasm() { }
	HRESULT	Run(const UINT * pShader);
    HRESULT Run(char *pTitle, const D3D11_1DDIARG_SIGNATURE_ENTRY * pEntry, UINT numEntries);
};
