/****************************************************************************
 *
 * ftoption.h
 *
 *   User-selectable configuration macros (specification only).
 *
 * Copyright (C) 1996-2021 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <freetype/config/ftoption.h>

#undef FT_CONFIG_OPTION_ENVIRONMENT_PROPERTIES
#undef FT_CONFIG_OPTION_USE_LZW
#undef FT_CONFIG_OPTION_USE_ZLIB
#undef FT_CONFIG_OPTION_POSTSCRIPT_NAMES
#undef FT_CONFIG_OPTION_ADOBE_GLYPH_LIST
#undef FT_CONFIG_OPTION_MAC_FONTS
#undef FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK
#undef FT_CONFIG_OPTION_INCREMENTAL

/* #undef TT_CONFIG_OPTION_EMBEDDED_BITMAPS */
#undef TT_CONFIG_OPTION_COLOR_LAYERS
#undef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
#undef TT_CONFIG_OPTION_SFNT_NAMES
#undef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#undef TT_CONFIG_OPTION_GX_VAR_SUPPORT
#undef TT_CONFIG_OPTION_BDF

#undef TT_CONFIG_CMAP_FORMAT_0
#undef TT_CONFIG_CMAP_FORMAT_2
/* #undef TT_CONFIG_CMAP_FORMAT_4 */
#undef TT_CONFIG_CMAP_FORMAT_6
#undef TT_CONFIG_CMAP_FORMAT_8
#undef TT_CONFIG_CMAP_FORMAT_10
#undef TT_CONFIG_CMAP_FORMAT_12
#undef TT_CONFIG_CMAP_FORMAT_13
#undef TT_CONFIG_CMAP_FORMAT_14

#undef TT_USE_BYTECODE_INTERPRETER
#undef TT_SUPPORT_COLRV1
#undef TT_SUPPORT_SUBPIXEL_HINTING_MINIMAL

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#define sfnt_find_encoding FT_ERR_CAT(sfnt_find_encoding, __LINE__)
#define sfnt_find_encoding1192(...) FT_ENCODING_UNICODE

/* END */
