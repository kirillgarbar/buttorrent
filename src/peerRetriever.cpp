#include "peerRetriever.hpp"
#include "utils.hpp"
#include <iostream>
#include <cpr/cpr.h>
#include <stdexcept>

#define PORT 8080
#define TIMEOUT 15000

using namespace std;

//Retturns list of peers from the tracker and peer refresh interval
RetrievedPeers PeerRetriever::retrievePeers(const TorrentFile& tf, const std::string& peerId) {

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

    if (res.status_code != 200) throw runtime_error("Unable to retrieve peers");

    auto data = bencode::decode(res.text);
    auto dict = get<bencode::dict>(data);

    int interval = (int) get<bencode::integer>(dict.find("interval")->second);

    //In case peers data is sent as binary blob
    try {
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
    } catch (std::bad_variant_access &e) {
        //Peers are stored as list of dicts
        vector<bencode::data> peersVector = get<bencode::list>(dict.find("peers")->second);

        vector<Peer> peers;
        for(int i = 0; i < peersVector.size(); i++) {
            auto peerDict = get<bencode::dict>(peersVector[i]);
            
            string ip = get<bencode::string>(peerDict.find("ip")->second);
            int port = get<bencode::integer>(peerDict.find("port")->second);

            peers.push_back({ip, port});
        }

        return { interval, peers };
    }
}

