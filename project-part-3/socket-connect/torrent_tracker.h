#pragma once

#include <string>
#include "torrent_file.h"
#include "peer.h"
#include "bencode.h"
#include <cpr/cpr.h>

class TorrentTracker {
    public:
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
    
            Bencode::BencodeParser parser(res.text);
            Bencode::BencodeValueWrapper parsedData = parser.Parse();
    
    
            if (std::holds_alternative<std::map<std::string, Bencode::BencodeValueWrapper>>(parsedData.value)) {
                auto& dict = std::get<std::map<std::string, Bencode::BencodeValueWrapper>>(parsedData.value); // from variant getting dict
                if (dict.find("peers") != dict.end()) {
                    Bencode::BencodeValueWrapper& peersData = dict["peers"];
                    if (std::holds_alternative<std::string>(peersData.value)) {
                        std::string peerData = std::get<std::string>(peersData.value);
                        for (size_t i = 0; i < peerData.size(); i += 6) {
                            std::string ip = std::to_string((unsigned char)peerData[i]) + "." + 
                                             std::to_string((unsigned char)peerData[i + 1]) + "." +
                                             std::to_string((unsigned char)peerData[i + 2]) + "." +
                                             std::to_string((unsigned char)peerData[i + 3]);
                            int port = (static_cast<unsigned char>(peerData[i + 4]) << 8) | static_cast<unsigned char> (peerData[i + 5]);
        
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
    