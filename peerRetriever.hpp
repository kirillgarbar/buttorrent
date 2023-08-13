#pragma once

#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <cpr/cpr.h>
#include <bitset>
#include "decoder.hpp"

#include <iostream>

#define PORT 8080
#define TIMEOUT 15000

struct Peer {
    std::string Ip;
    int Port;
};

struct RetrievedPeers {
    int interval;
    std::vector<Peer> peers;
};

struct PeerRetriever
{
private:
    static inline std::string hexDecode(const std::string& value) {
        int hashLength = value.length();
        std::string decodedHexString;
        for (int i = 0; i < hashLength; i += 2)
        {
            std::string byte = value.substr(i, 2);
            char c = (char) (int) strtol(byte.c_str(), nullptr, 16);
            decodedHexString.push_back(c);
        }
        return decodedHexString;
    }

    static inline int bytesToInt(std::string bytes) {
        std::string binStr;
        long byteCount = bytes.size();
        for (int i = 0; i < byteCount; i++)
            binStr += std::bitset<8>(bytes[i]).to_string();
        return stoi(binStr, 0, 2);
    }

public:
    static inline RetrievedPeers retrievePeers(TorrentFile& tf, std::string peerId) {
        using namespace std;

        cpr::Response res = cpr::Get(cpr::Url{tf.Announce}, cpr::Parameters {
            { "info_hash", hexDecode(tf.InfoHash) },
            { "peer_id", peerId },
            { "port", to_string(PORT) },
            { "uploaded", "0" },
            { "downloaded", "0" },
            { "left", to_string(tf.Length) },
            { "compact", "1" }
        }, cpr::Timeout{ TIMEOUT }
        );

        auto data = bencode::decode(res.text);
        auto dict = get<bencode::dict>(data);

        int interval = (int) get<bencode::integer>(dict.find("interval")->second);
        string peersString = get<bencode::string>(dict.find("peers")->second);

        vector<string> peerString;
        for(int i = 0; i < peersString.size(); i += 6) peerString.push_back(peersString.substr(i, 6));

        vector<Peer> peers;
        for(auto peer : peerString) {
            stringstream ip;
            ip << to_string((unsigned char)peer[0]) << ".";
            ip << to_string((unsigned char)peer[1]) << ".";
            ip << to_string((unsigned char)peer[2]) << ".";
            ip << to_string((unsigned char)peer[3]);
            
            int port = bytesToInt(peer.substr(4, 2));
            peers.push_back({ip.str(), port});
        }

        return { interval, peers };
    }
};

