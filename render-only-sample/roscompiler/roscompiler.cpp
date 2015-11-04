#include "roscompiler.h"

void __stdcall InitInstructionInfo();
void __stdcall VC4_InitializeName();

void __stdcall InitializeShaderCompilerLibrary()
{
    InitInstructionInfo();
#if VC4
    VC4_InitializeName();
#endif //
}

RosCompiler* RosCompilerCreate(UINT *pCode,
                               UINT numInputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                               UINT numOutputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                               UINT numPatchConstantSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries)
{
    return new RosCompiler(pCode,
        numInputSignatureEntries,
        pInputSignatureEntries,
        numOutputSignatureEntries,
        pOutputSignatureEntries,
        numPatchConstantSignatureEntries,
        pPatchConstantSignatureEntries);
}

RosCompiler::RosCompiler(UINT *pCode,
                         UINT numInputSignatureEntries,
                         D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                         UINT numOutputSignatureEntries,
                         D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                         UINT numPatchConstantSignatureEntries,
                         D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries) :
    m_pCode(pCode),
    m_numInputSignatureEntries(numInputSignatureEntries),
    m_pInputSignatureEntries(pInputSignatureEntries),
    m_numOutputSignatureEntries(numOutputSignatureEntries),
    m_pOutputSignatureEntries(pOutputSignatureEntries),
    m_numPatchConstantSignatureEntries(numPatchConstantSignatureEntries),
    m_pPatchConstantSignatureEntries(pPatchConstantSignatureEntries),
    m_pHwCode(NULL),
    m_HwCodeSize(0)
{
}

RosCompiler::~RosCompiler() 
{
    delete[] m_pHwCode;
}

BOOLEAN RosCompiler::Compile(UINT * puiShaderCodeSize)
{
	assert(puiShaderCodeSize);
	*puiShaderCodeSize = 0;

#if DBG
    Disassemble_Signatures();
	Disassemble_HLSL();
#endif // DBG
	
	UINT versionToken = m_pCode[0];
    UINT programType = (versionToken & D3D10_SB_TOKENIZED_PROGRAM_TYPE_MASK) >> D3D10_SB_TOKENIZED_PROGRAM_TYPE_SHIFT;
    
    if (D3D10_SB_VERTEX_SHADER == programType)
    {
        // Implement vertex shader compiling
        __debugbreak();
    }
    else if (D3D10_SB_PIXEL_SHADER == programType)
    {
#if VC4
        m_HwCodeSize = 9 * sizeof(VC4_QPU_INSTRUCTION);
        m_pHwCode = new BYTE[m_HwCodeSize];

        //
        // TODO: Before shader compiler is online, use a hard-coded shader for testing
        //
        UINT *  pShaderCode = (UINT *)m_pHwCode;
        *pShaderCode++ = 0x958e0dbf;
        *pShaderCode++ = 0xd1724823;    // mov r0, vary; mov r3.8d, 1.0
        *pShaderCode++ = 0x818e7176;
        *pShaderCode++ = 0x40024821;    // fadd r0, r0, r5; mov r1, vary
        *pShaderCode++ = 0x818e7376;
        *pShaderCode++ = 0x10024862;    // fadd r1, r1, r5; mov r2, vary
        *pShaderCode++ = 0x819e7540;
        *pShaderCode++ = 0x114248a3;    // fadd r2, r2, r5; mov r3.8a, r0
        *pShaderCode++ = 0x809e7009;
        *pShaderCode++ = 0x115049e3;    // nop; mov r3.8b, r1
        *pShaderCode++ = 0x809e7012;
        *pShaderCode++ = 0x116049e3;    // nop; mov r3.8c, r2
        *pShaderCode++ = 0x159e76c0;
        *pShaderCode++ = 0x30020ba7;    // mov tlbc, r3; nop; thrend
        *pShaderCode++ = 0x009e7000;
        *pShaderCode++ = 0x100009e7;    // nop; nop; nop
        *pShaderCode++ = 0x009e7000;
        *pShaderCode++ = 0x500009e7;    // nop; nop; sbdone
#if DBG
		Disassemble_HW();
#endif // DBG
        *puiShaderCodeSize = PAGE_SIZE;
        return TRUE;
#else
        __debugbreak();
#endif
    }
   
    return FALSE;
}

BYTE * RosCompiler::GetShaderCode()
{
    return m_pHwCode;
}
