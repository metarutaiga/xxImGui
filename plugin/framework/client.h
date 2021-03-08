//==============================================================================
// xxImGui : Plugin Framework Header
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

class Connection;
class Client
{
    Connection* client;

    std::string address;
    std::string port;

    char message[256];

public:
    Client(const char* address, const char* port);
    ~Client();

    bool Update(const UpdateData& updateData);
};