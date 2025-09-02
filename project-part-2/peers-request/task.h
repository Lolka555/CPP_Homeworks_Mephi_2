#pragma once

#include "peer.h"
#include "torrent_file.h"
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <cpr/cpr.h>

struct BencodeValueWrapper {

    using BencodeValue = std::variant< // устройство бенкод
    int64_t,
    std::string,
    std::vector<BencodeValueWrapper>,
    std::map<std::string, BencodeValueWrapper>
    >;

    BencodeValueWrapper() : value(int64_t(0)) {}

    BencodeValue value;
    BencodeValueWrapper(int64_t v) : value(v) {}
    BencodeValueWrapper(std::string v) : value(std::move(v)) {}
    BencodeValueWrapper(std::vector<BencodeValueWrapper> v) : value(std::move(v)) {}
    BencodeValueWrapper(std::map<std::string, BencodeValueWrapper> v) : value(std::move(v)) {}
};


struct BencodeParser {
    const std::string& data;
    size_t pointer = 0;

    BencodeParser(const std::string& input) : data(input) {} // constructor

    BencodeValueWrapper Parse() {
        if (pointer >= data.size()) {
            throw std::runtime_error("Unexpected end of data");
        }

        char ch = data[pointer];

        if (ch == 'i') return ParseInteger();
        else if (std::isdigit(ch)) return ParseString();
        else if (ch == 'l') return ParseList();
        else if (ch == 'd') return ParseDict();
        else throw std::runtime_error("Invalid bencode at pointer " + std::to_string(pointer));
    }

private:
    BencodeValueWrapper ParseInteger() {
        pointer++; // skip 'i'
        size_t end = data.find('e', pointer); // если не найдем 'e' получим npos
        if (end == std::string::npos) throw std::runtime_error("Invalid INT");

        int64_t val = std::stoll(data.substr(pointer, end - pointer)); // string to long long (stoll)
        pointer = end + 1;
        return val;
    }

    BencodeValueWrapper ParseString() {
        size_t colon = data.find(':', pointer);
        if (colon == std::string::npos) throw std::runtime_error("Invalid str");

        size_t len = std::stoul(data.substr(pointer, colon - pointer)); // str to unsigned long (stoul) but anyway can convert to size_t
        pointer = colon + 1;
        if (pointer + len > data.size()) throw std::runtime_error("Str len out of bounds");

        std::string str = data.substr(pointer, len);
        pointer += len;
        return str;
    }

    BencodeValueWrapper ParseList() {
        pointer++; // skip 'l'
        std::vector<BencodeValueWrapper> list;
        while (data[pointer] != 'e') {
            list.push_back(Parse());
        }
        pointer++; // skip 'e'
        return list;
    }

    BencodeValueWrapper ParseDict() {
        pointer++; // skip 'd'
        std::map<std::string, BencodeValueWrapper> dict;
        while (data[pointer] != 'e') {
            BencodeValueWrapper keyWrapper = Parse();
            if (!std::holds_alternative<std::string>(keyWrapper.value)) { // holds_alternative проверяет на отличие от заданного типа
                throw std::runtime_error("Dict keys must be str's");
            }
            std::string key = std::get<std::string>(keyWrapper.value);
            BencodeValueWrapper val = Parse();
            dict[std::move(key)] = std::move(val);
        }
        pointer++; // skip 'e'
        return dict;
    }
};

class TorrentTracker {
public:
    /*
     * url - адрес трекера, берется из поля announce в .torrent-файле
     */
    TorrentTracker(const std::string& url) : url_(url) {};

