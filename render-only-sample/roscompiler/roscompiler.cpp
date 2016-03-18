#include "precomp.h"
#include "roscompiler.h"

void InitializeShaderCompilerLibrary()
{
    InitInstructionInfo();
}

RosCompiler* RosCompilerCreate(D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
                               const UINT *pCode,
                               const UINT *pLinkageDownstreamCode,
                               const UINT *pLinkageUpstreamCode,
                               const D3D11_1_DDI_BLEND_DESC* pBlendState,
                               const D3D10_DDI_DEPTH_STENCIL_DESC* pDepthState,
                               const D3D11_1_DDI_RASTERIZER_DESC* pRasterState,
                               const RosUmdRenderTargetView** ppRenderTargetView,
                               const RosUmdShaderResourceView** ppShaderResouceView,
                               UINT numInputSignatureEntries,
                               const D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                               UINT numOutputSignatureEntries,
                               const D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                               UINT numPatchConstantSignatureEntries,
                               const D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries)
{
    RosCompiler *pCompiler = new RosCompiler(
        ProgramType,
        pCode,
        pLinkageDownstreamCode,
        pLinkageUpstreamCode,
        pBlendState,
        pDepthState,
        pRasterState,
        ppRenderTargetView,
        ppShaderResouceView,
        numInputSignatureEntries,
        pInputSignatureEntries,
        numOutputSignatureEntries,
        pOutputSignatureEntries,
        numPatchConstantSignatureEntries,
        pPatchConstantSignatureEntries);
    if (pCompiler)
    {
        if (FAILED(pCompiler->InitializeStorage()))
        {
            delete pCompiler;
            pCompiler = NULL;
        }
    }
    return pCompiler;
}

RosCompiler::RosCompiler(D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
                         const UINT *pCode,
                         const UINT *pLinkageDownstreamCode,
                         const UINT *pLinkageUpstreamCode,
                         const D3D11_1_DDI_BLEND_DESC* pBlendState,
                         const D3D10_DDI_DEPTH_STENCIL_DESC* pDepthState,
                         const D3D11_1_DDI_RASTERIZER_DESC* pRasterState,
                         const RosUmdRenderTargetView** ppRenderTargetView,
                         const RosUmdShaderResourceView** ppShaderResouceView,
                         UINT numInputSignatureEntries,
                         const D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                         UINT numOutputSignatureEntries,
                         const D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                         UINT numPatchConstantSignatureEntries,
                         const D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries) :
    m_ProgramType(ProgramType),
    m_pCode(pCode),
    m_pDownstreamCode(pLinkageDownstreamCode),
    m_pUpstreamCode(pLinkageUpstreamCode),
    m_pBlendState(pBlendState),
    m_pDepthState(pDepthState),
    m_pRasterState(pRasterState),
    m_ppRenderTargetView(ppRenderTargetView),
    m_ppShaderResouceView(ppShaderResouceView),
    m_numInputSignatureEntries(numInputSignatureEntries),
    m_pInputSignatureEntries(pInputSignatureEntries),
    m_numOutputSignatureEntries(numOutputSignatureEntries),
    m_pOutputSignatureEntries(pOutputSignatureEntries),
    m_numPatchConstantSignatureEntries(numPatchConstantSignatureEntries),
    m_pPatchConstantSignatureEntries(pPatchConstantSignatureEntries),
    m_cShaderInput(0),
    m_cShaderOutput(0)
{
}

RosCompiler::~RosCompiler() 
{
}

