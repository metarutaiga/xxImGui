# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_compile_options( -fno-exceptions
                     -fno-rtti
                     -fno-threadsafe-statics
                     -fvisibility=hidden )

# Houdini don't known llvm-ld
# if(NOT ${ANDROID_SYSROOT_ABI} STREQUAL "arm")
#     set( CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld -nostdlib++ -Wl,--build-id=sha1 -Wl,--Tbss=0x400" )
# endif()

# ARMv7 hidden symbols is broken since ndk-r17
# if(${ANDROID_SYSROOT_ABI} STREQUAL "arm")
#     add_compile_options( -mno-global-merge )
# endif()

add_library( # Sets the name of the library.
             xxImGui

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             dummy.cpp
             ../../../../../3rdParty/imgui/misc/freetype/imgui_freetype.cpp
             ../../../../../xxImGui/DearImGui.cpp
             ../../../../../xxImGui/mainJNI.cpp
             ../../../../../xxImGui/Plugin.cpp
             ../../../../../xxImGui/Renderer.cpp
             ../../../../../xxImGui/implement/imgui_impl_xx.cpp )

target_compile_definitions( xxImGui
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( xxImGui
                            PUBLIC
                            ../../../../..
                            ../../../../../3rdParty )

add_dependencies( xxImGui
                  freetype
                  imgui
                  xxGraphic )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       xxImGui

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       android
                       freetype
                       imgui
                       xxGraphic )

# The FreeType 2 library

add_library( freetype
             SHARED
             ../../../../../3rdParty/freetype/src/autofit/autofit.c
             ../../../../../3rdParty/freetype/src/base/ftbase.c
             ../../../../../3rdParty/freetype/src/base/ftbbox.c
             ../../../../../3rdParty/freetype/src/base/ftbdf.c
             ../../../../../3rdParty/freetype/src/base/ftbitmap.c
             ../../../../../3rdParty/freetype/src/base/ftcid.c
             ../../../../../3rdParty/freetype/src/base/ftdebug.c
             ../../../../../3rdParty/freetype/src/base/ftfstype.c
             ../../../../../3rdParty/freetype/src/base/ftgasp.c
             ../../../../../3rdParty/freetype/src/base/ftglyph.c
             ../../../../../3rdParty/freetype/src/base/ftgxval.c
             ../../../../../3rdParty/freetype/src/base/ftinit.c
             ../../../../../3rdParty/freetype/src/base/ftmm.c
             ../../../../../3rdParty/freetype/src/base/ftotval.c
             ../../../../../3rdParty/freetype/src/base/ftpatent.c
             ../../../../../3rdParty/freetype/src/base/ftpfr.c
             ../../../../../3rdParty/freetype/src/base/ftstroke.c
             ../../../../../3rdParty/freetype/src/base/ftsynth.c
             ../../../../../3rdParty/freetype/src/base/ftsystem.c
             ../../../../../3rdParty/freetype/src/base/fttype1.c
             ../../../../../3rdParty/freetype/src/base/ftwinfnt.c
             ../../../../../3rdParty/freetype/src/bdf/bdf.c
             ../../../../../3rdParty/freetype/src/cache/ftcache.c
             ../../../../../3rdParty/freetype/src/cff/cff.c
             ../../../../../3rdParty/freetype/src/cid/type1cid.c
             ../../../../../3rdParty/freetype/src/gzip/ftgzip.c
             ../../../../../3rdParty/freetype/src/lzw/ftlzw.c
             ../../../../../3rdParty/freetype/src/pcf/pcf.c
             ../../../../../3rdParty/freetype/src/pfr/pfr.c
             ../../../../../3rdParty/freetype/src/psaux/psaux.c
             ../../../../../3rdParty/freetype/src/pshinter/pshinter.c
             ../../../../../3rdParty/freetype/src/psnames/psmodule.c
             ../../../../../3rdParty/freetype/src/raster/raster.c
             ../../../../../3rdParty/freetype/src/sfnt/sfnt.c
             ../../../../../3rdParty/freetype/src/smooth/smooth.c
             ../../../../../3rdParty/freetype/src/truetype/truetype.c
             ../../../../../3rdParty/freetype/src/type1/type1.c
             ../../../../../3rdParty/freetype/src/type42/type42.c
             ../../../../../3rdParty/freetype/src/winfonts/winfnt.c )

