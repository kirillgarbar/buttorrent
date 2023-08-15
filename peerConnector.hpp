#pragma once

#include "peerRetriever.hpp"
#include "bitTorrentMessage.hpp"

//Connects client to peers
struct PeerConnector {
private:
    //Set socket to blocking and nonblocking mode
    static bool setSocketBlocking(int sock, bool blocking);

public:
    static int sendData(const std::string& data, int sockfd);

    static std::string receiveData(int sock, int size = 0);

    //Receive message from peer
    static BitTorrentMessage receiveMessage(int sock);

    static std::string receiveBitField(int sock);

    //Open socket, set timeout and establishe connection to the peer
    static int connectToPeer(const Peer& peer);

    //Open socket and connect to peer with connectToPeer, send and receive handshake message
    static int handshake(const Peer& peer, const std::string& infoHash, const std::string& peerId);

    //Send interested message
    static void interested(int sock);
};