HRESULT RosCompiler::Compile()
{
    HRESULT hr = E_NOTIMPL;

    assert(m_pCode);
    assert(m_ProgramType == (D3D10_SB_TOKENIZED_PROGRAM_TYPE)((m_pCode[0] & D3D10_SB_TOKENIZED_PROGRAM_TYPE_MASK) >> D3D10_SB_TOKENIZED_PROGRAM_TYPE_SHIFT));
    
#if DBG
    // Disassemble HLSL
    Disassemble_Signatures();
    Disassemble_HLSL();
#endif // DBG

#if VC4
    Vc4Shader Vc4ShaderCompiler(this);

    // Set HLSL bytecode.
    Vc4ShaderCompiler.SetShaderCode(m_pCode);
    if (m_pDownstreamCode)
    {
        Vc4ShaderCompiler.SetDownstreamShaderCode(m_pDownstreamCode);
    }
    if (m_pUpstreamCode)
    {
        Vc4ShaderCompiler.SetUpstreamShaderCode(m_pUpstreamCode);
    }

    switch (m_ProgramType)
    {
    case D3D10_SB_VERTEX_SHADER:
        // Set output hw shader storage.
        // for vertex shader.
        Vc4ShaderCompiler.SetShaderStorage(
            &m_Storage[ROS_VERTEX_SHADER_STORAGE],
            &m_Storage[ROS_VERTEX_SHADER_UNIFORM_STORAGE]);
        // for coordinate shader
        Vc4ShaderCompiler.SetShaderStorageAux(
            &m_Storage[ROS_COORDINATE_SHADER_STORAGE],
            &m_Storage[ROS_COORDINATE_SHADER_UNIFORM_STORAGE]);

        try
        {
            // Compile shader
            hr = Vc4ShaderCompiler.Translate_VS();
        }
        catch (RosCompilerException & e)
        {
            hr = e.GetError();
        }

        if (SUCCEEDED(hr))
        {
            m_cShaderInput = Vc4ShaderCompiler.GetInputCount();
            m_cShaderOutput = Vc4ShaderCompiler.GetOutputCount();

#if DBG
            // Disassemble h/w shader.
            Disassemble_HW(m_Storage[ROS_VERTEX_SHADER_STORAGE], TEXT("VC4 Vertex shader"));
            Dump_UniformTable(m_Storage[ROS_VERTEX_SHADER_UNIFORM_STORAGE], TEXT("VC4 Vertex shader Uniform"));

            Disassemble_HW(m_Storage[ROS_COORDINATE_SHADER_STORAGE], TEXT("VC4 Coordinate shader"));
            Dump_UniformTable(m_Storage[ROS_COORDINATE_SHADER_UNIFORM_STORAGE], TEXT("VC4 Coordinate shader Uniform"));
#endif // DBG
        }

        break;

    case D3D10_SB_PIXEL_SHADER:
        // Set output hw shader storage.
        Vc4ShaderCompiler.SetShaderStorage(
            &m_Storage[ROS_PIXEL_SHADER_STORAGE],
            &m_Storage[ROS_PIXEL_SHADER_UNIFORM_STORAGE]);;

        try
        {
            // Compile shader
            hr = Vc4ShaderCompiler.Translate_PS();
        }
        catch (RosCompilerException & e)
        {
            hr = e.GetError();
        }

        if (SUCCEEDED(hr))
        {
            m_cShaderInput = Vc4ShaderCompiler.GetInputCount();
            m_cShaderOutput = Vc4ShaderCompiler.GetOutputCount();

#if DBG
            // Disassemble h/w shader.
            Disassemble_HW(m_Storage[ROS_PIXEL_SHADER_STORAGE], TEXT("VC4 Pixel shader"));
            Dump_UniformTable(m_Storage[ROS_PIXEL_SHADER_UNIFORM_STORAGE], TEXT("VC4 Vertex shader Uniform"));
#endif // DBG

        }

        break;

    default:
        assert(false);
    }
#else
    assert(false);
#endif // VC4

    return hr;
}

#if 0

// #define SHAREDTEX_CVS   1
// #define PASSTHROUGH_CVS 1
// #define SIMPLETRANS_CVS 1
#define CUBETEST_CVFS   1
       
