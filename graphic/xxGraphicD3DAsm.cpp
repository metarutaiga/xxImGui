#include "xxGraphicD3DAsm.h"

#include "dxsdk/d3d9.h"
#include "dxsdk/d3d10TokenizedProgramFormat.hpp"

//==============================================================================
//  Direct3D 8.0 Assembly Shader
//==============================================================================
#define D3DSP_WRITEMASK_X       0x00010000
#define D3DSP_WRITEMASK_XY      0x00030000
#define D3DSP_WRITEMASK_XYZ     0x00070000
#define D3DSP_WRITEMASK_XYZW    0x000F0000
#define D3DDECLUSAGE_UNKNOWN    D3DSTT_UNKNOWN
#define D3DDECLUSAGE_2D         D3DSTT_2D
#define D3DDECLUSAGE_CUBE       D3DSTT_CUBE
#define D3DDECLUSAGE_VOLUME     D3DSTT_VOLUME
#define _SIO(sio, length)       (D3DSIO_ ## sio | (length << D3DSI_INSTLENGTH_SHIFT))
#define _TOKEN(mask, value)     ((value << D3DSP_ ## mask ## _SHIFT))                             | 0x80000000
#define _TOKEN1(mask, value)    ((value << D3DSP_ ## mask ## _SHIFT)  & D3DSP_ ## mask ## _MASK)  | 0x80000000
#define _TOKEN2(mask, value)    ((value << D3DSP_ ## mask ## _SHIFT2) & D3DSP_ ## mask ## _MASK2) | _TOKEN1(mask, value)
#define _DCL(dcl)               _TOKEN(DCL_USAGE, D3DDECLUSAGE_ ## dcl)
#define _SRC(reg, index)        _TOKEN2(REGTYPE, D3DSPR_ ## reg) | D3DVS_W_W     | D3DVS_Z_Z     | D3DVS_Y_Y     | D3DVS_X_X     | index
#define _SRCM(reg, index, m)    _TOKEN2(REGTYPE, D3DSPR_ ## reg) | D3DVS_W_ ## m | D3DVS_Z_ ## m | D3DVS_Y_ ## m | D3DVS_X_ ## m | index
#define _DST(reg, index)        _TOKEN2(REGTYPE, D3DSPR_ ## reg) | D3DSP_WRITEMASK_ALL   | index
#define _DSTM(reg, index, m)    _TOKEN2(REGTYPE, D3DSPR_ ## reg) | D3DSP_WRITEMASK_ ## m | index
//------------------------------------------------------------------------------
const DWORD vertexShaderCode10[] =
{ 
    D3DVS_VERSION(1, 0),                                                            // vs_1_0
    D3DSIO_MUL, _DST(TEMP, 0), _SRC(CONST, 0), _SRCM(INPUT, 0, X),                  // mul r0, c0, v0.x,
    D3DSIO_MAD, _DST(TEMP, 0), _SRC(CONST, 1), _SRCM(INPUT, 0, Y), _SRC(TEMP, 0),   // mad r0, c1, v0.y, r0
    D3DSIO_MAD, _DST(TEMP, 0), _SRC(CONST, 2), _SRCM(INPUT, 0, Z), _SRC(TEMP, 0),   // mad r0, c2, v0.z, r0
    D3DSIO_ADD, _DST(RASTOUT, 0), _SRC(TEMP, 0), _SRC(CONST, 3),                    // add oPos, r0, c3
    D3DSIO_MOV, _DST(ATTROUT, 0), _SRC(INPUT, 1),                                   // mov oD0, v1
    D3DSIO_MOV, _DSTM(TEXCRDOUT, 0, XY), _SRC(INPUT, 2),                            // mov oT0.xy, v2
    D3DSIO_END
};
//------------------------------------------------------------------------------
const DWORD vertexShaderCode11[] =
{ 
    D3DVS_VERSION(1, 1),                                                            // vs_1_1
    D3DSIO_DCL, _DCL(POSITION), _DST(INPUT, 0),                                     // dcl_position v0
    D3DSIO_DCL, _DCL(COLOR), _DST(INPUT, 1),                                        // dcl_color v1
    D3DSIO_DCL, _DCL(TEXCOORD), _DST(INPUT, 2),                                     // dcl_texcoord v2
    D3DSIO_MUL, _DST(TEMP, 0), _SRC(CONST, 0), _SRCM(INPUT, 0, X),                  // mul r0, c0, v0.x,
    D3DSIO_MAD, _DST(TEMP, 0), _SRC(CONST, 1), _SRCM(INPUT, 0, Y), _SRC(TEMP, 0),   // mad r0, c1, v0.y, r0
    D3DSIO_MAD, _DST(TEMP, 0), _SRC(CONST, 2), _SRCM(INPUT, 0, Z), _SRC(TEMP, 0),   // mad r0, c2, v0.z, r0
    D3DSIO_ADD, _DST(RASTOUT, 0), _SRC(TEMP, 0), _SRC(CONST, 3),                    // add oPos, r0, c3
    D3DSIO_MOV, _DST(ATTROUT, 0), _SRC(INPUT, 1),                                   // mov oD0, v1
    D3DSIO_MOV, _DSTM(TEXCRDOUT, 0, XY), _SRC(INPUT, 2),                            // mov oT0.xy, v2
    D3DSIO_END
};
//------------------------------------------------------------------------------
const DWORD pixelShaderCode11[] =
{
    D3DPS_VERSION(1, 1),                                                            // ps_1_1
    D3DSIO_TEX, _DST(TEXTURE, 0),                                                   // tex t0
    D3DSIO_MUL, _DST(TEMP, 0), _SRC(TEXTURE, 0), _SRC(INPUT, 0),                    // mul r0, t0, v0
    D3DSIO_END
};
//------------------------------------------------------------------------------
const DWORD pixelShaderCode20[] =
{
    D3DPS_VERSION(2, 0),                                                            // ps_2_0
    _SIO(DCL, 2), _DCL(UNKNOWN), _DST(INPUT, 0),                                    // dcl v0
    _SIO(DCL, 2), _DCL(UNKNOWN), _DST(TEXTURE, 0),                                  // dcl t0.xy
    _SIO(DCL, 2), _DCL(2D), _DST(SAMPLER, 0),                                       // dcl_2d s0
    _SIO(TEX, 3), _DST(TEMP, 0), _SRC(TEXTURE, 0), _SRC(SAMPLER, 0),                // texld r0, t0, s0
    _SIO(MUL, 3), _DST(TEMP, 0), _SRC(TEMP, 0), _SRC(INPUT, 0),                     // mul r0, r0, v0
    _SIO(MOV, 2), _DST(COLOROUT, 0), _SRC(TEMP, 0),                                 // mov oC0, r0
    _SIO(END, 0)
};
//------------------------------------------------------------------------------
#undef _SRC
#undef _SRCM
#undef _DST
#undef _DSTM
//==============================================================================
//  Direct3D 10.0 Assembly Shader
//==============================================================================
#define _DXBC                                   0x43425844
#define _ISGN                                   0x4e475349
#define _OSGN                                   0x4e47534f
#define _SHDR                                   0x52444853
#define D3D10_SB_4_COMPONENT_XXXX               0x00
#define D3D10_SB_4_COMPONENT_XYXX               0x04
#define D3D10_SB_4_COMPONENT_XYZW               0xe4
#define D3D10_SB_4_COMPONENT_YYYY               0x55
#define D3D10_SB_4_COMPONENT_ZZZZ               0xaa
#define D3D10_SB_4_COMPONENT_WWWW               0xff
#define D3D10_SB_OPERAND_4_COMPONENT_MASK_XY    0x30
#define D3D10_SB_OPERAND_4_COMPONENT_MASK_XYZ   0x70
#define D3D10_SB_OPERAND_4_COMPONENT_MASK_XYZW  0xf0
#define D3D10_SB_RETURN_TYPE_FLOAT4             (D3D10_SB_RETURN_TYPE_FLOAT << 0) | \
                                                (D3D10_SB_RETURN_TYPE_FLOAT << 4) | \
                                                (D3D10_SB_RETURN_TYPE_FLOAT << 8) | \
                                                (D3D10_SB_RETURN_TYPE_FLOAT << 12)
