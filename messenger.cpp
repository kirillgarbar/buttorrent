#include "messenger.hpp"
#include "utils.hpp"
#include <sstream>
#include <netinet/in.h>
#include <cstring>

using namespace std;

string Messenger::createHandshake(const std::string& infoHash, const std::string& peerId) {
    static string pstr = "BitTorrent protocol";
    
    stringstream message;
    message << (char)19 << pstr;
    for (int i = 0; i < 8; i++) message << (char)0;
    message << hexDecode(infoHash) << peerId;

    return message.str();
}

string Messenger::createInterested() { return BitTorrentMessage(MessageId::interested).toString(); }

string Messenger::createRequest(unsigned int i, unsigned int o, unsigned int s) {
    char temp[12];
    unsigned int index = htonl(i);
    unsigned int offset = htonl(o);
    unsigned int size = htonl(s);
    memcpy(temp, &index, 4);
    memcpy(temp + 4, &offset, 4);
    memcpy(temp + 8, &size, 4);
    string message;
    for (int i = 0; i < 12; i++) message.push_back(temp[i]);
    return BitTorrentMessage(MessageId::request, message).toString();
}