#if SHAREDTEX_CVS

        VC4_QPU_INSTRUCTION vertexShader[] =
        {
            0xe0024c6700401a00, // load_imm vr_setup, nop, 0x00401a00 (0.000000)    // vr_setup = 0x401a00          ;       // 16bits, horizontal, 1 stride, 4 reads from VPM
            0x1002082715c27d80, // mov r0, vpm_read; nop nop, r0, r0                // r0  = vpm_read (X)           ;
            0x1002504035c20d87, // mov rb1, vpm_read; fmul ra0, r0, uni             // rb1 = vpm_read (Y)           ; ra0 = r0 * uni[0] // transform (X) by uniform[0], [1], [2], [4]
            0x100059c120827006, // nop nop, r0, r0; fmul ra1, r0, uni               //                              ; ra1 = r0 * uni[1]
            0x100240a135c20d87, // mov ra2, vpm_read; fmul r1, r0, uni              // ra2 = vpm_read (Varying0)    ; r1 = r0 * uni[2]  // ra2 = s:TEXCOORD0
            0x100049e220827006, // nop nop, r0, r0; fmul r2, r0, uni                //                              ; r2 = r0 * uni[3]
            0x100049e02080103e, // nop nop, r0, r0; fmul r0, rb1, uni               //                              ; r0 = rb1 * uni[4] // transform (Y) by uniform[4], [5], [6], [7]
            0x10021027019e7040, // fadd rb0, r0, r1; nop nop, r0, r0                // rb0 = r0 + r1                ;
            0x100049e02080103e, // nop nop, r0, r0; fmul r0, rb1, uni               //                              ; r0 = rb1 * uni[5]
            0x100248e0218010be, // fadd r3, r0, r2; fmul r0, rb1, uni               // r3 = r0 + r2                 ; r0 = rb1 * uni[6]
            0x1002086701027180, // fadd r1, r0, ra0; nop nop, r0, r0                // r1 = r0 + ra0                ;
            0x100049e02080103e, // nop nop, r0, r0; fmul r0, rb1, uni               //                              ; r0 = rb1 * uni[7]
            0x1002082701067180, // fadd r0, r0, ra1; nop nop, r0, r0                // r0 = r0 + ra1                ;
            0x1002480281c20e36, // fadd r0, uni, r0; mov rb2, vpm_read              // r0 = uni[8] + r0             ; rb2 = vpm_read (Varying1)    // rb2 = t:TEXCOORD0
            0x10021d27159e7000, // mov sfu_recip, r0; nop nop, r0, r0               // r4 = sfu_recip(r0)           ;       // r4 = 1/r0 (W)
            0x100208a701800dc0, // fadd r2, uni, rb0; nop nop, r0, r0               // r2 = uni[9] + rb0            ;
            0x100049e020827016, // nop nop, r0, r0; fmul r0, r2, uni                //                              ; r0 = r2 * uni[10]     // X scale by uniform ?
            0x100248e021827cc4, // fadd r3, uni, r3; fmul r0, r0, r4                // r3 = uni[11] + r3            ; r0 = r0 * r4          // perspective divide (Y*1/W)
            0x101240202782701e, // ftoi ra0.16a, r0, r0; fmul r0, r3, uni           // ra0.16a = ftoi(r0)           ; r0 = r3 * uni[12]     // Y scale by uniform ?
            0x1002486021827c44, // fadd r1, uni, r1; fmul r0, r0, r4                // r1 = uni[13] + r1            ; r0 = r0 * r4          // perspective divide (X*1/W)
            0x102240202782700e, // ftoi ra0.16b, r0, r0; fmul r0, r1, uni           // ra0.16b = ftoi(r0)           ; r0 = r1 * uni[14]     // Z scale by uniform ?
            0xe0025c6700001a00, // load_imm vw_setup, nop, 0x00001a00 (0.000000)    // vw_setup = 0x1a00            ;                       // 16bits, horizontal, 1 stride.
            0x10024c2035027d84, // mov vpm, ra0; fmul r0, r0, r4                    // vpm = ra0 (Ys/Xs)            ; r0 = r0 * r4          // perspective divide (Z*1/W)
            0x10020c2701827180, // fadd vpm, r0, uni; nop nop, r0, r0               // vpm = r0 + uni[15] (Zs)      ;                       // Adjust Z with uniform
            0x10020c27159e7900, // mov vpm, r4; nop nop, r0, r0                     // vpm = r4 (1/Wc)              ;
            0x10020c27150a7d80, // mov vpm, ra2; nop nop, r0, r0                    // vpm = ra2 (Varying0)         ;       // vpm = s:TEXCOORD0
            0x10020c27159c2fc0, // mov vpm, rb2; nop nop, r0, r0                    // vpm = rb2 (Varying1)         ;       // vpm = t:TEXCOORD1
            0xd0020c27159e0fc0, // sig_small_imm mov vpm, 1.0; nop nop, r0, r0      // vpm = 1 (Varying2)           ;       // vpm = w?
            0x300009e7009e7000, // sig_end nop nop, r0, r0; nop nop, r0, r0         // program end                  ;
            0x100009e7009e7000, // nop nop, r0, r0; nop nop, r0, r0                 //
            0x100009e7009e7000, // nop nop, r0, r0; nop nop, r0, r0                 //
        };

