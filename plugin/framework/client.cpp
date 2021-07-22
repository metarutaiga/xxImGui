//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <interface.h>
#include <ConcurrencyNetworkFramework/Connection.h>
#include "client.h"

//------------------------------------------------------------------------------
Client::Client(const char* address, const char* port)
{
    thiz.client = nullptr;
    thiz.address = address;
    thiz.port = port;
    thiz.message[0] = '\0';
}
//------------------------------------------------------------------------------
Client::~Client()
{
    if (thiz.client)
        thiz.client->Disconnect();
}
//------------------------------------------------------------------------------
bool Client::Update(const UpdateData& updateData)
{
    bool result = true;

    char label[64];
    snprintf(label, 64, "Client %p", this);

    if (ImGui::Begin(label, &result, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (client == nullptr)
        {
            if (ImGui::Button("Connect"))
            {
                client = new Connection(thiz.address.c_str(), thiz.port.c_str());
                if (client && client->ConnectTCP() == false)
                {
                    client->Disconnect();
                    client = nullptr;
                }
            }
        }
        else
        {
            if (ImGui::Button("Disconnect") || client->Alive() == false)
            {
                client->Disconnect();
                client = nullptr;
            }
            ImGui::SameLine();
            if (ImGui::Button("ConnectUDP"))
            {
                client->ConnectUDP();
            }
        }

        ImGui::InputText("MESSAGE", message, sizeof(message));
        if (ImGui::Button("Send"))
        {
            if (client)
            {
                client->Send(Buffer::Get(4));
            }
        }

        ImGui::End();
    }

    return result;
}
//------------------------------------------------------------------------------
