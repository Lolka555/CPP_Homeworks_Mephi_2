#include "bencode.h"

namespace Bencode {
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
}