target_compile_definitions( freetype
                            PUBLIC
                            FT2_BUILD_LIBRARY )

target_include_directories( freetype
                            PUBLIC
                            ../../../../../3rdParty/freetype/include )

# Dear ImGui

add_library( imgui
             SHARED
             ../../../../../3rdParty/imgui/imgui.cpp
             ../../../../../3rdParty/imgui/imgui_demo.cpp
             ../../../../../3rdParty/imgui/imgui_draw.cpp
             ../../../../../3rdParty/imgui/imgui_tables.cpp
             ../../../../../3rdParty/imgui/imgui_widgets.cpp )

target_compile_definitions( imgui
                            PUBLIC
                            IMGUI_USER_CONFIG="../../xxImGui/implement/imgui_user_config.h"
                            IMGUI_API=__attribute__\(\(visibility\(\"default\"\)\)\) )

target_include_directories( imgui
                            PUBLIC
                            ../../../../../3rdParty/imgui
                            ../../../../../3rdParty/stb )

# xxGraphic

add_library( xxGraphic
             SHARED
             ../../../../../xxGraphic/internal/xxGraphicInternalGL.cpp
             ../../../../../xxGraphic/internal/xxGraphicInternalVulkan.cpp
             ../../../../../xxGraphic/xxGraphic.cpp
             ../../../../../xxGraphic/xxGraphicEGL.cpp
             ../../../../../xxGraphic/xxGraphicGLES2.cpp
             ../../../../../xxGraphic/xxGraphicGLES3.cpp
             ../../../../../xxGraphic/xxGraphicGLES31.cpp
             ../../../../../xxGraphic/xxGraphicGLES32.cpp
             ../../../../../xxGraphic/xxGraphicNULL.cpp
             ../../../../../xxGraphic/xxGraphicVulkan.cpp
             ../../../../../xxGraphic/xxGraphicVulkanAsm.cpp
             ../../../../../xxGraphic/xxSystem.cpp )

target_compile_definitions( xxGraphic
                            PUBLIC
                            XX_BUILD_LIBRARY )

target_include_directories( xxGraphic
                            PUBLIC
                            ../../../../../xxGraphic )

target_link_libraries( xxGraphic
                       ${log-lib}
                       android )

# xxGraphicPlus

add_library( xxGraphicPlus
             SHARED
             ../../../../../xxGraphic/utility/xxImage.cpp
             ../../../../../xxGraphic/utility/xxMaterial.cpp
             ../../../../../xxGraphic/utility/xxMath.cpp
             ../../../../../xxGraphic/utility/xxMesh.cpp
             ../../../../../xxGraphic/utility/xxNode.cpp )

target_compile_definitions( xxGraphicPlus
                            PUBLIC
                            XXPLUS_BUILD_LIBRARY )

target_include_directories( xxGraphicPlus
                            PUBLIC
                            ../../../../../xxGraphic )

target_link_libraries( xxGraphicPlus
                       ${log-lib}
                       android
                       xxGraphic )

# sample

add_library( sample
             SHARED
             ../../../../../plugin/sample/sample.cpp )

target_compile_definitions( sample
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( sample
                            PUBLIC
                            ../../../../..
                            ../../../../../3rdParty
                            ../../../../../plugin )

target_link_libraries( sample
                       imgui )

# validator

add_library( validator
             SHARED
             ../../../../../plugin/validator/validator.cpp )

target_compile_definitions( validator
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( validator
                            PUBLIC
                            ../../../../..
                            ../../../../../3rdParty
                            ../../../../../xxGraphic
                            ../../../../../plugin )

target_link_libraries( validator
                       imgui
                       xxGraphic
                       xxGraphicPlus )
