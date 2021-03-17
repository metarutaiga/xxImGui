//==============================================================================
// xxImGui : Plugin YUV Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#include <xxYUV/rgb2yuv.h>
#include <xxYUV/yuv2rgb.h>
#include <interface.h>

#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#endif

#define LIBYUV 1
#if defined(LIBYUV)
#include <libyuv.h>
#endif

#if defined(__APPLE__) && defined(__aarch64__)
void yuv2rgb_amx(int width, int height, const void* y, const void* u, const void* v, int strideY, int strideU, int strideV, void* rgb, int strideRGB);
#endif

#define PLUGIN_NAME     "YUV"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static int lennaWidth = 512;
static int lennaHeight = 512;
static unsigned char* lennaRGB = nullptr;
static unsigned char* lennaYU12 = nullptr;
static unsigned char* lennaYV12 = nullptr;
static unsigned char* lennaNV12 = nullptr;
static unsigned char* lennaNV21 = nullptr;

static uint64_t source = 0;
static uint64_t target = 0;

//------------------------------------------------------------------------------
template<unsigned int targetPixel, unsigned int sourcePixel>
static void loadTextureFromImage(uint64_t& texture, uint64_t device, const void* data, unsigned int width, unsigned int height)
{
    if (texture != 0)
        xxDestroyTexture(texture);
    texture = 0;

    if (data == nullptr)
        return;

    texture = xxCreateTexture(device, 0, width, height, 1, 1, 1, nullptr);
    if (texture)
    {
        int stride = 0;
        char* pointer = (char*)xxMapTexture(device, texture, &stride, 0, 0);
        if (pointer)
        {
            char* input = (char*)data;
            for (unsigned int y = 0; y < height; ++y)
            {
                char* output = pointer;
                for (unsigned int x = 0; x < width; ++x)
                {
#if defined(xxWINDOWS)
                    if (targetPixel >= 1) (*output++) = (sourcePixel >= 1) ? input[x * sourcePixel + 0] : -1;
                    if (targetPixel >= 2) (*output++) = (sourcePixel >= 2) ? input[x * sourcePixel + 1] : -1;
                    if (targetPixel >= 3) (*output++) = (sourcePixel >= 3) ? input[x * sourcePixel + 2] : -1;
                    if (targetPixel >= 4) (*output++) = (sourcePixel >= 4) ? input[x * sourcePixel + 3] : -1;
#else
                    if (targetPixel >= 1) (*output++) = (sourcePixel >= 1) ? input[x * sourcePixel + 2] : -1;
                    if (targetPixel >= 2) (*output++) = (sourcePixel >= 2) ? input[x * sourcePixel + 1] : -1;
                    if (targetPixel >= 3) (*output++) = (sourcePixel >= 3) ? input[x * sourcePixel + 0] : -1;
                    if (targetPixel >= 4) (*output++) = (sourcePixel >= 4) ? input[x * sourcePixel + 3] : -1;
#endif
                }
                pointer += stride;
                input += width * sourcePixel;
            }

            xxUnmapTexture(device, texture, 0, 0);
        }
    }
}
//------------------------------------------------------------------------------
static void loadTexture(const char* baseFolder, bool videoRange)
{
    char path[1024];
#if defined(xxMACOS)
    snprintf(path, 1024, "%s/../../../../%s/%s", baseFolder, "resource", "lenna.rgb");
#elif defined(xxIOS)
    snprintf(path, 1024, "%s/%s", baseFolder, "lenna.rgb");
#else
    snprintf(path, 1024, "%s/%s/%s", baseFolder, "resource", "lenna.rgb");
#endif

    FILE* file = fopen(path, "rb");
    if (file)
    {
        lennaRGB = (unsigned char*)malloc(3 * lennaWidth * lennaHeight);
        if (lennaRGB)
        {
            fread(lennaRGB, 3, lennaWidth * lennaHeight, file);
        }
        fclose(file);

        int sizeY = lennaWidth * lennaHeight;
        int sizeUV = lennaWidth / 2 * lennaHeight / 2;

        // YU12
        lennaYU12 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 256);
        if (lennaYU12 && lennaRGB)
        {
            rgb2yuv_yu12(lennaWidth, lennaHeight, lennaRGB, lennaYU12, 3, true);
        }

        // YV12
        lennaYV12 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 256);
        if (lennaYV12 && lennaRGB)
        {
            rgb2yuv_yv12(lennaWidth, lennaHeight, lennaRGB, lennaYV12, 3, true);
        }

        // NV12
        lennaNV12 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 256);
        if (lennaNV12 && lennaRGB)
        {
            rgb2yuv_nv12(lennaWidth, lennaHeight, lennaRGB, lennaNV12, 3, true);
        }

        // NV21
        lennaNV21 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 256);
        if (lennaNV21 && lennaRGB)
        {
            rgb2yuv_nv21(lennaWidth, lennaHeight, lennaRGB, lennaNV21, 3, true);
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI const char* Create(const CreateData& createData)
{
    loadTexture(createData.baseFolder, false);

    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{
    free(lennaRGB);
    xxAlignedFree(lennaYU12);
    xxAlignedFree(lennaYV12);
    xxAlignedFree(lennaNV12);
    xxAlignedFree(lennaNV21);
}
//------------------------------------------------------------------------------
pluginAPI void Update(const UpdateData& updateData)
{
    static bool showDialog = false;
    static bool showAbout = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Dialog", nullptr, &showDialog);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showAbout)
    {
        if (ImGui::Begin("About " PLUGIN_NAME, &showAbout))
        {
            ImGui::Text("%s Plugin Version %d.%d", PLUGIN_NAME, PLUGIN_MAJOR, PLUGIN_MINOR);
            ImGui::Separator();
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
            ImGui::End();
        }
    }

    if (showDialog)
    {
        if (ImGui::Begin("Dialog", &showDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            bool click = false;

            static bool encodeFullRange = false;
            click |= ImGui::Checkbox("Encode FullRange", &encodeFullRange);
            ImGui::SameLine();

            static int format = 0;
            click |= ImGui::RadioButton("YU12", &format, 0);
            ImGui::SameLine();
            click |= ImGui::RadioButton("YV12", &format, 1);
            ImGui::SameLine();
            click |= ImGui::RadioButton("NV12", &format, 2);
            ImGui::SameLine();
            click |= ImGui::RadioButton("NV21", &format, 3);

            static bool decodeFullRange = false;
            click |= ImGui::Checkbox("Decode FullRange", &decodeFullRange);
            ImGui::SameLine();

            static bool libyuv = false;
            static bool accelerate = false;
#if defined(__APPLE__) && defined(__aarch64__)
            static bool appleamx = false;
#endif

#if LIBYUV
            if (ImGui::Checkbox("libYUV", &libyuv))
            {
                click = true;
                accelerate &= !libyuv;
#if defined(__APPLE__) && defined(__aarch64__)
                appleamx &= !libyuv;
#endif
            }
            ImGui::SameLine();
#endif

#if defined(__APPLE__)
            if (ImGui::Checkbox("Accelerate", &accelerate))
            {
                click = true;
                libyuv &= !accelerate;
#if defined(__APPLE__) && defined(__aarch64__)
                appleamx &= !accelerate;
#endif
            }
            ImGui::SameLine();
#endif

#if defined(__APPLE__) && defined(__aarch64__)
            if (ImGui::Checkbox("Apple AMX", &appleamx))
            {
                click = true;
                libyuv &= !appleamx;
                accelerate &= !appleamx;
            }
            ImGui::SameLine();
#endif

            static uint64_t encodeTSC = 0;
            static uint64_t decodeTSC = 0;
            static float encodeTime = 0.0f;
            static float decodeTime = 0.0f;
            if (ImGui::Button("Convert") || click)
            {
                unsigned char* temp = (unsigned char*)xxAlignedAlloc(4 * lennaWidth * lennaHeight, 256);
                int sizeY = lennaWidth * lennaHeight;
                int sizeUV = lennaWidth / 2 * lennaHeight / 2;
                int count = 1000;

                if (temp)
                {
                    for (int y = 0; y < lennaHeight; ++y)
                    {
                        for (int x = 0; x < lennaWidth; ++x)
                        {
                            temp[(y * lennaWidth + x) * 4 + 0] = lennaRGB[(y * lennaWidth + x) * 3 + 0];
                            temp[(y * lennaWidth + x) * 4 + 1] = lennaRGB[(y * lennaWidth + x) * 3 + 1];
                            temp[(y * lennaWidth + x) * 4 + 2] = lennaRGB[(y * lennaWidth + x) * 3 + 2];
                        }
                    }

                    uint64_t startTSC = xxTSC();
                    for (int i = 0; i < count; ++i)
                    {
                        switch (format)
                        {
                        default:
                        case 0:
#if LIBYUV
                            if (libyuv)
                            {
                                auto converter = encodeFullRange ? libyuv::ARGBToJ420 : libyuv::ARGBToI420;
                                converter(temp, lennaWidth * 4, lennaYU12, lennaWidth, lennaYU12 + sizeY, lennaWidth / 2, lennaYU12 + sizeY + sizeUV, lennaWidth / 2, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            if (temp && lennaYU12)
                            {
                                rgb2yuv_yu12(lennaWidth, lennaHeight, temp, lennaYU12, 4, true, encodeFullRange);
                            }
                            break;
                        case 1:
#if LIBYUV
                            if (libyuv)
                            {
                                auto converter = encodeFullRange ? libyuv::ARGBToJ420 : libyuv::ARGBToI420;
                                converter(temp, lennaWidth * 4, lennaYV12, lennaWidth, lennaYV12 + sizeY + sizeUV, lennaWidth / 2, lennaYV12 + sizeY, lennaWidth / 2, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            if (temp && lennaYV12)
                            {
                                rgb2yuv_yv12(lennaWidth, lennaHeight, temp, lennaYV12, 4, true, encodeFullRange);
                            }
                            break;
                        case 2:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::ARGBToNV12(temp, lennaWidth * 4, lennaNV12, lennaWidth, lennaNV12 + sizeY, lennaWidth, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            if (temp && lennaNV12)
                            {
                                rgb2yuv_nv12(lennaWidth, lennaHeight, temp, lennaNV12, 4, true, encodeFullRange);
                            }
                            break;
                        case 3:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::ARGBToNV21(temp, lennaWidth * 4, lennaNV21, lennaWidth, lennaNV21 + sizeY, lennaWidth, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            if (temp && lennaNV21)
                            {
                                rgb2yuv_nv21(lennaWidth, lennaHeight, temp, lennaNV21, 4, true, encodeFullRange);
                            }
                            break;
                        }
                    }
                    encodeTSC = (xxTSC() - startTSC) / count;
                    encodeTime = encodeTSC / float(xxTSCFrequency());
                }

                if (temp)
                {
                    memset(temp, 0, 4 * lennaWidth * lennaHeight);

                    uint64_t startTSC = xxTSC();
                    for (int i = 0; i < count; ++i)
                    {
                        switch (format)
                        {
                        default:
                        case 0:
#if LIBYUV
                            if (libyuv)
                            {
                                auto converter = decodeFullRange ? libyuv::J420ToARGB : libyuv::I420ToARGB;
                                converter(lennaYU12, lennaWidth, lennaYU12 + sizeY, lennaWidth / 2, lennaYU12 + sizeY + sizeUV, lennaWidth / 2, temp, lennaWidth * 4, lennaWidth, lennaHeight);
                                break;
                            }
#endif
#if defined(__APPLE__)
                            if (accelerate)
                            {
                                static vImage_YpCbCrToARGB fullRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_Cb8_Cr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();
                                static vImage_YpCbCrToARGB videoRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_Cb8_Cr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();

                                vImage_Buffer Y = { lennaYU12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer Cb = { lennaYU12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer Cr = { lennaYU12 + sizeY + sizeUV, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_Cb8_Cr8ToARGB8888(&Y, &Cb, &Cr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                                break;
                            }
#endif
#if defined(__APPLE__) && defined(__aarch64__)
                            if (appleamx)
                            {
                                yuv2rgb_amx(lennaWidth, lennaHeight, lennaYU12,  lennaYU12 + sizeY, lennaYU12 + sizeY + sizeUV, lennaWidth, lennaWidth / 2, lennaWidth / 2, temp, lennaWidth * 4);
                                if (i == count - 1)
                                {
                                    for (int x = 0; x < lennaWidth; ++x)
                                    {
                                        for (int y = 0; y < lennaHeight; ++y)
                                        {
                                            temp[(y * lennaWidth + x) * 4 + 3] = 0xFF;
                                        }
                                    }
                                }
                                break;
                            }
#endif
                            yuv2rgb_yu12(lennaWidth, lennaHeight, lennaYU12, temp, decodeFullRange, 4, true);
                            break;
                        case 1:
#if LIBYUV
                            if (libyuv)
                            {
                                auto converter = decodeFullRange ? libyuv::J420ToARGB : libyuv::I420ToARGB;
                                converter(lennaYV12, lennaWidth, lennaYV12 + sizeY + sizeUV, lennaWidth / 2, lennaYV12 + sizeY, lennaWidth / 2, temp, lennaWidth * 4, lennaWidth, lennaHeight);
                                break;
                            }
#endif
#if defined(__APPLE__)
                            if (accelerate)
                            {
                                static vImage_YpCbCrToARGB fullRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_Cb8_Cr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();
                                static vImage_YpCbCrToARGB videoRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_Cb8_Cr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();

                                vImage_Buffer Y = { lennaYV12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer Cb = { lennaYV12 + sizeY + sizeUV, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer Cr = { lennaYV12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_Cb8_Cr8ToARGB8888(&Y, &Cb, &Cr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                                break;
                            }
#endif
                            yuv2rgb_yv12(lennaWidth, lennaHeight, lennaYV12, temp, decodeFullRange, 4, true);
                            break;
                        case 2:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::NV12ToARGB(lennaNV12, lennaWidth, lennaNV12 + sizeY, lennaWidth, temp, lennaWidth * 4, lennaWidth, lennaHeight);
                                break;
                            }
#endif
#if defined(__APPLE__)
                            if (accelerate)
                            {
                                static vImage_YpCbCrToARGB fullRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_CbCr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();
                                static vImage_YpCbCrToARGB videoRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_CbCr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();

                                vImage_Buffer Y = { lennaNV12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer CbCr = { lennaNV12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_CbCr8ToARGB8888(&Y, &CbCr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                                break;
                            }
#endif
                            yuv2rgb_nv12(lennaWidth, lennaHeight, lennaNV12, temp, decodeFullRange, 4, true);
                            break;
                        case 3:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::NV21ToARGB(lennaNV21, lennaWidth, lennaNV21 + sizeY, lennaWidth, temp, lennaWidth * 4, lennaWidth, lennaHeight);
                                break;
                            }
#endif
#if defined(__APPLE__)
                            if (accelerate)
                            {
                                static vImage_YpCbCrToARGB fullRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_CbCr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();
                                static vImage_YpCbCrToARGB videoRange = []()
                                {
                                    vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                    vImage_YpCbCrToARGB info;
                                    vImageConvert_YpCbCrToARGB_GenerateConversion(kvImage_YpCbCrToARGBMatrix_ITU_R_709_2, &range, &info, kvImage420Yp8_CbCr8, kvImageARGB8888, kvImageNoFlags);
                                    return info;
                                }();

                                vImage_Buffer Y = { lennaNV21, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer CbCr = { lennaNV21 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_CbCr8ToARGB8888(&Y, &CbCr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                                break;
                            }
#endif
                            yuv2rgb_nv21(lennaWidth, lennaHeight, lennaNV21, temp, decodeFullRange, 4, true);
                            break;
                        }
                    }
                    decodeTSC = (xxTSC() - startTSC) / count;
                    decodeTime = decodeTSC / float(xxTSCFrequency());

                    loadTextureFromImage<4, 4>(target, updateData.device, temp, lennaWidth, lennaHeight);
                }

                xxAlignedFree(temp);
            }

            float windowScale = updateData.windowScale;
#if defined(xxIOS)
            windowScale = 1.0f;
#endif

            if (source == 0)
            {
                loadTextureFromImage<4, 3>(source, updateData.device, lennaRGB, lennaWidth, lennaHeight);
            }
            if (source != 0)
            {
                ImGui::Image((ImTextureID)source, ImVec2(lennaWidth * windowScale, lennaHeight * windowScale));
            }
            if (target != 0)
            {
                ImGui::SameLine();
                ImGui::Image((ImTextureID)target, ImVec2(lennaWidth * windowScale, lennaHeight * windowScale));
            }

            ImGui::Text("Encode TSC : %.3fus %llu", encodeTime * 1000.0f, encodeTSC);
            ImGui::Text("Decode TSC : %.3fus %llu", decodeTime * 1000.0f, decodeTSC);

            ImGui::End();
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
#if defined(__APPLE__) && defined(__aarch64__)
//------------------------------------------------------------------------------
//  https://gist.github.com/dougallj/7cba721da1a94da725ee37c1e9cd1f21
//------------------------------------------------------------------------------
union amx_access
{
    struct
    {
        uint64_t address:56;
        uint64_t register_index:6;
        uint64_t double_width:1;
    };
    uint64_t value;
};
//------------------------------------------------------------------------------
union amx_operands_matrix
{
    struct
    {
        uint64_t offset_x:10;
        uint64_t offset_y:10;
        uint64_t offset_z:7;
        uint64_t skip_z:1;
        uint64_t skip_y:1;
        uint64_t skip_x:1;
        uint64_t dummy_30:2;
        uint64_t disable_x:7;
        uint64_t dummy_39:2;
        uint64_t disable_y:7;
        uint64_t dummy_48:13;
        uint64_t mode_8:1;
        uint64_t mode_32:1;
        uint64_t vector_matrix_add:1;
    };
    uint64_t value;
};
union amx_operands_vector
{
    struct
    {
        uint64_t offset_y:10;
        uint64_t offset_x:10;
        uint64_t offset_z:7;
        uint64_t count_y:2;
        uint64_t count_x:2;
        uint64_t dummy_31:1;
        uint64_t mask:10;
        uint64_t extended:4;
        uint64_t dummy_46:1;
        uint64_t neg:1;
        uint64_t add:1;
        uint64_t dummy_49:9;
        uint64_t shift_right:5;
        uint64_t sign:1;
    };
    uint64_t value;
};
//------------------------------------------------------------------------------
#define AMX_LDX(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 0 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_LDY(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 1 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_STX(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 2 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_STY(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 3 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_LDZ(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 4 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_STZ(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 5 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_EXTRX(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 8 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_EXTRY(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 9 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_FMA64(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (10 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_FMA32(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (12 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_MAC16(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (14 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_FMA16(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (15 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_START()     __asm__ volatile("nop \n nop \n nop \n .word (0x201000 | (17 << 5) | 0)" ::: "memory")
#define AMX_STOP()      __asm__ volatile("nop \n nop \n nop \n .word (0x201000 | (17 << 5) | 1)" ::: "memory")
#define AMX_VECINT(V)   __asm__ volatile("mov x0, %0        \n .word (0x201000 | (18 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_VECFP(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (19 << 5) | 0)" ::"r"(V) : "x0", "memory")
//------------------------------------------------------------------------------
// BT.709 - Full Range
//     Y         U         V
// R = 1.000000  0.000000  1.581000
// G = 1.000000 -0.188062 -0.469967
// B = 1.000000  1.862906  0.000000
#define fY   1.000000
#define fUG -0.188062
#define fUB  1.862906
#define fVR  1.581000
#define fVG -0.469967
//------------------------------------------------------------------------------
void dump_value(const void* data, size_t index)
{
    uint8_t row[64];
    memcpy(row, data, 64);
    printf("%2zd : ", index);
    for (int i = 0; i < 64; ++i)
    {
        printf("%02X", row[i]);
        if (i % 8 == 7)
            printf(" ");
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void dump_amx(size_t index, int16_t hint)
{
    uint8_t row[64];
    AMX_STZ((amx_access{ .address = (uint64_t)row, .register_index = index }));
    printf("%2zd : ", index);
    for (int i = 0; i < 64; ++i)
    {
        if (i == 0)
        {
            printf("(%04X) ", __builtin_bswap16(hint) & 0xFFFF);
        }
        printf("%02X", row[i]);
        if (i % 8 == 7)
            printf(" ");
    }
    printf("\n");
}
//------------------------------------------------------------------------------
void yuv2rgb_amx(int width, int height, const void* y, const void* u, const void* v, int strideY, int strideU, int strideV, void* rgb, int strideRGB)
{
    int halfWidth = width >> 1;
    int halfHeight = height >> 1;

    int16_t Y, UG, UB, VR, VG;
    if (true)
    {
        Y = (int)(fY * 256);
        UG = (int)(fUG * 255); UB = (int)(fUB * 255);
        VR = (int)(fVR * 255); VG = (int)(fVG * 255);
        Y >>= 1;
        UG >>= 1;
        UB >>= 3;
        VR >>= 2;
        VG >>= 1;
    }

    static const bool dump = false;
    static const int16_t vector256[32] =
    {
        256, 256, 256, 256, 256, 256, 256, 256,
        256, 256, 256, 256, 256, 256, 256, 256,
        256, 256, 256, 256, 256, 256, 256, 256,
        256, 256, 256, 256, 256, 256, 256, 256,
    };

    static const int16_t vectorN128[32] =
    {
        -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128,
    };

    static const int16_t vectorY[32] =
    {
        Y, Y, Y, Y, Y, Y, Y, Y,
        Y, Y, Y, Y, Y, Y, Y, Y,
        Y, Y, Y, Y, Y, Y, Y, Y,
        Y, Y, Y, Y, Y, Y, Y, Y,
    };

    static const int16_t vectorVR[32] =
    {
        VR, VR, VR, VR, VR, VR, VR, VR,
        VR, VR, VR, VR, VR, VR, VR, VR,
        VR, VR, VR, VR, VR, VR, VR, VR,
        VR, VR, VR, VR, VR, VR, VR, VR,
    };

    static const int16_t vectorUG[32] =
    {
        UG, UG, UG, UG, UG, UG, UG, UG,
        UG, UG, UG, UG, UG, UG, UG, UG,
        UG, UG, UG, UG, UG, UG, UG, UG,
        UG, UG, UG, UG, UG, UG, UG, UG,
    };

    static const int16_t vectorVG[32] =
    {
        VG, VG, VG, VG, VG, VG, VG, VG,
        VG, VG, VG, VG, VG, VG, VG, VG,
        VG, VG, VG, VG, VG, VG, VG, VG,
        VG, VG, VG, VG, VG, VG, VG, VG,
    };

    static const int16_t vectorUB[32] =
    {
        UB, UB, UB, UB, UB, UB, UB, UB,
        UB, UB, UB, UB, UB, UB, UB, UB,
        UB, UB, UB, UB, UB, UB, UB, UB,
        UB, UB, UB, UB, UB, UB, UB, UB,
    };

    AMX_START();
    AMX_LDY((amx_access{ .address = (uint64_t)vector256, .register_index = 1 }));
    AMX_LDY((amx_access{ .address = (uint64_t)vectorN128, .register_index = 2 }));
    AMX_LDY((amx_access{ .address = (uint64_t)vectorY, .register_index = 3 }));
    AMX_LDY((amx_access{ .address = (uint64_t)vectorVR, .register_index = 4 }));
    AMX_LDY((amx_access{ .address = (uint64_t)vectorUG, .register_index = 5 }));
    AMX_LDY((amx_access{ .address = (uint64_t)vectorVG, .register_index = 6 }));
    AMX_LDY((amx_access{ .address = (uint64_t)vectorUB, .register_index = 7 }));
    for (int h = 0; h < halfHeight; ++h)
    {
        const unsigned char* y0 = (unsigned char*)y;
        const unsigned char* y1 = y0 + strideY;         y = y1 + strideY;
        const unsigned char* u0 = (unsigned char*)u;    u = u0 + strideU;
        const unsigned char* v0 = (unsigned char*)v;    v = v0 + strideV;
        unsigned char* rgb0 = (unsigned char*)rgb;
        unsigned char* rgb1 = rgb0 + strideRGB;         rgb = rgb1 + strideRGB;
        int halfWidth128 = width / 128;
        for (int w = 0; w < halfWidth128; ++w)
        {
            AMX_MAC16((amx_operands_matrix{ .skip_x = 1, .skip_y = 1, .skip_z = 1, .mode_32 = 1 }));
            if (dump && w == 2 && h == 8)
            {
                dump_value(y0, 0);
                dump_value(y1, 1);
            }

#if 1
            //  0 RGBA
            //  8 RGBA
            // 16 RGBA
            // 24 RGBA
            // 32 R = Y     + V
            // 40 G = Y + U + V
            // 48 B = Y + U     (Y0 Y1)
            // 56 U
            // 60 V
            int16_t hintY0 = y0[0];
            int16_t hintY1 = y1[0];
            int16_t hintU = u0[0] - 128;
            int16_t hintV = v0[0] - 128;
            int16_t hintR = ((hintY0 * Y) >> 7) +                       ((hintV * VR) >> 6);
            int16_t hintG = ((hintY0 * Y) >> 7) + ((hintU * UG) >> 7) + ((hintV * VG) >> 7);
            int16_t hintB = ((hintY0 * Y) >> 7) + ((hintU * UB) >> 5);

            // Load
            AMX_LDX((amx_access{ .address = (uint64_t)y0 +  0, .register_index = 0 }));
            AMX_LDX((amx_access{ .address = (uint64_t)y0 + 64, .register_index = 1 }));  y0 += 128;
            AMX_LDX((amx_access{ .address = (uint64_t)y1 +  0, .register_index = 2 }));
            AMX_LDX((amx_access{ .address = (uint64_t)y1 + 64, .register_index = 3 }));  y1 += 128;
            AMX_LDX((amx_access{ .address = (uint64_t)u0 +  0, .register_index = 4 }));  u0 += 64;
            AMX_LDX((amx_access{ .address = (uint64_t)v0 +  0, .register_index = 5 }));  v0 += 64;

            // Y
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x0C0, .offset_z = 32, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x0C0, .offset_z = 40, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x0C0, .offset_z = 48, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x0C0, .offset_z = 34, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x0C0, .offset_z = 42, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x0C0, .offset_z = 50, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x0C0, .offset_z = 36, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x0C0, .offset_z = 44, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x0C0, .offset_z = 52, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x0C0, .offset_z = 38, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x0C0, .offset_z = 46, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x0C0, .offset_z = 54, .count_x = 1, .mask = 64, .extended = 11, .shift_right = 7 }));
            if (dump && w == 2 && h == 8)
            {
                dump_amx(48, hintY0);
                dump_amx(52, hintY1);
            }

            // UV
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x080, .offset_z = 56, .count_x = 2, .extended = 12, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x080, .offset_z = 60, .count_x = 2, .extended = 12, .add = 1 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x000, .offset_z = 56 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x040, .offset_z = 57 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x080, .offset_z = 58 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x0C0, .offset_z = 59 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 60 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 61 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 62 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 63 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x200 - 2, .offset_y = 0, .offset_z = 56, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040 - 2, .offset_y = 0, .offset_z = 57, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080 - 2, .offset_y = 0, .offset_z = 58, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0 - 2, .offset_y = 0, .offset_z = 59, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100 - 2, .offset_y = 0, .offset_z = 60, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140 - 2, .offset_y = 0, .offset_z = 61, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180 - 2, .offset_y = 0, .offset_z = 62, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0 - 2, .offset_y = 0, .offset_z = 63, .add = 1 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x000, .offset_z = 56 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x040, .offset_z = 57 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x080, .offset_z = 58 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x0C0, .offset_z = 59 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 60 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 61 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 62 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 63 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x140, .offset_z = 40, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x140, .offset_z = 44, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x140, .offset_z = 41, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x140, .offset_z = 45, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x140, .offset_z = 42, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x140, .offset_z = 46, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x140, .offset_z = 43, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x140, .offset_z = 47, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x1C0, .offset_z = 48, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x1C0, .offset_z = 52, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x1C0, .offset_z = 49, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x1C0, .offset_z = 53, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x1C0, .offset_z = 50, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x1C0, .offset_z = 54, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x1C0, .offset_z = 51, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x1C0, .offset_z = 55, .shift_right = 5 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x100, .offset_z = 32, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x100, .offset_z = 36, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x100, .offset_z = 33, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x100, .offset_z = 37, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x100, .offset_z = 34, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x100, .offset_z = 38, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x100, .offset_z = 35, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x100, .offset_z = 39, .shift_right = 6 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x180, .offset_z = 40, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x180, .offset_z = 44, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x180, .offset_z = 41, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x180, .offset_z = 45, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x180, .offset_z = 42, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x180, .offset_z = 46, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x180, .offset_z = 43, .shift_right = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x180, .offset_z = 47, .shift_right = 7 }));
            if (dump && w == 2 && h == 8)
            {
                dump_amx(56, hintU);
                dump_amx(60, hintV);
            }

            // RGBA
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x000, .offset_z = 48 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x040, .offset_z = 49 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x080, .offset_z = 50 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x0C0, .offset_z = 51 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 52 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 53 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 54 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 55 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x000, .offset_z =  0, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x000, .offset_z =  4, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x000, .offset_z =  8, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x000, .offset_z = 12, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x000, .offset_z = 16, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x000, .offset_z = 20, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x000, .offset_z = 24, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x000, .offset_z = 28, .count_x = 1, .count_y = 1, .extended = 10, .add = 1 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x000, .offset_z = 40 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x040, .offset_z = 41 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x080, .offset_z = 42 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x0C0, .offset_z = 43 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 44 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 45 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 46 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 47 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x040, .offset_z =  0, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x040, .offset_z =  4, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x040, .offset_z =  8, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x040, .offset_z = 12, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x040, .offset_z = 16, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x040, .offset_z = 20, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x040, .offset_z = 24, .count_x = 1, .extended = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x040, .offset_z = 28, .count_x = 1, .extended = 12 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x000, .offset_z = 32 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x040, .offset_z = 33 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x080, .offset_z = 34 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x0C0, .offset_z = 35 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 36 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 37 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 38 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 39 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1E0, .offset_y = 0x040, .offset_z =  0, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x000, .offset_y = 0x040, .offset_z =  1, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x020, .offset_y = 0x040, .offset_z =  4, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x040, .offset_y = 0x040, .offset_z =  5, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x060, .offset_y = 0x040, .offset_z =  8, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x080, .offset_y = 0x040, .offset_z =  9, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0A0, .offset_y = 0x040, .offset_z = 12, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0C0, .offset_y = 0x040, .offset_z = 13, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x0E0, .offset_y = 0x040, .offset_z = 16, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100, .offset_y = 0x040, .offset_z = 17, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x120, .offset_y = 0x040, .offset_z = 20, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140, .offset_y = 0x040, .offset_z = 21, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x160, .offset_y = 0x040, .offset_z = 24, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180, .offset_y = 0x040, .offset_z = 25, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1A0, .offset_y = 0x040, .offset_z = 28, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0, .offset_y = 0x040, .offset_z = 29, .count_x = 1, .mask = 1, .extended = 6, .shift_right = 8 }));
            if (dump && w == 2 && h == 8)
            {
                dump_amx(0, 0);
                dump_amx(8, 0);
                dump_amx(16, 0);
                dump_amx(24, 0);
                dump_amx(32, hintR);
                dump_amx(40, hintG);
                dump_amx(48, hintB);
             }

            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 +   0, .register_index =  0 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 +  64, .register_index =  1 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 128, .register_index =  4 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 192, .register_index =  5 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 256, .register_index =  8 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 320, .register_index =  9 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 384, .register_index = 12 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 448, .register_index = 13 }));  rgb0 += 128 * 4;
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 +   0, .register_index = 16 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 +  64, .register_index = 17 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 128, .register_index = 20 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 192, .register_index = 21 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 256, .register_index = 24 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 320, .register_index = 25 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 384, .register_index = 28 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 448, .register_index = 29 }));  rgb1 += 128 * 4;
#else
            AMX_LDX((amx_access{ .address = (uint64_t)y0, .register_index = 2, .double_width = 1 }));  y0 += 128;
            AMX_LDX((amx_access{ .address = (uint64_t)y1, .register_index = 4, .double_width = 1 }));  y1 += 128;
            AMX_LDX((amx_access{ .address = (uint64_t)u0, .register_index = 0 }));  u0 += 64;
            AMX_LDX((amx_access{ .address = (uint64_t)v0, .register_index = 1 }));  v0 += 64;
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x00, .offset_y = 0x40, .offset_z = 0, .count_x = 2, .dummy_42 = 12 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x40, .offset_y = 0x40, .offset_z = 4, .count_x = 2, .dummy_42 = 12 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 0 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 1 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 2 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 3 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100 - 2, .offset_y = 0, .offset_z = 0, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140 - 2, .offset_y = 0, .offset_z = 1, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180 - 2, .offset_y = 0, .offset_z = 2, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0 - 2, .offset_y = 0, .offset_z = 3, .add = 1 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x100, .offset_z = 4 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x140, .offset_z = 5 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x180, .offset_z = 6 }));
            AMX_EXTRX((amx_operands_vector{ .offset_x = 0x1C0, .offset_z = 7 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x100 - 2, .offset_y = 0, .offset_z = 4, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x140 - 2, .offset_y = 0, .offset_z = 5, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x180 - 2, .offset_y = 0, .offset_z = 6, .add = 1 }));
            AMX_VECINT((amx_operands_vector{ .offset_x = 0x1C0 - 2, .offset_y = 0, .offset_z = 7, .add = 1 }));
            if (dump && w == 0 && h == 0)
            {
                dump_amx(0);
                dump_amx(1);
                dump_amx(2);
                dump_amx(3);
            }

            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 +   0, .register_index =  0 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 +  64, .register_index =  1 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 128, .register_index =  2 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb0 + 192, .register_index =  3 })); rgb0 += 64 * 4;
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 +   0, .register_index =  8 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 +  64, .register_index =  9 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 128, .register_index = 10 }));
            AMX_STZ((amx_access{ .address = (uint64_t)rgb1 + 192, .register_index = 11 })); rgb1 += 64 * 4;
#endif
        }
    }
    AMX_STOP();
}
#endif
//------------------------------------------------------------------------------
