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
    explicit BitTorrentMessage(const std::string& message);
    BitTorrentMessage(char id = -1, const std::string& message = "");
    BitTorrentMessage(BitTorrentMessage&& message) = default;
    BitTorrentMessage& operator=(const BitTorrentMessage&) = default;

    std::string getPayload();
    char getId();
    std::string toString();
};
