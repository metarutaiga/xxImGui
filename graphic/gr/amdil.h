//=====================================================================
// Copyright 2012 (c), Advanced Micro Devices, Inc. All rights reserved.
//=====================================================================
#ifndef _AMDIL_H_
#define _AMDIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IL_OP_DMAX IL_OP_D_MAX
#define IL_OP_DMIN IL_OP_D_MIN

#define IL_MINOR_VERSION 0
#define IL_MAJOR_VERSION 2

// comments in this enum will be added to the il spec
enum IL_Shader_Type
{
    IL_SHADER_VERTEX, // This code describes a pixel shader
    IL_SHADER_PIXEL, // This code describes a vertex shader
    IL_SHADER_GEOMETRY, // This code describes a geometry shader(R6xx and later)
    IL_SHADER_COMPUTE, // This code describes a compute shader (r7xx and later)
    IL_SHADER_HULL, //This code describes a hull shader (Rxx and later)
    IL_SHADER_DOMAIN, //This code describes a domain shader (Rxx and later)
    IL_SHADER_LAST /* dimension the enumeration */
};

// comments in this enum will be added to the il spec
enum IL_Language_Type
{
    IL_LANG_GENERIC, // allows for language specific overrides
    IL_LANG_OPENGL, // any flavor of opengl
    IL_LANG_DX8_PS, // direct x 1.x pixel shader
    IL_LANG_DX8_VS, // direct x 1.x vertex shader
    IL_LANG_DX9_PS, // direct x 2.x and 3.x pixel shader
    IL_LANG_DX9_VS, // direct x 2.x and 3.x vertex shader
    IL_LANG_DX10_PS, // direct x 4.x pixel shader
    IL_LANG_DX10_VS, // direct x 4.x vertex shader
    IL_LANG_DX10_GS, // direct x 4.x geometry shader
    IL_LANG_DX11_PS, // direct x 5.x pixel shader
    IL_LANG_DX11_VS, // direct x 5.x vertex shader
    IL_LANG_DX11_GS, // direct x 5.x Geometry shader
    IL_LANG_DX11_CS, // direct x 5.x Compute shader
    IL_LANG_DX11_HS, // direct x 5.x Hull shader
    IL_LANG_DX11_DS, // direct x 5.x Domain shader
    IL_LANG_LAST /* dimension the enumeration */
};

