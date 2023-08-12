
#include <string>
#include <iostream>
#include <fstream>
#include "decoder.hpp"
#include "peerRetriever.hpp"

int main() {
    using namespace std;
    // parse user input
    
    // parse torrent
    TorrentFile tf = Decoder::getTorrentFile("Debian.torrent");

    PeerRetriever::retrievePeers(tf);
    // download

    return 0;
}