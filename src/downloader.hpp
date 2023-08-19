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
    TorrentDownloader(std::string&& fileName, std::string&& outputPath);
    TorrentDownloader(const std::string& fileName, const std::string& outputPath);

    int download();
    void downloadPiece(Peer& peer, int length, int sock);
};
