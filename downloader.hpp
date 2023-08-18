#pragma once

#include "decoder.hpp"
#include "peerRetriever.hpp"
#include "peerConnector.hpp"

struct TorrentDownloader
{
private:
    TorrentFile torrentFile;
    RetrievedPeers peers;
    std::string peerId;

    int piece = 0;
    bool choke = true;
    bool interested = false;
    bool requestSent = false;
    std::string bitField;

    std::ofstream downloadedFile;

    std::string generatePeerId();    
public:
    TorrentDownloader(std::string fileName);

    int download();
    void downloadPiece(Peer& peer, int length, int sock);
};
