#include "byte_tools.h"

int BytesToInt(std::string_view bytes) {
    if (bytes.size() != 4) {
        throw std::invalid_argument("Expected 4 bytes");
    }
    int BE_int = ((static_cast<unsigned char>(bytes[0]) << 24) |
    (static_cast<unsigned char>(bytes[1]) << 16) |
    (static_cast<unsigned char>(bytes[2]) << 8)  |
    (static_cast<unsigned char>(bytes[3])));

    return BE_int;
}