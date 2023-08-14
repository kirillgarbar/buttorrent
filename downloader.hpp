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

    std::string generatePeerId();    
public:
    TorrentDownloader(std::string fileName);

    int download();
};
