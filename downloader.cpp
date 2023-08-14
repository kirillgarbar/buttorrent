#include "downloader.hpp"
#include <chrono>

using namespace std;

TorrentDownloader::TorrentDownloader(std::string fileName) {
    torrentFile = Decoder::getTorrentFile(fileName);
    peerId = generatePeerId();
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

int TorrentDownloader::download() {
    int progress = 0;
    
    while (progress < 100) {
        peers = PeerRetriever::retrievePeers(torrentFile, peerId);
        auto lastRetrieved = getTimestamp();
        int retrieveInterval = peers.interval * 1000;

        int peer = 0;
        while (chrono::duration_cast<chrono::milliseconds>(getTimestamp() - lastRetrieved).count() < retrieveInterval 
        && peer < peers.peers.size())
        {
            int sockfd = PeerConnector::handshake(this->peers.peers[peer], this->torrentFile.InfoHash, this->peerId);
            if (sockfd > -1) {
                close(sockfd);
            }
            peer++;
            cout << to_string(chrono::duration_cast<chrono::milliseconds>(getTimestamp() - lastRetrieved).count()) << endl;
        }
        progress += 10;
        cout << "Making progress" << endl;
    }
    return 0;
}