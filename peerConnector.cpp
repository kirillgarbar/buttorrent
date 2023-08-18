#include "peerConnector.hpp"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <stdexcept>
#include "utils.hpp"
#include "messenger.hpp"

#define INFO_HASH_STARTING_POS 28
#define HASH_LEN 20
#define READ_TIMEOUT 10000
#define READ_TIMEOUT_SHORT 1000

using namespace std;

//Set socket to blocking and nonblocking mode
bool PeerConnector::setSocketBlocking(int sock, bool blocking) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(sock, F_SETFL, flags) == 0);
}

int PeerConnector::sendData(const std::string& data, int sockfd) {
    return send(sockfd, data.c_str(), data.size(), 0);
}

string PeerConnector::receiveData(int sock, int size) {
    string reply;

    //used for receiving non-handshake messages, length is first 4 bytes of message
    if (!size) {
        struct pollfd fd;
        int ret;
        fd.fd = sock;
        fd.events = POLLIN;
        ret = poll(&fd, 1, READ_TIMEOUT);

        if (ret <= 0) throw runtime_error("Data receive timeout");

        const int lengthSize = 4;
        char buffer[lengthSize];

        long bytesRead = recv(sock, buffer, lengthSize, 0);

        if (bytesRead != lengthSize) throw runtime_error("Receiving message length failed");
        
        string length;
        for(int i = 0; i < lengthSize; i++) length.push_back(buffer[i]);
        size = bytesToInt(length);
    }

    // If the buffer size is greater than uint16_t max, a segfault will
    // occur when initializing the buffer
    if (size > std::numeric_limits<uint16_t>::max())
        throw std::runtime_error("Received corrupted data");

    char buffer[size];
    // Receives reply from the host
    // Keeps reading from the buffer until all expected bytes are received
    long bytesRead = 0;
    long bytesToRead = size;

    do
    {
        struct pollfd fd;
        fd.fd = sock;
        fd.events = POLLIN;
        int ret = poll(&fd, 1, READ_TIMEOUT_SHORT);

        if (ret <= 0) throw runtime_error("Data receive timeout");

        bytesRead = recv(sock, buffer, size, 0);

        if (bytesRead <= 0) throw runtime_error("Nothing to receive");
            
        bytesToRead -= bytesRead;
        for (int i = 0; i < bytesRead; i++)
            reply.push_back(buffer[i]);
    }
    while (bytesToRead > 0);

    return reply;
}

//Receive message from peer
BitTorrentMessage PeerConnector::receiveMessage(int sock) {
    using namespace std;

    string data = receiveData(sock);

    if (!data.size()) return BitTorrentMessage(MessageId::keepAlive);
    return BitTorrentMessage(data);
}

std::string PeerConnector::receiveBitField(int sock) {
    BitTorrentMessage message = receiveMessage(sock);
    if (message.getId() != MessageId::bitField) return "";
    return message.getPayload();
}

//Open socket, set timeout and establishe connection to the peer
int PeerConnector::connectToPeer(const Peer& peer) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Make adress
    struct sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(peer.Port);
    inet_pton(AF_INET, peer.Ip.c_str(), &addr.sin_addr);

    //Non blocking mode
    if (!setSocketBlocking(sockfd, false)) return -1;

    int status = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    //Set timeout
    fd_set fdset;
    struct timeval tv;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0)
        {
            // Sets socket to blocking mode
            if (!setSocketBlocking(sockfd, true)) { close(sockfd); return -1; }
            return sockfd;
        }
    }
    close(sockfd);
    throw runtime_error("Connect to peer failed");
}

//Send handshake message
int PeerConnector::handshake(const Peer& peer, const std::string& infoHash, const std::string& peerId) {
    int sockfd = connectToPeer(peer);

    string handshakeMessage = Messenger::createHandshake(infoHash, peerId);

    //Send
    sendData(handshakeMessage, sockfd);

    //Receive
    string reply = receiveData(sockfd, handshakeMessage.size());
    
    //Verify handshake
    if (reply.size() != handshakeMessage.size()) { close(sockfd); throw runtime_error("Failed to receive handshake"); }
    if ((reply.substr(INFO_HASH_STARTING_POS, HASH_LEN) == infoHash) != 0) { 
        close(sockfd); 
        throw runtime_error("Failed to receive handshake - hashes are not equal"); 
    }
    
    return sockfd;
}

void PeerConnector::interested(int sock) {
    string message = Messenger::createInterested();
    sendData(message, sock);
}

void PeerConnector::requestPiece(int index, int offset, int size, int sock) {
    string message = Messenger::createRequest(index, offset, size);
    sendData(message, sock);
}