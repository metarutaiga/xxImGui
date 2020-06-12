//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <interface.h>
#include "../../ConcurrencyNetworkFramework/Connection.h"
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

    if (ImGui::Begin(label, &result))
    {
        if (ImGui::Button("Connect"))
        {
            if (client == nullptr)
            {
                client = new Connection(thiz.address.c_str(), thiz.port.c_str());
                if (client && client->Connect() == false)
                {
                    client->Disconnect();
                    client = nullptr;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Disconnect"))
        {
            client->Disconnect();
            client = nullptr;
        }

        ImGui::InputText("MESSAGE", message, sizeof(message));
        if (ImGui::Button("Send"))
        {
            if (client)
            {
                BufferPtr bufferPtr = BufferPtr::make_shared(message, message + strlen(message));
                client->Send(bufferPtr);
            }
        }

        ImGui::End();
    }

    return result;
}
//------------------------------------------------------------------------------