enum ILOpCode
{
    IL_OP_UNKNOWN,
    IL_OP_ABS,
    IL_OP_ACOS,
    IL_OP_ADD,
    IL_OP_ASIN,
    IL_OP_ATAN,
    IL_OP_BREAK,
    IL_OP_BREAKC,
    IL_OP_CALL,
    IL_OP_CALLNZ,
    IL_OP_CLAMP,
    IL_OP_CLG,
    IL_OP_CMOV,
    IL_OP_CMP,
    IL_OP_COLORCLAMP,
    IL_OP_COMMENT,
    IL_OP_CONTINUE,
    IL_OP_CONTINUEC,
    IL_OP_COS,
    IL_OP_CRS,
    IL_OP_DCLARRAY,
    IL_OP_DCLDEF,
    IL_OP_DCLPI,
    IL_OP_DCLPIN,
    IL_OP_DCLPP,
    IL_OP_DCLPT,
    IL_OP_DCLV,
    IL_OP_DCLVOUT,
    IL_OP_DEF,
    IL_OP_DEFB,
    IL_OP_DET,
    IL_OP_DIST,
    IL_OP_DIV,
    IL_OP_DP2ADD,
    IL_OP_DP3,
    IL_OP_DP4,
    IL_OP_DST,
    IL_OP_DSX,
    IL_OP_DSY,
    IL_OP_ELSE,
    IL_OP_END,
    IL_OP_ENDIF,
    IL_OP_ENDLOOP,
    IL_OP_ENDMAIN,
    IL_OP_EXN,
    IL_OP_EXP,
    IL_OP_EXPP,
    IL_OP_FACEFORWARD,
    IL_OP_FLR,
    IL_OP_FRC,
    IL_OP_FUNC,
    IL_OP_FWIDTH,
    IL_OP_IFC,
    IL_OP_IFNZ,
    IL_OP_INITV,
    IL_OP_KILL,
    IL_OP_LEN,
    IL_OP_LIT,
    IL_OP_LN,
    IL_OP_LOD,
    IL_OP_LOG,
    IL_OP_LOGP,
    IL_OP_LOOP,
    IL_OP_LRP,
    IL_OP_MAD,
    IL_OP_MAX,
    IL_OP_MEMEXPORT,
    IL_OP_MEMIMPORT,
    IL_OP_MIN,
    IL_OP_MMUL,
    IL_OP_MOD,
    IL_OP_MOV,
    IL_OP_MOVA,
    IL_OP_MUL,
    IL_OP_NOISE,
    IL_OP_NOP,
    IL_OP_NRM,
    IL_OP_PIREDUCE,
    IL_OP_POW,
    IL_OP_PRECOMP,
    IL_OP_PROJECT,
    IL_OP_RCP,
    IL_OP_REFLECT,
    IL_OP_RET,
    IL_OP_RND,
    IL_OP_RSQ,
    IL_OP_SET,
    IL_OP_SGN,
    IL_OP_SIN,
    IL_OP_SINCOS,
    IL_OP_SQRT,
    IL_OP_SUB,
    IL_OP_TAN,
    IL_OP_TEXLD,
    IL_OP_TEXLDB,
    IL_OP_TEXLDD,
    IL_OP_TEXLDMS,
    IL_OP_TEXWEIGHT,
    IL_OP_TRANSPOSE,
    IL_OP_TRC,
    IL_OP_DXSINCOS,
    IL_OP_BREAK_LOGICALZ,
    IL_OP_BREAK_LOGICALNZ,
    IL_OP_CALL_LOGICALZ,
    IL_OP_CALL_LOGICALNZ,
    IL_OP_CASE,
    IL_OP_CONTINUE_LOGICALZ,
    IL_OP_CONTINUE_LOGICALNZ,
    IL_OP_DEFAULT,
    IL_OP_ENDSWITCH,
    IL_OP_ENDFUNC,
    IL_OP_IF_LOGICALZ,
    IL_OP_IF_LOGICALNZ,
    IL_OP_WHILE,
    IL_OP_SWITCH,
    IL_OP_RET_DYN,
    IL_OP_RET_LOGICALZ,
    IL_OP_RET_LOGICALNZ,
    IL_DCL_CONST_BUFFER,
    IL_DCL_INDEXED_TEMP_ARRAY,
    IL_DCL_INPUT_PRIMITIVE,
    IL_DCL_LITERAL,
    IL_DCL_MAX_OUTPUT_VERTEX_COUNT,
    IL_DCL_ODEPTH,
    IL_DCL_OUTPUT_TOPOLOGY,
    IL_DCL_OUTPUT,
    IL_DCL_INPUT,
    IL_DCL_VPRIM,
    IL_DCL_RESOURCE,
    IL_OP_CUT,
    IL_OP_DISCARD_LOGICALZ,
    IL_OP_DISCARD_LOGICALNZ,
    IL_OP_EMIT,
    IL_OP_EMIT_THEN_CUT,
    IL_OP_LOAD,
    IL_OP_RESINFO,
    IL_OP_SAMPLE,
    IL_OP_SAMPLE_B,
    IL_OP_SAMPLE_G,
    IL_OP_SAMPLE_L,
    IL_OP_SAMPLE_C,
    IL_OP_SAMPLE_C_LZ,
    IL_OP_I_NOT,
    IL_OP_I_OR,
    IL_OP_I_XOR,
    IL_OP_I_ADD,
    IL_OP_I_MAD,
    IL_OP_I_MAX,
    IL_OP_I_MIN,
    IL_OP_I_MUL,
    IL_OP_I_MUL_HIGH,
    IL_OP_I_EQ,
    IL_OP_I_GE,
    IL_OP_I_LT,
    IL_OP_I_NEGATE,
    IL_OP_I_NE,
    IL_OP_I_SHL,
    IL_OP_I_SHR,
    IL_OP_U_SHR,
    IL_OP_U_DIV,
    IL_OP_U_MOD,
    IL_OP_U_MAD,
    IL_OP_U_MAX,
    IL_OP_U_MIN,
    IL_OP_U_LT,
    IL_OP_U_GE,
    IL_OP_U_MUL,
    IL_OP_U_MUL_HIGH,
    IL_OP_FTOI,
    IL_OP_FTOU,
    IL_OP_ITOF,
    IL_OP_UTOF,
    IL_OP_AND,
    IL_OP_CMOV_LOGICAL,
    IL_OP_EQ,
    IL_OP_EXP_VEC,
    IL_OP_GE,
    IL_OP_LOG_VEC,
    IL_OP_LT,
    IL_OP_NE,
    IL_OP_ROUND_NEAR,
    IL_OP_ROUND_NEG_INF,
    IL_OP_ROUND_PLUS_INF,
    IL_OP_ROUND_ZERO,
    IL_OP_RSQ_VEC,
    IL_OP_SIN_VEC,
    IL_OP_COS_VEC,
    IL_OP_SQRT_VEC,
    IL_OP_DP2,
    IL_OP_INV_MOV,
    IL_OP_SCATTER,
    IL_OP_D_FREXP,
    IL_OP_D_ADD,
    IL_OP_D_MUL,
    IL_OP_D_2_F,
    IL_OP_F_2_D,
    IL_OP_D_LDEXP,
    IL_OP_D_FRAC,
    IL_OP_D_MULADD,
    IL_OP_FETCH4,
    IL_OP_SAMPLEINFO,
    IL_OP_GETLOD, // the dx10.1 version of lod
    IL_DCL_PERSIST,
    IL_OP_DNE,
    IL_OP_DEQ,
    IL_OP_DGE,
    IL_OP_DLT,
    IL_OP_SAMPLEPOS,
    IL_OP_D_DIV,
    IL_OP_DCL_SHARED_TEMP,
    IL_OP_INIT_SR, // a special init inst for 7xx CS
    IL_OP_INIT_SR_HELPER, // an internal IL OP, only used by SC
    IL_OP_DCL_NUM_THREAD_PER_GROUP, // thread group = thread block
    IL_OP_DCL_TOTAL_NUM_THREAD_GROUP,
    IL_OP_DCL_LDS_SIZE_PER_THREAD,
    IL_OP_DCL_LDS_SHARING_MODE,
    IL_OP_LDS_READ_VEC, // R7xx LDS
    IL_OP_LDS_WRITE_VEC,
    IL_OP_FENCE, // R7xx/Evergreen fence
    IL_OP_LDS_LOAD_VEC, // DX10_CS: DX11 style LDS instruction in vector (4 dwords)
    IL_OP_LDS_STORE_VEC, // DX10_CS: DX11 style LDS instruction in vector (4 dwords)
    IL_OP_DCL_UAV, // Evergreen UAV
    IL_OP_DCL_RAW_UAV,
    IL_OP_DCL_STRUCT_UAV,
    IL_OP_UAV_LOAD,
    IL_OP_UAV_RAW_LOAD,
    IL_OP_UAV_STRUCT_LOAD,
    IL_OP_UAV_STORE,
    IL_OP_UAV_RAW_STORE,
    IL_OP_UAV_STRUCT_STORE,
    IL_OP_DCL_ARENA_UAV, // 8xx/OpenCL Arena UAV
    IL_OP_UAV_ARENA_LOAD,
    IL_OP_UAV_ARENA_STORE,
    IL_OP_UAV_ADD,
    IL_OP_UAV_SUB,
    IL_OP_UAV_RSUB,
    IL_OP_UAV_MIN,
    IL_OP_UAV_MAX,
    IL_OP_UAV_UMIN,
    IL_OP_UAV_UMAX,
    IL_OP_UAV_AND,
    IL_OP_UAV_OR,
    IL_OP_UAV_XOR,
    IL_OP_UAV_CMP,
    IL_OP_UAV_READ_ADD,
    IL_OP_UAV_READ_SUB,
    IL_OP_UAV_READ_RSUB,
    IL_OP_UAV_READ_MIN,
    IL_OP_UAV_READ_MAX,
    IL_OP_UAV_READ_UMIN,
    IL_OP_UAV_READ_UMAX,
    IL_OP_UAV_READ_AND,
    IL_OP_UAV_READ_OR,
    IL_OP_UAV_READ_XOR,
    IL_OP_UAV_READ_XCHG,
    IL_OP_UAV_READ_CMP_XCHG,
    IL_OP_APPEND_BUF_ALLOC, // Evergreen Append buf aloc/consum
    IL_OP_APPEND_BUF_CONSUME,
    IL_OP_DCL_RAW_SRV, // Evergreen SRV
    IL_OP_DCL_STRUCT_SRV,
    IL_OP_SRV_RAW_LOAD,
    IL_OP_SRV_STRUCT_LOAD,
    IL_DCL_LDS, // Evergreen LDS
    IL_DCL_STRUCT_LDS,
    IL_OP_LDS_LOAD,
    IL_OP_LDS_STORE,
    IL_OP_LDS_ADD,
    IL_OP_LDS_SUB,
    IL_OP_LDS_RSUB,
    IL_OP_LDS_MIN,
    IL_OP_LDS_MAX,
    IL_OP_LDS_UMIN,
    IL_OP_LDS_UMAX,
    IL_OP_LDS_AND,
    IL_OP_LDS_OR,
    IL_OP_LDS_XOR,
    IL_OP_LDS_CMP,
    IL_OP_LDS_READ_ADD,
    IL_OP_LDS_READ_SUB,
    IL_OP_LDS_READ_RSUB,
    IL_OP_LDS_READ_MIN,
    IL_OP_LDS_READ_MAX,
    IL_OP_LDS_READ_UMIN,
    IL_OP_LDS_READ_UMAX,
    IL_OP_LDS_READ_AND,
    IL_OP_LDS_READ_OR,
    IL_OP_LDS_READ_XOR,
    IL_OP_LDS_READ_XCHG,
    IL_OP_LDS_READ_CMP_XCHG,
    IL_OP_CUT_STREAM,
    IL_OP_EMIT_STREAM,
    IL_OP_EMIT_THEN_CUT_STREAM,
    IL_OP_SAMPLE_C_L,
    IL_OP_SAMPLE_C_G,
    IL_OP_SAMPLE_C_B,
    IL_OP_I_COUNTBITS,
    IL_OP_I_FIRSTBIT,
    IL_OP_I_CARRY,
    IL_OP_I_BORROW,
    IL_OP_I_BIT_EXTRACT,
    IL_OP_U_BIT_EXTRACT,
    IL_OP_U_BIT_REVERSE,
    IL_DCL_NUM_ICP,
    IL_DCL_NUM_OCP,
    IL_DCL_NUM_INSTANCES,
    IL_OP_HS_CP_PHASE,
    IL_OP_HS_FORK_PHASE,
    IL_OP_HS_JOIN_PHASE,
    IL_OP_ENDPHASE,
    IL_DCL_TS_DOMAIN,
    IL_DCL_TS_PARTITION,
    IL_DCL_TS_OUTPUT_PRIMITIVE,
    IL_DCL_MAX_TESSFACTOR,
    IL_OP_DCL_FUNCTION_BODY,
    IL_OP_DCL_FUNCTION_TABLE,
    IL_OP_DCL_INTERFACE_PTR,
    IL_OP_FCALL,
    IL_OP_U_BIT_INSERT,
    IL_OP_BUFINFO,
    IL_OP_FETCH4_C,
    IL_OP_FETCH4_PO,
    IL_OP_FETCH4_PO_C,
    IL_OP_D_MAX,
    IL_OP_D_MIN,
    IL_OP_F_2_F16,
    IL_OP_F16_2_F,
    IL_OP_UNPACK0,
    IL_OP_UNPACK1,
    IL_OP_UNPACK2,
    IL_OP_UNPACK3,
    IL_OP_BIT_ALIGN,
    IL_OP_BYTE_ALIGN,
    IL_OP_U4LERP,
    IL_OP_SAD,
    IL_OP_SAD_HI,
    IL_OP_SAD_4,
    IL_OP_F_2_U4,
    IL_OP_EVAL_SNAPPED,
    IL_OP_EVAL_SAMPLE_INDEX,
    IL_OP_EVAL_CENTROID,
    IL_OP_D_MOV,
    IL_OP_D_MOVC,
    IL_OP_D_SQRT,
    IL_OP_D_RCP,
    IL_OP_D_RSQ,
    IL_OP_MACRODEF,
    IL_OP_MACROEND,
    IL_OP_MACROCALL,
    IL_DCL_STREAM,
    IL_DCL_GLOBAL_FLAGS,
    IL_OP_RCP_VEC,
    IL_OP_LOAD_FPTR,
    IL_DCL_MAX_THREAD_PER_GROUP,
    IL_OP_PREFIX,
    // GDS
    IL_DCL_GDS,
    IL_DCL_STRUCT_GDS,
    IL_OP_GDS_LOAD,
    IL_OP_GDS_STORE,
    IL_OP_GDS_ADD,
    IL_OP_GDS_SUB,
    IL_OP_GDS_RSUB,
    IL_OP_GDS_INC,
    IL_OP_GDS_DEC,
    IL_OP_GDS_MIN,
    IL_OP_GDS_MAX,
    IL_OP_GDS_UMIN,
    IL_OP_GDS_UMAX,
    IL_OP_GDS_AND,
    IL_OP_GDS_OR,
    IL_OP_GDS_XOR,
    IL_OP_GDS_MSKOR,
    IL_OP_GDS_CMP_STORE,
    IL_OP_GDS_READ_ADD,
    IL_OP_GDS_READ_SUB,
    IL_OP_GDS_READ_RSUB,
    IL_OP_GDS_READ_INC,
    IL_OP_GDS_READ_DEC,
    IL_OP_GDS_READ_MIN,
    IL_OP_GDS_READ_MAX,
    IL_OP_GDS_READ_UMIN,
    IL_OP_GDS_READ_UMAX,
    IL_OP_GDS_READ_AND,
    IL_OP_GDS_READ_OR,
    IL_OP_GDS_READ_XOR,
    IL_OP_GDS_READ_MSKOR,
    IL_OP_GDS_READ_XCHG,
    IL_OP_GDS_READ_CMP_XCHG,
    IL_OP_U_MAD24,
    IL_OP_U_MUL24,
    IL_OP_FMA,
    IL_OP_UAV_UINC,
    IL_OP_UAV_UDEC,
    IL_OP_I_MAD24,
    IL_OP_I_MUL24,
    IL_OP_UAV_READ_UINC,
    IL_OP_UAV_READ_UDEC,
    IL_OP_LDS_LOAD_BYTE,
    IL_OP_LDS_LOAD_SHORT,
    IL_OP_LDS_LOAD_UBYTE,
    IL_OP_LDS_LOAD_USHORT,
    IL_OP_LDS_STORE_BYTE,
    IL_OP_LDS_STORE_SHORT,
    IL_OP_UAV_BYTE_LOAD,
    IL_OP_UAV_SHORT_LOAD,
    IL_OP_UAV_UBYTE_LOAD,
    IL_OP_UAV_USHORT_LOAD,
    IL_OP_UAV_BYTE_STORE,
    IL_OP_UAV_SHORT_STORE,
    IL_OP_I64_ADD,
    IL_OP_I64_EQ,
    IL_OP_I64_GE,
    IL_OP_I64_LT,
    IL_OP_I64_MAX,
    IL_OP_I64_MIN,
    IL_OP_I64_NE,
    IL_OP_I64_NEGATE,
    IL_OP_I64_SHL,
    IL_OP_I64_SHR,
    IL_OP_U64_GE,
    IL_OP_U64_LT,
    IL_OP_U64_MAX,
    IL_OP_U64_MIN,
    IL_OP_U64_SHR,
    IL_OP_DCL_TYPED_UAV,
    IL_OP_DCL_TYPELESS_UAV,
    IL_OP_I_MUL24_HIGH,
    IL_OP_U_MUL24_HIGH,
    IL_OP_LDS_INC,
    IL_OP_LDS_DEC,
    IL_OP_LDS_READ_INC,
    IL_OP_LDS_READ_DEC,
    IL_OP_LDS_MSKOR,
    IL_OP_LDS_READ_MSKOR,
    IL_OP_F_2_F16_NEAR,
    IL_OP_F_2_F16_NEG_INF,
    IL_OP_F_2_F16_PLUS_INF,
    IL_OP_I64_MUL,
    IL_OP_U64_MUL,
    IL_OP_LDEXP,
    IL_OP_FREXP_EXP,
    IL_OP_FREXP_MANT,
    IL_OP_D_FREXP_EXP,
    IL_OP_D_FREXP_MANT,
    IL_OP_DTOI,
    IL_OP_DTOU,
    IL_OP_ITOD,
    IL_OP_UTOD,
    IL_OP_FTOI_RPI,
    IL_OP_FTOI_FLR,
    IL_OP_MIN3,
    IL_OP_MED3,
    IL_OP_MAX3,
    IL_OP_I_MIN3,
    IL_OP_I_MED3,
    IL_OP_I_MAX3,
    IL_OP_U_MIN3,
    IL_OP_U_MED3,
    IL_OP_U_MAX3,
    IL_OP_CLASS,
    IL_OP_D_CLASS,
    IL_OP_SAMPLE_RETURN_CODE,
    IL_OP_CU_ID,
    IL_OP_WAVE_ID,
    IL_OP_I64_SUB,
    IL_OP_I64_DIV,
    IL_OP_U64_DIV,
    IL_OP_I64_MOD,
    IL_OP_U64_MOD,
    IL_DCL_GWS_THREAD_COUNT,
    IL_DCL_SEMAPHORE,
    IL_OP_SEMAPHORE_INIT,
    IL_OP_SEMAPHORE_SIGNAL,
    IL_OP_SEMAPHORE_WAIT,
    IL_OP_DIV_SCALE,
    IL_OP_DIV_FMAS,
    IL_OP_DIV_FIXUP,
    IL_OP_D_DIV_SCALE,
    IL_OP_D_DIV_FMAS,
    IL_OP_D_DIV_FIXUP,
    IL_OP_D_TRIG_PREOP,
    IL_OP_MSAD_U8,
    IL_OP_QSAD_U8,
    IL_OP_MQSAD_U8,
    IL_OP_LAST /* dimension the enumeration */
};

