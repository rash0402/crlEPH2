#include <gtest/gtest.h>
#include <cmath>
#include "eph_agent/eph_agent.hpp"

using namespace eph;
using namespace eph::agent;

// === コンストラクタテスト ===

TEST(EPHAgent, Constructor_InitializesCorrectly) {
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.0, 0.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    EXPECT_DOUBLE_EQ(agent.kappa(), 1.0);
    EXPECT_DOUBLE_EQ(agent.state().position.x(), 0.0);
    EXPECT_DOUBLE_EQ(agent.state().position.y(), 0.0);
    EXPECT_DOUBLE_EQ(agent.state().velocity.x(), 0.0);
    EXPECT_DOUBLE_EQ(agent.state().velocity.y(), 0.0);
    EXPECT_DOUBLE_EQ(agent.state().kappa, 1.0);
    EXPECT_DOUBLE_EQ(agent.state().fatigue, 0.0);
}

TEST(EPHAgent, Constructor_SetsKappaCorrectly) {
    AgentState initial_state;
    initial_state.position = Vec2(1.0, 2.0);
    initial_state.velocity = Vec2(0.5, -0.3);
    initial_state.kappa = 0.8;  // この値は上書きされる
    initial_state.fatigue = 0.2;

    EPHAgent agent(initial_state, 1.5);  // kappa=1.5で初期化

    // コンストラクタでkappaが上書きされる
    EXPECT_DOUBLE_EQ(agent.kappa(), 1.5);
    EXPECT_DOUBLE_EQ(agent.state().kappa, 1.5);

    // その他の状態は保持される
    EXPECT_DOUBLE_EQ(agent.state().position.x(), 1.0);
    EXPECT_DOUBLE_EQ(agent.state().position.y(), 2.0);
    EXPECT_DOUBLE_EQ(agent.state().velocity.x(), 0.5);
    EXPECT_DOUBLE_EQ(agent.state().velocity.y(), -0.3);
    EXPECT_DOUBLE_EQ(agent.state().fatigue, 0.2);
}

// === Haze推定テスト ===

TEST(EPHAgent, EstimateHaze_ReturnsValidRange) {
    AgentState initial_state;
    initial_state.kappa = 1.0;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    // ランダムSPMデータ
    auto R1 = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    auto F4 = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    auto F5 = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::R1, R1);
    spm.set_channel(ChannelID::F4, F4);
    spm.set_channel(ChannelID::F5, F5);

    auto haze = agent.estimate_haze(spm, 0.5);

    // Haze ∈ [0, 1]
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_GE(haze(a, b), 0.0);
            EXPECT_LE(haze(a, b), 1.0);
        }
    }
}

TEST(EPHAgent, EstimateHaze_UpdatesInternalState) {
    AgentState initial_state;
    initial_state.kappa = 1.0;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;

    // 初期状態: Hazeはゼロ
    auto haze_before = agent.haze();
    EXPECT_DOUBLE_EQ(haze_before.sum(), 0.0);

    // Haze推定実行
    agent.estimate_haze(spm, 0.5);

    // 内部状態が更新される
    auto haze_after = agent.haze();
    EXPECT_GT(haze_after.sum(), 0.0);  // ゼロではなくなる
}

TEST(EPHAgent, EstimateHaze_ConsecutiveCalls_UsesEMA) {
    AgentState initial_state;
    initial_state.kappa = 1.0;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;

    // 複数回呼び出してEMAが動作することを確認
    auto haze1 = agent.estimate_haze(spm, 0.3);
    auto haze2 = agent.estimate_haze(spm, 0.3);
    auto haze3 = agent.estimate_haze(spm, 0.3);

    // EMAフィルタが動作して、徐々に収束する
    // （厳密な比較は難しいが、すべて妥当な範囲にあることを確認）
    EXPECT_GT(haze1.mean(), 0.0);
    EXPECT_LT(haze1.mean(), 1.0);
    EXPECT_GT(haze2.mean(), 0.0);
    EXPECT_LT(haze2.mean(), 1.0);
    EXPECT_GT(haze3.mean(), 0.0);
    EXPECT_LT(haze3.mean(), 1.0);
}

