#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "peerRetriever.hpp"
#include "bitTorrentMessage.hpp"

#include "utils.hpp"

#define INFO_HASH_STARTING_POS 28
#define HASH_LEN 20
#define READ_TIMEOUT 15000

//Connects client to peers
struct PeerConnector {
private:
    //Set socket to blocking and nonblocking mode
    static inline bool setSocketBlocking(int sock, bool blocking) {
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) return false;
        flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
        return (fcntl(sock, F_SETFL, flags) == 0);
    }

    static inline std::string createHandshake(const std::string& infoHash, const std::string& peerId) {
        using namespace std;

        static string pstr = "BitTorrent protocol";
        
        stringstream message;
        message << (char)19 << pstr;
        for (int i = 0; i < 8; i++) message << (char)0;
        message << hexDecode(infoHash) << peerId;

        return message.str();
    }

public:
    static inline int sendData(const std::string& data, int sockfd) {
        return send(sockfd, data.c_str(), data.size(), 0);
    }

    static inline std::string receiveData(int sock, int size = 0) {
        std::string reply;

        //used for receiving non-handshake messages, length is first 4 bytes of message
        if (!size) {
            struct pollfd fd;
            int ret;
            fd.fd = sock;
            fd.events = POLLIN;
            ret = poll(&fd, 1, READ_TIMEOUT);

            if (ret <= 0) { return ""; }

            const int lengthSize = 4;
            char buffer[lengthSize];

            long bytesRead = recv(sock, buffer, lengthSize, 0);

            if (bytesRead != lengthSize) { return ""; }
            
            std::string length;
            for(int i = 0; i < lengthSize; i++) length.push_back(buffer[i]);
            size = bytesToInt(length);
        }

        char buffer[size];
        // Receives reply from the host
        // Keeps reading from the buffer until all expected bytes are received
        long bytesRead = 0;
        long bytesToRead = size;
        do
        {
            bytesRead = recv(sock, buffer, size, 0);

            if (bytesRead <= 0) return "";
                
            bytesToRead -= bytesRead;
            for (int i = 0; i < bytesRead; i++)
                reply.push_back(buffer[i]);
        }
        while (bytesToRead > 0);

        return reply;
    }

    //Receive message from peer
    static inline BitTorrentMessage receiveMessage(int sock) {
        using namespace std;

        string data = receiveData(sock);

        string mes = to_string(MessageId::keepAlive);
        if (!data.size()) return BitTorrentMessage(mes);
        return BitTorrentMessage(data);
    }

    static inline std::string receiveBitField(int sock) {
        BitTorrentMessage message = receiveMessage(sock);
        if (message.getId() != MessageId::bitField) return "";
        return message.getPayload();
    }

    //Open socket, set timeout and establishe connection to the peer
    static inline int connectToPeer(const Peer& peer) {
        using namespace std;

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
        return -1;
    }

    //Send handshake message
    static inline int handshake(const Peer& peer, const std::string& infoHash, const std::string& peerId) {
        using namespace std;

        int sockfd = connectToPeer(peer);
        if (sockfd == -1) return -1;

        string handshakeMessage = createHandshake(infoHash, peerId);

        //Send
        sendData(handshakeMessage, sockfd);

        //Receive
        std::string reply = receiveData(sockfd, handshakeMessage.size());
        cout << reply << endl;
        
        //Verify handshake
        if (reply.size() != handshakeMessage.size()) { close(sockfd); return -1; }
        if ((reply.substr(INFO_HASH_STARTING_POS, HASH_LEN) == infoHash) != 0) { close(sockfd); return -1; }
        
        return sockfd;
    }
};