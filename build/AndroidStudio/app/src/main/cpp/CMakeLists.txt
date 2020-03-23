# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             xxImGui

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             dummy.cpp
             ../../../../../../xxImGui/DearImGui.cpp
             ../../../../../../xxImGui/mainJNI.cpp
             ../../../../../../xxImGui/Plugin.cpp
             ../../../../../../xxImGui/Renderer.cpp
             ../../../../../../xxImGui/implement/imgui_impl_xx.cpp
             ../../../../../../xxImGui/implement/imgui_freetype.cpp )

target_compile_definitions( xxImGui
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( xxImGui
                            PUBLIC
                            ../../../../../.. )

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
             ../../../../../../freetype/src/autofit/autofit.c
             ../../../../../../freetype/src/base/ftbase.c
             ../../../../../../freetype/src/base/ftbbox.c
             ../../../../../../freetype/src/base/ftbdf.c
             ../../../../../../freetype/src/base/ftbitmap.c
             ../../../../../../freetype/src/base/ftcid.c
             ../../../../../../freetype/src/base/ftdebug.c
             ../../../../../../freetype/src/base/ftfstype.c
             ../../../../../../freetype/src/base/ftgasp.c
             ../../../../../../freetype/src/base/ftglyph.c
             ../../../../../../freetype/src/base/ftgxval.c
             ../../../../../../freetype/src/base/ftinit.c
             ../../../../../../freetype/src/base/ftmm.c
             ../../../../../../freetype/src/base/ftotval.c
             ../../../../../../freetype/src/base/ftpatent.c
             ../../../../../../freetype/src/base/ftpfr.c
             ../../../../../../freetype/src/base/ftstroke.c
             ../../../../../../freetype/src/base/ftsynth.c
             ../../../../../../freetype/src/base/ftsystem.c
             ../../../../../../freetype/src/base/fttype1.c
             ../../../../../../freetype/src/base/ftwinfnt.c
             ../../../../../../freetype/src/bdf/bdf.c
             ../../../../../../freetype/src/cache/ftcache.c
             ../../../../../../freetype/src/cff/cff.c
             ../../../../../../freetype/src/cid/type1cid.c
             ../../../../../../freetype/src/gzip/ftgzip.c
             ../../../../../../freetype/src/lzw/ftlzw.c
             ../../../../../../freetype/src/pcf/pcf.c
             ../../../../../../freetype/src/pfr/pfr.c
             ../../../../../../freetype/src/psaux/psaux.c
             ../../../../../../freetype/src/pshinter/pshinter.c
             ../../../../../../freetype/src/psnames/psmodule.c
             ../../../../../../freetype/src/raster/raster.c
             ../../../../../../freetype/src/sfnt/sfnt.c
             ../../../../../../freetype/src/smooth/smooth.c
             ../../../../../../freetype/src/truetype/truetype.c
             ../../../../../../freetype/src/type1/type1.c
             ../../../../../../freetype/src/type42/type42.c
             ../../../../../../freetype/src/winfonts/winfnt.c )

target_compile_definitions( freetype
                            PUBLIC
                            FT2_BUILD_LIBRARY )

target_include_directories( freetype
                            PUBLIC
                            ../../../../../../freetype/include )

# Dear ImGui

add_library( imgui
             SHARED
             ../../../../../../imgui/imgui.cpp
             ../../../../../../imgui/imgui_demo.cpp
             ../../../../../../imgui/imgui_draw.cpp
             ../../../../../../imgui/imgui_widgets.cpp )

target_compile_definitions( imgui
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( imgui
                            PUBLIC
                            ../../../../../../imgui )

# xxGraphic

add_library( xxGraphic
             SHARED
             ../../../../../../xxGraphic/utility/xxImage.cpp
             ../../../../../../xxGraphic/utility/xxMaterial.cpp
             ../../../../../../xxGraphic/utility/xxMath.cpp
             ../../../../../../xxGraphic/utility/xxMesh.cpp
             ../../../../../../xxGraphic/utility/xxNode.cpp
             ../../../../../../xxGraphic/xxGraphic.cpp
             ../../../../../../xxGraphic/xxGraphicEGL.cpp
             ../../../../../../xxGraphic/xxGraphicGL.cpp
             ../../../../../../xxGraphic/xxGraphicGLES2.cpp
             ../../../../../../xxGraphic/xxGraphicGLES3.cpp
             ../../../../../../xxGraphic/xxGraphicGLES31.cpp
             ../../../../../../xxGraphic/xxGraphicGLES32.cpp
             ../../../../../../xxGraphic/xxGraphicNULL.cpp
             ../../../../../../xxGraphic/xxGraphicVulkan.cpp
             ../../../../../../xxGraphic/xxGraphicVulkanAsm.cpp
             ../../../../../../xxGraphic/xxSystem.cpp )

target_compile_definitions( xxGraphic
                            PUBLIC
                            XX_BUILD_LIBRARY )

target_include_directories( xxGraphic
                            PUBLIC
                            ../../../../../../xxGraphic )

target_link_libraries( xxGraphic
                       ${log-lib}
                       android )

# sample

add_library( sample
             SHARED
             ../../../../../../plugin/sample/sample.cpp )

target_compile_definitions( sample
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( sample
                            PUBLIC
                            ../../../../../..
                            ../../../../../../plugin )

target_link_libraries( sample
                       imgui )

# validator

add_library( validator
             SHARED
             ../../../../../../plugin/validator/validator.cpp )

target_compile_definitions( validator
                            PUBLIC
                            IMGUI_USER_CONFIG="../xxImGui/implement/imgui_user_config.h" )

target_include_directories( validator
                            PUBLIC
                            ../../../../../..
                            ../../../../../../xxGraphic
                            ../../../../../../plugin )

target_link_libraries( validator
                       imgui
                       xxGraphic )