    /*
     * Получить список пиров у трекера и сохранить его для дальнейшей работы.
     * Запрос пиров происходит посредством HTTP GET запроса, данные передаются в формате bencode.
     * Такой же формат использовался в .torrent файле.
     * Подсказка: посмотрите, что было написано в main.cpp в домашнем задании torrent-file
     *
     * tf: структура с разобранными данными из .torrent файла из предыдущего домашнего задания.
     * peerId: id, под которым представляется наш клиент.
     * port: порт, на котором наш клиент будет слушать входящие соединения (пока что мы не слушаем и на этот порт никто
     *  не сможет подключиться).
     */
    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port){

        cpr::Response res = cpr::Get(
            cpr::Url{tf.announce},
            cpr::Parameters {
                    {"info_hash", tf.infoHash},
                    {"peer_id", peerId},
                    {"port", std::to_string(port)},
                    {"uploaded", std::to_string(0)},
                    {"downloaded", std::to_string(0)},
                    {"left", std::to_string(tf.length)},
                    {"compact", std::to_string(1)}
            },
            cpr::Timeout{20000}
        );

        if (res.status_code != 200) {
            throw std::runtime_error("Failed to HTTP GET because" + res.error.message);
        }

        BencodeParser parser(res.text);
        BencodeValueWrapper parsedData = parser.Parse();


        if (std::holds_alternative<std::map<std::string, BencodeValueWrapper>>(parsedData.value)) {
            auto& dict = std::get<std::map<std::string, BencodeValueWrapper>>(parsedData.value); // from variant getting dict
            if (dict.find("peers") != dict.end()) {
                BencodeValueWrapper& peersData = dict["peers"];
                if (std::holds_alternative<std::string>(peersData.value)) {
                    std::string peerData = std::get<std::string>(peersData.value);
                    for (size_t i = 0; i < peerData.size(); i += 6) {
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 4209868 (peers2)
                        std::string ip = std::to_string((unsigned char)peerData[i]) + "." + 
                                         std::to_string((unsigned char)peerData[i + 1]) + "." +
                                         std::to_string((unsigned char)peerData[i + 2]) + "." +
                                         std::to_string((unsigned char)peerData[i + 3]);
<<<<<<< HEAD
<<<<<<< HEAD
                        int port = (static_cast<unsigned char>(peerData[i + 4]) << 8) | static_cast<unsigned char> (peerData[i + 5]);
=======
                        int port = (static_cast<unsigned char>(peerData[i + 4]) << 8) |
                                    static_cast<unsigned char>(peerData[i + 5]);
>>>>>>> 9722ee6 (peers1)
=======
                        std::string ip = std::to_string(peerData[i]) + "." + 
                                         std::to_string(peerData[i + 1]) + "." +
                                         std::to_string(peerData[i + 2]) + "." +
                                         std::to_string(peerData[i + 3]);
                        int port = ((peerData[i + 4]) << 8) | (peerData[i + 5]);
>>>>>>> 81d0703 (peers1)
=======
                        int port = (static_cast<unsigned char>(peerData[i + 4]) << 8) | static_cast<unsigned char> (peerData[i + 5]);
>>>>>>> 4209868 (peers2)
    
                        peers_.emplace_back(Peer{ip, port});
                    }
                }
            }
        }
    }

    const std::vector<Peer>& GetPeers() const {
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};


std::string EncodeBencode(const BencodeValueWrapper& val) {
    const auto& v = val.value;

    if (std::holds_alternative<int64_t>(v)) {
        return "i" + std::to_string(std::get<int64_t>(v)) + "e";
    }
    if (std::holds_alternative<std::string>(v)) {
        const std::string& s = std::get<std::string>(v);
        return std::to_string(s.size()) + ":" + s;
    }
    if (std::holds_alternative<std::vector<BencodeValueWrapper>>(v)) {
        std::string res = "l";
        for (const auto& elem : std::get<std::vector<BencodeValueWrapper>>(v)) {
            res += EncodeBencode(elem);
        }
        res += "e";
        return res;
    }
    if (std::holds_alternative<std::map<std::string, BencodeValueWrapper>>(v)) {
        std::string res = "d";
        for (const auto& [k, val] : std::get<std::map<std::string, BencodeValueWrapper>>(v)) {
            res += std::to_string(k.size()) + ":" + k;
            res += EncodeBencode(val);
        }
        res += "e";
        return res;
    }

    throw std::runtime_error("Unknown bencode type");
}

TorrentFile LoadTorrentFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string data = oss.str();

    BencodeParser parser(data);
    BencodeValueWrapper root = parser.Parse();

    if (!std::holds_alternative<std::map<std::string, BencodeValueWrapper>>(root.value)) {
        throw std::runtime_error("Root of .torrent must be a dict");
    }

    const auto& dict = std::get<std::map<std::string, BencodeValueWrapper>>(root.value);
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

    const BencodeValueWrapper& info_value = info_it->second;

    auto encodeBencode = [](const BencodeValueWrapper& val) -> std::string { // info в bencode
        const auto& v = val.value;
        if (std::holds_alternative<int64_t>(v)) {
            return "i" + std::to_string(std::get<int64_t>(v)) + "e";
        }
        if (std::holds_alternative<std::string>(v)) {
            const std::string& s = std::get<std::string>(v);
            return std::to_string(s.size()) + ":" + s;
        }
        if (std::holds_alternative<std::vector<BencodeValueWrapper>>(v)) {
            std::string res = "l";
            for (const auto& elem : std::get<std::vector<BencodeValueWrapper>>(v)) {
                res += EncodeBencode(elem);
            }
            res += "e";
            return res;
        }
        if (std::holds_alternative<std::map<std::string, BencodeValueWrapper>>(v)) {
            std::string res = "d";
            for (const auto& [k, val] : std::get<std::map<std::string, BencodeValueWrapper>>(v)) {
                res += std::to_string(k.size()) + ":" + k;
                res += EncodeBencode(val);
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

    if (!std::holds_alternative<std::map<std::string, BencodeValueWrapper>>(info_value.value)) {
        throw std::runtime_error("info must be a dict");
    }
    const auto& info = std::get<std::map<std::string, BencodeValueWrapper>>(info_value.value);

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
