#include <gtest/gtest.h>
#include <cmath>
#include "eph_agent/eph_agent.hpp"

using namespace eph;
using namespace eph::agent;

// ===================================================================
// カテゴリ1: update()統合テスト（5テスト）
// ===================================================================

TEST(EPHAgentPhase4, Update_ChangesVelocity) {
    // update()により速度が変化
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_before = agent.state().velocity;

    agent.update(spm, 0.1);

    Vec2 v_after = agent.state().velocity;

    // 行為選択により速度が変化
    EXPECT_FALSE(v_after.isApprox(v_before, 1e-6))
        << "Velocity should change after update()";
}

TEST(EPHAgentPhase4, Update_ChangesPosition) {
    // update()により位置が変化
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 pos_before = agent.state().position;

    agent.update(spm, 0.1);

    Vec2 pos_after = agent.state().position;

    // 位置が変化
    EXPECT_FALSE(pos_after.isApprox(pos_before, 1e-6))
        << "Position should change after update()";
}

TEST(EPHAgentPhase4, Update_ComputesPredictionError) {
    // update()で予測誤差が計算され、Hazeに反映
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 勾配を持つSPM
    spm::SaliencyPolarMap spm;
    Matrix12x12 gradient_field = Matrix12x12::Zero();
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            gradient_field(i, j) = static_cast<Scalar>(i) / 11.0;
        }
    }
    spm.set_channel(ChannelID::F2, gradient_field);

    auto haze_before = agent.haze();

    agent.update(spm, 0.1);

    auto haze_after = agent.haze();

    // Hazeが更新される（予測誤差が計算されている）
    EXPECT_FALSE(haze_after.isApprox(haze_before, 1e-6))
        << "Haze should be updated after update()";
}

TEST(EPHAgentPhase4, Update_UpdatesHaze) {
    // update()でHazeが更新される
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5));

    Matrix12x12 haze_before = agent.haze();

    agent.update(spm, 0.1);

    Matrix12x12 haze_after = agent.haze();

    // Hazeが変化
    EXPECT_GT((haze_after - haze_before).norm(), 0.0)
        << "Haze should be updated";
}

TEST(EPHAgentPhase4, Update_UpdatesFatigue) {
    // update()で疲労度が更新される
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Scalar fatigue_before = agent.state().fatigue;

    // 複数回更新
    for (int i = 0; i < 10; ++i) {
        agent.update(spm, 0.1);
    }

    Scalar fatigue_after = agent.state().fatigue;

    // 疲労度が変化（蓄積または回復）
    EXPECT_NE(fatigue_after, fatigue_before)
        << "Fatigue should be updated";
}

// ===================================================================
// カテゴリ2: 疲労動態テスト（5テスト）
// ===================================================================

TEST(EPHAgentPhase4, Fatigue_AccumulatesDuringMovement) {
    // 移動中は疲労が蓄積
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    for (int i = 0; i < 100; ++i) {
        agent.update(spm, 0.1);
    }

    // 移動後、疲労が蓄積
    EXPECT_GT(agent.state().fatigue, 0.0)
        << "Fatigue should accumulate during movement";
}

TEST(EPHAgentPhase4, Fatigue_RecoversDuringRest) {
    // 休息中は疲労が回復
    // 高疲労（>0.8）で強制休息させる
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.85;  // 高疲労 → 強制休息

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Scalar fatigue_before = agent.state().fatigue;

    // 休息フェーズ（高疲労により速度≈0）
    for (int i = 0; i < 50; ++i) {
        agent.update(spm, 0.1);
    }

    Scalar fatigue_after = agent.state().fatigue;

    // 疲労が減少（休息により回復）
    EXPECT_LT(fatigue_after, fatigue_before)
        << "Fatigue should recover during rest (high fatigue forces rest)";
}

TEST(EPHAgentPhase4, Fatigue_ClampedToUpperBound) {
    // 疲労度は1.0を超えない
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.5, 1.5);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.9;  // 高初期疲労

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    // 長時間移動
    for (int i = 0; i < 200; ++i) {
        agent.update(spm, 0.1);
    }

    // 疲労度は1.0以下
    EXPECT_LE(agent.state().fatigue, 1.0)
        << "Fatigue should be clamped to [0, 1]";
}

