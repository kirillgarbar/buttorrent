#include "messenger.hpp"
#include "utils.hpp"
#include <sstream>

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

