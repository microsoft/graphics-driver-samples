#pragma once

class HLSLDisasm : public BaseDisasm
{
public:
	HLSLDisasm() { }
	~HLSLDisasm() { }
	HRESULT	HLSLDisasm::Run(const UINT * pShader);
};
