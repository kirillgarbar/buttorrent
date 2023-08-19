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

TorrentDownloader::TorrentDownloader(const std::string& fileName) {
    torrentFile = Decoder::getTorrentFile(fileName);
    peerId = generatePeerId();
    piece = 0;
    fileManager = FileManager{torrentFile.Name};
    fileManager.createFile();
}

TorrentDownloader::TorrentDownloader(std::string&& fileName) {
    torrentFile = Decoder::getTorrentFile(fileName);
    peerId = generatePeerId();
    piece = 0;
    fileManager = FileManager{torrentFile.Name};
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
                cout << "Piece received" << endl;
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
        cout << "Message id " << (int)message.getId() << endl;
        if (block < totalBlocks && !requestSent && !choke) {
            requestSent = true;
            if (block == totalBlocks - 1) PeerConnector::requestPiece(piece, offset, lastBlockLength, sock);
            else PeerConnector::requestPiece(piece, offset, BlockSize, sock);
        }
        cout << "Current block " << block << endl;
    }
    string downloadedPiece = pieceString.str();
    string pieceHash = hexDecode(Decoder::sha1(downloadedPiece));

    //Piece downloaded successfully
    if (pieceHash == torrentFile.PieceHashes[piece])  { 
        this->piece++;
        fileManager.writeToFile(downloadedPiece);
    }
}

void closeSocket(int sock) {
    if (sock > -1) close(sock);
}

int TorrentDownloader::download() {
    static int totalPieces = torrentFile.PieceHashes.size();
    static int lastPieceSize = torrentFile.Length - (totalPieces - 1) * torrentFile.PieceLength;

    cout << torrentFile.PieceLength << " " << lastPieceSize << endl;

    while (piece < totalPieces) {
        peers = PeerRetriever::retrievePeers(torrentFile, peerId);
        auto lastRetrieved = getTimestamp();
        int retrieveInterval = peers.interval * 1000;

        cout << "Interval: " << retrieveInterval << endl;

        int peer = 0;
        while (chrono::duration_cast<chrono::milliseconds>(getTimestamp() - lastRetrieved).count() < retrieveInterval 
        && peer < peers.peers.size())
        {
            int sockfd = -1;
            try {
                sockfd = PeerConnector::handshake(this->peers.peers[peer], this->torrentFile.InfoHash, this->peerId);
                bitField = PeerConnector::receiveBitField(sockfd);

                cout << "Bitfield received, size: " << bitField.size() << endl;

                //If peer has interesting piece
                while (getBit(bitField, piece)) {
                    cout << "Start downloading piece: " << piece << endl;
                    cout << "Choked: " << choke << endl;
                    cout << "Request sent: " << requestSent << endl;
                    PeerConnector::interested(sockfd);
                    if (piece == totalPieces - 1) downloadPiece(peers.peers[peer], lastPieceSize, sockfd);
                    else downloadPiece(peers.peers[peer], torrentFile.PieceLength, sockfd);
                }
                cout << "Piece " << piece << " not found" << endl;
            } catch (runtime_error& e) {
                //Failed to connect, try another peer
                cout << e.what() << endl;
            }
            peer++;
            choke = true;
            requestSent = false;
            closeSocket(sockfd);
        }
    }
    return 0;
}