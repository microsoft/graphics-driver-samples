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

BOOLEAN RosCompiler::Compile(UINT * puiShaderCodeSize,
                             UINT * pCoordinateShaderOffset)
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
#if VC4
        UINT vertexShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00701a00, 0xe0020c67, /* ldi vr_setup, 0x00701a00 */
            /* 0x00000008: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000010: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000018: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000038: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000040: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000048: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000050: */ 0x15c27d80, 0x100201a7, /* mov ra6, vpm ; nop */
            /* 0x00000058: */ 0x00000080, 0xe0020827, /* ldi r0, 128.0 */
            /* 0x00000060: */ 0x20027030, 0x100049e1, /* nop; fmul r1, ra0, r0 */
            /* 0x00000068: */ 0x019e7200, 0x10020867, /* fadd r1, r1, r0; nop */
            /* 0x00000070: */ 0x20067030, 0x100049e2, /* nop; fmul r2, ra1, r0 */
            /* 0x00000078: */ 0x019e7400, 0x100208a7, /* fadd r2, r2, r0; nop */
            /* 0x00000080: */ 0x079e7240, 0x10120827, /* ftoi r0.16a, r1 ; nop */
            /* 0x00000088: */ 0x079e7480, 0x10220827, /* ftoi r0.16b, r2 ; nop */
            /* 0x00000090: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000098: */ 0x159e7000, 0x10020c27, /* mov vpm, r0 ; nop */
            /* 0x000000a0: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x000000a8: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x000000b0: */ 0x15127d80, 0x10020c27, /* mov vpm, ra4 ; nop */
            /* 0x000000b8: */ 0x15167d80, 0x10020c27, /* mov vpm, ra5 ; nop */
            /* 0x000000c0: */ 0x151a7d80, 0x10020c27, /* mov vpm, ra6 ; nop */
            /* 0x000000c8: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x000000d0: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x000000d8: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

        UINT coordinateShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00701a00, 0xe0020c67, /* ldi vr_setup, 0x00701a00 */
            /* 0x00000008: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000010: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000018: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000038: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000040: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000048: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000050: */ 0x15c27d80, 0x100201a7, /* mov ra6, vpm ; nop */
            /* 0x00000058: */ 0x00000080, 0xe0020827, /* ldi r0, 128.0 */
            /* 0x00000060: */ 0x20027030, 0x100049e1, /* nop; fmul r1, ra0, r0 */
            /* 0x00000068: */ 0x019e7200, 0x10020867, /* fadd r1, r1, r0; nop */
            /* 0x00000070: */ 0x20067030, 0x100049e2, /* nop; fmul r2, ra1, r0 */
            /* 0x00000078: */ 0x019e7400, 0x100208a7, /* fadd r2, r2, r0; nop */
            /* 0x00000080: */ 0x079e7240, 0x10120827, /* ftoi r0.16a, r1 ; nop */
            /* 0x00000088: */ 0x079e7480, 0x10220827, /* ftoi r0.16b, r2 ; nop */
            /* 0x00000090: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000098: */ 0x15027d80, 0x10020c27, /* mov vpm, ra0 */
            /* 0x000000a0: */ 0x15067d80, 0x10020c27, /* mov vpm, ra1 */
            /* 0x000000a8: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 */
            /* 0x000000b0: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 */
            /* 0x000000b8: */ 0x159e7000, 0x10020c27, /* mov vpm, r0 ; nop */
            /* 0x000000c0: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x000000c8: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x000000d0: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x000000d8: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x000000e0: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

        m_HwCodeSize = sizeof(vertexShader) + sizeof(coordinateShader);

        m_pHwCode = new BYTE[m_HwCodeSize];

        CopyMemory(m_pHwCode, vertexShader, sizeof(vertexShader));
        CopyMemory(m_pHwCode + sizeof(vertexShader), coordinateShader, sizeof(coordinateShader));

#if DBG
        Disassemble_HW();
#endif // DBG

        *pCoordinateShaderOffset = sizeof(vertexShader);
        *puiShaderCodeSize = m_HwCodeSize;
        return TRUE;
#else
        __debugbreak();
#endif
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

UINT RosCompiler::GetInputSignature(D3D11_1DDIARG_SIGNATURE_ENTRY ** ppInputSignatureEntries)
{
    *ppInputSignatureEntries = m_pInputSignatureEntries;

    return m_numInputSignatureEntries;
}

UINT RosCompiler::GetOutputSignature(D3D11_1DDIARG_SIGNATURE_ENTRY ** ppOutputSignatureEntries)
{
    *ppOutputSignatureEntries = m_pOutputSignatureEntries;

    return m_numOutputSignatureEntries;
}

