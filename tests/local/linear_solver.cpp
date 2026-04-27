#include <gtest/gtest.h>
#include "local/linear_solver.hpp"

using namespace fims;

// Helper to create a hash with a single specific bit set
Hash make_bit_hash(size_t bit) {
    uint8_t data[32] = {0};
    data[bit >> 3] |= (1 << (bit & 7));
    return Hash(data);
}

TEST(LinearSolverTest, MultiNodeSolve) {
    // Setup:
    // Node 0 has Bit 10
    // Node 1 has Bit 20
    // Local (Node 2) has Bit 30
    
    Hash h10 = make_bit_hash(10);
    Hash h20 = make_bit_hash(20);
    Hash h30 = make_bit_hash(30);

    std::vector<Hash> node0_blobs = { h10 };
    std::vector<Hash> node1_blobs = { h20 };
    
    std::vector<std::span<const Hash>> remote_nodes;
    remote_nodes.push_back(node0_blobs);
    remote_nodes.push_back(node1_blobs);

    LinearSolver solver;
    solver.Push(h30);

    // Target is XOR of all three
    Hash target = h10 ^ h20 ^ h30;

    auto result = solver.Solve(target, remote_nodes);

    // We expect 3 vectors (Node 0, Node 1, and Local)
    ASSERT_EQ(result.size(), 3);
    
    // Node 0 should return index 0
    ASSERT_EQ(result[0].size(), 1);
    EXPECT_EQ(result[0][0], 0);

    // Node 1 should return index 0
    ASSERT_EQ(result[1].size(), 1);
    EXPECT_EQ(result[1][0], 0);

    // Local (last vector) should return index 0
    ASSERT_EQ(result[2].size(), 1);
    EXPECT_EQ(result[2][0], 0);
}

TEST(LinearSolverTest, NoSolutionReturnsEmpty) {
    LinearSolver solver;
    Hash target = make_bit_hash(5); // Something we don't have
    
    std::vector<Hash> remote_node = { make_bit_hash(1) };
    std::vector<std::span<const Hash>> remotes = { remote_node };

    auto result = solver.Solve(target, remotes);
    
    // Mathematically impossible, should return empty outer vector
    EXPECT_TRUE(result.empty());
}

TEST(LinearSolverTest, LocalOnlySolve) {
    LinearSolver solver;
    Hash h1 = make_bit_hash(1);
    Hash h2 = make_bit_hash(2);
    solver.Push(h1);
    solver.Push(h2);

    // Target is h1 ^ h2
    auto result = solver.Solve(h1 ^ h2, {});

    // Only one node involved (local)
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].size(), 2); // Indices 0 and 1
}

TEST(LinearSolverTest, SimdOperatorVerification) {
    Hash a = make_bit_hash(0);
    Hash b = make_bit_hash(1);
    Hash c = a ^ b;

    EXPECT_EQ(c.bytes[0], 0x03); // 0b00000011
    
    Hash d = c;
    d ^= b; // Should recover 'a'
    EXPECT_EQ(d, a);
}