// \todo remove this define once sc is promoted to dxx and dx=>il has been updated.
#define IL_OP_I_BIT_INSERT IL_OP_U_BIT_INSERT

// comments in this enum will be added to the il spec
enum ILRegType
{
    IL_REGTYPE_CONST_BOOL, // single bit boolean constant
    IL_REGTYPE_CONST_FLOAT,
    IL_REGTYPE_CONST_INT,
    IL_REGTYPE_ADDR,
    IL_REGTYPE_TEMP,
    IL_REGTYPE_VERTEX,
    IL_REGTYPE_INDEX,
    IL_REGTYPE_OBJECT_INDEX,
    IL_REGTYPE_BARYCENTRIC_COORD,
    IL_REGTYPE_PRIMITIVE_INDEX,
    IL_REGTYPE_QUAD_INDEX,
    IL_REGTYPE_VOUTPUT,
    IL_REGTYPE_PINPUT,
    IL_REGTYPE_SPRITE,
    IL_REGTYPE_POS,
    IL_REGTYPE_INTERP,
    IL_REGTYPE_FOG,
    IL_REGTYPE_TEXCOORD,
    IL_REGTYPE_PRICOLOR,
    IL_REGTYPE_SECCOLOR,
    IL_REGTYPE_SPRITECOORD,
    IL_REGTYPE_FACE,
    IL_REGTYPE_WINCOORD,
    IL_REGTYPE_PRIMCOORD,
    IL_REGTYPE_PRIMTYPE,
    IL_REGTYPE_PCOLOR,
    IL_REGTYPE_DEPTH,
    IL_REGTYPE_STENCIL,
    IL_REGTYPE_CLIP,
    IL_REGTYPE_VPRIM,
    IL_REGTYPE_ITEMP,
    IL_REGTYPE_CONST_BUFF,
    IL_REGTYPE_LITERAL,
    IL_REGTYPE_INPUT,
    IL_REGTYPE_OUTPUT,
    IL_REGTYPE_IMMED_CONST_BUFF,
    IL_REGTYPE_OMASK,
    IL_REGTYPE_PERSIST,
    IL_REGTYPE_GLOBAL,
    IL_REGTYPE_PS_OUT_FOG,
    IL_REGTYPE_SHARED_TEMP,
    IL_REGTYPE_THREAD_ID_IN_GROUP, // 3-dim
    IL_REGTYPE_THREAD_ID_IN_GROUP_FLAT, // 1-dim
    IL_REGTYPE_ABSOLUTE_THREAD_ID, // 3-dim
    IL_REGTYPE_ABSOLUTE_THREAD_ID_FLAT, // 1-dim
    IL_REGTYPE_THREAD_GROUP_ID, // 3-dim
    IL_REGTYPE_THREAD_GROUP_ID_FLAT, // 1-dim
    IL_REGTYPE_GENERIC_MEM, // generic memory type, used w/mask of lds_write
    IL_REGTYPE_INPUTCP, // tessellation, input control-point register
    IL_REGTYPE_PATCHCONST, // tessellation, patch constants
    IL_REGTYPE_DOMAINLOCATION, // domain shader, domain location
    IL_REGTYPE_OUTPUTCP, // tessellation, output control-point register
    IL_REGTYPE_OCP_ID, // tessellation, output control-point id
    IL_REGTYPE_SHADER_INSTANCE_ID, // tessellation hs fork/join instance id or gs instance id
    IL_REGTYPE_THIS,
    IL_REGTYPE_EDGEFLAG, //edge flag
    IL_REGTYPE_DEPTH_LE, //dx11 conservative depth guaranteed to be <= raster depth
    IL_REGTYPE_DEPTH_GE, //dx11 conservative depth guaranteed to be >= raster depth
    IL_REGTYPE_INPUT_COVERAGE_MASK, //dx11 ps input coverage mask
    IL_REGTYPE_TIMER,
    IL_REGTYPE_LINE_STIPPLE, // Evergreen+, anti-aliased line stipple
    IL_REGTYPE_INPUT_ARG, // macro processor input
    IL_REGTYPE_OUTPUT_ARG, // macro processor output
    IL_REGTYPE_LAST, // Must be < 64, we only have 6 bits for encoding the reg type
    // SoftIL requires IL_REGTYPE_LAST <= 63
    // We cannot add any more IL_REGTYPE without using extended field
};

