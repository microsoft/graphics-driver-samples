#include "roscompiler.h"

void __stdcall InitInstructionInfo();
void __stdcall VC4_InitializeName();

void __stdcall InitializeShaderCompilerLibrary()
{
    InitInstructionInfo();
}

RosCompiler* RosCompilerCreate(D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
                               UINT *pCode,
                               UINT numInputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                               UINT numOutputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                               UINT numPatchConstantSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries)
{
    return new RosCompiler(
        ProgramType,
        pCode,
        numInputSignatureEntries,
        pInputSignatureEntries,
        numOutputSignatureEntries,
        pOutputSignatureEntries,
        numPatchConstantSignatureEntries,
        pPatchConstantSignatureEntries);
}

RosCompiler::RosCompiler(D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
                         UINT *pCode,
                         UINT numInputSignatureEntries,
                         D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                         UINT numOutputSignatureEntries,
                         D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                         UINT numPatchConstantSignatureEntries,
                         D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries) :
    m_ProgramType(ProgramType),
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

    assert(m_pCode);
    assert(m_ProgramType == (D3D10_SB_TOKENIZED_PROGRAM_TYPE)((m_pCode[0] & D3D10_SB_TOKENIZED_PROGRAM_TYPE_MASK) >> D3D10_SB_TOKENIZED_PROGRAM_TYPE_SHIFT));

#if DBG
    Disassemble_Signatures();
    Disassemble_HLSL();
#endif // DBG
        
    if (D3D10_SB_VERTEX_SHADER == m_ProgramType)
    {
        // Implement vertex shader compiling
        __debugbreak();
    }
    else if (D3D10_SB_PIXEL_SHADER == m_ProgramType)
    {
#if VC4
        VC4_QPU_INSTRUCTION PS[] =
        {
            0xd1724823958e0dbf,    // load_sm   ; mov r0, vary    ; mov r3.8d, 1.0
            0x40024821818e7176,    // sbwait    ; fadd r0, r0, r5 ; mov r1, vary
            0x10024862818e7376,    //             fadd r1, r1, r5 ; mov r2, vary     
            0x114248a3819e7540,    //             fadd r2, r2, r5 ; mov r3.8a, r0
            0x115049e3809e7009,    //             nop             ; mov r3.8b, r1
            0x116049e3809e7012,    //             nop             ; mov r3.8c, r2
            0x10020b27159cffc0,    //             mov tlb_z, rb15 ; nop
            0x30020ba7159e76c0,    // thrend    ; mov tlb_c, r3   ; nop
            0x100009e7009e7000,    //             nop             ; nop
            0x500009e7009e7000     // sbdone    ; nop             ; nop
        };
   
        m_HwCodeSize = sizeof(PS);
        m_pHwCode = new BYTE[m_HwCodeSize];
        CopyMemory(m_pHwCode, &PS[0], m_HwCodeSize);

#if DBG
        Disassemble_HW();
#endif // DBG
        *puiShaderCodeSize = m_HwCodeSize;
        return TRUE;
#else
        __debugbreak();
#endif
    }
    else
    {
        __debugbreak();
    }
   
    return FALSE;
}

BYTE * RosCompiler::GetShaderCode()
{
    return m_pHwCode;
}
