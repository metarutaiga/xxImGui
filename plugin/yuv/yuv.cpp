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

#define PLUGIN_NAME     "YUV"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static int lennaWidth = 512;
static int lennaHeight = 512;
static char* lennaRGB = nullptr;
static char* lennaYU12 = nullptr;
static char* lennaYV12 = nullptr;
static char* lennaNV12 = nullptr;
static char* lennaNV21 = nullptr;

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
pluginAPI const char* Create(const CreateData& createData)
{
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s/%s/%s", createData.baseFolder, "resource", "lenna.rgb");

    FILE* file = fopen(path, "rb");
    if (file)
    {
        lennaRGB = (char*)malloc(3 * lennaWidth * lennaHeight);
        if (lennaRGB)
        {
            fread(lennaRGB, 3, lennaWidth * lennaHeight, file);
        }
        fclose(file);

        int sizeY = lennaWidth * lennaHeight;
        int sizeUV = lennaWidth / 2 * lennaHeight / 2;

        // YU12
        lennaYU12 = (char*)malloc(sizeY + 2 * sizeUV);
        if (lennaYU12 && lennaRGB)
        {
            rgb2yuv_yu12(lennaWidth, lennaHeight, lennaRGB, lennaYU12);
        }

        // YV12
        lennaYV12 = (char*)malloc(sizeY + 2 * sizeUV);
        if (lennaYV12 && lennaRGB)
        {
            rgb2yuv_yv12(lennaWidth, lennaHeight, lennaRGB, lennaYV12);
        }

        // NV12
        lennaNV12 = (char*)malloc(sizeY + 2 * sizeUV);
        if (lennaNV12 && lennaRGB)
        {
            rgb2yuv_nv12(lennaWidth, lennaHeight, lennaRGB, lennaNV12);
        }

        // NV21
        lennaNV21 = (char*)malloc(sizeY + 2 * sizeUV);
        if (lennaNV21 && lennaRGB)
        {
            rgb2yuv_nv21(lennaWidth, lennaHeight, lennaRGB, lennaNV21);
        }
    }

    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{
    free(lennaRGB);
    free(lennaYU12);
    free(lennaYV12);
    free(lennaNV12);
    free(lennaNV21);
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
            if (source == 0)
            {
                loadTextureFromImage<4, 3>(source, updateData.device, lennaRGB, lennaWidth, lennaHeight);
            }
            ImGui::Image((ImTextureID)source, ImVec2(lennaWidth, lennaHeight));
            if (target != 0)
            {
                ImGui::SameLine();
                ImGui::Image((ImTextureID)target, ImVec2(lennaWidth, lennaHeight));
            }

            static int format = 0;

            bool click = false;
            click |= ImGui::RadioButton("YU12", &format, 0);
            ImGui::SameLine();
            click |= ImGui::RadioButton("YV12", &format, 1);
            ImGui::SameLine();
            click |= ImGui::RadioButton("NV12", &format, 2);
            ImGui::SameLine();
            click |= ImGui::RadioButton("NV21", &format, 3);
            ImGui::SameLine();

            if (ImGui::Button("Convert") || click)
            {
                unsigned char* temp = (unsigned char*)malloc(4 * lennaWidth * lennaHeight);
                if (temp)
                {
                    int sizeY = lennaWidth * lennaHeight;
                    int sizeUV = lennaWidth / 2 * lennaHeight / 2;

                    switch (format)
                    {
                    default:
                    case 0:
                        yuv2rgb_yu12(lennaWidth, lennaHeight, lennaYU12, temp, 4);
                        break;
                    case 1:
                        yuv2rgb_yv12(lennaWidth, lennaHeight, lennaYV12, temp, 4);
                        break;
                    case 2:
                        yuv2rgb_nv12(lennaWidth, lennaHeight, lennaNV12, temp, 4);
                        break;
                    case 3:
                        yuv2rgb_nv21(lennaWidth, lennaHeight, lennaNV21, temp, 4);
                        break;
                    }

                    loadTextureFromImage<4, 4>(target, updateData.device, temp, lennaWidth, lennaHeight);

                    free(temp);
                }
            }
            ImGui::End();
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