enum ILMatrix
{
    IL_MATRIX_4X4,
    IL_MATRIX_4X3,
    IL_MATRIX_3X4,
    IL_MATRIX_3X3,
    IL_MATRIX_3X2,
    IL_MATRIX_LAST /* dimension the enumeration */
};

enum ILDivComp
{
    IL_DIVCOMP_NONE, /* None */
    IL_DIVCOMP_Y, /* Divide the x component by y */
    IL_DIVCOMP_Z, /* Divide the x and y components by z */
    IL_DIVCOMP_W, /* Divide the x, y, and z components by w */
    IL_DIVCOMP_UNKNOWN, /* Divide each component by the value of AS_**** */
    IL_DIVCOMP_LAST
};

enum ILZeroOp
{
    IL_ZEROOP_FLTMAX,
    IL_ZEROOP_0,
    IL_ZEROOP_INFINITY,
    IL_ZEROOP_INF_ELSE_MAX,
    IL_ZEROOP_LAST /* dimension the enumeration */
};

enum ILModDstComponent
{
    IL_MODCOMP_NOWRITE, // do not write this component
    IL_MODCOMP_WRITE, // write the result to this component
    IL_MODCOMP_0, // force the component to float 0.0
    IL_MODCOMP_1, // force the component to float 1.0
    IL_MODCOMP_LAST /* dimension the enumeration */
};

