#pragma once

#include "decoder.hpp"

struct Peer {
    std::string Ip;
    int Port;
};

struct RetrievedPeers {
    int interval;
    std::vector<Peer> peers;
};

//Sends announce to the tracker and receives list of peers
struct PeerRetriever {
    //Retturns list of peers from the tracker and peer refresh interval
    static RetrievedPeers retrievePeers(const TorrentFile& tf, const std::string& peerId);
};