#define _VER(type, major, minor)                (ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(D3D10_SB_ ## type, major, minor))
#define _OP(opcode, length)                     (D3D10_SB_OPCODE_ ## opcode | ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(length))
#define _OPDIM(opcode, length, dim)             (_OP(opcode, length) | ENCODE_D3D10_SB_RESOURCE_DIMENSION(D3D10_SB_RESOURCE_DIMENSION_ ## dim))
#define _OPINT(opcode, length, int)             (_OP(opcode, length) | ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(D3D10_SB_INTERPOLATION_ ## int))
#define _0(index, reg)                          ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(D3D10_SB_OPERAND_INDEX_ ## index) | \
                                                ENCODE_D3D10_SB_OPERAND_TYPE(D3D10_SB_OPERAND_TYPE_ ## reg) | \
                                                ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(D3D10_SB_OPERAND_0_COMPONENT)
#define _4MM(index, reg, mask)                  ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(D3D10_SB_OPERAND_INDEX_ ## index) | \
                                                ENCODE_D3D10_SB_OPERAND_TYPE(D3D10_SB_OPERAND_TYPE_ ## reg) | \
                                                D3D10_SB_OPERAND_4_COMPONENT_MASK_ ## mask | \
                                                ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE) | \
                                                ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(D3D10_SB_OPERAND_4_COMPONENT)
#define _4M(index, reg)                         _4MM(index, reg, XYZW)
#define _4SS(index, reg, swizzle)               ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(D3D10_SB_OPERAND_INDEX_ ## index) | \
                                                ENCODE_D3D10_SB_OPERAND_TYPE(D3D10_SB_OPERAND_TYPE_ ## reg) | \
                                                (D3D10_SB_4_COMPONENT_ ## swizzle << D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SHIFT) | \
                                                ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) | \
                                                ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(D3D10_SB_OPERAND_4_COMPONENT)
#define _4S(index, reg)                         _4SS(index, reg, XYZW)
//------------------------------------------------------------------------------
const DWORD vertexShaderCode40[] =
{
    _DXBC, 0xe51cbc0d, 0x8dacc42e, 0x6b679e5d, 0xc68b0d11,
    0x00000001, 0x00000240, 3,
    0x0000002c, 0x0000009c, 0x00000110,
    _ISGN, 0x00000068, 3, 0x00000008,
    0x00000050, 0, 0, 3, 0, 0x0707,
    0x00000059, 0, 0, 3, 1, 0x0f0f,
    0x0000005f, 0, 0, 3, 2, 0x0303,
    0x49534f50, 0x4e4f4954, 0x4c4f4300, 0x5400524f, 0x4f435845, 0x0044524f,
    _OSGN, 0x0000006c, 3, 0x00000008,
    0x00000050, 0, 1, 3, 0, 0x000f,
    0x0000005c, 0, 0, 3, 1, 0x000f,
    0x00000062, 0, 0, 3, 2, 0x0c03,
    0x505f5653, 0x5449534f, 0x004e4f49, 0x4f4c4f43, 0x45540052, 0x4f4f4358, 0xab004452,
    _SHDR, 0x00000128, _VER(VERTEX_SHADER, 4, 0), 0x0000004a,
    _OP(DCL_CONSTANT_BUFFER, 4), _4S(2D, CONSTANT_BUFFER), D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED, 4,       // dcl_constantbuffer CB0[4], immediateIndexed
    _OP(DCL_INPUT, 3), _4MM(1D, INPUT, XYZ), 0,                                                                 // dcl_input v0.xyz
    _OP(DCL_INPUT, 3), _4M(1D, INPUT), 1,                                                                       // dcl_input v1.xyzw
    _OP(DCL_INPUT, 3), _4MM(1D, INPUT, XY), 2,                                                                  // dcl_input v2.xy
    _OP(DCL_OUTPUT_SIV, 4), _4M(1D, OUTPUT), 0, 1,                                                              // dcl_output o0.xyzw
    _OP(DCL_OUTPUT, 3), _4M(1D, OUTPUT), 1,                                                                     // dcl_output o1.xyzw
    _OP(DCL_OUTPUT, 3), _4MM(1D, OUTPUT, XY), 2,                                                                // dcl_output o2.xy
    _OP(DCL_TEMPS, 2), 1,                                                                                       // dcl_temps 1
    _OP(MUL, 8), _4M(1D, TEMP), 0, _4SS(1D, INPUT, XXXX), 0, _4S(2D, CONSTANT_BUFFER), 0, 0,                    // mul r0.xyzw, v0.xxxx, cb0[0].xyzw
    _OP(MAD, 10), _4M(1D, TEMP), 0, _4S(2D, CONSTANT_BUFFER), 0, 1, _4SS(1D, INPUT, YYYY), 0, _4S(1D, TEMP), 0, // mad r0.xyzw, cb0[1].xyzw, v0.yyyy, r0.xyzw
    _OP(MAD, 10), _4M(1D, TEMP), 0, _4S(2D, CONSTANT_BUFFER), 0, 2, _4SS(1D, INPUT, ZZZZ), 0, _4S(1D, TEMP), 0, // mad r0.xyzw, cb0[2].xyzw, v0.zzzz, r0.xyzw
    _OP(ADD, 8), _4M(1D, OUTPUT), 0, _4S(1D, TEMP), 0, _4S(2D, CONSTANT_BUFFER), 0, 3,                          // add o0.xyzw, r0.xyzw, cb0[3].xyzw
    _OP(MOV, 5), _4M(1D, OUTPUT), 1, _4S(1D, INPUT), 1,                                                         // mov o1.xyzw, v1.xyzw
    _OP(MOV, 5), _4MM(1D, OUTPUT, XY), 2, _4SS(1D, INPUT, XYXX), 2,                                             // mov o2.xy, v2.xyxx
    _OP(RET, 1)                                                                                                 // ret
};
//------------------------------------------------------------------------------
const DWORD pixelShaderCode40[] =
{
    _DXBC, 0xa5e778be, 0xeb700c19, 0x1facb14c, 0x97e98416,
    0x00000001, 0x00000170, 3,
    0x0000002c, 0x000000a0, 0x000000d4,
    _ISGN, 0x0000006c, 3, 0x00000008,
    0x00000050, 0, 1, 3, 0, 0x000f,
    0x0000005c, 0, 0, 3, 1, 0x0f0f,
    0x00000062, 0, 0, 3, 2, 0x0303,
    0x505f5653, 0x5449534f, 0x004e4f49, 0x4f4c4f43, 0x45540052, 0x4f4f4358, 0xab004452,
    _OSGN, 0x0000002c, 1, 0x00000008,
    0x00000020, 0, 0, 3, 0, 0x000f,
    0x545f5653, 0x45475241, 0xabab0054,
    _SHDR, 0x00000094, _VER(PIXEL_SHADER, 4, 0), 0x00000025,
    _OP(DCL_SAMPLER, 3), _0(1D, SAMPLER), D3D10_SB_SAMPLER_MODE_DEFAULT,                                        // dcl_sampler s0, mode_default
    _OPDIM(DCL_RESOURCE, 4, TEXTURE2D), _0(1D, RESOURCE), 0, D3D10_SB_RETURN_TYPE_FLOAT4,                       // dcl_resource_texture2d(float,float,float,float) t0
    _OPINT(DCL_INPUT_PS, 3, LINEAR), _4M(1D, INPUT), 1,                                                         // dcl_input_ps linear v1.xyzw
    _OPINT(DCL_INPUT_PS, 3, LINEAR), _4MM(1D, INPUT, XY), 2,                                                    // dcl_input_ps linear v2.xy
    _OP(DCL_OUTPUT, 3), _4M(1D, OUTPUT), 0,                                                                     // dcl_output o0.xyzw
    _OP(DCL_TEMPS, 2), 1,                                                                                       // dcl_temps 1
    _OP(SAMPLE, 9), _4M(1D, TEMP), 0, _4SS(1D, INPUT, XYXX), 2, _4S(1D, RESOURCE), 0, _0(1D, SAMPLER), 0,       // sample r0.xyzw, v2.xyxx, t0.xyzw, s0
    _OP(MUL, 7), _4M(1D, OUTPUT), 0, _4S(1D, TEMP), 0, _4S(1D, INPUT), 1,                                       // mul o0.xyzw, r0.xyzw, v1.xyzw
    _OP(RET, 1)                                                                                                 // ret
};
//------------------------------------------------------------------------------