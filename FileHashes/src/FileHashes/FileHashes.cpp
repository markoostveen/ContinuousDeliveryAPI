#include "FileHashes.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>

#include <filesystem>

// OpenSSL Library
#include <openssl/sha.h>

std::string SHA256(const char* const path)
{
    std::ifstream fp(path, std::ios::in | std::ios::binary);

    if (!fp.good()) {
        std::ostringstream os;
        throw std::runtime_error(os.str());
    }

    constexpr const std::size_t buffer_size{ 1 << 12 };
    char buffer[buffer_size];

    auto* hash = new unsigned char[SHA256_DIGEST_LENGTH] { 0 };

    std::stringstream ss;
    while (fp.good()) {
        ss.clear();

        fp.read(buffer, buffer_size);
        for (size_t i = 0; i < buffer_size; i++)
        {
            ss << buffer[i];
        }

        std::string block = ss.str();
        hash = SHA256((const unsigned char*)block.c_str(), block.size(), hash);
    }
    fp.close();

    std::ostringstream os;
    os << std::hex << std::setfill('0');

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        os << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }

    delete hash;

    return os.str();
}

std::string GetFileHashForFile(std::string filePath)
{
    std::string hash = "";
    try {
        hash = SHA256(filePath.c_str());
    }
    catch (const std::exception& e) {
        std::cerr << "[fatal] " << e.what() << std::endl;
    }

    return hash;
}