enum ILComponentSelect
{
    IL_COMPSEL_X_R, //select the 1st component (x/red) for the channel.
    IL_COMPSEL_Y_G, //select the 2nd component (y/green) for the channel.
    IL_COMPSEL_Z_B, // select the 3rd component (z/blue) for the channel.
    IL_COMPSEL_W_A, //select the 4th component (w/alpha) for the channel.
    IL_COMPSEL_0, //Force this channel to 0.0
    IL_COMPSEL_1, //Force this channel to 1.0
    IL_COMPSEL_LAST /* dimension the enumeration */
};

enum ILShiftScale
{
    IL_SHIFT_NONE = 0,
    IL_SHIFT_X2,
    IL_SHIFT_X4,
    IL_SHIFT_X8,
    IL_SHIFT_D2,
    IL_SHIFT_D4,
    IL_SHIFT_D8,
    IL_SHIFT_LAST /* dimension the enumeration */
};

enum ILRelOp
{
    IL_RELOP_NE, /* != */
    IL_RELOP_EQ, /* == */
    IL_RELOP_GE, /* >= */
    IL_RELOP_GT, /* > */
    IL_RELOP_LE, /* <= */
    IL_RELOP_LT, /* < */
    IL_RELOP_LAST /* dimension the enumeration */
};

