#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <bitset>

#include "downloader.hpp"
#include "utils.hpp"

using namespace std;

#define BlockSize 16384

TorrentDownloader::TorrentDownloader(const std::string& fileName, const std::string& outputPath) {
    torrentFile = Decoder::getTorrentFile(fileName);
    peerId = generatePeerId();
    piece = 0;
    fileManager = FileManager{torrentFile.Name, outputPath};
    fileManager.createFile();
}

TorrentDownloader::TorrentDownloader(std::string&& fileName, std::string&& outputPath) {
    torrentFile = Decoder::getTorrentFile(fileName);
    peerId = generatePeerId();
    piece = 0;
    fileManager = FileManager{torrentFile.Name, outputPath};
    fileManager.createFile();
}

std::string TorrentDownloader::generatePeerId() {
    using random_bytes_engine = std::independent_bits_engine<
        std::default_random_engine, CHAR_BIT, unsigned char>;

    random_bytes_engine rbe;
    vector<unsigned char> data(20);
    generate(data.begin(), data.end(), rbe);
    string peerId;
    for(char c : data) peerId.push_back(c);

    return peerId;
}

chrono::steady_clock::time_point getTimestamp() { return chrono::steady_clock::now(); }

bool TorrentDownloader::verifyPiece(const string& piece) {
    string pieceHash = hexDecode(Decoder::sha1(piece));

    //Piece downloaded successfully
    return pieceHash == torrentFile.PieceHashes[this->piece];
}

void TorrentDownloader::savePiece(const string& piece) {
    lock.lock();
    this->piece++;
    lock.unlock();
    fileManager.writeToFile(piece);
}

void TorrentDownloader::downloadPiece(Peer& peer, int length, int sock) {
    //Number of blocks of BlockSize
    //Most of the time PieceLength will be divisible by BlockSize
    int totalFullBlocks = length / BlockSize;
    int lastBlockLength = length % BlockSize == 0 ? BlockSize : length % BlockSize;
    int totalBlocks = lastBlockLength < BlockSize ? totalFullBlocks + 1 : totalFullBlocks;
    int block = 0;
    int offset = 0;
    std::stringstream pieceString;

    while (block < totalBlocks) {
        //In case previous piece downloaded from same peer
        //to prevent receive message timeout when no request was sent
        if (!choke && !requestSent) {
            requestSent = true;
            if (block == totalBlocks - 1) PeerConnector::requestPiece(piece, offset, lastBlockLength, sock);
            else PeerConnector::requestPiece(piece, offset, BlockSize, sock);
        }

        BitTorrentMessage message = PeerConnector::receiveMessage(sock);
        switch (message.getId()) {
            case MessageId::unchoke:
                choke = false;
                break;

            case MessageId::choke:
                choke = true;
                break;

            case MessageId::have:
            {
                int pieceIndex = bytesToInt(message.getPayload());
                setBit(bitField, pieceIndex);
                break;
            }
            
            case MessageId::piece:
            {
                requestSent = false;
                std::string payload = message.getPayload();
                int index = bytesToInt(payload.substr(0, 4));
                int begin = bytesToInt(payload.substr(4, 4));
                if (index == piece) {
                    if (begin == offset) { 
                        pieceString << payload.substr(8); 
                        offset += payload.size() - 8;
                        block++;
                    }
                }
                break;
            }

            default:
                break;
        }
        if (block < totalBlocks && !requestSent && !choke) {
            requestSent = true;
            if (block == totalBlocks - 1) PeerConnector::requestPiece(piece, offset, lastBlockLength, sock);
            else PeerConnector::requestPiece(piece, offset, BlockSize, sock);
        }
    }

    string resultPiece = pieceString.str();
    if (verifyPiece(resultPiece)) savePiece(resultPiece);
}

void closeSocket(int sock) {
    if (sock > -1) close(sock);
}

int TorrentDownloader::download() {
    static int totalPieces = torrentFile.PieceHashes.size();
    static int lastPieceSize = torrentFile.Length - (totalPieces - 1) * torrentFile.PieceLength;

    while (piece < totalPieces) {
        peers = PeerRetriever::retrievePeers(torrentFile, peerId);
        auto lastRetrieved = getTimestamp();
        int retrieveInterval = peers.interval * 1000;

        int peer = 0;
        while (chrono::duration_cast<chrono::milliseconds>(getTimestamp() - lastRetrieved).count() < retrieveInterval 
        && peer < peers.peers.size())
        {
            int sockfd = -1;
            try {
                sockfd = PeerConnector::handshake(this->peers.peers[peer], this->torrentFile.InfoHash, this->peerId);
                bitField = PeerConnector::receiveBitField(sockfd);

                //If peer has interesting piece
                while (getBit(bitField, piece)) {
                    PeerConnector::interested(sockfd);
                    if (piece == totalPieces - 1) downloadPiece(peers.peers[peer], lastPieceSize, sockfd);
                    else downloadPiece(peers.peers[peer], torrentFile.PieceLength, sockfd);
                }
            } catch (runtime_error& e) {
                //Failed to connect, try another peer
            }
            peer++;
            choke = true;
            requestSent = false;
            closeSocket(sockfd);
        }
    }
    return 0;
}