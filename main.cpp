
#include <string>
#include <iostream>
#include <fstream>
#include "decoder.hpp"

int main() {
    using namespace std;
    // parse user input
    
    // parse torrent
    TorrentFile tf = Decoder::getTorrentFile("Debian.torrent");
    cout << tf.InfoHash;

    // get peers 

    // download

    return 0;
}