//==============================================================================
// xxImGui : Plugin Validator Source
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <interface.h>

#include <xxGraphic/xxNode.h>

#define PLUGIN_NAME     "Validator"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

//------------------------------------------------------------------------------
pluginAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{

}
//------------------------------------------------------------------------------
pluginAPI void Update(const UpdateData& updateData)
{
    static bool showNode = false;
    static bool showAbout = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("Validate Node", nullptr, &showNode);
            ImGui::Separator();
            ImGui::MenuItem("About Validator", nullptr, &showAbout);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showNode)
    {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Validate Node", &showNode))
        {
            static char text[4096];
            ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);

            if (ImGui::Button("Validate"))
            {
                int step = 0;

                // 1. Create Root
                xxNodePtr root = xxNode::Create();
                step += snprintf(text + step, sizeof(text) - step, "Root : %p\n", root.get());

                // 2. Create Child
                xxNodePtr child = xxNode::Create();
                step += snprintf(text + step, sizeof(text) - step, "Child : %p\n", child.get());

                // 3. Attach Child
                bool attachChild = root->AttachChild(child);
                step += snprintf(text + step, sizeof(text) - step, "Attach Child : %s\n", attachChild ? "TRUE" : "FALSE");

                // 4. Get Children Count
                step += snprintf(text + step, sizeof(text) - step, "Root Children Count : %u\n", root->GetChildCount());
                for (uint32_t i = 0; i < root->GetChildSize(); ++i)
                {
                    xxNodePtr node = root->GetChild(i);
                    if (node != nullptr)
                    {
                        step += snprintf(text + step, sizeof(text) - step, "Children (%u) : %p\n", i, node.get());
                    }
                }

                // 5. Create GrandChild
                xxNodePtr grandChild = xxNode::Create();
                step += snprintf(text + step, sizeof(text) - step, "Child : %p\n", grandChild.get());

                // 6. Attach GrandChild
                bool attachGrandChild = child->AttachChild(grandChild);
                step += snprintf(text + step, sizeof(text) - step, "Attach Child : %s\n", attachChild ? "TRUE" : "FALSE");

                // 7. Get Child's Children Count
                step += snprintf(text + step, sizeof(text) - step, "Child's Children Count : %u\n", child->GetChildCount());
                for (uint32_t i = 0; i < child->GetChildSize(); ++i)
                {
                    xxNodePtr node = child->GetChild(i);
                    if (node != nullptr)
                    {
                        step += snprintf(text + step, sizeof(text) - step, "Children (%u) : %p\n", i, node.get());
                    }
                }

                // 8. Set Local Matrix
                root->SetLocalMatrix({ xxVector4::Z, -xxVector4::Y, xxVector4::X, xxVector4::WHITE });
                child->SetLocalMatrix({ xxVector4::Y, -xxVector4::X, xxVector4::Z, xxVector4::WHITE });
                grandChild->SetLocalMatrix({ xxVector4::Y, -xxVector4::X, xxVector4::Z, xxVector4::WHITE });
                step += snprintf(text + step, sizeof(text) - step, "SetLocalMatrix\n");

                // 9. Update
                root->Update(updateData.time);
                step += snprintf(text + step, sizeof(text) - step, "Update Root : %f\n", updateData.time);

                // 10. Get Root World Matrix
                for (int i = 0; i < 4; ++i)
                {
                    xxMatrix4 worldMatrix = root->GetWorldMatrix();
                    step += snprintf(text + step, sizeof(text) - step, "Root Matrix (%u) : %f %f %f %f\n", i, worldMatrix.v[i].x, worldMatrix.v[i].y, worldMatrix.v[i].z, worldMatrix.v[i].w);
                }

                // 11. Get Child World Matrix
                for (int i = 0; i < 4; ++i)
                {
                    xxMatrix4 worldMatrix = child->GetWorldMatrix();
                    step += snprintf(text + step, sizeof(text) - step, "Child Matrix (%u) : %f %f %f %f\n", i, worldMatrix.v[i].x, worldMatrix.v[i].y, worldMatrix.v[i].z, worldMatrix.v[i].w);
                }

                // 12. Get GrandChild World Matrix
                for (int i = 0; i < 4; ++i)
                {
                    xxMatrix4 worldMatrix = grandChild->GetWorldMatrix();
                    step += snprintf(text + step, sizeof(text) - step, "GrandChild Matrix (%u) : %f %f %f %f\n", i, worldMatrix.v[i].x, worldMatrix.v[i].y, worldMatrix.v[i].z, worldMatrix.v[i].w);
                }
            }

            ImGui::End();
        }
    }

    if (showAbout)
    {
        if (ImGui::Begin("About Validator", &showAbout))
        {
            ImGui::Text("%s Plugin Version %d.%d", PLUGIN_NAME, PLUGIN_MAJOR, PLUGIN_MINOR);
            ImGui::Separator();
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
            ImGui::End();
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