enum ILDefaultVal
{
    IL_DEFVAL_NONE = 0,
    IL_DEFVAL_0,
    IL_DEFVAL_1,
    IL_DEFVAL_LAST /* dimension the enumeration */
};

enum ILImportComponent
{
    IL_IMPORTSEL_UNUSED = 0,
    IL_IMPORTSEL_DEFAULT0,
    IL_IMPORTSEL_DEFAULT1,
    IL_IMPORTSEL_UNDEFINED,
    IL_IMPORTSEL_LAST /* dimension the enum */
};

enum ILImportUsage
{
    IL_IMPORTUSAGE_POS = 0,
    IL_IMPORTUSAGE_POINTSIZE,
    IL_IMPORTUSAGE_COLOR,
    IL_IMPORTUSAGE_BACKCOLOR,
    IL_IMPORTUSAGE_FOG,
    IL_IMPORTUSAGE_PIXEL_SAMPLE_COVERAGE,
    IL_IMPORTUSAGE_GENERIC,
    IL_IMPORTUSAGE_CLIPDISTANCE,
    IL_IMPORTUSAGE_CULLDISTANCE,
    IL_IMPORTUSAGE_PRIMITIVEID,
    IL_IMPORTUSAGE_VERTEXID,
    IL_IMPORTUSAGE_INSTANCEID,
    IL_IMPORTUSAGE_ISFRONTFACE,
    IL_IMPORTUSAGE_LOD,
    IL_IMPORTUSAGE_COLORING,
    IL_IMPORTUSAGE_NODE_COLORING,
    IL_IMPORTUSAGE_NORMAL,
    IL_IMPORTUSAGE_RENDERTARGET_ARRAY_INDEX,
    IL_IMPORTUSAGE_VIEWPORT_ARRAY_INDEX,
    IL_IMPORTUSAGE_UNDEFINED,
    IL_IMPORTUSAGE_SAMPLE_INDEX,
    IL_IMPORTUSAGE_EDGE_TESSFACTOR,
    IL_IMPORTUSAGE_INSIDE_TESSFACTOR,
    IL_IMPORTUSAGE_DETAIL_TESSFACTOR,
    IL_IMPORTUSAGE_DENSITY_TESSFACTOR,
    IL_IMPORTUSAGE_LAST /* dimension the enum */
};

