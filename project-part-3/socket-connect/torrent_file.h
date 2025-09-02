#pragma once

#include <string>
#include <vector>
#include <openssl/sha.h>
#include <map>
#include <list>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "bencode.h"

struct TorrentFile {
    std::string announce; // URL трекера
    std::string comment; // Comment
    std::vector<std::string> pieceHashes; // 20-byte hashes of all chunks .torrent
    size_t pieceLength; // Lenght of chunk
    size_t length; // Lenght of file
    std::string name; // Name of file
    std::string infoHash; // SHA1 of "info" dict
};

TorrentFile LoadTorrentFile(const std::string& filename);