#include "local/linear_solver.hpp"
#include <algorithm>
#include <bitset>
#include <fstream>
#include <numeric>
#include <cstdint>

// Cross-platform trailing zero counter
#ifdef _MSC_VER
#include <intrin.h>
static inline int fast_ctz64(uint64_t x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
}
#else
static inline int fast_ctz64(uint64_t x) {
    return __builtin_ctzll(x);
}
#endif

namespace fims {

Hash::Hash() { simd = _mm256_setzero_si256(); }

Hash::Hash(const void* data) {
    memcpy(bytes.data(), data, HashBytes);
}

LinearSolver::LinearSolver(std::span<const Hash> initial_blobs)
    : m_local_blobs(initial_blobs.begin(), initial_blobs.end()) {}

LinearSolver::LinearSolver(const std::filesystem::path& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) return;

    Hash buffer;
    while (file.read(reinterpret_cast<char*>(buffer.bytes.data()), HashBytes)) {
        m_local_blobs.push_back(buffer);
    }
}

void LinearSolver::Push(const Hash& hash) { m_local_blobs.push_back(hash); }
void LinearSolver::Pop() { if (!m_local_blobs.empty()) m_local_blobs.pop_back(); }

std::vector<std::vector<size_t>> LinearSolver::Solve(
    const Hash& target,
    std::span<std::span<const Hash>> remote_blobs) const
{
    const size_t num_remote_nodes = remote_blobs.size();
    const size_t total_nodes = num_remote_nodes + 1;

    std::vector<std::vector<size_t>> results(total_nodes);

    struct Candidate {
        Hash hash;
        size_t node_list_id;
        size_t original_index;
    };
    
    std::vector<Candidate> candidates;
    candidates.reserve(m_local_blobs.size() +
                       std::accumulate(remote_blobs.begin(), remote_blobs.end(), size_t{0},[](size_t s, auto& span) { return s + span.size(); }));

    for (size_t node_id = 0; node_id < num_remote_nodes; ++node_id) {
        for (size_t i = 0; i < remote_blobs[node_id].size(); ++i) {
            candidates.push_back({remote_blobs[node_id][i], node_id, i});
        }
    }
    for (size_t i = 0; i < m_local_blobs.size(); ++i) {
        candidates.push_back({m_local_blobs[i], num_remote_nodes, i});
    }

    if (candidates.empty()) return {};

    std::vector<Hash> basis(256);
    std::vector<std::bitset<4096>> basis_comb(256);
    std::vector<bool> pivot_filled(256, false);

    for (size_t c_idx = 0; c_idx < candidates.size(); ++c_idx) {
        Hash val = candidates[c_idx].hash;
        std::bitset<4096> current_comb;
        current_comb.set(c_idx);

        while (true) {
            // Fast zero test using AVX
            if (_mm256_testz_si256(val.simd, val.simd)) break;

            alignas(32) uint64_t chunks[4];
            _mm256_store_si256(reinterpret_cast<__m256i*>(chunks), val.simd);

            int pivot = -1;
            for (int j = 0; j < 4; ++j) {
                if (chunks[j]) {
                    pivot = static_cast<int>(j * 64 + fast_ctz64(chunks[j]));
                    break;
                }
            }

            if (!pivot_filled[pivot]) {
                basis[pivot] = val;
                basis_comb[pivot] = current_comb;
                pivot_filled[pivot] = true;
                break;
            }

            val ^= basis[pivot];
            current_comb ^= basis_comb[pivot];
        }
    }

    Hash current_goal = target;
    std::bitset<4096> final_solution;

    while (true) {
        if (_mm256_testz_si256(current_goal.simd, current_goal.simd)) break;

        alignas(32) uint64_t chunks[4];
        _mm256_store_si256(reinterpret_cast<__m256i*>(chunks), current_goal.simd);

        int pivot = -1;
        for (int j = 0; j < 4; ++j) {
            if (chunks[j]) {
                pivot = static_cast<int>(j * 64 + fast_ctz64(chunks[j]));
                break;
            }
        }

        if (!pivot_filled[pivot]) return {}; // Unreachable target

        current_goal ^= basis[pivot];
        final_solution ^= basis_comb[pivot];
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        if (final_solution.test(i)) {
            const auto& cand = candidates[i];
            results[cand.node_list_id].push_back(cand.original_index);
        }
    }

    return results;
}

} // namespace fims