#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "bencode.hpp"
#include "sha1.hpp"

struct BencodedTorrentFile
{
    bencode::string Announce;
    bencode::string Pieces;
    bencode::integer PieceLength;
    bencode::integer Length;
    bencode::string Name;
    std::string InfoHash;
};

struct TorrentFile
{
    std::string Name;
    std::string Announce;
    std::vector<std::string> PieceHashes;
    std::string InfoHash;
    int PieceLength;
    int Length;
};

//Decodes bencoded torrent file and parses it into struct
struct Decoder {
private:
    static inline BencodedTorrentFile decode(const std::string& filename) {
        using namespace std;

        BencodedTorrentFile res{};
        ifstream stream;

        stream.open(filename);
        auto data = bencode::decode(stream);
        stream.close();

        auto dict = get<bencode::dict>(data); //announce
        auto dictInfo = get<bencode::dict>(dict.find("info")->second); //name, piece length, pieces

        res.Announce = get<bencode::string>(dict.find("announce")->second);
        res.Name = get<bencode::string>(dictInfo.find("name")->second);
        res.PieceLength = get<bencode::integer>(dictInfo.find("piece length")->second);
        res.Length = get<bencode::integer>(dictInfo.find("length")->second);
        res.Pieces = get<bencode::string>(dictInfo.find("pieces")->second);
        res.InfoHash = sha1(dictInfo);
        return res;
    }

    static inline std::string sha1(bencode::dict& dictInfo) {
        using namespace std;

        //encode dict in file
        {
            ofstream infoHashFile("InfoHash.torrent");
            bencode::encode(infoHashFile, dictInfo); 
        }
        
        //sha1 of file
        {
            ifstream infoHashFile("InfoHash.torrent");
            SHA1 sha{};
            sha.update(infoHashFile);
            return sha.final();
        }
    }

    static inline TorrentFile parse(BencodedTorrentFile& bencodedTorrent) {
        using namespace std;

        TorrentFile res{};

        string announce = bencodedTorrent.Announce;
        string name = bencodedTorrent.Name;
        string pieces = bencodedTorrent.Pieces;
        long pieceLength = bencodedTorrent.PieceLength;
        long length = bencodedTorrent.Length;

        res.Announce = announce;
        res.Name = name;
        res.PieceLength = pieceLength;
        res.Length = length;
        res.InfoHash = bencodedTorrent.InfoHash;

        //Hashes
        vector<string> pieceHashes;
        if (pieces.size() % 20 > 0) throw std::invalid_argument("Torrent file's hashes are corrupted");
        else {
            for (int i = 0; i < pieces.size(); i += 20) {
                pieceHashes.push_back(pieces.substr(i, 20));
            }
        }
        res.PieceHashes = move(pieceHashes);

        return res;
    }
    
public:
    static inline std::string sha1(const std::string& s) {
        SHA1 sha{};
        sha.update(s);
        return sha.final();
    }

    static inline TorrentFile getTorrentFile(const std::string& filename) {
        BencodedTorrentFile btf = decode(filename);
        return parse(btf);
    }
};