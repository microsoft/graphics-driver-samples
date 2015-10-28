#include "roscompiler.h"

//----------------------------------------------------------------------------
// Misc functions
//----------------------------------------------------------------------------
HRESULT PrintComponent(HLSLDisasm *pCtx, D3D10_SB_4_COMPONENT_NAME CompName)
{
    switch(CompName)
    {
        case D3D10_SB_4_COMPONENT_X: pCtx->xprintf("x");break;
        case D3D10_SB_4_COMPONENT_Y: pCtx->xprintf("y");break;
        case D3D10_SB_4_COMPONENT_Z: pCtx->xprintf("z");break;
        case D3D10_SB_4_COMPONENT_W: pCtx->xprintf("w");break;
        default: return E_FAIL;
    }
    return S_OK;
}

HRESULT PrintRegister(HLSLDisasm *pCtx, D3D10_SB_OPERAND_TYPE Type)
{
    switch(Type)
    {
        case D3D10_SB_OPERAND_TYPE_TEMP:                       pCtx->xprintf("r"); break;
        case D3D10_SB_OPERAND_TYPE_INPUT:                      pCtx->xprintf("v"); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT:                     pCtx->xprintf("o"); break;
        case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP:             pCtx->xprintf("x"); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:                pCtx->xprintf("i32"); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:                pCtx->xprintf("i64"); break;
        case D3D10_SB_OPERAND_TYPE_SAMPLER:                    pCtx->xprintf("s"); break;
        case D3D10_SB_OPERAND_TYPE_RESOURCE:                   pCtx->xprintf("t"); break;
        case D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER:            pCtx->xprintf("cb"); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER:  pCtx->xprintf("icb"); break;
        case D3D10_SB_OPERAND_TYPE_LABEL:                      pCtx->xprintf("l"); break;
        case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:          pCtx->xprintf("primID"); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:               pCtx->xprintf("oDepth"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:         pCtx->xprintf("oStencilRef"); break;
        case D3D10_SB_OPERAND_TYPE_NULL:                       pCtx->xprintf("null"); break;
        case D3D11_SB_OPERAND_TYPE_STREAM:                     pCtx->xprintf("stream"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_BODY:              pCtx->xprintf("fb"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_TABLE:             pCtx->xprintf("ft"); break;
        case D3D11_SB_OPERAND_TYPE_INTERFACE:                  pCtx->xprintf("fp"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_INPUT:             pCtx->xprintf("f_input"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_OUTPUT:            pCtx->xprintf("f_output"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:    pCtx->xprintf("out_cpid"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID:     pCtx->xprintf("in_fork_instID"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID:     pCtx->xprintf("in_join_instID"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT:        pCtx->xprintf("vicp"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT:       pCtx->xprintf("vocp"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:       pCtx->xprintf("in_patch_const"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT:         pCtx->xprintf("domainpt"); break;
        case D3D11_SB_OPERAND_TYPE_THIS_POINTER:               pCtx->xprintf("this"); break;
        case D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW:      pCtx->xprintf("uav"); break;
        case D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY: pCtx->xprintf("tgsm"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID:            pCtx->xprintf("thread_id"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:      pCtx->xprintf("thread_gid"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP:   pCtx->xprintf("thread_idgid"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK:        pCtx->xprintf("in_covMask"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED: pCtx->xprintf("thread_id_flatGroup"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:       pCtx->xprintf("gs_instId"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL: pCtx->xprintf("out_depthGE"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:    pCtx->xprintf("out_depthLE"); break;
        case D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER:              pCtx->xprintf("cycleCounter"); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK:       pCtx->xprintf("coverageMask"); break;
        case D3D10_SB_OPERAND_TYPE_RASTERIZER:                 pCtx->xprintf("rasterizer"); break;
        case D3D11_SB_OPERAND_TYPE_INNER_COVERAGE:             pCtx->xprintf("innerCoverage"); break;
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
        pCtx->xprintf("-");
    }

    if ( pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
         pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG )
    {
        pCtx->xprintf("|");
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
                pCtx->xprintf("%d", pOperand->m_Index[0].m_RegIndex);
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
                        pCtx->xprintf("%d", pOperand->m_Index[i].m_RegIndex );
                    }
                    else
                    {
                        pCtx->xprintf("[%d]", pOperand->m_Index[i].m_RegIndex);
                    }
                }

                if ( pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_RELATIVE ||
                     pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE )
                {
                    pCtx->xprintf("[");

                    hr = PrintRegister( pCtx, pOperand->m_Index[i].m_RelRegType );
                    if ( FAILED( hr ) )
                    {
                        return hr;
                    }

                    if ( 2 == pOperand->m_Index[i].m_IndexDimension )
                    {
                        //reminder we should disallow D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER as indices
                        assert(pOperand->m_Index[i].m_RelRegType != D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);

                        pCtx->xprintf("_%d_%d.", pOperand->m_Index[i].m_RelIndex1, pOperand->m_Index[i].m_RelIndex);
                        PrintComponent(pCtx, pOperand->m_Index[i].m_ComponentName);
                        pCtx->xprintf(" + %d]", pOperand->m_Index[i].m_RegIndex );
                    }
                    else
                    {
                        pCtx->xprintf("%d.", pOperand->m_Index[i].m_RelIndex);
                        PrintComponent(pCtx, pOperand->m_Index[i].m_ComponentName);
                        pCtx->xprintf(" + %d]", pOperand->m_Index[i].m_RegIndex );
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
                pCtx->xprintf(".");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X)
                    pCtx->xprintf("x");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y)
                    pCtx->xprintf("y");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z)
                    pCtx->xprintf("z");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W)
                    pCtx->xprintf("w");
            }
        }
        else if (pOperand->m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE)
        {
            if ( ( D3D10_SB_4_COMPONENT_X != pOperand->m_Swizzle[0] ) ||
                 ( D3D10_SB_4_COMPONENT_Y != pOperand->m_Swizzle[1] ) ||
                 ( D3D10_SB_4_COMPONENT_Z != pOperand->m_Swizzle[2] ) ||
                 ( D3D10_SB_4_COMPONENT_W != pOperand->m_Swizzle[3] ) )
            {
                pCtx->xprintf(".");
                PrintComponent(pCtx,(D3D10_SB_4_COMPONENT_NAME)pOperand->m_Swizzle[0]);
                for(UINT i = 1; i < 4; ++i)
                {
                    for(UINT j = i; j < 4; ++j)
                    {
                        if ( pOperand->m_Swizzle[j] != pOperand->m_Swizzle[i-1] )
                        {
                            PrintComponent(pCtx,(D3D10_SB_4_COMPONENT_NAME)pOperand->m_Swizzle[i]);
                            break;
                        }
                    }
                }
            }
        }
        else if(pOperand->m_Type != D3D10_SB_OPERAND_TYPE_RESOURCE )
        {
            pCtx->xprintf(".");
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
                pCtx->xprintf("(%f)",pOperand->m_Valuef[0]);
            else
                pCtx->xprintf("(%f, %f, %f, %f)", pOperand->m_Valuef[0], pOperand->m_Valuef[1], pOperand->m_Valuef[2], pOperand->m_Valuef[3]);
        }
        else
        {
            if(pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
            {
                if(abs((INT)pOperand->m_Value[0]) > 10000)
                    pCtx->xprintf("(0x%08x)",pOperand->m_Value[0]);
                else
                    pCtx->xprintf("(%d)",pOperand->m_Value[0]);
            }
            else
            {
                pCtx->xprintf("(");
                for(UINT i = 0;i < 4; i++)
                {
                    if(abs((INT)pOperand->m_Value[i]) > 10000)
                        pCtx->xprintf("0x%08x",pOperand->m_Value[i]);
                    else
                        pCtx->xprintf("%d",pOperand->m_Value[i]);

                    if(i != 3)
                        pCtx->xprintf(", ");
                }
                pCtx->xprintf(")");
            }
        }
        pCtx->UnsetColor();
    }

    if (pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
        pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG)
    {
        pCtx->xprintf("|");
    }

    if (pOperand->m_MinPrecision != D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
    {
        switch(pOperand->m_MinPrecision)
        {
            case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16:
                pCtx->xprintf("[f16]");
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8:
                pCtx->xprintf("[f2_8]");
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
                pCtx->xprintf("[sint16]");
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
                pCtx->xprintf("[uint16]");
                break;
        }
    }

    return S_OK;
}

static HRESULT PrintName(HLSLDisasm *pCtx, UINT uName)
{
    switch(uName)
    {
        case D3D10_SB_NAME_UNDEFINED:                           pCtx->xprintf("undefined"); break;
        case D3D10_SB_NAME_CLIP_DISTANCE:                       pCtx->xprintf("clip_distance"); break;
        case D3D10_SB_NAME_CULL_DISTANCE:                       pCtx->xprintf("cull_distance"); break;
        case D3D10_SB_NAME_POSITION:                            pCtx->xprintf("position"); break;
        case D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX:           pCtx->xprintf("rendertarget_array_index"); break;
        case D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX:                pCtx->xprintf("viewport_array_index"); break;
        case D3D10_SB_NAME_VERTEX_ID:                           pCtx->xprintf("vertex_id"); break;
        case D3D10_SB_NAME_PRIMITIVE_ID:                        pCtx->xprintf("primitive_id"); break;
        case D3D10_SB_NAME_INSTANCE_ID:                         pCtx->xprintf("instance_id"); break;
        case D3D10_SB_NAME_IS_FRONT_FACE:                       pCtx->xprintf("is_front_face"); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_ueq0"); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_veq0"); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_ueq1"); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_veq1"); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR:      pCtx->xprintf("final_quad_uinside"); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR:      pCtx->xprintf("final_quad_vinside"); break;
        case D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf("final_tri_ueq0"); break;
        case D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf("final_tri_veq0"); break;
        case D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf("final_tri_weq0"); break;
        case D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR:         pCtx->xprintf("final_tri_inside"); break;
        case D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR:        pCtx->xprintf("final_line_detail"); break;
        case D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR:       pCtx->xprintf("final_line_density"); break;
        default:
            return E_FAIL;
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
        this->xprintf("  ");
    }

    switch(uShaderType)
    {
        case D3D10_SB_PIXEL_SHADER:
            this->xprintf("ps_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D10_SB_VERTEX_SHADER:
            this->xprintf("vs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D10_SB_GEOMETRY_SHADER:
            this->xprintf("gs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_COMPUTE_SHADER:
            this->xprintf("cs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_HULL_SHADER:
            this->xprintf("hs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_DOMAIN_SHADER:
            this->xprintf("ds_%d_%d", uMajorVersion, uMinorVersion);
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
            this->xprintf("  ");
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

        this->xprintf("%s", g_InstructionInfo[Instruction.m_OpCode].m_Name);

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
                this->xprintf("_sat");
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
                this->xprintf("_z");
                break;

            case D3D10_SB_INSTRUCTION_TEST_NONZERO:
                this->xprintf("_nz");
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
                this->xprintf("_O(%i,%i,%i) ",
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
                    this->xprintf("_rcpfloat ");
                    break;
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_UINT:
                    this->xprintf("_uint ");
                    break;
                default:
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_FLOAT:
                    this->xprintf(" ");
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

                this->xprintf(" x%i[%i], %d", Instruction.m_IndexableTempDecl.IndexableTempNumber,
                                                 Instruction.m_IndexableTempDecl.NumRegisters,
                                                 count);
                bOperands = FALSE;
                break;
            }
        case D3D10_SB_OPCODE_DCL_INDEX_RANGE:

            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));

            this->xprintf(" %i", Instruction.m_IndexRangeDecl.RegCount);
            bOperands = FALSE;

            break;
        case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
            {
                bool hasFlags = false;
                this->xprintf(" ");
                if( Instruction.m_GlobalFlagsDecl.Flags & D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED )
                {
                    hasFlags = true;
                    this->xprintf("refactoringAllowed" );
                }

                if( Instruction.m_GlobalFlagsDecl.Flags & D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL )
                {
                    hasFlags = true;
                    this->xprintf("forceEarlyDepthStencil " );
                }

                if(!hasFlags)
                {
                    this->xprintf("none" );
                }
                bOperands = FALSE;
            }
            break;
        case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
            switch(Instruction.m_OutputTopologyDecl.Topology)
            {
            case D3D10_PRIMITIVE_TOPOLOGY_POINTLIST: this->xprintf(" pointlist");break;
            case D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP: this->xprintf(" linestrip"); break;
            case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: this->xprintf(" triangestrip"); break;
            case D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED: this->xprintf(" undefined"); break;
            default: this->xprintf(" ???"); break;
            }
            break;
        case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
            this->xprintf(" %u", Instruction.m_GSMaxOutputVertexCountDecl.MaxOutputVertexCount); break;
        case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
            switch(Instruction.m_InputPrimitiveDecl.Primitive)
            {
            case D3D10_PRIMITIVE_POINT: this->xprintf(" point"); break;
            case D3D10_PRIMITIVE_LINE: this->xprintf(" line"); break;
            case D3D10_PRIMITIVE_TRIANGLE: this->xprintf(" triangle"); break;
            case D3D10_PRIMITIVE_LINE_ADJ: this->xprintf(" lineadj"); break;
            case D3D10_PRIMITIVE_TRIANGLE_ADJ: this->xprintf(" triangleadj"); break;
            case D3D11_SB_PRIMITIVE_1_CONTROL_POINT_PATCH: this->xprintf(" controlPoint1"); break;
            case D3D11_SB_PRIMITIVE_2_CONTROL_POINT_PATCH: this->xprintf(" controlPoint2"); break;
            case D3D11_SB_PRIMITIVE_3_CONTROL_POINT_PATCH: this->xprintf(" controlPoint3"); break;
            case D3D11_SB_PRIMITIVE_4_CONTROL_POINT_PATCH: this->xprintf(" controlPoint4"); break;
            case D3D11_SB_PRIMITIVE_5_CONTROL_POINT_PATCH: this->xprintf(" controlPoint5"); break;
            case D3D11_SB_PRIMITIVE_6_CONTROL_POINT_PATCH: this->xprintf(" controlPoint6"); break;
            case D3D11_SB_PRIMITIVE_7_CONTROL_POINT_PATCH: this->xprintf(" controlPoint7"); break;
            case D3D11_SB_PRIMITIVE_8_CONTROL_POINT_PATCH: this->xprintf(" controlPoint8"); break;
            case D3D11_SB_PRIMITIVE_9_CONTROL_POINT_PATCH: this->xprintf(" controlPoint9"); break;
            case D3D11_SB_PRIMITIVE_10_CONTROL_POINT_PATCH: this->xprintf(" controlPoint10"); break;
            case D3D11_SB_PRIMITIVE_11_CONTROL_POINT_PATCH: this->xprintf(" controlPoint11"); break;
            case D3D11_SB_PRIMITIVE_12_CONTROL_POINT_PATCH: this->xprintf(" controlPoint12"); break;
            case D3D11_SB_PRIMITIVE_13_CONTROL_POINT_PATCH: this->xprintf(" controlPoint13"); break;
            case D3D11_SB_PRIMITIVE_14_CONTROL_POINT_PATCH: this->xprintf(" controlPoint14"); break;
            case D3D11_SB_PRIMITIVE_15_CONTROL_POINT_PATCH: this->xprintf(" controlPoint15"); break;
            case D3D11_SB_PRIMITIVE_16_CONTROL_POINT_PATCH: this->xprintf(" controlPoint16"); break;
            case D3D11_SB_PRIMITIVE_17_CONTROL_POINT_PATCH: this->xprintf(" controlPoint17"); break;
            case D3D11_SB_PRIMITIVE_18_CONTROL_POINT_PATCH: this->xprintf(" controlPoint18"); break;
            case D3D11_SB_PRIMITIVE_19_CONTROL_POINT_PATCH: this->xprintf(" controlPoint19"); break;
            case D3D11_SB_PRIMITIVE_20_CONTROL_POINT_PATCH: this->xprintf(" controlPoint20"); break;
            case D3D11_SB_PRIMITIVE_21_CONTROL_POINT_PATCH: this->xprintf(" controlPoint21"); break;
            case D3D11_SB_PRIMITIVE_22_CONTROL_POINT_PATCH: this->xprintf(" controlPoint22"); break;
            case D3D11_SB_PRIMITIVE_23_CONTROL_POINT_PATCH: this->xprintf(" controlPoint23"); break;
            case D3D11_SB_PRIMITIVE_24_CONTROL_POINT_PATCH: this->xprintf(" controlPoint24"); break;
            case D3D11_SB_PRIMITIVE_25_CONTROL_POINT_PATCH: this->xprintf(" controlPoint25"); break;
            case D3D11_SB_PRIMITIVE_26_CONTROL_POINT_PATCH: this->xprintf(" controlPoint26"); break;
            case D3D11_SB_PRIMITIVE_27_CONTROL_POINT_PATCH: this->xprintf(" controlPoint27"); break;
            case D3D11_SB_PRIMITIVE_28_CONTROL_POINT_PATCH: this->xprintf(" controlPoint28"); break;
            case D3D11_SB_PRIMITIVE_29_CONTROL_POINT_PATCH: this->xprintf(" controlPoint29"); break;
            case D3D11_SB_PRIMITIVE_30_CONTROL_POINT_PATCH: this->xprintf(" controlPoint30"); break;
            case D3D11_SB_PRIMITIVE_31_CONTROL_POINT_PATCH: this->xprintf(" controlPoint31"); break;
            case D3D11_SB_PRIMITIVE_32_CONTROL_POINT_PATCH: this->xprintf(" controlPoint32"); break;
            case D3D10_PRIMITIVE_UNDEFINED: this->xprintf(" undefined"); break;
            default: this->xprintf(" ???");
            }
            break;

        case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
                this->xprintf(" %i", Instruction.m_HSForkPhaseDecl.InstanceCount);
                bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
                this->xprintf(" %i", Instruction.m_HSJoinPhaseDecl.InstanceCount);
                bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
            switch(Instruction.m_TessellatorDomainDecl.TessellatorDomain)
            {
            case D3D11_SB_TESSELLATOR_DOMAIN_ISOLINE: this->xprintf(" isoline"); break;
            case D3D11_SB_TESSELLATOR_DOMAIN_TRI: this->xprintf(" tri"); break;
            case D3D11_SB_TESSELLATOR_DOMAIN_QUAD: this->xprintf(" quad"); break;
                default: this->xprintf(" ???");
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
            switch(Instruction.m_TessellatorOutputPrimitiveDecl.TessellatorOutputPrimitive)
            {
            case D3D11_SB_TESSELLATOR_OUTPUT_POINT: this->xprintf(" point"); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_LINE: this->xprintf(" line"); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CCW: this->xprintf(" tri_ccw"); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CW: this->xprintf(" tri_cw"); break;
                default: this->xprintf(" ???");
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
            switch(Instruction.m_TessellatorPartitioningDecl.TessellatorPartitioning)
            {
            case D3D11_SB_TESSELLATOR_PARTITIONING_POW2: this->xprintf(" pow2"); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_INTEGER: this->xprintf(" int"); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN: this->xprintf(" fract-even"); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD: this->xprintf(" fract-odd"); break;
                default: this->xprintf(" ???");
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
            this->xprintf(" %i", Instruction.m_InputControlPointCountDecl.InputControlPointCount);
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
            this->xprintf(" %i", Instruction.m_OutputControlPointCountDecl.OutputControlPointCount);
            bOperands = false;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));

            if(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV == Instruction.m_OpCode)
            {
                this->xprintf(", ");
                PrintName(this, Instruction.m_OutputDeclSIV.Name);
            }

            switch(Instruction.m_InputPSDeclSIV.InterpolationMode)
            {
                case D3D10_SB_INTERPOLATION_UNDEFINED: this->xprintf(", undefined"); break;
                case D3D10_SB_INTERPOLATION_CONSTANT:  this->xprintf(", constant"); break;
                case D3D10_SB_INTERPOLATION_LINEAR: this->xprintf(", linear"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_CENTROID: this->xprintf(", linear centroid"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE: this->xprintf(", linear noperspective"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID: this->xprintf(", linear noperspective centroid"); break;
                default : this->xprintf(" ???"); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS:

            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));

            if(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV == Instruction.m_OpCode)
            {
                this->xprintf(", ");
                PrintName(this, Instruction.m_OutputDeclSIV.Name);
            }

            switch(Instruction.m_InputPSDecl.InterpolationMode)
            {
                case D3D10_SB_INTERPOLATION_UNDEFINED: this->xprintf(", undefined"); break;
                case D3D10_SB_INTERPOLATION_CONSTANT:  this->xprintf(", constant"); break;
                case D3D10_SB_INTERPOLATION_LINEAR: this->xprintf(", linear"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_CENTROID: this->xprintf(", linear centroid"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE: this->xprintf(", linear noperspective"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID: this->xprintf(", linear noperspective centroid"); break;
                default : this->xprintf(" ???"); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(", ");

            PrintName(this, Instruction.m_OutputDeclSGV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(", ");

            PrintName(this, Instruction.m_OutputDeclSIV.Name);
            bOperands = FALSE;
            break;

       case D3D10_SB_OPCODE_DCL_INPUT_SIV:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(", ");

            PrintName(this, Instruction.m_InputDeclSIV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(", ");
            PrintName(this, Instruction.m_InputPSDeclSGV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_SGV:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            this->xprintf(", ");
            PrintName(this, Instruction.m_InputDeclSIV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf("cb%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                this->xprintf("cb%d[%d]", Instruction.m_Operands[0].m_Index[0].m_RegIndex, Instruction.m_Operands[0].m_Index[1].m_RegIndex);
            }

            if (Instruction.m_ResourceDecl.CBInfo.AccessPattern == D3D10_SB_CONSTANT_BUFFER_DYNAMIC_INDEXED)
                this->xprintf(", dynamicIndexed");
            else
                this->xprintf(", immediateIndexed");

            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_TEMPS:
            this->xprintf(" %d",  Instruction.m_TempsDecl.NumTemps);
            break;

        case D3D10_SB_OPCODE_DCL_RESOURCE:

            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf("t%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            switch( Instruction.m_ResourceDecl.SRVInfo.Dimension)
            {
                case D3D10_SB_RESOURCE_DIMENSION_BUFFER: this->xprintf(", buffer"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D: this->xprintf(", texture1d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D: this->xprintf(", texture2d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS: this->xprintf(", texture2dms"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D: this->xprintf(", texture3d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE: this->xprintf(", texturecube"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY: this->xprintf(", texture1darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY: this->xprintf(", texture2darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY: this->xprintf(", texture2dmsarray"); break;
            }

            if ((Instruction.m_ResourceDecl.SRVInfo.Dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS) ||
                (Instruction.m_ResourceDecl.SRVInfo.Dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY))
            {
                this->xprintf("[%d]", Instruction.m_ResourceDecl.SRVInfo.SampleCount);
            }

            if ((Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[1]) &&
                (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[2]) &&
                (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[3]))
            {
                switch (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0])
                {
                case D3D10_SB_RETURN_TYPE_MIXED: this->xprintf(", mixed"); break;
                case D3D10_SB_RETURN_TYPE_UNORM: this->xprintf(", unorm"); break;
                case D3D10_SB_RETURN_TYPE_SNORM: this->xprintf(", snorm"); break;
                case D3D10_SB_RETURN_TYPE_SINT:  this->xprintf(", sint"); break;
                case D3D10_SB_RETURN_TYPE_UINT:  this->xprintf(", uint"); break;
                case D3D10_SB_RETURN_TYPE_FLOAT: this->xprintf(", float"); break;
                }
            }
            else
            {
                this->xprintf(", (");
                for(UINT i = 0;i < 4; i++)
                {
                    switch (Instruction.m_ResourceDecl.SRVInfo.ReturnType[i])
                    {
                    case D3D10_SB_RETURN_TYPE_MIXED: this->xprintf(" mixed"); break;
                    case D3D10_SB_RETURN_TYPE_UNORM: this->xprintf(" unorm"); break;
                    case D3D10_SB_RETURN_TYPE_SNORM: this->xprintf(" snorm"); break;
                    case D3D10_SB_RETURN_TYPE_SINT:  this->xprintf(" sint"); break;
                    case D3D10_SB_RETURN_TYPE_UINT:  this->xprintf(" uint"); break;
                    case D3D10_SB_RETURN_TYPE_FLOAT: this->xprintf(" float"); break;
                    }
                    if(i != 3)
                        this->xprintf(",");
                }
                this->xprintf(" )");
            }
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf("t%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf("t%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            this->xprintf(", stride=%u", Instruction.m_ResourceDecl.UAVInfo.Stride);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(" uav%d[%d-%d], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(" uav%d[%d-%d], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            this->xprintf(", stride=%u", Instruction.m_ResourceDecl.UAVInfo.Stride);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
            this->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                this->xprintf(" uav%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(this, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            switch (Instruction.m_ResourceDecl.UAVInfo.Dimension)
            {
                case D3D10_SB_RESOURCE_DIMENSION_BUFFER: this->xprintf(", buffer"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D: this->xprintf(", texture1d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D: this->xprintf(", texture2d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS: this->xprintf(", texture2dms"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D: this->xprintf(", texture3d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE: this->xprintf(", texturecube"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY: this->xprintf(", texture1darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY: this->xprintf(", texture2darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY: this->xprintf(", texture2dmsarray"); break;
            }

            switch (Instruction.m_ResourceDecl.UAVInfo.Type)
            {
            case D3D10_SB_RETURN_TYPE_MIXED: this->xprintf(", mixed"); break;
            case D3D10_SB_RETURN_TYPE_UNORM: this->xprintf(", unorm"); break;
            case D3D10_SB_RETURN_TYPE_SNORM: this->xprintf(", snorm"); break;
            case D3D10_SB_RETURN_TYPE_SINT:  this->xprintf(", sint"); break;
            case D3D10_SB_RETURN_TYPE_UINT:  this->xprintf(", uint"); break;
            case D3D10_SB_RETURN_TYPE_FLOAT: this->xprintf(", float"); break;
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
                        this->xprintf("dcl_immediateConstantBuffer");
                        this->UnsetColor();
                        this->xprintf(" { ");
                        for (UINT uNumVecs = 0; uNumVecs < Instruction.m_CustomData.DataSizeInBytes / 16;)
                        {
                            this->xprintf("{ ");
                            for(UINT i = 0;i < 4; i++)
                            {
                                UINT uVal;
                                float *pfVal = (float*)&uVal;
                                uVal = ((UINT*)Instruction.m_CustomData.pData)[uNumVecs*4 + i];
                                UINT uMasked = uVal & 0x7F800000;
                                if(uMasked == 0x7F800000 || uMasked == 0)
                                {
                                    if(abs((INT)uVal) > 10000)
                                        this->xprintf("0x%08x",uVal);
                                    else
                                        this->xprintf("%d",(INT)uVal);
                                } else
                                {
                                    this->xprintf("%f",*pfVal);
                                }

                                if(i != 3)
                                    this->xprintf(", ");
                            }
                            this->xprintf("}");
                            uNumVecs++;
                            if (uNumVecs < Instruction.m_CustomData.DataSizeInBytes / 16)
                            {
                                this->xprintf(",\n                              ");
                            }
                        }
                        this->xprintf(" }");
                    }
                    break;

                    case D3D10_SB_CUSTOMDATA_COMMENT:
                        this->xprintf(" %d bytes of comment custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D10_SB_CUSTOMDATA_DEBUGINFO:
                        this->xprintf(" %d bytes of debuginfo custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D10_SB_CUSTOMDATA_OPAQUE:
                        this->xprintf(" %d bytes of opaque custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D11_SB_CUSTOMDATA_SHADER_MESSAGE:
                        this->xprintf(" %d bytes of shader message custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    default:
                        this->xprintf(" %d bytes of unknown custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;
                }
            }
            break;

        case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
            this->xprintf(" fb%u", Instruction.m_FunctionBodyIdx);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
            this->xprintf(" ft%u = {", Instruction.m_FunctionTable.FtIdx);
            for(UINT i = 0; i < Instruction.m_FunctionTable.FbCount; i++)
            {
                this->xprintf("fb%u", Instruction.m_FunctionTable.pFbStartToken[i]);
                if(i < Instruction.m_FunctionTable.FbCount-1) this->xprintf(", ");
            }
            this->xprintf("}");

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_INTERFACE:
            this->xprintf(" fp%u[%u][%u] = {", Instruction.m_InterfaceTable.FpIdx,
                Instruction.m_InterfaceTable.FpArraySize, Instruction.m_InterfaceTable.NumCallSites);
            for(UINT i = 0; i < Instruction.m_InterfaceTable.FtCount; i++)
            {
                this->xprintf("ft%u", Instruction.m_InterfaceTable.pFtStartToken[i]);
                if(i < Instruction.m_InterfaceTable.FtCount-1) this->xprintf(", ");
            }
            this->xprintf("}");

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_INTERFACE_CALL:
            this->xprintf(" ");
            IFC (PrintOperandD3D10(this, &Instruction.m_Operands[0], false, false));
            this->xprintf("[%u]", Instruction.m_InterfaceCallSiteIdx);

            bOperands = FALSE;
            break;

        default:
            this->xprintf(" ");
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
                    this->xprintf(", ");

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
                    this->xprintf(", mode_default");
                    break;
                case D3D10_SB_SAMPLER_MODE_COMPARISON:
                    this->xprintf(", mode_comparison");
                    break;
                case D3D10_SB_SAMPLER_MODE_MONO:
                    this->xprintf(", mode_mono");
                    break;
            }
            break;
        }

        this->Flush(++line);
    }

Cleanup:
    RRETURN(hr);
}

