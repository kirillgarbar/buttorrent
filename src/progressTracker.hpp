#pragma once

#include "downloader.hpp"

struct ProgressTracker {
private:
    const TorrentDownloader &downloader;
    int piecesDownloadedLastTime = 0;
    int piecesDownloaded = 0;
public:
    ProgressTracker(const TorrentDownloader &downloader);

    ProgressStatistics getStatistics();
    bool displayProgress();
    void trackProgress();
};
