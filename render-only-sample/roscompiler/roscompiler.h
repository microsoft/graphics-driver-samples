#include "d3dumddi_.h"

class RosCompiler
{
public:

    RosCompiler(UINT *pCode,
		UINT numInputSignatureEntries,
		D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
		UINT numOutputSignatureEntries,
		D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries);
    ~RosCompiler();

	BOOLEAN Compile(UINT * puiShaderCodeSize);
	BYTE *GetShaderCode();

private:

    //
    // Data from runime.
    //
    UINT *m_pCode;
    UINT m_numInputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *m_pInputSignatureEntries;
    UINT m_numOutputSignatureEntries;
    D3D11_1DDIARG_SIGNATURE_ENTRY *m_pOutputSignatureEntries;

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
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries);
