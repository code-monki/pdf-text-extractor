// File: src/core/file_digest.cpp
// Purpose: SHA-256 file digest for inventory and NFR-003 verification.

#include "core/file_digest.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace pte::core {

namespace {

/**
 * @brief Minimal SHA-256 implementation for local file identity checks.
 */
class Sha256 {
public:
    void update(const unsigned char* data, std::size_t length) {
        for (std::size_t index = 0; index < length; ++index) {
            buffer_[bufferLength_++] = data[index];
            bitLength_ += 8;
            if (bufferLength_ == blockSize) {
                transform(buffer_.data());
                bufferLength_ = 0;
            }
        }
    }

    std::string finalHex() {
        buffer_[bufferLength_++] = 0x80;
        if (bufferLength_ > 56) {
            while (bufferLength_ < blockSize) {
                buffer_[bufferLength_++] = 0;
            }
            transform(buffer_.data());
            bufferLength_ = 0;
        }
        while (bufferLength_ < 56) {
            buffer_[bufferLength_++] = 0;
        }

        for (int shift = 56; shift >= 0; shift -= 8) {
            buffer_[bufferLength_++] = static_cast<unsigned char>((bitLength_ >> shift) & 0xff);
        }
        transform(buffer_.data());

        std::ostringstream output;
        output << std::hex << std::setfill('0');
        for (const auto word : state_) {
            output << std::setw(8) << word;
        }
        return output.str();
    }

private:
    static constexpr std::size_t blockSize = 64;
    std::array<unsigned char, blockSize> buffer_{};
    std::size_t bufferLength_ = 0;
    std::uint64_t bitLength_ = 0;
    std::array<std::uint32_t, 8> state_{
        0x6a09e667,
        0xbb67ae85,
        0x3c6ef372,
        0xa54ff53a,
        0x510e527f,
        0x9b05688c,
        0x1f83d9ab,
        0x5be0cd19
    };

    static std::uint32_t rotateRight(std::uint32_t value, std::uint32_t bits) {
        return (value >> bits) | (value << (32 - bits));
    }

    void transform(const unsigned char* data) {
        static constexpr std::array<std::uint32_t, 64> constants{
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        std::array<std::uint32_t, 64> words{};
        for (std::size_t index = 0; index < 16; ++index) {
            const std::size_t byteIndex = index * 4;
            words[index] = (static_cast<std::uint32_t>(data[byteIndex]) << 24)
                | (static_cast<std::uint32_t>(data[byteIndex + 1]) << 16)
                | (static_cast<std::uint32_t>(data[byteIndex + 2]) << 8)
                | static_cast<std::uint32_t>(data[byteIndex + 3]);
        }
        for (std::size_t index = 16; index < 64; ++index) {
            const auto s0 = rotateRight(words[index - 15], 7)
                ^ rotateRight(words[index - 15], 18)
                ^ (words[index - 15] >> 3);
            const auto s1 = rotateRight(words[index - 2], 17)
                ^ rotateRight(words[index - 2], 19)
                ^ (words[index - 2] >> 10);
            words[index] = words[index - 16] + s0 + words[index - 7] + s1;
        }

        auto a = state_[0];
        auto b = state_[1];
        auto c = state_[2];
        auto d = state_[3];
        auto e = state_[4];
        auto f = state_[5];
        auto g = state_[6];
        auto h = state_[7];

        for (std::size_t index = 0; index < 64; ++index) {
            const auto s1 = rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
            const auto choice = (e & f) ^ ((~e) & g);
            const auto temp1 = h + s1 + choice + constants[index] + words[index];
            const auto s0 = rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
            const auto majority = (a & b) ^ (a & c) ^ (b & c);
            const auto temp2 = s0 + majority;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }
};

} // namespace

std::optional<std::string> sha256HexFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }

    Sha256 sha;
    std::array<char, 8192> buffer{};
    while (input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto readCount = input.gcount();
        if (readCount > 0) {
            sha.update(reinterpret_cast<const unsigned char*>(buffer.data()),
                       static_cast<std::size_t>(readCount));
        }
    }
    return sha.finalHex();
}

} // namespace pte::core
