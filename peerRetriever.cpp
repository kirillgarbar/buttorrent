#include "peerRetriever.hpp"
#include "utils.hpp"
#include <iostream>
#include <cpr/cpr.h>

#define PORT 8080
#define TIMEOUT 15000

using namespace std;

//Retturns list of peers from the tracker and peer refresh interval
RetrievedPeers PeerRetriever::retrievePeers(TorrentFile& tf, std::string peerId) {

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

