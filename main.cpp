#include <string>
#include <iostream>
#include <fstream>
#include "downloader.hpp"

int main() {
    using namespace std;

    TorrentDownloader d{"Debian.torrent"};
    d.download();
    return 0;
}