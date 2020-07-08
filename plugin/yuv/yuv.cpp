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

#define LIBYUV 1
#if defined(LIBYUV)
#include <libyuv.h>
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
        unsigned int stride = 0;
        char* pointer = (char*)xxMapTexture(device, texture, &stride, 0, 0);
        if (pointer)
        {
            char* input = (char*)data;
            for (unsigned int y = 0; y < height; ++y)
            {
                char* output = pointer;
                for (unsigned int x = 0; x < width; ++x)
                {
                    if (targetPixel >= 1) (*output++) = (sourcePixel >= 1) ? input[x * sourcePixel + 0] : -1;
                    if (targetPixel >= 2) (*output++) = (sourcePixel >= 2) ? input[x * sourcePixel + 1] : -1;
                    if (targetPixel >= 3) (*output++) = (sourcePixel >= 3) ? input[x * sourcePixel + 2] : -1;
                    if (targetPixel >= 4) (*output++) = (sourcePixel >= 4) ? input[x * sourcePixel + 3] : -1;
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
        lennaYU12 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 64);
        if (lennaYU12 && lennaRGB)
        {
            rgb2yuv_yu12(lennaWidth, lennaHeight, lennaRGB, lennaYU12);
        }

        // YV12
        lennaYV12 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 64);
        if (lennaYV12 && lennaRGB)
        {
            rgb2yuv_yv12(lennaWidth, lennaHeight, lennaRGB, lennaYV12);
        }

        // NV12
        lennaNV12 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 64);
        if (lennaNV12 && lennaRGB)
        {
            rgb2yuv_nv12(lennaWidth, lennaHeight, lennaRGB, lennaNV12);
        }

        // NV21
        lennaNV21 = (unsigned char*)xxAlignedAlloc(sizeY + 2 * sizeUV, 64);
        if (lennaNV21 && lennaRGB)
        {
            rgb2yuv_nv21(lennaWidth, lennaHeight, lennaRGB, lennaNV21);
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
            ImGui::MenuItem("Dialog", nullptr, &showDialog);
            ImGui::Separator();
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
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

#if LIBYUV
            static bool libyuv = false;
            click |= ImGui::Checkbox("libYUV", &libyuv);
            ImGui::SameLine();
#endif

            static uint64_t encodeTSC = 0;
            static uint64_t decodeTSC = 0;
            if (ImGui::Button("Convert") || click)
            {
                unsigned char* temp = (unsigned char*)xxAlignedAlloc(4 * lennaWidth * lennaHeight, 64);
                int sizeY = lennaWidth * lennaHeight;
                int sizeUV = lennaWidth / 2 * lennaHeight / 2;

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
                    for (int i = 0; i < 100; ++i)
                    {
                        switch (format)
                        {
                        default:
                        case 0:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::ARGBToI420(temp, lennaWidth * 4, lennaYU12, lennaWidth, lennaYU12 + sizeY, lennaWidth / 2, lennaYU12 + sizeY + sizeUV, lennaWidth / 2, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            if (lennaYU12 && temp)
                            {
                                rgb2yuv_yu12(lennaWidth, lennaHeight, temp, lennaYU12, 4, false, encodeFullRange);
                            }
                            break;
                        case 1:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::ARGBToI420(temp, lennaWidth * 4, lennaYV12, lennaWidth, lennaYV12 + sizeY + sizeUV, lennaWidth / 2, lennaYV12 + sizeY, lennaWidth / 2, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            if (lennaYV12 && lennaRGB)
                            {
                                rgb2yuv_yv12(lennaWidth, lennaHeight, temp, lennaYV12, 4, false, encodeFullRange);
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
                            if (lennaNV12 && lennaRGB)
                            {
                                rgb2yuv_nv12(lennaWidth, lennaHeight, temp, lennaNV12, 4, false, encodeFullRange);
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
                            if (lennaNV21 && lennaRGB)
                            {
                                rgb2yuv_nv21(lennaWidth, lennaHeight, temp, lennaNV21, 4, false, encodeFullRange);
                            }
                            break;
                        }
                    }
                    encodeTSC = (xxTSC() - startTSC) / 100;
                }

                if (temp)
                {
                    memset(temp, 0, 4 * lennaWidth * lennaHeight);

                    uint64_t startTSC = xxTSC();
                    for (int i = 0; i < 100; ++i)
                    {
                        switch (format)
                        {
                        default:
                        case 0:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::H420ToARGB(lennaYU12, lennaWidth, lennaYU12 + sizeY, lennaWidth / 2, lennaYU12 + sizeY + sizeUV, lennaWidth / 2, temp, lennaWidth * 4, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            yuv2rgb_yu12(lennaWidth, lennaHeight, lennaYU12, temp, decodeFullRange, 4, true);
                            break;
                        case 1:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::H420ToARGB(lennaYV12, lennaWidth, lennaYV12 + sizeY + sizeUV, lennaWidth / 2, lennaYV12 + sizeY, lennaWidth / 2, temp, lennaWidth * 4, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            yuv2rgb_yv12(lennaWidth, lennaHeight, lennaYV12, temp, decodeFullRange, 4, true);
                            break;
                        case 2:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::NV12ToARGB(lennaNV12, lennaWidth, lennaNV12 + sizeY, lennaWidth, temp, lennaWidth * 4, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            yuv2rgb_nv12(lennaWidth, lennaHeight, lennaNV12, temp, decodeFullRange, 4, true);
                            break;
                        case 3:
#if LIBYUV
                            if (libyuv)
                            {
                                libyuv::NV21ToARGB(lennaNV21, lennaWidth, lennaNV21 + sizeY, lennaWidth, temp, lennaWidth * 4, lennaWidth, lennaWidth);
                                break;
                            }
#endif
                            yuv2rgb_nv21(lennaWidth, lennaHeight, lennaNV21, temp, decodeFullRange, 4, true);
                            break;
                        }
                    }
                    decodeTSC = (xxTSC() - startTSC) / 100;

                    loadTextureFromImage<4, 4>(target, updateData.device, temp, lennaWidth, lennaHeight);
                }

                xxAlignedFree(temp);
            }

            if (source == 0)
            {
                loadTextureFromImage<4, 3>(source, updateData.device, lennaRGB, lennaWidth, lennaHeight);
            }
            if (source != 0)
            {
                ImGui::Image((ImTextureID)source, ImVec2(lennaWidth, lennaHeight));
            }
            if (target != 0)
            {
                ImGui::SameLine();
                ImGui::Image((ImTextureID)target, ImVec2(lennaWidth, lennaHeight));
            }

            ImGui::Text("Encode TSC : %llu", encodeTSC);
            ImGui::Text("Decode TSC : %llu", decodeTSC);

            ImGui::End();
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
