#pragma once

#include "bitTorrentMessage.hpp"

//Class for creating messages
struct Messenger
{
    static std::string createHandshake(const std::string& infoHash, const std::string& peerId);
    static std::string createRequest(unsigned int index, unsigned int offset, unsigned int size);
    static std::string createInterested();
};
