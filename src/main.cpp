#include <string>
#include <iostream>
#include <fstream>
#include "downloader.hpp"
#include "cxxopts.hpp"

int main(int argc, char * argv[]) {
    using namespace std;

    cxxopts::Options options("Buttorrent", "Simple torrent client");
    options.add_options()
    ("f,torrentFile", "Torrent file path", cxxopts::value<string>())
    ("o,outputDirectory", "Output directory", cxxopts::value<string>()->default_value("../../result/"))
    ;

    try {
        auto result = options.parse(argc, argv);

        if (result.count("torrentFile")) {
            TorrentDownloader d{result["f"].as<string>(), result["o"].as<string>()};
            d.download();
        }
    } catch (cxxopts::OptionException &e) {
        cout << e.what() << endl;
        return 1;
    }
    
    return 0;
}