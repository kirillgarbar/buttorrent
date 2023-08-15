#pragma once

#include <string>

enum MessageId {
    keepAlive = -1,
    choke = 0,
    unchoke = 1,
    interested = 2,
    notInterested = 3,
    have = 4,
    bitField = 5,
    request = 6,
    piece = 7,
    cancel = 8,
    port = 9
};

struct BitTorrentMessage
{
private:
    char id;
    std::string payload;
public:
    explicit BitTorrentMessage(std::string& message);
    std::string getPayload();
    char getId();
    std::string toString();
};
