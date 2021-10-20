//==============================================================================
// xxImGui : Plugin YUV Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#include <xxYUV/rgb2yuv.h>
#include <xxYUV/yuv2rgb.h>
#include <xxYUV/yuv2rgb_amx.h>
#include <interface.h>

#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#endif

#define LIBYUV 1
#if defined(LIBYUV)
#include <libyuv.h>
#include <libyuv/convert_argb.h>
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
    snprintf(path, 1024, "%s/../%s/%s", baseFolder, "Resources", "lenna.rgb");
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
    static int loopCount = 1000;

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

            ImGui::SameLine();
            ImGui::SliderInt("Loop", &loopCount, 1, 10000);

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
                int count = loopCount;

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
#if LIBYUV
                    if (libyuv)
                    {
                        switch (format)
                        {
                        case 0:
                            for (int i = 0; i < count; ++i)
                            {
                                auto converter = encodeFullRange ? libyuv::ARGBToJ420 : libyuv::ARGBToI420;
                                converter(temp, lennaWidth * 4, lennaYU12, lennaWidth, lennaYU12 + sizeY, lennaWidth / 2, lennaYU12 + sizeY + sizeUV, lennaWidth / 2, lennaWidth, lennaWidth);
                            }
                            break;
                        case 1:
                            for (int i = 0; i < count; ++i)
                            {
                                auto converter = encodeFullRange ? libyuv::ARGBToJ420 : libyuv::ARGBToI420;
                                converter(temp, lennaWidth * 4, lennaYV12, lennaWidth, lennaYV12 + sizeY + sizeUV, lennaWidth / 2, lennaYV12 + sizeY, lennaWidth / 2, lennaWidth, lennaWidth);
                            }
                            break;
                        case 2:
                            for (int i = 0; i < count; ++i)
                            {
                                libyuv::ARGBToNV12(temp, lennaWidth * 4, lennaNV12, lennaWidth, lennaNV12 + sizeY, lennaWidth, lennaWidth, lennaWidth);
                            }
                            break;
                        case 3:
                            for (int i = 0; i < count; ++i)
                            {
                                libyuv::ARGBToNV21(temp, lennaWidth * 4, lennaNV21, lennaWidth, lennaNV21 + sizeY, lennaWidth, lennaWidth, lennaWidth);
                            }
                            break;
                        }
                    }
                    else
