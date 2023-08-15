#pragma once

#include "bitTorrentMessage.hpp"

//Class for creating messages
struct Messenger
{
    static std::string createHandshake(const std::string& infoHash, const std::string& peerId);
    static std::string createRequest();
    static std::string createInterested();
};