// === Hazeリセットテスト ===

TEST(EPHAgent, ResetHazeEstimator_ClearsState) {
    AgentState initial_state;
    initial_state.kappa = 1.0;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;

    // Haze推定を実行
    agent.estimate_haze(spm, 0.8);
    auto haze_before_reset = agent.haze();
    EXPECT_GT(haze_before_reset.sum(), 0.0);

    // リセット
    agent.reset_haze_estimator();

    // Hazeがゼロにクリアされる
    auto haze_after_reset = agent.haze();
    EXPECT_DOUBLE_EQ(haze_after_reset.sum(), 0.0);
}

// === 状態アクセステスト ===

TEST(EPHAgent, StateAccess_ReturnsCorrectValues) {
    AgentState initial_state;
    initial_state.position = Vec2(3.0, 4.0);
    initial_state.velocity = Vec2(1.0, -1.0);
    initial_state.kappa = 0.8;
    initial_state.fatigue = 0.3;

    EPHAgent agent(initial_state, 1.2);

    const auto& state = agent.state();
    EXPECT_DOUBLE_EQ(state.position.x(), 3.0);
    EXPECT_DOUBLE_EQ(state.position.y(), 4.0);
    EXPECT_DOUBLE_EQ(state.velocity.x(), 1.0);
    EXPECT_DOUBLE_EQ(state.velocity.y(), -1.0);
    EXPECT_DOUBLE_EQ(state.kappa, 1.2);  // コンストラクタで上書き
    EXPECT_DOUBLE_EQ(state.fatigue, 0.3);
}

TEST(EPHAgent, HazeAccess_ReturnsCurrentHaze) {
    AgentState initial_state;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;

    // 初期状態
    const auto& haze_initial = agent.haze();
    EXPECT_DOUBLE_EQ(haze_initial.sum(), 0.0);

    // Haze推定後
    agent.estimate_haze(spm, 0.5);
    const auto& haze_updated = agent.haze();
    EXPECT_GT(haze_updated.sum(), 0.0);
}

// === 更新テスト（Phase 2ではプレースホルダー） ===

TEST(EPHAgent, Update_DoesNotCrash) {
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.0, 0.0);
    initial_state.kappa = 1.0;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;

    // Phase 2では何もしないが、クラッシュしないことを確認
    EXPECT_NO_THROW(agent.update(spm, 0.01));
}

// === 数値安定性テスト ===

TEST(EPHAgent, EstimateHaze_ExtremeKappa_StillStable) {
    AgentState initial_state;

    // 極端なκ値
    EPHAgent leader(initial_state, 0.3);   // 最小値
    EPHAgent follower(initial_state, 1.5); // 最大値

    spm::SaliencyPolarMap spm;
    auto R1 = Matrix12x12::Ones();
    spm.set_channel(ChannelID::R1, R1);

    auto haze_leader = leader.estimate_haze(spm, 0.5);
    auto haze_follower = follower.estimate_haze(spm, 0.5);

    // 両方とも妥当な範囲
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_GE(haze_leader(a, b), 0.0);
            EXPECT_LE(haze_leader(a, b), 1.0);
            EXPECT_GE(haze_follower(a, b), 0.0);
            EXPECT_LE(haze_follower(a, b), 1.0);
        }
    }
}

TEST(EPHAgent, EstimateHaze_NeverProducesNaN) {
    AgentState initial_state;
    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;

    // 極端な入力
    spm.set_channel(ChannelID::R1, Matrix12x12::Ones());
    spm.set_channel(ChannelID::F4, Matrix12x12::Zero());
    spm.set_channel(ChannelID::F5, Matrix12x12::Ones());

    auto haze = agent.estimate_haze(spm, 1.0);

    // NaN/Infチェック
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_FALSE(std::isnan(haze(a, b)));
            EXPECT_FALSE(std::isinf(haze(a, b)));
        }
    }
}
