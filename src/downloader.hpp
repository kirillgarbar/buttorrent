#pragma once

#include <ctime>
#include <mutex>
#include "decoder.hpp"
#include "peerRetriever.hpp"
#include "peerConnector.hpp"
#include "fileManager.hpp"

struct ProgressStatistics {
    time_t startTime;
    time_t currentTime;

    int downloadedPieces;
    int totalPieces;
    int pieceSize;
};

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

    mutable std::mutex lock;

    //Stats
    time_t startTime;

    std::string generatePeerId();    
public:
    TorrentDownloader(std::string&& fileName, std::string&& outputPath);
    TorrentDownloader(const std::string& fileName, const std::string& outputPath);

    int download();
    void downloadPiece(Peer& peer, int length, int sock);

    //Check piece hash
    bool verifyPiece(const std::string& piece);

    //Write piece to file
    void savePiece(const std::string& piece);

    ProgressStatistics getStatistics() const;
};