#elif PASSTHROUGH_CVS

        UINT vertexShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00701a00, 0xe0020c67, /* ldi vr_setup, 0x00701a00 */
            /* 0x00000008: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000010: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000018: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000038: */ 0x15c27d80, 0x100201a7, /* mov ra6, vpm ; nop */
            /* 0x00000040: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000048: */ 0x3f800000, 0xe00208e7, /* ldi r3, 0x3f800000 */
            /* 0x00000050: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x00000058: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x00000060: */ 0x159e76c0, 0x10020c27, /* mov vpm, r3 ; nop */
            /* 0x00000068: */ 0x15127d80, 0x10020c27, /* mov vpm, ra4 ; nop */
            /* 0x00000070: */ 0x15167d80, 0x10020c27, /* mov vpm, ra5 ; nop */
            /* 0x00000078: */ 0x151a7d80, 0x10020c27, /* mov vpm, ra6 ; nop */
            /* 0x00000080: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x00000088: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000090: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

#elif SIMPLETRANS_CVS

        UINT vertexShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00701a00, 0xe0020c67, /* ldi vr_setup, 0x00701a00 */
            /* 0x00000008: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000010: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000018: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000038: */ 0x15c27d80, 0x100201a7, /* mov ra6, vpm ; nop */
            /* 0x00000040: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000048: */ 0x15027d80, 0x10020867, /* mov r1, ra0  ; nop */
            /* 0x00000050: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000058: */ 0x079e7000, 0x10120227, /* ftoi ra8.16a, r0 ; nop */
            /* 0x00000060: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000068: */ 0x15067d80, 0x10020867, /* mov r1, ra1  ; nop */
            /* 0x00000070: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000078: */ 0x079e7000, 0x10220227, /* ftoi ra8.16b, r0 ; nop */
            /* 0x00000080: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000088: */ 0x15227d80, 0x10020c27, /* mov vpm, ra8 ; nop */
            /* 0x00000090: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x00000098: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x000000a0: */ 0x15127d80, 0x10020c27, /* mov vpm, ra4 */
            /* 0x000000a8: */ 0x15167d80, 0x10020c27, /* mov vpm, ra5 */
            /* 0x000000b0: */ 0x151a7d80, 0x10020c27, /* mov vpm, ra6 */
            /* 0x000000b8: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x000000c0: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x000000c8: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

#elif CUBETEST_CVFS

        UINT vertexShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00601a00, 0xe0020c67, /* ldi vr_setup, 0x00601a00 */
            /* 0x00000008: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000010: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000018: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000038: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000040: */ 0x15027d80, 0x10020867, /* mov r1, ra0  ; nop */
            /* 0x00000048: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000050: */ 0x079e7000, 0x10120227, /* ftoi ra8.16a, r0 ; nop */
            /* 0x00000058: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000060: */ 0x15067d80, 0x10020867, /* mov r1, ra1  ; nop */
            /* 0x00000068: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000070: */ 0x079e7000, 0x10220227, /* ftoi ra8.16b, r0 ; nop */
            /* 0x00000078: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000080: */ 0x15227d80, 0x10020c27, /* mov vpm, ra8 ; nop */
            /* 0x00000088: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x00000090: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x00000098: */ 0x15127d80, 0x10020c27, /* mov vpm, ra4 */
            /* 0x000000a0: */ 0x15167d80, 0x10020c27, /* mov vpm, ra5 */
            /* 0x000000a8: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x000000b0: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x000000b8: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

#else

#endif

#if SHAREDTEX_CVS

        VC4_QPU_INSTRUCTION coordinateShader[] =
        {
            0xe0024c6700201a00, // load_imm vr_setup, nop, 0x00201a00 (0.000000)    // vr_setup = 0x201a00      ;           // 16bits, horizontal, 1 stride, 2 reads from VPM, 
            0x1002082715c27d80, //  mov r0, vpm_read; nop nop, r0, r0               // r0 = vpm_read (X)        ;
            0x1002404035c20d87, // mov ra1, vpm_read; fmul rb0, r0, uni             // ra1 = vpm_read (Y)       ; rb0 = r0 * uni[0]     // transform (X) by uniform[0], [1], [2], [3]
            0x100049e120827006, // nop nop, r0, r0; fmul r1, r0, uni                //                          ; r1 = r0 * uni[1]
            0x100049e220827006, // nop nop, r0, r0; fmul r2, r0, uni                //                          ; r2 = r0 * uni[2]
            0x100049e320827006, // nop nop, r0, r0; fmul r3, r0, uni                //                          ; r3 = r0 * uni[3]
            0x100049e020060037, // nop nop, r0, r0; fmul r0, ra1, uni               //                          ; r0 = ra1 * uni[4]     // transform (Y) by uniform[4], [5], [6], [7]
            0x1002402021060077, // fadd ra0, r0, r1; fmul r0, ra1, uni              // ra0 = r0 + r1            ; r0 = ra1 * uni[5]
            0x100248a0210600b7, // fadd r2, r0, r2; fmul r0, ra1, uni               // r2 = r0 + r2             ; r0 = ra1 * uni[6]
            0x10024860210600f7, // fadd r1, r0, r3; fmul r0, ra1, uni               // r1 = r0 + r3             ; r0 = ra1 * uni[7]
            0x10020827019c01c0, // fadd r0, r0, rb0; nop nop, r0, r0                // r0 = r0 + rb0            ;
            0x100208e701020f80, // fadd r3, uni, ra0; nop nop, r0, r0               // r3 = uni[8] + ra0        ;
            0xe0025c6700001a00, // load_imm vw_setup, nop, 0x00001a00 (0.000000)    // vw_setup = 0x1a00        ;           // 16bits, horizontal, 1 stride.
            0x1002483081827c1b, // fadd r0, uni, r0; mov vpm, r3                    // r0 = uni[9] + r0         ; vpm = r3 (Xc)
            0x100208a701827c80, // fadd r2, uni, r2; nop nop, r0, r0                // r2 = uni[10] + r2        ;
            0x1002487081827c52, // fadd r1, uni, r1; mov vpm, r2                    // r1 = uni[11] + r1        ; vpm = r2 (Yc)
            0x10021d27159e7000, // mov sfu_recip, r0; nop nop, r0, r0               // r4 = sfu_recip(r0)       ;           // r4 = 1/w
            0x10020c27159e7240, // mov vpm, r1; nop nop, r0, r0                     // vpm = r1 (Zc)            ;
            0x10024c203582701e, // mov vpm, r0; fmul r0, r3, uni                    // vpm = r0 (Wc)            ; r0 = r3 * uni[12]     // X scale by uniform ?
            0x100049e0209e7004, // nop nop, r0, r0; fmul r0, r0, r4                 //                          ; r0 = r0 * r4          // perspective divide (Y*1/w)
            0x1012402027827016, // ftoi ra0.16a, r0, r0; fmul r0, r2, uni           // ra0.16a = ftoi(r0)       ; r0 = r2 * uni[13]     // Y scale by uniform ?
            0x100049e0209e7004, // nop nop, r0, r0; fmul r0, r0, r4                 //                          ; r0 = r0 * r4          // perspective divide (X*1/w)
            0x102240202782700e, // ftoi ra0.16b, r0, r0; fmul r0, r1, uni           // ra0.16b = ftoi(r0)       ; r0 = r1 * uni[14]     // Z scale by uniform ?
            0x100049e0209e7004, // nop nop, r0, r0; fmul r0, r0, r4                 //                          ; r0 = r0 * r4          // perspective divide (Z*1/w)
            0x10020c2715027d80, // mov vpm, ra0; nop nop, r0, r0                    // vpm = ra0 (Ys/Xs)        ;
            0x10020c2701827180, // fadd vpm, r0, uni; nop nop, r0, r0               // vpm = r0 + uni[15] (Zs)  ;           // Adjust Z with uniform
            0x10020c27159e7900, // mov vpm, r4; nop nop, r0, r0                     // vpm = r4 (1/Wc)
            0x300009e7009e7000, // sig_end nop nop, r0, r0; nop nop, r0, r0
            0x100009e7009e7000, // nop nop, r0, r0; nop nop, r0, r0
            0x100009e7009e7000, // nop nop, r0, r0; nop nop, r0, r0
        };

#elif PASSTHROUGH_CVS

        UINT coordinateShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00701a00, 0xe0020c67, /* ldi vr_setup, 0x00701a00 */
            /* 0x00000008: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000010: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000018: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000038: */ 0x15c27d80, 0x100201a7, /* mov ra6, vpm ; nop */
            /* 0x00000040: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000048: */ 0x3f800000, 0xe00208e7, /* ldi r3, 0x3f800000 */
            /* 0x00000050: */ 0x15027d80, 0x10020c27, /* mov vpm, ra0 */
            /* 0x00000058: */ 0x15067d80, 0x10020c27, /* mov vpm, ra1 */
            /* 0x00000060: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 */
            /* 0x00000068: */ 0x159e76c0, 0x10020c27, /* mov vpm, r3 */
            /* 0x00000070: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x00000078: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x00000080: */ 0x159e76c0, 0x10020c27, /* mov vpm, r3 ; nop */
            /* 0x00000088: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x00000090: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x00000098: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

#elif SIMPLETRANS_CVS

        UINT coordinateShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00701a00, 0xe0020c67, /* ldi vr_setup, 0x00701a00 */
            /* 0x00000008: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000010: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000018: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000038: */ 0x15c27d80, 0x100201a7, /* mov ra6, vpm ; nop */
            /* 0x00000040: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000048: */ 0x15027d80, 0x10020867, /* mov r1, ra0  ; nop */
            /* 0x00000050: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000058: */ 0x079e7000, 0x10120227, /* ftoi ra8.16a, r0 ; nop */
            /* 0x00000060: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000068: */ 0x15067d80, 0x10020867, /* mov r1, ra1  ; nop */
            /* 0x00000070: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000078: */ 0x079e7000, 0x10220227, /* ftoi ra8.16b, r0 ; nop */
            /* 0x00000080: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000088: */ 0x15027d80, 0x10020c27, /* mov vpm, ra0 */
            /* 0x00000090: */ 0x15067d80, 0x10020c27, /* mov vpm, ra1 */
            /* 0x00000098: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 */
            /* 0x000000a0: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 */
            /* 0x000000a8: */ 0x15227d80, 0x10020c27, /* mov vpm, ra8 ; nop */
            /* 0x000000b0: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x000000b8: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x000000c0: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x000000c8: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x000000d0: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

#elif CUBETEST_CVFS

        UINT coordinateShader[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x00601a00, 0xe0020c67, /* ldi vr_setup, 0x00601a00 */
            /* 0x00000008: */ 0x15c27d80, 0x10020027, /* mov ra0, vpm ; nop */
            /* 0x00000010: */ 0x15c27d80, 0x10020067, /* mov ra1, vpm ; nop */
            /* 0x00000018: */ 0x15c27d80, 0x100200a7, /* mov ra2, vpm ; nop */
            /* 0x00000020: */ 0x15c27d80, 0x100200e7, /* mov ra3, vpm ; nop */
            /* 0x00000028: */ 0x15c27d80, 0x10020127, /* mov ra4, vpm ; nop */
            /* 0x00000030: */ 0x15c27d80, 0x10020167, /* mov ra5, vpm ; nop */
            /* 0x00000038: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000040: */ 0x15027d80, 0x10020867, /* mov r1, ra0  ; nop */
            /* 0x00000048: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000050: */ 0x079e7000, 0x10120227, /* ftoi ra8.16a, r0 ; nop */
            /* 0x00000058: */ 0x15827d80, 0x100208a7, /* mov r2, unif ; nop */
            /* 0x00000060: */ 0x15067d80, 0x10020867, /* mov r1, ra1  ; nop */
            /* 0x00000068: */ 0x209e700a, 0x100049e0, /* nop ; fmul r0, r1, r2 */
            /* 0x00000070: */ 0x079e7000, 0x10220227, /* ftoi ra8.16b, r0 ; nop */
            /* 0x00000078: */ 0x00001a00, 0xe0021c67, /* ldi vw_setup, 0x00001a00 */
            /* 0x00000080: */ 0x15027d80, 0x10020c27, /* mov vpm, ra0 */
            /* 0x00000088: */ 0x15067d80, 0x10020c27, /* mov vpm, ra1 */
            /* 0x00000090: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 */
            /* 0x00000098: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 */
            /* 0x000000a0: */ 0x15227d80, 0x10020c27, /* mov vpm, ra8 ; nop */
            /* 0x000000a8: */ 0x150a7d80, 0x10020c27, /* mov vpm, ra2 ; nop */
            /* 0x000000b0: */ 0x150e7d80, 0x10020c27, /* mov vpm, ra3 ; nop */
            /* 0x000000b8: */ 0x009e7000, 0x300009e7, /* nop ; nop ; thrend */
            /* 0x000000c0: */ 0x009e7000, 0x100009e7, /* nop ; nop */
            /* 0x000000c8: */ 0x009e7000, 0x100009e7, /* nop ; nop */
        };