enum ILCmpVal
{
    IL_CMPVAL_0_0 = 0, /* compare vs. 0.0 */
    IL_CMPVAL_0_5, /* compare vs. 0.5 */
    IL_CMPVAL_1_0, /* compare vs. 1.0 */
    IL_CMPVAL_NEG_0_5, /* compare vs. -0.5 */
    IL_CMPVAL_NEG_1_0, /* compare vs. -1.0 */
    IL_CMPVAL_LAST /* dimension the enumeration */
};

// dependent upon this table:
// - dx10interpreter.cpp - table that maps dx names to il
// - iltables.cpp - il_pixtex_props_table
enum ILPixTexUsage
{
    // dx9 only il allows 0-7 values for dclpt
    IL_USAGE_PIXTEX_UNKNOWN = 0,
    IL_USAGE_PIXTEX_1D,
    IL_USAGE_PIXTEX_2D,
    IL_USAGE_PIXTEX_3D,
    IL_USAGE_PIXTEX_CUBEMAP,
    IL_USAGE_PIXTEX_2DMSAA,
    // dx10 formats after this point
    IL_USAGE_PIXTEX_4COMP,
    IL_USAGE_PIXTEX_BUFFER,
    IL_USAGE_PIXTEX_1DARRAY,
    IL_USAGE_PIXTEX_2DARRAY,
    IL_USAGE_PIXTEX_2DARRAYMSAA,
    IL_USAGE_PIXTEX_2D_PLUS_W, // Pele hardware feature
    IL_USAGE_PIXTEX_CUBEMAP_PLUS_W, // Pele hardware feature
    // dx 10.1
    IL_USAGE_PIXTEX_CUBEMAP_ARRAY,
    IL_USAGE_PIXTEX_LAST /* dimension the enumeration */
};

enum ILTexCoordMode
{
    IL_TEXCOORDMODE_UNKNOWN = 0,
    IL_TEXCOORDMODE_NORMALIZED,
    IL_TEXCOORDMODE_UNNORMALIZED,
    IL_TEXCOORDMODE_LAST /* dimension the enumeration */
};

enum ILElementFormat
{
    IL_ELEMENTFORMAT_UNKNOWN = 0,
    IL_ELEMENTFORMAT_SNORM,
    IL_ELEMENTFORMAT_UNORM,
    IL_ELEMENTFORMAT_SINT,
    IL_ELEMENTFORMAT_UINT,
    IL_ELEMENTFORMAT_FLOAT,
    IL_ELEMENTFORMAT_SRGB,
    IL_ELEMENTFORMAT_MIXED,
    IL_ELEMENTFORMAT_LAST
};

enum ILTexShadowMode
{
    IL_TEXSHADOWMODE_NEVER = 0,
    IL_TEXSHADOWMODE_Z,
    IL_TEXSHADOWMODE_UNKNOWN,
    IL_TEXSHADOWMODE_LAST /* dimension the enumeration */
};

enum ILTexFilterMode
{
    IL_TEXFILTER_UNKNOWN = 0,
    IL_TEXFILTER_POINT,
    IL_TEXFILTER_LINEAR,
    IL_TEXFILTER_ANISO,
    IL_TEXFILTER_LAST /* dimension the enumeration */
};

enum ILMipFilterMode
{
    IL_MIPFILTER_UNKNOWN = 0,
    IL_MIPFILTER_POINT,
    IL_MIPFILTER_LINEAR,
    IL_MIPFILTER_BASE,
    IL_MIPFILTER_LAST /* dimension the enumeration */
};

enum ILAnisoFilterMode
{
    IL_ANISOFILTER_UNKNOWN = 0,
    IL_ANISOFILTER_DISABLED,
    IL_ANISOFILTER_MAX_1_TO_1,
    IL_ANISOFILTER_MAX_2_TO_1,
    IL_ANISOFILTER_MAX_4_TO_1,
    IL_ANISOFILTER_MAX_8_TO_1,
    IL_ANISOFILTER_MAX_16_TO_1,
    IL_ANISOFILTER_LAST /* dimension the enumeration */
};

enum ILNoiseType
{
    IL_NOISETYPE_PERLIN1D = 0,
    IL_NOISETYPE_PERLIN2D,
    IL_NOISETYPE_PERLIN3D,
    IL_NOISETYPE_PERLIN4D,
    IL_NOISETYPE_LAST /* dimension the enumeration */
};

enum ILAddressing
{
    IL_ADDR_ABSOLUTE = 0,
    IL_ADDR_RELATIVE,
    IL_ADDR_REG_RELATIVE,
    IL_ADDR_LAST /* dimension the enumeration */
};

enum ILInterpMode
{
    IL_INTERPMODE_NOTUSED = 0,
    IL_INTERPMODE_CONSTANT,
    IL_INTERPMODE_LINEAR,
    IL_INTERPMODE_LINEAR_CENTROID,
    IL_INTERPMODE_LINEAR_NOPERSPECTIVE,
    IL_INTERPMODE_LINEAR_NOPERSPECTIVE_CENTROID,
    IL_INTERPMODE_LINEAR_SAMPLE,
    IL_INTERPMODE_LINEAR_NOPERSPECTIVE_SAMPLE,
    IL_INTERPMODE_LAST /* dimension the enumeration */
};

// types for scatter
enum IL_SCATTER
{
    IL_SCATTER_BY_PIXEL,
    IL_SCATTER_BY_QUAD
};

