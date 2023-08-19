#pragma once

#include "decoder.hpp"
#include "peerRetriever.hpp"
#include "peerConnector.hpp"
#include "fileManager.hpp"

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

    FileManager fileManager;

    std::string generatePeerId();    
public:
    explicit TorrentDownloader(std::string&& fileName);
    explicit TorrentDownloader(const std::string& fileName);

    int download();
    void downloadPiece(Peer& peer, int length, int sock);
};
