#include "progressTracker.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <cmath>
#include "utils.hpp"

#define PROGRESS_DISPLAY_INTERVAL 1

using namespace std;

ProgressTracker::ProgressTracker(const TorrentDownloader &downloader): downloader{downloader} {}

ProgressStatistics ProgressTracker::getStatistics() { return downloader.getStatistics(); }

bool ProgressTracker::displayProgress() {
    ProgressStatistics stats = getStatistics();
    stringstream info;

    piecesDownloadedLastTime = stats.downloadedPieces - piecesDownloaded;
    piecesDownloaded = stats.downloadedPieces;
    
    double downloadSpeed = double(piecesDownloadedLastTime * stats.pieceSize) / 1024; //KBs
    double progress = double(stats.downloadedPieces) / stats.totalPieces;
    long timeSpent = floor(stats.currentTime - stats.startTime);

    info << fixed << setprecision(2) << downloadSpeed << " KB/s, ";
    info << piecesDownloaded << "/" << stats.totalPieces << " Pieces, ";
    info << progress * 100 << "%, ";
    info << formatTime(timeSpent);

    cout << info.str() << '\r';
    cout.flush();

    return floor(progress);
}

void ProgressTracker::trackProgress() {
    while (!displayProgress()) { usleep(PROGRESS_DISPLAY_INTERVAL * pow(10, 6)); }
}