#endif
#if defined(__APPLE__)
                    if (accelerate)
                    {
                        switch (format)
                        {
                        case 0:
                        {
                            static vImage_ARGBToYpCbCr fullRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_Cb8_Cr8, kvImageNoFlags);
                                return info;
                            }();
                            static vImage_ARGBToYpCbCr videoRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_Cb8_Cr8, kvImageNoFlags);
                                return info;
                            }();

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaYU12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer Cb = { lennaYU12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer Cr = { lennaYU12 + sizeY + sizeUV, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_ARGB8888To420Yp8_Cb8_Cr8(&RGBA, &Y, &Cb, &Cr, decodeFullRange ? &fullRange : &videoRange, permuteMap, kvImageDoNotTile);
                            }
                            break;
                        }
                        case 1:
                        {
                            static vImage_ARGBToYpCbCr fullRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_Cb8_Cr8, kvImageNoFlags);
                                return info;
                            }();
                            static vImage_ARGBToYpCbCr videoRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_Cb8_Cr8, kvImageNoFlags);
                                return info;
                            }();

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaYV12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer Cb = { lennaYV12 + sizeY + sizeUV, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer Cr = { lennaYV12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_ARGB8888To420Yp8_Cb8_Cr8(&RGBA, &Y, &Cb, &Cr, decodeFullRange ? &fullRange : &videoRange, permuteMap, kvImageDoNotTile);
                            }
                            break;
                        }
                        case 2:
                        {
                            static vImage_ARGBToYpCbCr fullRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_CbCr8, kvImageNoFlags);
                                return info;
                            }();
                            static vImage_ARGBToYpCbCr videoRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_CbCr8, kvImageNoFlags);
                                return info;
                            }();

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaNV12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer CbCr = { lennaNV12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_ARGB8888To420Yp8_CbCr8(&RGBA, &Y, &CbCr, decodeFullRange ? &fullRange : &videoRange, permuteMap, kvImageDoNotTile);
                            }
                            break;
                        }
                        case 3:
                        {
                            static vImage_ARGBToYpCbCr fullRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 0, 128, 255, 255, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_CbCr8, kvImageNoFlags);
                                return info;
                            }();
                            static vImage_ARGBToYpCbCr videoRange = []()
                            {
                                vImage_YpCbCrPixelRange range = { 16, 128, 235, 240, 255, 0, 255, 0 };
                                vImage_ARGBToYpCbCr info;
                                vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2, &range, &info, kvImageARGB8888, kvImage420Yp8_CbCr8, kvImageNoFlags);
                                return info;
                            }();

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaNV21, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer CbCr = { lennaNV21 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_ARGB8888To420Yp8_CbCr8(&RGBA, &Y, &CbCr, decodeFullRange ? &fullRange : &videoRange, permuteMap, kvImageDoNotTile);
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    else
#endif
                    {
                        switch (format)
                        {
                        case 0:
                            if (temp && lennaYU12)
                            {
                                for (int i = 0; i < count; ++i)
                                    rgb2yuv_yu12(lennaWidth, lennaHeight, temp, lennaYU12, 4, true, encodeFullRange);
                            }
                            break;
                        case 1:
                            if (temp && lennaYV12)
                            {
                                for (int i = 0; i < count; ++i)
                                    rgb2yuv_yv12(lennaWidth, lennaHeight, temp, lennaYV12, 4, true, encodeFullRange);
                            }
                            break;
                        case 2:
                            if (temp && lennaNV12)
                            {
                                for (int i = 0; i < count; ++i)
                                    rgb2yuv_nv12(lennaWidth, lennaHeight, temp, lennaNV12, 4, true, encodeFullRange);
                            }
                            break;
                        case 3:
                            if (temp && lennaNV21)
                            {
                                for (int i = 0; i < count; ++i)
                                    rgb2yuv_nv21(lennaWidth, lennaHeight, temp, lennaNV21, 4, true, encodeFullRange);
                            }
                            break;
                        default:
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
#if defined(__APPLE__) && defined(__aarch64__)
                    if (appleamx)
                    {
                        for (int i = 0; i < count; ++i)
                        {
                            yuv2rgb_yu12_amx(lennaWidth, lennaHeight, lennaYU12, temp, decodeFullRange, 4, true);
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
                        }
                    }
                    else
#endif
#if LIBYUV
                    if (libyuv)
                    {
                        switch (format)
                        {
                        case 0:
                            for (int i = 0; i < count; ++i)
                            {
                                libyuv::I420ToARGBMatrix(lennaYU12, lennaWidth,
                                                         lennaYU12 + sizeY, lennaWidth / 2,
                                                         lennaYU12 + sizeY + sizeUV, lennaWidth / 2,
                                                         temp, lennaWidth * 4,
                                                         decodeFullRange ? &libyuv::kYuvF709Constants : &libyuv::kYuvH709Constants,
                                                         lennaWidth, lennaHeight);
                            }
                            break;
                        case 1:
                            for (int i = 0; i < count; ++i)
                            {
                                libyuv::I420ToARGBMatrix(lennaYV12, lennaWidth,
                                                         lennaYV12 + sizeY + sizeUV, lennaWidth / 2,
                                                         lennaYV12 + sizeY, lennaWidth / 2,
                                                         temp, lennaWidth * 4,
                                                         decodeFullRange ? &libyuv::kYuvF709Constants : &libyuv::kYuvH709Constants,
                                                         lennaWidth, lennaHeight);
                            }
                            break;
                        case 2:
                            for (int i = 0; i < count; ++i)
                            {
                                libyuv::NV12ToARGBMatrix(lennaNV12, lennaWidth,
                                                         lennaNV12 + sizeY, lennaWidth,
                                                         temp, lennaWidth * 4,
                                                         decodeFullRange ? &libyuv::kYuvF709Constants : &libyuv::kYuvH709Constants,
                                                         lennaWidth, lennaHeight);
                            }
                            break;
                        case 3:
                            for (int i = 0; i < count; ++i)
                            {
                                libyuv::NV21ToARGBMatrix(lennaNV21, lennaWidth,
                                                         lennaNV21 + sizeY, lennaWidth,
                                                         temp, lennaWidth * 4,
                                                         decodeFullRange ? &libyuv::kYuvF709Constants : &libyuv::kYuvH709Constants,
                                                         lennaWidth, lennaHeight);
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    else
#endif
#if defined(__APPLE__)
                    if (accelerate)
                    {
                        switch (format)
                        {
                        case 0:
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

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaYU12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer Cb = { lennaYU12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer Cr = { lennaYU12 + sizeY + sizeUV, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_Cb8_Cr8ToARGB8888(&Y, &Cb, &Cr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                            }
                            break;
                        }
                        case 1:
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

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaYV12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer Cb = { lennaYV12 + sizeY + sizeUV, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer Cr = { lennaYV12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth / 2 };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_Cb8_Cr8ToARGB8888(&Y, &Cb, &Cr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                            }
                            break;
                        }
                        case 2:
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

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaNV12, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer CbCr = { lennaNV12 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_CbCr8ToARGB8888(&Y, &CbCr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                            }
                            break;
                        }
                        case 3:
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

                            for (int i = 0; i < count; ++i)
                            {
                                vImage_Buffer Y = { lennaNV21, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth };
                                vImage_Buffer CbCr = { lennaNV21 + sizeY, (vImagePixelCount)lennaHeight / 2, (vImagePixelCount)lennaWidth / 2, (size_t)lennaWidth };
                                vImage_Buffer RGBA = { temp, (vImagePixelCount)lennaHeight, (vImagePixelCount)lennaWidth, (size_t)lennaWidth * 4 };

                                uint8_t permuteMap[4] = { 3, 2, 1, 0 };
                                vImageConvert_420Yp8_CbCr8ToARGB8888(&Y, &CbCr, &RGBA, decodeFullRange ? &fullRange : &videoRange, permuteMap, 255, kvImageDoNotTile);
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    else
#endif
                    {
                        switch (format)
                        {
                        default:
                        case 0:
                            for (int i = 0; i < count; ++i)
                                yuv2rgb_yu12(lennaWidth, lennaHeight, lennaYU12, temp, decodeFullRange, 4, true);
                            break;
                        case 1:
                            for (int i = 0; i < count; ++i)
                                yuv2rgb_yv12(lennaWidth, lennaHeight, lennaYV12, temp, decodeFullRange, 4, true);
                            break;
                        case 2:
                            for (int i = 0; i < count; ++i)
                                yuv2rgb_nv12(lennaWidth, lennaHeight, lennaNV12, temp, decodeFullRange, 4, true);
                            break;
                        case 3:
                            for (int i = 0; i < count; ++i)
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

            ImGui::Text("Encode TSC : %.3fus %llu", encodeTime * 1000000.0f, encodeTSC);
            ImGui::Text("Decode TSC : %.3fus %llu", decodeTime * 1000000.0f, decodeTSC);

            ImGui::End();
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
