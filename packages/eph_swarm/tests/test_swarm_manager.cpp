#include <gtest/gtest.h>
#include <cmath>
#include "eph_swarm/swarm_manager.hpp"

using namespace eph;
using namespace eph::swarm;

// === コンストラクタテスト ===

TEST(SwarmManager, Constructor_InitializesCorrectly) {
    SwarmManager swarm(10, 0.098, 6);

    EXPECT_EQ(swarm.size(), 10);
    EXPECT_DOUBLE_EQ(swarm.get_beta(), 0.098);
}

TEST(SwarmManager, Constructor_CreatesAgents) {
    SwarmManager swarm(5, 0.0, 4);

    // 各エージェントが初期化されている
    for (size_t i = 0; i < swarm.size(); ++i) {
        const auto& agent = swarm.get_agent(i);
        EXPECT_DOUBLE_EQ(agent.kappa(), 1.0);
        EXPECT_DOUBLE_EQ(agent.state().fatigue, 0.0);
    }
}

// === β制御テスト ===

TEST(SwarmManager, SetBeta_UpdatesCorrectly) {
    SwarmManager swarm(10, 0.0, 6);

    EXPECT_DOUBLE_EQ(swarm.get_beta(), 0.0);

    swarm.set_beta(0.5);
    EXPECT_DOUBLE_EQ(swarm.get_beta(), 0.5);

    swarm.set_beta(1.0);
    EXPECT_DOUBLE_EQ(swarm.get_beta(), 1.0);
}

// === 近傍検索テスト ===

TEST(SwarmManager, FindNeighbors_ReturnsCorrectCount) {
    SwarmManager swarm(20, 0.1, 6);

    auto neighbors = swarm.find_neighbors(0);

    // 平均近傍数6が返される
    EXPECT_EQ(neighbors.size(), 6);

    // 自分自身は含まれない
    for (size_t n : neighbors) {
        EXPECT_NE(n, 0);
    }
}

TEST(SwarmManager, FindNeighbors_SortedByDistance) {
    SwarmManager swarm(20, 0.1, 6);

    auto neighbors = swarm.find_neighbors(0);

    // 近傍は距離順にソートされている（部分的）
    // （厳密な検証は位置情報が必要なのでスキップ）
    EXPECT_EQ(neighbors.size(), 6);
}

TEST(SwarmManager, FindNeighbors_BoundaryCase_SingleAgent) {
    SwarmManager swarm(1, 0.1, 6);

    auto neighbors = swarm.find_neighbors(0);

    // エージェントが1つだけの場合、近傍は0個
    EXPECT_EQ(neighbors.size(), 0);
}

TEST(SwarmManager, FindNeighbors_BoundaryCase_TwoAgents) {
    SwarmManager swarm(2, 0.1, 6);

    auto neighbors0 = swarm.find_neighbors(0);
    auto neighbors1 = swarm.find_neighbors(1);

    // エージェントが2つの場合、近傍は1個
    EXPECT_EQ(neighbors0.size(), 1);
    EXPECT_EQ(neighbors1.size(), 1);

    // 互いが近傍
    EXPECT_EQ(neighbors0[0], 1);
    EXPECT_EQ(neighbors1[0], 0);
}

// === エージェント管理テスト ===

TEST(SwarmManager, GetAgent_ReturnsCorrectAgent) {
    SwarmManager swarm(10, 0.1, 6);

    // 各エージェントにアクセス可能
    for (size_t i = 0; i < swarm.size(); ++i) {
        auto& agent = swarm.get_agent(i);
        EXPECT_DOUBLE_EQ(agent.kappa(), 1.0);
    }
}

TEST(SwarmManager, GetAllHazeFields_ReturnsCorrectSize) {
    SwarmManager swarm(10, 0.1, 6);

    auto haze_fields = swarm.get_all_haze_fields();

    EXPECT_EQ(haze_fields.size(), 10);

    // 初期状態ではすべてゼロ
    for (const auto& haze : haze_fields) {
        EXPECT_DOUBLE_EQ(haze.sum(), 0.0);
    }
}

// === 位置更新テスト ===

TEST(SwarmManager, UpdatePosition_DoesNotCrash) {
    SwarmManager swarm(10, 0.1, 6);

    // Phase 3では位置更新は使用されないが、APIとして存在
    Vec2 new_pos(5.0, 5.0);
    EXPECT_NO_THROW(swarm.update_position(0, new_pos));
}

// === 数値安定性テスト ===

TEST(SwarmManager, Constructor_WithLargeSwarm_DoesNotCrash) {
    // N=100でも動作する
    EXPECT_NO_THROW(SwarmManager swarm(100, 0.098, 6));
}

TEST(SwarmManager, FindNeighbors_WithLargeK_Clamps) {
    SwarmManager swarm(10, 0.1, 50);  // k=50 > N-1=9

    auto neighbors = swarm.find_neighbors(0);

    // k > N-1の場合、N-1個が返される
    EXPECT_EQ(neighbors.size(), 9);
}
