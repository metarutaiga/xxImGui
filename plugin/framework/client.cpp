//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <interface.h>
#include "../../ConcurrencyNetworkFramework/Connect.h"
#include "client.h"

//------------------------------------------------------------------------------
Client::Client(const char* address, const char* port)
{
    thiz.client = nullptr;
    thiz.address = address;
    thiz.port = port;
}
//------------------------------------------------------------------------------
Client::~Client()
{
    delete thiz.client;
}
//------------------------------------------------------------------------------
bool Client::Update(const UpdateData& updateData)
{
    bool result = true;

    char label[64];
    snprintf(label, 64, "Client %p", this);

    if (ImGui::Begin(label, &result))
    {
        if (ImGui::Button("Connect"))
        {
            if (client == nullptr)
            {
                client = new Connect(thiz.address.c_str(), thiz.port.c_str());
                if (client && client->Start() == false)
                {
                    delete client;
                    client = nullptr;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Disconnect"))
        {
            delete client;
            client = nullptr;
        }

        ImGui::End();
    }

    return result;
}
//------------------------------------------------------------------------------
