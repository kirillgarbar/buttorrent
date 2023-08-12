#pragma once

#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <cpr/cpr.h>
#include "decoder.hpp"

#include <iostream>

#define PORT 8080
#define TIMEOUT 15000

using random_bytes_engine = std::independent_bits_engine<
    std::default_random_engine, CHAR_BIT, unsigned char>;

struct Peer {
    
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

    static inline std::string generatePeerId() {
        using namespace std;

        random_bytes_engine rbe;
        vector<unsigned char> data(20);
        generate(data.begin(), data.end(), rbe);
        string peerId;
        for(char c : data) peerId.push_back(c);

        return peerId;
    }

public:
    static inline std::vector<Peer> retrievePeers(TorrentFile& tf) {
        using namespace std;

        string peerId = generatePeerId();

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
        
        return {};
    }
};
