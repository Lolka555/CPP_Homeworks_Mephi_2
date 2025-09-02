#pragma once

#include <string>
#include <stdexcept>

/*
 * Преобразовать 4 байта в формате big endian в int
 */
int BytesToInt(std::string_view bytes);