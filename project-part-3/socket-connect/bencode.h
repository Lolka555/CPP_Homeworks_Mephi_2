#pragma once

#include <string>
#include <vector>
#include <variant>
#include <list>
#include <map>
#include <stdexcept>

namespace Bencode {

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

    std::string EncodeBencode(const BencodeValueWrapper& val);

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

}
