#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "peerRetriever.hpp"

struct PeerConnector {
private:
    static inline bool setSocketBlocking(int sock, bool blocking) {
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) return false;
        flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
        return (fcntl(sock, F_SETFL, flags) == 0);
    }

public:
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

    static inline std::string createHandshake(const std::string& infoHash, const std::string& peerId) {
        using namespace std;

        static string pstr = "BitTorrent protocol";
        
        stringstream message;
        message << to_string((unsigned char)19) << pstr;
        for (int i = 0; i < 8; i++) message << to_string((unsigned char)0);
        message << infoHash << peerId;

        return message.str();
    }

    static inline int handshake(const Peer& peer, const std::string& infoHash, const std::string& peerId) {
        using namespace std;

        int sockfd = connectToPeer(peer);
        if (sockfd == -1) return -1;

        string handshakeMessage = createHandshake(infoHash, peerId);

        cout << send(sockfd, handshakeMessage.c_str(), handshakeMessage.size(), 0) << endl;
        return sockfd;
    }
};