//==============================================================================
// xxImGui : Plugin Framework Header
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

class Connect;
class Client
{
    Connect* client;

    std::string address;
    std::string port;

public:
    Client(const char* address, const char* port);
    ~Client();

    bool Update(const UpdateData& updateData);
};
