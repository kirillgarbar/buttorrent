#include "bitTorrentMessage.hpp"
#include <sstream>

#include <iostream>

using namespace std;

BitTorrentMessage::BitTorrentMessage(string& message) {
    char id = message[0];
    string payload = message.substr(1, message.size()-1);

    this->id = id;
    this->payload = payload;
}

char BitTorrentMessage::getId() { return id; }

string BitTorrentMessage::getPayload() { return payload; }

string BitTorrentMessage::toString() {
    std::stringstream buffer;
    uint messageLength = payload.size() + 1;
    char* messageLengthAddr = (char*) &messageLength;
    std::string messageLengthStr;
    // Bytes are pushed in reverse order, assuming the data
    // is stored in little-endian order locally.
    for (int i = 0; i < 4; i++)
        messageLengthStr.push_back((char) messageLengthAddr[3 - i]);
    buffer << messageLengthStr;
    buffer << (char) id;
    buffer << payload;
    return buffer.str();
}