#else

#endif

#if CUBETEST_CVFS

#if 1
     ULONGLONG PS[] = {
            0x10020827158e7d80, //        ; mov r0, varying ; nop          // pm = 0, sf = 0, ws = 0
            0x10020867158e7d80, //        ; mov r1, varying ; nop          // pm = 0, sf = 0, ws = 0
            0x400208a7019e7140, // sbwait ; fadd r2, r0, r5 ; nop          // pm = 0, sf = 0, ws = 0
            0x100208e7019e7340, //        ; fadd r3, r1, r5 ; nop          // pm = 0, sf = 0, ws = 0
            0x10020e67159e7480, //        ; mov tmu0_t, r2 ; nop   // pm = 0, sf = 0, ws = 0
            0x10020e27159e76c0, //        ; mov tmu0_s, r3 ; nop   // pm = 0, sf = 0, ws = 0
            0xa00009e7009e7000, // ldtmu0 ; nop  ; nop
            0x30020ba7159e7900, // thrend ; mov tlb_colour, r4 ; nop       // pm = 0, sf = 0, ws = 0
            0x100009e7009e7000, //        ; nop  ; nop
            0x500009e7009e7000, // sbdone ; nop  ; nop
    };
#else
        DWORD PS[] =
        {
            /* Assembled Program */
            /* 0x00000000: */ 0x158e7d80, 0x10020827, /* mov r0, vary    ; nop */
            /* 0x00000008: */ 0x818e7176, 0x10024821, /* fadd r0, r0, r5 ; mov r1, vary */
            /* 0x00000010: */ 0x019e7340, 0x40020867, /* sbwait    ; fadd r1, r1, r5 ; nop */
            /* 0x00000018: */ 0x159e7000, 0x10020e67, /* mov t0t, r0     ; nop */
            /* 0x00000020: */ 0x159e7240, 0x10020e27, /* mov t0s, r1     ; nop */
            /* 0x00000028: */ 0x009e7000, 0xa00009e7, /* ldtmu0          ; nop */
            /* 0x00000030: */ 0x159e7900, 0x19020827, /* mov r0, r4.8a   ; nop */
            /* 0x00000038: */ 0x35827906, 0x1b624860, /* mov r1, r4.8b   ; fmul r0.8c, r0, unif */
            /* 0x00000040: */ 0x3582790e, 0x1d5248a0, /* mov r2, r4.8c   ; fmul r0.8b, r1, unif */
            /* 0x00000048: */ 0x20827016, 0x114049e0, /* nop             ; fmul r0.8a, r2, unif */
            /* 0x00000050: */ 0x809e003f, 0xd17049e0, /* nop             ; mov r0.8d, 1.0 */
            /* 0x00000058: */ 0x159cffc0, 0x10020b27, /* mov tlbz, rb15  ; nop */
            /* 0x00000060: */ 0x159e7000, 0x30020ba7, /* thrend    ; mov tlbc, r0    ; nop */
            /* 0x00000068: */ 0x009e7000, 0x100009e7, /* nop             ; nop */
            /* 0x00000070: */ 0x009e7000, 0x500009e7, /* sbdone    ; nop             ; nop */
        };
#endif

#else

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

#endif

#endif // 0


