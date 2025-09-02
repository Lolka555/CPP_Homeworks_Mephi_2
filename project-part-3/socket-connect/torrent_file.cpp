#include "torrent_file.h"
#include "bencode.h"

TorrentFile LoadTorrentFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string data = oss.str();

    Bencode::BencodeParser parser(data);
    Bencode::BencodeValueWrapper root = parser.Parse();

    if (!std::holds_alternative<std::map<std::string, Bencode::BencodeValueWrapper>>(root.value)) {
        throw std::runtime_error("Root of .torrent must be a dict");
    }

    const auto& dict = std::get<std::map<std::string, Bencode::BencodeValueWrapper>>(root.value);
    TorrentFile result;

    if (auto it = dict.find("announce"); it != dict.end()) { // announce
        if (std::holds_alternative<std::string>(it->second.value)) {
            result.announce = std::get<std::string>(it->second.value);
        }
    }

    if (auto it = dict.find("comment"); it != dict.end()) { // comment
        if (std::holds_alternative<std::string>(it->second.value)) {
            result.comment = std::get<std::string>(it->second.value);
        }
    }

    auto info_it = dict.find("info"); // info
    if (info_it == dict.end()) {
        throw std::runtime_error("info dictionary not found");
    }

    const Bencode::BencodeValueWrapper& info_value = info_it->second;

    auto encodeBencode = [](const Bencode::BencodeValueWrapper& val) -> std::string { // info в bencode
        const auto& v = val.value;
        if (std::holds_alternative<int64_t>(v)) {
            return "i" + std::to_string(std::get<int64_t>(v)) + "e";
        }
        if (std::holds_alternative<std::string>(v)) {
            const std::string& s = std::get<std::string>(v);
            return std::to_string(s.size()) + ":" + s;
        }
        if (std::holds_alternative<std::vector<Bencode::BencodeValueWrapper>>(v)) {
            std::string res = "l";
            for (const auto& elem : std::get<std::vector<Bencode::BencodeValueWrapper>>(v)) {
                res += Bencode::EncodeBencode(elem);
            }
            res += "e";
            return res;
        }
        if (std::holds_alternative<std::map<std::string, Bencode::BencodeValueWrapper>>(v)) {
            std::string res = "d";
            for (const auto& [k, val] : std::get<std::map<std::string, Bencode::BencodeValueWrapper>>(v)) {
                res += std::to_string(k.size()) + ":" + k;
                res += Bencode::EncodeBencode(val);
            }
            res += "e";
            return res;
        }
        throw std::runtime_error("Unknown bencode type");
    };

    std::string info_raw = encodeBencode(info_value);

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(info_raw.data()), info_raw.size(), hash);
    result.infoHash = std::string(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);;

    if (!std::holds_alternative<std::map<std::string, Bencode::BencodeValueWrapper>>(info_value.value)) {
        throw std::runtime_error("info must be a dict");
    }
    const auto& info = std::get<std::map<std::string, Bencode::BencodeValueWrapper>>(info_value.value);

    if (auto it = info.find("name"); it != info.end()) {
        if (std::holds_alternative<std::string>(it->second.value)) {
            result.name = std::get<std::string>(it->second.value);
        }
    }

    if (auto it = info.find("piece length"); it != info.end()) {
        if (std::holds_alternative<int64_t>(it->second.value)) {
            result.pieceLength = static_cast<size_t>(std::get<int64_t>(it->second.value));
        }
    }

    if (auto it = info.find("length"); it != info.end()) {
        if (std::holds_alternative<int64_t>(it->second.value)) {
            result.length = static_cast<size_t>(std::get<int64_t>(it->second.value));
        }
    }

    if (auto it = info.find("pieces"); it != info.end()) {
        if (std::holds_alternative<std::string>(it->second.value)) {
            const std::string& pieces = std::get<std::string>(it->second.value);
            if (pieces.size() % 20 != 0) {
                throw std::runtime_error("Invalid pieces len");
            }

            for (size_t i = 0; i < pieces.size(); i += 20) {
                result.pieceHashes.push_back(pieces.substr(i, 20));
            }
        }
    }

    return result;
}