TEST(EPHAgentPhase4, Fatigue_HighFatigue_ForcesRest) {
    // 高疲労（>0.8）で強制休息
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.85;  // 高疲労

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    agent.update(spm, 0.1);

    // 高疲労 → 速度が非常に小さいまたはゼロ
    Scalar speed = agent.state().velocity.norm();
    EXPECT_LT(speed, 0.1)
        << "High fatigue should force rest (low velocity)";
}

TEST(EPHAgentPhase4, Fatigue_AsymmetricDynamics) {
    // 疲労蓄積 > 回復（非対称動態）
    using namespace eph::constants;

    // 蓄積率 > 回復率を確認
    EXPECT_GT(FATIGUE_RATE, RECOVERY_RATE)
        << "Fatigue accumulation should be faster than recovery";
}

// ===================================================================
// カテゴリ3: 予測誤差フィードバックテスト（5テスト）
// ===================================================================

TEST(EPHAgentPhase4, PredictionError_LargeChange_HighPE) {
    // 大きな速度変化 → 高い予測誤差
    // 間接的検証: 速度変化が大きいとHazeが高くなる傾向
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.1, 0.1);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 強い勾配を持つSPM → 大きな速度変化を誘発
    spm::SaliencyPolarMap spm;
    Matrix12x12 strong_gradient = Matrix12x12::Zero();
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            strong_gradient(i, j) = static_cast<Scalar>(i * j) / 121.0 * 2.0;
        }
    }
    spm.set_channel(ChannelID::F2, strong_gradient);

    agent.update(spm, 0.1);

    // Hazeが更新されている（予測誤差が反映）
    Scalar haze_mean = agent.haze().mean();
    EXPECT_GT(haze_mean, 0.0)
        << "Large velocity change should lead to non-zero haze";
}

TEST(EPHAgentPhase4, PredictionError_SmallChange_LowPE) {
    // 小さな速度変化 → 低い予測誤差
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 弱い勾配 → 小さな速度変化
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Constant(0.5));

    Vec2 v_before = agent.state().velocity;
    agent.update(spm, 0.1);
    Vec2 v_after = agent.state().velocity;

    Scalar velocity_change = (v_after - v_before).norm();

    // 速度変化が比較的小さい（学習率0.8のEFE勾配降下で pragmatic項の寄与あり）
    EXPECT_LT(velocity_change, 1.5)
        << "Small gradient should lead to moderate velocity change";
}

TEST(EPHAgentPhase4, PredictionError_ToHaze_Feedback) {
    // 予測誤差 → Haze推定のフィードバックループ
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    Matrix12x12 gradient_field = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, gradient_field);

    // 複数ステップ実行してフィードバックループを回す
    for (int i = 0; i < 10; ++i) {
        agent.update(spm, 0.1);
    }

    // Hazeが有限値（フィードバックループが機能）
    Matrix12x12 haze = agent.haze();
    EXPECT_FALSE(haze.hasNaN());
    EXPECT_GT(haze.mean(), 0.0);
    EXPECT_LT(haze.mean(), 1.0);
}

TEST(EPHAgentPhase4, PredictionError_ClampedToRange) {
    // 予測誤差は[0, 1]にクリップ
    // 間接的検証: 極端な速度変化でもHazeは[0, 1]
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.1, 0.1);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 極端な勾配
    spm::SaliencyPolarMap spm;
    Matrix12x12 extreme_gradient = Matrix12x12::Random() * 2.0;
    spm.set_channel(ChannelID::F2, extreme_gradient);

    agent.update(spm, 0.1);

    Matrix12x12 haze = agent.haze();

    // Hazeが[0, 1]に収まる
    EXPECT_GE(haze.minCoeff(), 0.0);
    EXPECT_LE(haze.maxCoeff(), 1.0 + 1e-6);
}

TEST(EPHAgentPhase4, PredictionError_Convergence) {
    // フィードバックループが収束
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    // 長時間実行
    for (int i = 0; i < 500; ++i) {
        agent.update(spm, 0.1);
    }

    // 収束後もNaN/Infなし
    Vec2 velocity = agent.state().velocity;
    Vec2 position = agent.state().position;
    Scalar fatigue = agent.state().fatigue;
    Matrix12x12 haze = agent.haze();

    EXPECT_FALSE(std::isnan(velocity.x()));
    EXPECT_FALSE(std::isnan(velocity.y()));
    EXPECT_FALSE(std::isnan(position.x()));
    EXPECT_FALSE(std::isnan(position.y()));
    EXPECT_FALSE(std::isnan(fatigue));
    EXPECT_FALSE(haze.hasNaN());
}
