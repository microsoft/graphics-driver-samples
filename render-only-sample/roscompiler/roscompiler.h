#include "d3dumddi_.h"
#include "DisasmBase.hpp"
#include "HLSLBinary.hpp"
#include "HLSLDisasm.hpp"
#if VC4
#include "Vc4Disasm.hpp"
#endif // VC4

class RosCompiler
{
public:

    RosCompiler(
        D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
        UINT *pCode,
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
    void Disassemble_HLSL()    { HLSLDisasm().Run(m_pCode); }
    void Disassemble_HW()
    {
#if VC4
        Vc4Disasm().Run((const VC4_QPU_INSTRUCTION*)m_pHwCode, m_HwCodeSize);
#endif // VC4
    }
    
    void Disassemble_Signatures()
    {
        HLSLDisasm().Run(TEXT("Input Signature Entries"), m_pInputSignatureEntries, m_numInputSignatureEntries);
        HLSLDisasm().Run(TEXT("Output Signature Entries"), m_pOutputSignatureEntries, m_numOutputSignatureEntries);
        HLSLDisasm().Run(TEXT("Patch Constant Signature Entries"), m_pPatchConstantSignatureEntries, m_numPatchConstantSignatureEntries);
    }

private:

    //
    // Data from runime.
    //
    D3D10_SB_TOKENIZED_PROGRAM_TYPE m_ProgramType;
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

RosCompiler* RosCompilerCreate(D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
                               UINT *pCode,
                               UINT numInputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                               UINT numOutputSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                               UINT numPatchConstantSignatureEntries,
                               D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries);
