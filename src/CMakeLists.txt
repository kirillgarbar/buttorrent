include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

set(SOURCES main.cpp downloader.cpp utils.cpp bitTorrentMessage.cpp peerConnector.cpp peerRetriever.cpp messenger.cpp fileManager.cpp)

add_executable(Buttorrent ${SOURCES})

target_link_libraries(Buttorrent PRIVATE cpr::cpr)