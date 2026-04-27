#pragma once

#include <immintrin.h>
#include <filesystem>
#include <concepts>
#include <cstring>
#include <vector>
#include <array>
#include <span>

namespace fims {

static constexpr size_t HashBytes = 32;

struct alignas(32) Hash {
    union {
        std::array<uint8_t, HashBytes> bytes;
        __m256i simd;
    };

    Hash();
    explicit Hash(const void* data);
    
     inline Hash operator^(const Hash& other) const {
        Hash result;
        result.simd = _mm256_xor_si256(this->simd, other.simd);
        return result;
    }

    inline void operator^=(const Hash& other) {
        this->simd = _mm256_xor_si256(this->simd, other.simd);
    }

    bool operator==(const Hash& other) const {
        return std::memcmp(bytes.data(), other.bytes.data(), HashBytes) == 0;
    }
};

class LinearSolver {
public:

    LinearSolver() = default;
    explicit LinearSolver(std::span<const Hash> initial_blobs);
    explicit LinearSolver(const std::filesystem::path& file_path);

    void Push(const Hash& hash);
    void Pop();

    std::vector<std::vector<size_t>> Solve(const Hash& target, std::span<std::span<const Hash>> remote_blobs) const;

private:
    std::vector<Hash> m_local_blobs;
};

} // namespace fims
