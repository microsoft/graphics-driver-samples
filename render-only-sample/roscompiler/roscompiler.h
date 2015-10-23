#include "d3dumddi_.h"
#include "HLSLBinary.hpp"

class RosCompiler
{
public:

    RosCompiler(UINT *pCode,
		UINT numInputSignatureEntries,
		D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
		UINT numOutputSignatureEntries,
		D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
		UINT numPatchConstantSignatureEntries,
	    D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries);
    ~RosCompiler();

	BOOLEAN Compile(UINT * puiShaderCodeSize);
	BYTE *GetShaderCode();

private:
	void Disassemble_HLSL()	{ HLSLDisasm().Run(m_pCode); }
	void Disassemble_HW() 	{		/* m_pHwCode; */	 }
	
private:

    //
    // Data from runime.
    //
    UINT *m_pCode;
    UINT m_numInputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *m_pInputSignatureEntries;
    UINT m_numOutputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *m_pOutputSignatureEntries;
	UINT m_numPatchConstantSignatureEntries;
	D3D11_1DDIARG_SIGNATURE_ENTRY *m_pPatchConstantSignatureEntries;
	
	//
	// Hardware code.
	//
	BYTE *m_pHwCode;
	UINT m_HwCodeSize;
};

RosCompiler* RosCompilerCreate(UINT *pCode,
                               UINT numInputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                               UINT numOutputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
							   UINT numPatchConstantSignatureEntries,
							   D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries);
