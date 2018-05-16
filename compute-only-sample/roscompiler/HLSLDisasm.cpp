#include "precomp.h"
#include "roscompiler.h"

//----------------------------------------------------------------------------
// Misc functions
//----------------------------------------------------------------------------
HRESULT PrintComponent(HLSLDisasm *pCtx, D3D10_SB_4_COMPONENT_NAME CompName)
{
    switch(CompName)
    {
        case D3D10_SB_4_COMPONENT_X: pCtx->xprintf(TEXT("x"));break;
        case D3D10_SB_4_COMPONENT_Y: pCtx->xprintf(TEXT("y"));break;
        case D3D10_SB_4_COMPONENT_Z: pCtx->xprintf(TEXT("z"));break;
        case D3D10_SB_4_COMPONENT_W: pCtx->xprintf(TEXT("w"));break;
        default: return E_FAIL;
    }
    return S_OK;
}

HRESULT PrintRegister(HLSLDisasm *pCtx, D3D10_SB_OPERAND_TYPE Type)
{
    switch(Type)
    {
        case D3D10_SB_OPERAND_TYPE_TEMP:                       pCtx->xprintf(TEXT("r")); break;
        case D3D10_SB_OPERAND_TYPE_INPUT:                      pCtx->xprintf(TEXT("v")); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT:                     pCtx->xprintf(TEXT("o")); break;
        case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP:             pCtx->xprintf(TEXT("x")); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:                pCtx->xprintf(TEXT("i32")); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:                pCtx->xprintf(TEXT("i64")); break;
        case D3D10_SB_OPERAND_TYPE_SAMPLER:                    pCtx->xprintf(TEXT("s")); break;
        case D3D10_SB_OPERAND_TYPE_RESOURCE:                   pCtx->xprintf(TEXT("t")); break;
        case D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER:            pCtx->xprintf(TEXT("cb")); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER:  pCtx->xprintf(TEXT("icb")); break;
        case D3D10_SB_OPERAND_TYPE_LABEL:                      pCtx->xprintf(TEXT("l")); break;
        case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:          pCtx->xprintf(TEXT("primID")); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:               pCtx->xprintf(TEXT("oDepth")); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:         pCtx->xprintf(TEXT("oStencilRef")); break;
        case D3D10_SB_OPERAND_TYPE_NULL:                       pCtx->xprintf(TEXT("null")); break;
        case D3D11_SB_OPERAND_TYPE_STREAM:                     pCtx->xprintf(TEXT("stream")); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_BODY:              pCtx->xprintf(TEXT("fb")); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_TABLE:             pCtx->xprintf(TEXT("ft")); break;
        case D3D11_SB_OPERAND_TYPE_INTERFACE:                  pCtx->xprintf(TEXT("fp")); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_INPUT:             pCtx->xprintf(TEXT("f_input")); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_OUTPUT:            pCtx->xprintf(TEXT("f_output")); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:    pCtx->xprintf(TEXT("out_cpid")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID:     pCtx->xprintf(TEXT("in_fork_instID")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID:     pCtx->xprintf(TEXT("in_join_instID")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT:        pCtx->xprintf(TEXT("vicp")); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT:       pCtx->xprintf(TEXT("vocp")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:       pCtx->xprintf(TEXT("in_patch_const")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT:         pCtx->xprintf(TEXT("domainpt")); break;
        case D3D11_SB_OPERAND_TYPE_THIS_POINTER:               pCtx->xprintf(TEXT("this")); break;
        case D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW:      pCtx->xprintf(TEXT("uav")); break;
        case D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY: pCtx->xprintf(TEXT("tgsm")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID:            pCtx->xprintf(TEXT("thread_id")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:      pCtx->xprintf(TEXT("thread_gid")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP:   pCtx->xprintf(TEXT("thread_idgid")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK:        pCtx->xprintf(TEXT("in_covMask")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED: pCtx->xprintf(TEXT("thread_id_flatGroup")); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:       pCtx->xprintf(TEXT("gs_instId")); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL: pCtx->xprintf(TEXT("out_depthGE")); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:    pCtx->xprintf(TEXT("out_depthLE")); break;
        case D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER:              pCtx->xprintf(TEXT("cycleCounter")); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK:       pCtx->xprintf(TEXT("coverageMask")); break;
        case D3D10_SB_OPERAND_TYPE_RASTERIZER:                 pCtx->xprintf(TEXT("rasterizer")); break;
        case D3D11_SB_OPERAND_TYPE_INNER_COVERAGE:             pCtx->xprintf(TEXT("innerCoverage")); break;
        default:
            return E_FAIL;
    }
    return S_OK;
}

HRESULT PrintOperandD3D10(HLSLDisasm *pCtx, COperandBase *pOperand, BOOL bFloat, BOOL bAmbiguous)
{
    HRESULT hr = S_OK;

    if ( pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_NEG ||
         pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG )
    {
        pCtx->xprintf(TEXT("-"));
    }

    if ( pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
         pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG )
    {
        pCtx->xprintf(TEXT("|"));
    }

    hr = PrintRegister( pCtx,pOperand->m_Type );
    if ( FAILED( hr ) )
    {
        return hr;
    }

    if ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_IMMEDIATE32 &&
         pOperand->m_Type != D3D10_SB_OPERAND_TYPE_IMMEDIATE64 )
    {
        if ( pOperand->m_IndexDimension == 1 &&
             pOperand->m_Index[0].m_RelRegType == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 )
        {
                pCtx->xprintf(TEXT("%d"), pOperand->m_Index[0].m_RegIndex);
        }
        else
        {
            //XXX I don't think this is right
            for ( UINT i = 0; i < (UINT)pOperand->m_IndexDimension && i < 3; i++ )
            {
                if ( pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32)
                {
                    if ( ( i == 0 ) &&
                         ( ( ( 1 == pOperand->m_IndexDimension ) ||
                             ( ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_INPUT ) &&
                               ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_OUTPUT ) &&
                               ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT) &&
                               ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT)
                             )
                           ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_THIS_POINTER )
                         ) ||
                         ( ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_INPUT )&&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_OUTPUT )&&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW) &&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_RESOURCE) &&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_INTERFACE ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_THIS_POINTER ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT )
                         )
                       )
                    {
                        pCtx->xprintf(TEXT("%d"), pOperand->m_Index[i].m_RegIndex );
                    }
                    else
                    {
                        pCtx->xprintf(TEXT("[%d]"), pOperand->m_Index[i].m_RegIndex);
                    }
                }

                if ( pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_RELATIVE ||
                     pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE )
                {
                    pCtx->xprintf(TEXT("["));

                    hr = PrintRegister( pCtx, pOperand->m_Index[i].m_RelRegType );
                    if ( FAILED( hr ) )
                    {
                        return hr;
                    }

                    if ( 2 == pOperand->m_Index[i].m_IndexDimension )
                    {
                        //reminder we should disallow D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER as indices
                        assert(pOperand->m_Index[i].m_RelRegType != D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);

                        pCtx->xprintf(TEXT("_%d_%d."), pOperand->m_Index[i].m_RelIndex1, pOperand->m_Index[i].m_RelIndex);
                        PrintComponent(pCtx, pOperand->m_Index[i].m_ComponentName);
                        pCtx->xprintf(TEXT(" + %d]"), pOperand->m_Index[i].m_RegIndex );
                    }
                    else
                    {
                        pCtx->xprintf(TEXT("%d."), pOperand->m_Index[i].m_RelIndex);
                        PrintComponent(pCtx, pOperand->m_Index[i].m_ComponentName);
                        pCtx->xprintf(TEXT(" + %d]"), pOperand->m_Index[i].m_RegIndex );
                    }
                }
            }
        }

        if ( pOperand->m_NumComponents == D3D10_SB_OPERAND_0_COMPONENT ||
             pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT )
        {
            //--
        }
        else if (pOperand->m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE)
        {
            if (pOperand->m_WriteMask != D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL)
            {
                pCtx->xprintf(TEXT("."));
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X)
                    pCtx->xprintf(TEXT("x"));
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y)
                    pCtx->xprintf(TEXT("y"));
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z)
                    pCtx->xprintf(TEXT("z"));
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W)
                    pCtx->xprintf(TEXT("w"));
            }
        }
        else if (pOperand->m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE)
        {
            if ((D3D10_SB_4_COMPONENT_X != pOperand->m_Swizzle[0]) ||
                (D3D10_SB_4_COMPONENT_Y != pOperand->m_Swizzle[1]) ||
                (D3D10_SB_4_COMPONENT_Z != pOperand->m_Swizzle[2]) ||
                (D3D10_SB_4_COMPONENT_W != pOperand->m_Swizzle[3]))
            {
                pCtx->xprintf(TEXT("."));
                for (UINT i = 0; i < 4; i++)
                {
                    PrintComponent(pCtx, (D3D10_SB_4_COMPONENT_NAME)pOperand->m_Swizzle[i]);
                }
            }
        }
        else if(pOperand->m_Type != D3D10_SB_OPERAND_TYPE_RESOURCE )
        {
            pCtx->xprintf(TEXT("."));
            PrintComponent(pCtx, pOperand->m_ComponentName);
        }
    }
    else
    {
        pCtx->SetColor(COLOR_LITERAL);
        if (bAmbiguous)
        {
            if (pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
            {
                UINT uVal = pOperand->m_Value[0] & 0x7F800000;
                if( pOperand->m_Value[0] == 0x80000000 )
                    bFloat = TRUE;
                else if(uVal == 0x7F800000 || uVal == 0)
                    bFloat = FALSE;
                else
                    bFloat = TRUE;

            }
            else
            {
                bFloat = FALSE;
                for( UINT i = 0; i < 4; i++ )
                {
                    UINT uVal = pOperand->m_Value[i] & 0x7F800000;
                    if( pOperand->m_Value[i] == 0x80000000 )
                        bFloat = TRUE;
                    if(uVal != 0x7F800000 && uVal != 0)
                        bFloat = TRUE;

                }
            }
        }

        if (bFloat)
        {
            if(pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
                pCtx->xprintf(TEXT("(%f)"),pOperand->m_Valuef[0]);
            else
                pCtx->xprintf(TEXT("(%f, %f, %f, %f)"), pOperand->m_Valuef[0], pOperand->m_Valuef[1], pOperand->m_Valuef[2], pOperand->m_Valuef[3]);
        }
        else
        {
            if(pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
            {
                if(abs((INT)pOperand->m_Value[0]) > 10000)
                    pCtx->xprintf(TEXT("(0x%08x)"),pOperand->m_Value[0]);
                else
                    pCtx->xprintf(TEXT("(%d)"),pOperand->m_Value[0]);
            }
            else
            {
                pCtx->xprintf(TEXT("("));
                for(UINT i = 0;i < 4; i++)
                {
                    if(abs((INT)pOperand->m_Value[i]) > 10000)
                        pCtx->xprintf(TEXT("0x%08x"),pOperand->m_Value[i]);
                    else
                        pCtx->xprintf(TEXT("%d"),pOperand->m_Value[i]);

                    if(i != 3)
                        pCtx->xprintf(TEXT(", "));
                }
                pCtx->xprintf(TEXT(")"));
            }
        }
        pCtx->UnsetColor();
    }

    if (pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
        pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG)
    {
        pCtx->xprintf(TEXT("|"));
    }

    if (pOperand->m_MinPrecision != D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
    {
        switch(pOperand->m_MinPrecision)
        {
            case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16:
                pCtx->xprintf(TEXT("[f16]"));
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8:
                pCtx->xprintf(TEXT("[f2_8]"));
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
                pCtx->xprintf(TEXT("[sint16]"));
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
                pCtx->xprintf(TEXT("[uint16]"));
                break;
        }
    }

    return S_OK;
}

static HRESULT PrintName(HLSLDisasm *pCtx, UINT uName)
{
    switch(uName)
    {
        case D3D10_SB_NAME_UNDEFINED:                           pCtx->xprintf(TEXT("undefined")); break;
        case D3D10_SB_NAME_CLIP_DISTANCE:                       pCtx->xprintf(TEXT("clip_distance")); break;
        case D3D10_SB_NAME_CULL_DISTANCE:                       pCtx->xprintf(TEXT("cull_distance")); break;
        case D3D10_SB_NAME_POSITION:                            pCtx->xprintf(TEXT("position")); break;
        case D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX:           pCtx->xprintf(TEXT("rendertarget_array_index")); break;
        case D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX:                pCtx->xprintf(TEXT("viewport_array_index")); break;
        case D3D10_SB_NAME_VERTEX_ID:                           pCtx->xprintf(TEXT("vertex_id")); break;
        case D3D10_SB_NAME_PRIMITIVE_ID:                        pCtx->xprintf(TEXT("primitive_id")); break;
        case D3D10_SB_NAME_INSTANCE_ID:                         pCtx->xprintf(TEXT("instance_id")); break;
        case D3D10_SB_NAME_IS_FRONT_FACE:                       pCtx->xprintf(TEXT("is_front_face")); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR:   pCtx->xprintf(TEXT("final_quad_ueq0")); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR:   pCtx->xprintf(TEXT("final_quad_veq0")); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR:   pCtx->xprintf(TEXT("final_quad_ueq1")); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR:   pCtx->xprintf(TEXT("final_quad_veq1")); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR:      pCtx->xprintf(TEXT("final_quad_uinside")); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR:      pCtx->xprintf(TEXT("final_quad_vinside")); break;
        case D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf(TEXT("final_tri_ueq0")); break;
        case D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf(TEXT("final_tri_veq0")); break;
        case D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf(TEXT("final_tri_weq0")); break;
        case D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR:         pCtx->xprintf(TEXT("final_tri_inside")); break;
        case D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR:        pCtx->xprintf(TEXT("final_line_detail")); break;
        case D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR:       pCtx->xprintf(TEXT("final_line_density")); break;
        default:
            return E_FAIL;
    }

    return S_OK;
}

static HRESULT PrintMask(HLSLDisasm *pCtx, BYTE cMask)
{
    if (cMask & 0x1) pCtx->xprintf(TEXT("x"));
    if (cMask & 0x2) pCtx->xprintf(TEXT("y"));
    if (cMask & 0x4) pCtx->xprintf(TEXT("z"));
    if (cMask & 0x8) pCtx->xprintf(TEXT("w"));
    return S_OK;
}

static HRESULT PrintRegisterComponentType(HLSLDisasm *pCtx, UINT uType)
{
    switch (uType)
    {
    case D3D10_SB_REGISTER_COMPONENT_UNKNOWN: pCtx->xprintf(TEXT("")); break;
    case D3D10_SB_REGISTER_COMPONENT_UINT32:  pCtx->xprintf(TEXT("reg component - uint")); break;
    case D3D10_SB_REGISTER_COMPONENT_SINT32:  pCtx->xprintf(TEXT("reg component - sint")); break;
    case D3D10_SB_REGISTER_COMPONENT_FLOAT32: pCtx->xprintf(TEXT("reg component - float")); break;
    default:
        return E_FAIL;
    }
    return S_OK;
}

static HRESULT PrintMinPrecision(HLSLDisasm *pCtx, UINT uPrecision)
{
    switch (uPrecision)
    {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT: break;
    case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16: pCtx->xprintf(TEXT("\t : ")); pCtx->xprintf(TEXT("min precision - float16")); break;
    case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8: pCtx->xprintf(TEXT("\t : ")); pCtx->xprintf(TEXT("min precision - float10")); break;
    case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16: pCtx->xprintf(TEXT("\t : ")); pCtx->xprintf(TEXT("min precision - sint16")); break;
    case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16: pCtx->xprintf(TEXT("\t : ")); pCtx->xprintf(TEXT("min precision - uint16")); break;
    default:
        return E_FAIL;
    }
    return S_OK;
}

HRESULT
PrintSignatureEntry(HLSLDisasm *pCtx, const D3D11_1DDIARG_SIGNATURE_ENTRY *pEntry)
{
    assert(pCtx);
    assert(pEntry);

    pCtx->xprintf(TEXT("    Register %d."), pEntry->Register);
    PrintMask(pCtx, pEntry->Mask);
    pCtx->xprintf(TEXT("\t : "));
    PrintName(pCtx, pEntry->SystemValue);
    PrintRegisterComponentType(pCtx, pEntry->RegisterComponentType);
    PrintMinPrecision(pCtx, pEntry->MinPrecision);

    pCtx->Flush(0);
    return S_OK;
}

HRESULT
HLSLDisasm::Run(const TCHAR *pTitile, const D3D11_1DDIARG_SIGNATURE_ENTRY *pEntry, UINT numEntries)
{
    if (numEntries)
    {
        this->xprintf(TEXT("%s\n"), pTitile);
    }
    for (UINT i = 0; i < numEntries; i++)
    {
        PrintSignatureEntry(this, pEntry); pEntry++;
    }
    return S_OK;
}

HRESULT
HLSLDisasm::Run(const UINT * pShader)
{
    HRESULT hr = S_OK;
    CShaderCodeParser Parser;

    Parser.SetShader(pShader);
    CInstruction Instruction;

    INT uIndent = 0;

    // print out target
    D3D10_SB_TOKENIZED_PROGRAM_TYPE uShaderType = Parser.ShaderType();
    UINT uMajorVersion = Parser.ShaderMajorVersion();
    UINT uMinorVersion = Parser.ShaderMinorVersion();

    for (INT i = 0; i < uIndent; i++)
    {
        this->xprintf(TEXT("  "));
    }

    switch(uShaderType)
    {
        case D3D10_SB_PIXEL_SHADER:
            this->xprintf(TEXT("ps_%d_%d"), uMajorVersion, uMinorVersion);
            break;
        case D3D10_SB_VERTEX_SHADER:
            this->xprintf(TEXT("vs_%d_%d"), uMajorVersion, uMinorVersion);
            break;
        case D3D10_SB_GEOMETRY_SHADER:
            this->xprintf(TEXT("gs_%d_%d"), uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_COMPUTE_SHADER:
            this->xprintf(TEXT("cs_%d_%d"), uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_HULL_SHADER:
            this->xprintf(TEXT("hs_%d_%d"), uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_DOMAIN_SHADER:
            this->xprintf(TEXT("ds_%d_%d"), uMajorVersion, uMinorVersion);
            break;
        default:
            hr = E_FAIL;
            goto Cleanup;
    }

    this->Flush(0);

    int line = 0;

    while (!Parser.EndOfShader())
    {
        IFC (Parser.ParseInstruction(&Instruction));

        switch (Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_ENDIF:
        case D3D10_SB_OPCODE_ENDLOOP:
        case D3D10_SB_OPCODE_ELSE:
        case D3D10_SB_OPCODE_ENDSWITCH:
              uIndent--;
              break;
        }

        for (INT i = 0; i < uIndent; i++)
        {
            this->xprintf(TEXT("  "));
        }

        //print opcode, validate first
        if (Instruction.m_OpCode >= D3D10_SB_NUM_OPCODES)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

        if ((g_InstructionInfo[Instruction.m_OpCode].m_NumSrcOperands +
             g_InstructionInfo[Instruction.m_OpCode].m_NumDstOperands ) !=
             (int)Instruction.m_NumOperands)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

        this->SetColor(COLOR_KEYWORD);

        this->xprintf(TEXT("%s"), g_InstructionInfo[Instruction.m_OpCode].m_Name);

        if (Instruction.m_bSaturate)
        {
            switch (Instruction.m_OpCode)
            {
            case D3D10_SB_OPCODE_DCL_INPUT:
            case D3D10_SB_OPCODE_DCL_OUTPUT:
            case D3D10_SB_OPCODE_DCL_INPUT_SGV:
            case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
            case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
            case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
            case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
            case D3D10_SB_OPCODE_DCL_INPUT_PS:
            case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            case D3D10_SB_OPCODE_DCL_SAMPLER:
            case D3D10_SB_OPCODE_DCL_RESOURCE:
            case D3D10_SB_OPCODE_DCL_INPUT_SIV:
            case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
            case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
            case D3D10_SB_OPCODE_DCL_TEMPS:
            case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
            case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:

            case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
            case D3D11_SB_OPCODE_DCL_STREAM:
            case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
            case D3D11_SB_OPCODE_DCL_INTERFACE:

            case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
            case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
            case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
            case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
            case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
            case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:
            case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
            case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:

            case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
            case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
            case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
            case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
            case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
            case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
            case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
            case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
            case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
                break;
            default:
                this->xprintf(TEXT("_sat"));
            }
        }

        switch (Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_DISCARD:
        case D3D10_SB_OPCODE_IF:
        case D3D10_SB_OPCODE_CONTINUEC:
        case D3D10_SB_OPCODE_BREAKC:
        case D3D10_SB_OPCODE_CALLC:
        case D3D10_SB_OPCODE_RETC:
            switch (Instruction.m_Test)
            {
            case D3D10_SB_INSTRUCTION_TEST_ZERO:
                this->xprintf(TEXT("_z"));
                break;

            case D3D10_SB_INSTRUCTION_TEST_NONZERO:
                this->xprintf(TEXT("_nz"));
                break;
            }
            break;
        }

        this->UnsetColor();

        BOOL bOperands = TRUE;

        if (Instruction.m_bExtended)
        {
            switch (Instruction.m_OpCodeEx[0])
            {
            case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS:
                this->xprintf(TEXT("_O(%i,%i,%i) "),
                    Instruction.m_TexelOffset[0],
                    Instruction.m_TexelOffset[1],
                    Instruction.m_TexelOffset[2]);
                break;
            }
        }

        // special things to do
        switch (Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_RESINFO:
            switch (Instruction.m_ResInfoReturnType)
            {
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_RCPFLOAT:
                    this->xprintf(TEXT("_rcpfloat "));
                    break;
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_UINT:
                    this->xprintf(TEXT("_uint "));
                    break;
                default:
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_FLOAT:
                    this->xprintf(TEXT(" "));
                    break;
            }
            break;
        case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
            {
                int count = 0;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X)
                    count++;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y)
                    count++;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z)
                    count++;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W)
                    count++;

                this->xprintf(TEXT(" x%i[%i], %d"), Instruction.m_IndexableTempDecl.IndexableTempNumber,
                                                    Instruction.m_IndexableTempDecl.NumRegisters,
                                                    count);
                bOperands = FALSE;
                break;
            }
        case D3D10_SB_OPCODE_DCL_INDEX_RANGE:

            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));

            this->xprintf(TEXT(" %i"), Instruction.m_IndexRangeDecl.RegCount);
            bOperands = FALSE;

            break;
        case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
            {
                bool hasFlags = false;
                this->xprintf(TEXT(" "));
                if( Instruction.m_GlobalFlagsDecl.Flags & D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED )
                {
                    hasFlags = true;
                    this->xprintf(TEXT("refactoringAllowed "));
                }

                if( Instruction.m_GlobalFlagsDecl.Flags & D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL )
                {
                    hasFlags = true;
                    this->xprintf(TEXT("forceEarlyDepthStencil "));
                }

                if(!hasFlags)
                {
                    this->xprintf(TEXT("none "));
                }
                bOperands = FALSE;
            }
            break;
        case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
            switch(Instruction.m_OutputTopologyDecl.Topology)
            {
            case D3D10_PRIMITIVE_TOPOLOGY_POINTLIST: this->xprintf(TEXT(" pointlist"));break;
            case D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP: this->xprintf(TEXT(" linestrip")); break;
            case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: this->xprintf(TEXT(" triangestrip")); break;
            case D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED: this->xprintf(TEXT(" undefined")); break;
            default: this->xprintf(TEXT(" ???")); break;
            }
            break;
        case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
            this->xprintf(TEXT(" %u"), Instruction.m_GSMaxOutputVertexCountDecl.MaxOutputVertexCount); break;
        case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
            switch(Instruction.m_InputPrimitiveDecl.Primitive)
            {
            case D3D10_PRIMITIVE_POINT: this->xprintf(TEXT(" point")); break;
            case D3D10_PRIMITIVE_LINE: this->xprintf(TEXT(" line")); break;
            case D3D10_PRIMITIVE_TRIANGLE: this->xprintf(TEXT(" triangle")); break;
            case D3D10_PRIMITIVE_LINE_ADJ: this->xprintf(TEXT(" lineadj")); break;
            case D3D10_PRIMITIVE_TRIANGLE_ADJ: this->xprintf(TEXT(" triangleadj")); break;
            case D3D11_SB_PRIMITIVE_1_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint1")); break;
            case D3D11_SB_PRIMITIVE_2_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint2")); break;
            case D3D11_SB_PRIMITIVE_3_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint3")); break;
            case D3D11_SB_PRIMITIVE_4_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint4")); break;
            case D3D11_SB_PRIMITIVE_5_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint5")); break;
            case D3D11_SB_PRIMITIVE_6_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint6")); break;
            case D3D11_SB_PRIMITIVE_7_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint7")); break;
            case D3D11_SB_PRIMITIVE_8_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint8")); break;
            case D3D11_SB_PRIMITIVE_9_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint9")); break;
            case D3D11_SB_PRIMITIVE_10_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint10")); break;
            case D3D11_SB_PRIMITIVE_11_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint11")); break;
            case D3D11_SB_PRIMITIVE_12_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint12")); break;
            case D3D11_SB_PRIMITIVE_13_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint13")); break;
            case D3D11_SB_PRIMITIVE_14_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint14")); break;
            case D3D11_SB_PRIMITIVE_15_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint15")); break;
            case D3D11_SB_PRIMITIVE_16_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint16")); break;
            case D3D11_SB_PRIMITIVE_17_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint17")); break;
            case D3D11_SB_PRIMITIVE_18_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint18")); break;
            case D3D11_SB_PRIMITIVE_19_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint19")); break;
            case D3D11_SB_PRIMITIVE_20_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint20")); break;
            case D3D11_SB_PRIMITIVE_21_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint21")); break;
            case D3D11_SB_PRIMITIVE_22_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint22")); break;
            case D3D11_SB_PRIMITIVE_23_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint23")); break;
            case D3D11_SB_PRIMITIVE_24_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint24")); break;
            case D3D11_SB_PRIMITIVE_25_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint25")); break;
            case D3D11_SB_PRIMITIVE_26_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint26")); break;
            case D3D11_SB_PRIMITIVE_27_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint27")); break;
            case D3D11_SB_PRIMITIVE_28_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint28")); break;
            case D3D11_SB_PRIMITIVE_29_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint29")); break;
            case D3D11_SB_PRIMITIVE_30_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint30")); break;
            case D3D11_SB_PRIMITIVE_31_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint31")); break;
            case D3D11_SB_PRIMITIVE_32_CONTROL_POINT_PATCH: this->xprintf(TEXT(" controlPoint32")); break;
            case D3D10_PRIMITIVE_UNDEFINED: this->xprintf(TEXT(" undefined")); break;
            default: this->xprintf(TEXT(" ???"));
            }
            break;

        case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
                this->xprintf(TEXT(" %i"), Instruction.m_HSForkPhaseDecl.InstanceCount);
                bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
                this->xprintf(TEXT(" %i"), Instruction.m_HSJoinPhaseDecl.InstanceCount);
                bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
            switch(Instruction.m_TessellatorDomainDecl.TessellatorDomain)
            {
            case D3D11_SB_TESSELLATOR_DOMAIN_ISOLINE: this->xprintf(TEXT(" isoline")); break;
            case D3D11_SB_TESSELLATOR_DOMAIN_TRI: this->xprintf(TEXT(" tri")); break;
            case D3D11_SB_TESSELLATOR_DOMAIN_QUAD: this->xprintf(TEXT(" quad")); break;
                default: this->xprintf(TEXT(" ???"));
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
            switch(Instruction.m_TessellatorOutputPrimitiveDecl.TessellatorOutputPrimitive)
            {
            case D3D11_SB_TESSELLATOR_OUTPUT_POINT: this->xprintf(TEXT(" point")); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_LINE: this->xprintf(TEXT(" line")); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CCW: this->xprintf(TEXT(" tri_ccw")); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CW: this->xprintf(TEXT(" tri_cw")); break;
                default: this->xprintf(TEXT(" ???"));
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
            switch(Instruction.m_TessellatorPartitioningDecl.TessellatorPartitioning)
            {
            case D3D11_SB_TESSELLATOR_PARTITIONING_POW2: this->xprintf(TEXT(" pow2")); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_INTEGER: this->xprintf(TEXT(" int")); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN: this->xprintf(TEXT(" fract-even")); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD: this->xprintf(TEXT(" fract-odd")); break;
                default: this->xprintf(TEXT(" ???"));
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
            this->xprintf(TEXT(" %i"), Instruction.m_InputControlPointCountDecl.InputControlPointCount);
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
            this->xprintf(TEXT(" %i"), Instruction.m_OutputControlPointCountDecl.OutputControlPointCount);
            bOperands = false;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));

            if(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV == Instruction.m_OpCode)
            {
                this->xprintf(TEXT(", "));
                PrintName(this, Instruction.m_OutputDeclSIV.Name);
            }

            switch(Instruction.m_InputPSDeclSIV.InterpolationMode)
            {
                case D3D10_SB_INTERPOLATION_UNDEFINED: this->xprintf(TEXT(", undefined")); break;
                case D3D10_SB_INTERPOLATION_CONSTANT:  this->xprintf(TEXT(", constant")); break;
                case D3D10_SB_INTERPOLATION_LINEAR: this->xprintf(TEXT(", linear")); break;
                case D3D10_SB_INTERPOLATION_LINEAR_CENTROID: this->xprintf(TEXT(", linear centroid")); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE: this->xprintf(TEXT(", linear noperspective")); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID: this->xprintf(TEXT(", linear noperspective centroid")); break;
                default : this->xprintf(TEXT(" ???")); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS:

            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));

            if(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV == Instruction.m_OpCode)
            {
                this->xprintf(TEXT(", "));
                PrintName(this, Instruction.m_OutputDeclSIV.Name);
            }

            switch(Instruction.m_InputPSDecl.InterpolationMode)
            {
                case D3D10_SB_INTERPOLATION_UNDEFINED: this->xprintf(TEXT(", undefined")); break;
                case D3D10_SB_INTERPOLATION_CONSTANT:  this->xprintf(TEXT(", constant")); break;
                case D3D10_SB_INTERPOLATION_LINEAR: this->xprintf(TEXT(", linear")); break;
                case D3D10_SB_INTERPOLATION_LINEAR_CENTROID: this->xprintf(TEXT(", linear centroid")); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE: this->xprintf(TEXT(", linear noperspective")); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID: this->xprintf(TEXT(", linear noperspective centroid")); break;
                default : this->xprintf(TEXT(" ???")); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(TEXT(", "));

            PrintName(this, Instruction.m_OutputDeclSGV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(TEXT(", "));

            PrintName(this, Instruction.m_OutputDeclSIV.Name);
            bOperands = FALSE;
            break;

       case D3D10_SB_OPCODE_DCL_INPUT_SIV:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(TEXT(", "));

            PrintName(this, Instruction.m_InputDeclSIV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(TEXT(", "));
            PrintName(this, Instruction.m_InputPSDeclSGV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_SGV:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(TEXT(", "));
            PrintName(this, Instruction.m_InputDeclSIV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT("cb%d[%u-%u], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                this->xprintf(TEXT("cb%d[%d]"), Instruction.m_Operands[0].m_Index[0].m_RegIndex, Instruction.m_Operands[0].m_Index[1].m_RegIndex);
            }

            if (Instruction.m_ResourceDecl.CBInfo.AccessPattern == D3D10_SB_CONSTANT_BUFFER_DYNAMIC_INDEXED)
                this->xprintf(TEXT(", dynamicIndexed"));
            else
                this->xprintf(TEXT(", immediateIndexed"));

            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_TEMPS:
            this->xprintf(TEXT(" %d"),  Instruction.m_TempsDecl.NumTemps);
            break;

        case D3D10_SB_OPCODE_DCL_RESOURCE:

            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT("t%d[%u-%u], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            switch( Instruction.m_ResourceDecl.SRVInfo.Dimension)
            {
                case D3D10_SB_RESOURCE_DIMENSION_BUFFER: this->xprintf(TEXT(", buffer")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D: this->xprintf(TEXT(", texture1d")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D: this->xprintf(TEXT(", texture2d")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS: this->xprintf(TEXT(", texture2dms")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D: this->xprintf(TEXT(", texture3d")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE: this->xprintf(TEXT(", texturecube")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY: this->xprintf(TEXT(", texture1darray")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY: this->xprintf(TEXT(", texture2darray")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY: this->xprintf(TEXT(", texture2dmsarray")); break;
            }

            if ((Instruction.m_ResourceDecl.SRVInfo.Dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS) ||
                (Instruction.m_ResourceDecl.SRVInfo.Dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY))
            {
                this->xprintf(TEXT("[%d]"), Instruction.m_ResourceDecl.SRVInfo.SampleCount);
            }

            if ((Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[1]) &&
                (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[2]) &&
                (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[3]))
            {
                switch (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0])
                {
                case D3D10_SB_RETURN_TYPE_MIXED: this->xprintf(TEXT(", mixed")); break;
                case D3D10_SB_RETURN_TYPE_UNORM: this->xprintf(TEXT(", unorm")); break;
                case D3D10_SB_RETURN_TYPE_SNORM: this->xprintf(TEXT(", snorm")); break;
                case D3D10_SB_RETURN_TYPE_SINT:  this->xprintf(TEXT(", sint")); break;
                case D3D10_SB_RETURN_TYPE_UINT:  this->xprintf(TEXT(", uint")); break;
                case D3D10_SB_RETURN_TYPE_FLOAT: this->xprintf(TEXT(", float")); break;
                }
            }
            else
            {
                this->xprintf(TEXT(", ("));
                for(UINT i = 0;i < 4; i++)
                {
                    switch (Instruction.m_ResourceDecl.SRVInfo.ReturnType[i])
                    {
                    case D3D10_SB_RETURN_TYPE_MIXED: this->xprintf(TEXT(" mixed")); break;
                    case D3D10_SB_RETURN_TYPE_UNORM: this->xprintf(TEXT(" unorm")); break;
                    case D3D10_SB_RETURN_TYPE_SNORM: this->xprintf(TEXT(" snorm")); break;
                    case D3D10_SB_RETURN_TYPE_SINT:  this->xprintf(TEXT(" sint")); break;
                    case D3D10_SB_RETURN_TYPE_UINT:  this->xprintf(TEXT(" uint")); break;
                    case D3D10_SB_RETURN_TYPE_FLOAT: this->xprintf(TEXT(" float")); break;
                    }
                    if(i != 3)
                        this->xprintf(TEXT(","));
                }
                this->xprintf(TEXT(" )"));
            }
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT("t%d[%u-%u], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT("t%d[%u-%u], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            this->xprintf(TEXT(", stride=%u"), Instruction.m_ResourceDecl.UAVInfo.Stride);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT(" uav%d[%d-%d], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT(" uav%d[%d-%d], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            this->xprintf(TEXT(", stride=%u"), Instruction.m_ResourceDecl.UAVInfo.Stride);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
            this->xprintf(TEXT(" "));

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(TEXT(" uav%d[%u-%u], space %d"), Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            switch (Instruction.m_ResourceDecl.UAVInfo.Dimension)
            {
                case D3D10_SB_RESOURCE_DIMENSION_BUFFER: this->xprintf(TEXT(", buffer")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D: this->xprintf(TEXT(", texture1d")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D: this->xprintf(TEXT(", texture2d")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS: this->xprintf(TEXT(", texture2dms")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D: this->xprintf(TEXT(", texture3d")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE: this->xprintf(TEXT(", texturecube")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY: this->xprintf(TEXT(", texture1darray")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY: this->xprintf(TEXT(", texture2darray")); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY: this->xprintf(TEXT(", texture2dmsarray")); break;
            }

            switch (Instruction.m_ResourceDecl.UAVInfo.Type)
            {
            case D3D10_SB_RETURN_TYPE_MIXED: this->xprintf(TEXT(", mixed")); break;
            case D3D10_SB_RETURN_TYPE_UNORM: this->xprintf(TEXT(", unorm")); break;
            case D3D10_SB_RETURN_TYPE_SNORM: this->xprintf(TEXT(", snorm")); break;
            case D3D10_SB_RETURN_TYPE_SINT:  this->xprintf(TEXT(", sint")); break;
            case D3D10_SB_RETURN_TYPE_UINT:  this->xprintf(TEXT(", uint")); break;
            case D3D10_SB_RETURN_TYPE_FLOAT: this->xprintf(TEXT(", float")); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_CUSTOMDATA:
            {
                switch (Instruction.m_CustomData.Type )
                {
                    case D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER:
                    {
                        this->SetColor(COLOR_KEYWORD);
                        this->xprintf(TEXT("dcl_immediateConstantBuffer"));
                        this->UnsetColor();
                        this->xprintf(TEXT(" { "));
                        for (UINT uNumVecs = 0; uNumVecs < Instruction.m_CustomData.DataSizeInBytes / 16;)
                        {
                            this->xprintf(TEXT("{ "));
                            for(UINT i = 0;i < 4; i++)
                            {
                                UINT uVal;
                                float *pfVal = (float*)&uVal;
                                uVal = ((UINT*)Instruction.m_CustomData.pData)[uNumVecs*4 + i];
                                UINT uMasked = uVal & 0x7F800000;
                                if(uMasked == 0x7F800000 || uMasked == 0)
                                {
                                    if(abs((INT)uVal) > 10000)
                                        this->xprintf(TEXT("0x%08x"),uVal);
                                    else
                                        this->xprintf(TEXT("%d"),(INT)uVal);
                                } else
                                {
                                    this->xprintf(TEXT("%f"),*pfVal);
                                }

                                if(i != 3)
                                    this->xprintf(TEXT(", "));
                            }
                            this->xprintf(TEXT("}"));
                            uNumVecs++;
                            if (uNumVecs < Instruction.m_CustomData.DataSizeInBytes / 16)
                            {
                                this->xprintf(TEXT(",\n                              "));
                            }
                        }
                        this->xprintf(TEXT(" }"));
                    }
                    break;

                    case D3D10_SB_CUSTOMDATA_COMMENT:
                        this->xprintf(TEXT(" %d bytes of comment custom data\n"), Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D10_SB_CUSTOMDATA_DEBUGINFO:
                        this->xprintf(TEXT(" %d bytes of debuginfo custom data\n"), Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D10_SB_CUSTOMDATA_OPAQUE:
                        this->xprintf(TEXT(" %d bytes of opaque custom data\n"), Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D11_SB_CUSTOMDATA_SHADER_MESSAGE:
                        this->xprintf(TEXT(" %d bytes of shader message custom data\n"), Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    default:
                        this->xprintf(TEXT(" %d bytes of unknown custom data\n"), Instruction.m_CustomData.DataSizeInBytes);
                        break;
                }
            }
            break;

        case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
            this->xprintf(TEXT(" fb%u"), Instruction.m_FunctionBodyIdx);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
            this->xprintf(TEXT(" ft%u = {"), Instruction.m_FunctionTable.FtIdx);
            for(UINT i = 0; i < Instruction.m_FunctionTable.FbCount; i++)
            {
                this->xprintf(TEXT("fb%u"), Instruction.m_FunctionTable.pFbStartToken[i]);
                if(i < Instruction.m_FunctionTable.FbCount-1) this->xprintf(TEXT(", "));
            }
            this->xprintf(TEXT("}"));

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_INTERFACE:
            this->xprintf(TEXT(" fp%u[%u][%u] = {"), Instruction.m_InterfaceTable.FpIdx,
                Instruction.m_InterfaceTable.FpArraySize, Instruction.m_InterfaceTable.NumCallSites);
            for(UINT i = 0; i < Instruction.m_InterfaceTable.FtCount; i++)
            {
                this->xprintf(TEXT("ft%u"), Instruction.m_InterfaceTable.pFtStartToken[i]);
                if(i < Instruction.m_InterfaceTable.FtCount-1) this->xprintf(TEXT(", "));
            }
            this->xprintf(TEXT("}"));

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_INTERFACE_CALL:
            this->xprintf(TEXT(" "));
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], false, false));
            this->xprintf(TEXT("[%u]"), Instruction.m_InterfaceCallSiteIdx);

            bOperands = FALSE;
            break;

        default:
            this->xprintf(TEXT(" "));
            break;
        }

        if(bOperands)
        {
            BOOL bFloat;
            BOOL bAmbiguous = FALSE;
            switch(Instruction.m_OpCode)
            {
            case D3D10_SB_OPCODE_ADD          :
            case D3D10_SB_OPCODE_DERIV_RTX    :
            case D3D10_SB_OPCODE_DERIV_RTY    :
            case D3D10_SB_OPCODE_DIV          :
            case D3D10_SB_OPCODE_DP2          :
            case D3D10_SB_OPCODE_DP3          :
            case D3D10_SB_OPCODE_DP4          :
            case D3D10_SB_OPCODE_EQ           :
            case D3D10_SB_OPCODE_EXP          :
            case D3D10_SB_OPCODE_FRC          :
            case D3D10_SB_OPCODE_FTOI         :
            case D3D10_SB_OPCODE_FTOU         :
            case D3D10_SB_OPCODE_GE           :
//            case D3D10_SB_OPCODE_LABEL        :
            case D3D10_SB_OPCODE_LOG          :
            case D3D10_SB_OPCODE_LT           :
            case D3D10_SB_OPCODE_MAD          :
            case D3D10_SB_OPCODE_MIN          :
            case D3D10_SB_OPCODE_MAX          :
            case D3D10_SB_OPCODE_MUL          :
            case D3D10_SB_OPCODE_NE           :
            case D3D10_SB_OPCODE_ROUND_NE     :
            case D3D10_SB_OPCODE_ROUND_NI     :
            case D3D10_SB_OPCODE_ROUND_PI     :
            case D3D10_SB_OPCODE_ROUND_Z      :
            case D3D10_SB_OPCODE_RSQ          :
            case D3D10_SB_OPCODE_SAMPLE       :
            case D3D10_SB_OPCODE_SAMPLE_B     :
            case D3D10_SB_OPCODE_SAMPLE_C     :
            case D3D10_SB_OPCODE_SAMPLE_C_LZ  :
            case D3D10_SB_OPCODE_SAMPLE_L     :
            case D3D10_SB_OPCODE_SAMPLE_D     :
            case D3D10_SB_OPCODE_SQRT         :
//            case D3D10_SB_OPCODE_SUB          :
            case D3D10_SB_OPCODE_SINCOS       :
                bFloat = TRUE;
                break;

            case D3D10_SB_OPCODE_MOV          :

            case D3D10_SB_OPCODE_MOVC         :
                bAmbiguous = TRUE;
                bFloat = FALSE;
                break;

            default:
                bFloat = FALSE;
                break;
            }
            for(UINT i = 0; i < Instruction.m_NumOperands;i++)
            {

                IFC (PrintOperandD3D10(this, &Instruction.m_Operands[i], bFloat, bAmbiguous));

                if(i != Instruction.m_NumOperands-1)
                    this->xprintf(TEXT(", "));

            }
        }

        switch(Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_IF:
        case D3D10_SB_OPCODE_LOOP:
        case D3D10_SB_OPCODE_ELSE:
        case D3D10_SB_OPCODE_SWITCH:
              uIndent++;
              break;
        case D3D10_SB_OPCODE_DCL_SAMPLER:
            switch(Instruction.m_ResourceDecl.SamplerInfo.SamplerMode)
            {
                case D3D10_SB_SAMPLER_MODE_DEFAULT:
                    this->xprintf(TEXT(", mode_default"));
                    break;
                case D3D10_SB_SAMPLER_MODE_COMPARISON:
                    this->xprintf(TEXT(", mode_comparison"));
                    break;
                case D3D10_SB_SAMPLER_MODE_MONO:
                    this->xprintf(TEXT(", mode_mono"));
                    break;
            }
            break;
        }

        this->Flush(++line);
    }

Cleanup:
    RRETURN(hr);
}