// types that can be input to a gemetry shader
enum IL_TOPOLOGY
{
    IL_TOPOLOGY_POINT,
    IL_TOPOLOGY_LINE,
    IL_TOPOLOGY_TRIANGLE,
    IL_TOPOLOGY_LINE_ADJ,
    IL_TOPOLOGY_TRIANGLE_ADJ,
    IL_TOPOLOGY_PATCH1,
    IL_TOPOLOGY_PATCH2,
    IL_TOPOLOGY_PATCH3,
    IL_TOPOLOGY_PATCH4,
    IL_TOPOLOGY_PATCH5,
    IL_TOPOLOGY_PATCH6,
    IL_TOPOLOGY_PATCH7,
    IL_TOPOLOGY_PATCH8,
    IL_TOPOLOGY_PATCH9,
    IL_TOPOLOGY_PATCH10,
    IL_TOPOLOGY_PATCH11,
    IL_TOPOLOGY_PATCH12,
    IL_TOPOLOGY_PATCH13,
    IL_TOPOLOGY_PATCH14,
    IL_TOPOLOGY_PATCH15,
    IL_TOPOLOGY_PATCH16,
    IL_TOPOLOGY_PATCH17,
    IL_TOPOLOGY_PATCH18,
    IL_TOPOLOGY_PATCH19,
    IL_TOPOLOGY_PATCH20,
    IL_TOPOLOGY_PATCH21,
    IL_TOPOLOGY_PATCH22,
    IL_TOPOLOGY_PATCH23,
    IL_TOPOLOGY_PATCH24,
    IL_TOPOLOGY_PATCH25,
    IL_TOPOLOGY_PATCH26,
    IL_TOPOLOGY_PATCH27,
    IL_TOPOLOGY_PATCH28,
    IL_TOPOLOGY_PATCH29,
    IL_TOPOLOGY_PATCH30,
    IL_TOPOLOGY_PATCH31,
    IL_TOPOLOGY_PATCH32,
    IL_TOPOLOGY_LAST /* dimension the enumeration */
};

enum IL_OUTPUT_TOPOLOGY
{
    IL_OUTPUT_TOPOLOGY_POINT_LIST,
    IL_OUTPUT_TOPOLOGY_LINE_STRIP,
    IL_OUTPUT_TOPOLOGY_TRIANGLE_STRIP,
    IL_OUTPUT_TOPOLOGY_LAST /* dimension the enumeration */
};

// for R7xx Compute shader
enum IL_LDS_SHARING_MODE
{
    IL_LDS_SHARING_MODE_RELATIVE = 0, // for wavefront_rel
    IL_LDS_SHARING_MODE_ABSOLUTE, // for wavefront_abs
    IL_LDS_SHARING_MODE_LAST
};

// for OpenCL Arena UAV load/store and others
enum IL_LOAD_STORE_DATA_SIZE
{
    IL_LOAD_STORE_DATA_SIZE_DWORD = 0, // dword, 32 bits
    IL_LOAD_STORE_DATA_SIZE_SHORT, // short, 16 bits
    IL_LOAD_STORE_DATA_SIZE_BYTE, // byte, 8 bits
    IL_LOAD_STORE_DATA_SIZE_LAST
};

enum IL_UAV_ACCESS_TYPE
{
    IL_UAV_ACCESS_TYPE_RW = 0, // read_write
    IL_UAV_ACCESS_TYPE_RO, // read_only
    IL_UAV_ACCESS_TYPE_WO, // write_only
    IL_UAV_ACCESS_TYPE_PRIVATE, // private
    IL_UAV_ACCESS_TYPE_LAST
};

// type of firstbit
enum IL_FIRSTBIT_TYPE
{
    IL_FIRSTBIT_TYPE_LOW_UINT,
    IL_FIRSTBIT_TYPE_HIGH_UINT,
    IL_FIRSTBIT_TYPE_HIGH_INT
};

enum ILTsDomain
{
    IL_TS_DOMAIN_ISOLINE = 0,
    IL_TS_DOMAIN_TRI = 1,
    IL_TS_DOMAIN_QUAD = 2,
    IL_TS_DOMAIN_LAST,
};

enum ILTsPartition
{
    IL_TS_PARTITION_INTEGER,
    IL_TS_PARTITION_POW2,
    IL_TS_PARTITION_FRACTIONAL_ODD,
    IL_TS_PARTITION_FRACTIONAL_EVEN,
    IL_TS_PARTITION_LAST,
};

enum ILTsOutputPrimitive
{
    IL_TS_OUTPUT_POINT,
    IL_TS_OUTPUT_LINE,
    IL_TS_OUTPUT_TRIANGLE_CW, // clockwise
    IL_TS_OUTPUT_TRIANGLE_CCW, // counter clockwise
    IL_TS_OUTPUT_LAST,
};

//Granularity of gradient
enum IL_DERIV_GRANULARITY
{
    IL_DERIVE_COARSE = 0,
    IL_DERIVE_FINE = 0x80
};

enum IL_IEEE_CONTROL
{
    IL_IEEE_IGNORE = 0,
    IL_IEEE_PRECISE = 0x1
};

enum IL_UAV_READ_TYPE
{
    IL_UAV_SC_DECIDE = 0,
    IL_UAV_FORCE_CACHED = 1,
    IL_UAV_FORCE_UNCACHED = 2
};

#ifdef __cplusplus
}
#endif

#endif // _AMDIL_H_
