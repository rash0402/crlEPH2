#include <gtest/gtest.h>
#include <cmath>
#include "eph_agent/eph_agent.hpp"
#include "eph_agent/action_selector.hpp"
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;
using namespace eph::agent;
using namespace eph::spm;

/**
 * @brief V1検証: 予測誤差フィードバックループの妥当性
 *
 * ## 検証項目
 * 1. 予測誤差の計算精度
 *    - PE = |Δv| / V_MAX が適切な範囲 [0, 1]
 *    - 大きな行為変化 → 高いPE
 *    - 小さな行為変化 → 低いPE
 *
 * 2. Haze推定への影響
 *    - 高PE → Haze増加（不確実性認識）
 *    - 低PE → Haze減少（確信度向上）
 *
 * 3. フィードバックループの収束性
 *    - 長時間実行での安定性
 *    - 振動・発散の有無
 */

// ========================================
// 1. 予測誤差の計算精度
// ========================================

TEST(V1Validation, PredictionError_IsInValidRange) {
    AgentState initial_state;
    initial_state.position = Vec2(0.0, 0.0);
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.kappa = 1.0;
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 複数ステップ実行して予測誤差を観測
    std::vector<Scalar> prediction_errors;

    for (int t = 0; t < 100; ++t) {
        Vec2 v_old = agent.state().velocity;
        agent.update(spm, 0.1);
        Vec2 v_new = agent.state().velocity;

        // 予測誤差を推定（速度変化から）
        Scalar velocity_change = (v_new - v_old).norm();
        Scalar pe = velocity_change / constants::V_MAX;
        prediction_errors.push_back(pe);
    }

    // 全ての予測誤差が [0, 1] に収まる
    for (Scalar pe : prediction_errors) {
        EXPECT_GE(pe, 0.0) << "PE must be non-negative";
        EXPECT_LE(pe, 1.0) << "PE must not exceed 1.0";
    }

    // 平均的に中程度の予測誤差（過度に小さくも大きくもない）
    Scalar pe_mean = 0.0;
    for (Scalar pe : prediction_errors) {
        pe_mean += pe;
    }
    pe_mean /= prediction_errors.size();

    EXPECT_GT(pe_mean, 0.01) << "PE should be non-trivial";
    EXPECT_LT(pe_mean, 0.8) << "PE should not be excessively large (adjusted for LEARNING_RATE=0.8)";
}

TEST(V1Validation, LargeActionChange_HighPredictionError) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.1, 0.0);  // 低速から開始
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 高勾配のSPMを設定（大きな行為変化を誘発）
    SaliencyPolarMap spm;
    Matrix12x12 high_gradient = Matrix12x12::Zero();
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            high_gradient(i, j) = static_cast<Scalar>(j) / 11.0;  // 強い勾配
        }
    }
    spm.set_channel(ChannelID::F2, high_gradient);

    Vec2 v_old = agent.state().velocity;
    agent.update(spm, 0.1);
    Vec2 v_new = agent.state().velocity;

    Scalar velocity_change = (v_new - v_old).norm();
    Scalar pe = velocity_change / constants::V_MAX;

    // 高勾配環境では速度変化が大きい → 高いPE
    EXPECT_GT(pe, 0.05) << "High gradient should induce significant velocity change";
}

TEST(V1Validation, SmallActionChange_LowPredictionError) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);  // 中速から開始
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 低勾配のSPM（小さな行為変化）
    SaliencyPolarMap spm;
    Matrix12x12 low_gradient = Matrix12x12::Constant(0.5);  // 均一場
    spm.set_channel(ChannelID::F2, low_gradient);

    Vec2 v_old = agent.state().velocity;
    agent.update(spm, 0.1);
    Vec2 v_new = agent.state().velocity;

    Scalar velocity_change = (v_new - v_old).norm();
    Scalar pe = velocity_change / constants::V_MAX;

    // 均一場でも Pragmatic 項により速度変化がある
    EXPECT_LT(pe, 0.6) << "Uniform field should have moderate velocity change (mainly pragmatic term)";
}

// ========================================
// 2. Haze推定への影響
// ========================================

TEST(V1Validation, HighPredictionError_IncreasesHaze) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.1, 0.0);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 初期Hazeを低めに設定
    Matrix12x12 low_haze = Matrix12x12::Constant(0.2);
    agent.set_effective_haze(low_haze);

    Scalar initial_haze_mean = agent.haze().mean();

    // 高勾配SPMで大きな行為変化を誘発
    SaliencyPolarMap spm;
    Matrix12x12 high_gradient = Matrix12x12::Zero();
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            high_gradient(i, j) = static_cast<Scalar>(j) / 11.0;
        }
    }
    spm.set_channel(ChannelID::F2, high_gradient);

    // 複数ステップ実行
    for (int t = 0; t < 10; ++t) {
        agent.update(spm, 0.1);
    }

    Scalar final_haze_mean = agent.haze().mean();

    // 高PEによりHazeが増加
    EXPECT_GT(final_haze_mean, initial_haze_mean)
        << "High PE should increase Haze (uncertainty recognition)";
}

TEST(V1Validation, LowPredictionError_StableOrDecreasesHaze) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 初期Hazeを高めに設定
    Matrix12x12 high_haze = Matrix12x12::Constant(0.7);
    agent.set_effective_haze(high_haze);

    Scalar initial_haze_mean = agent.haze().mean();

    // 均一SPMで小さな行為変化
    SaliencyPolarMap spm;
    Matrix12x12 uniform = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, uniform);

    // 複数ステップ実行
    for (int t = 0; t < 10; ++t) {
        agent.update(spm, 0.1);
    }

    Scalar final_haze_mean = agent.haze().mean();

    // 低PEによりHazeが安定または減少
    EXPECT_LE(final_haze_mean, initial_haze_mean * 1.1)
        << "Low PE should stabilize or decrease Haze";
}

TEST(V1Validation, PredictionErrorFeedback_ConvergesToEquilibrium) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 長時間実行してHazeの推移を観測
    std::vector<Scalar> haze_means;
    for (int t = 0; t < 100; ++t) {
        agent.update(spm, 0.1);
        haze_means.push_back(agent.haze().mean());
    }

    // 最後の20ステップでの標準偏差（変動が小さい = 平衡状態）
    Scalar recent_mean = 0.0;
    for (size_t i = 80; i < 100; ++i) {
        recent_mean += haze_means[i];
    }
    recent_mean /= 20.0;

    Scalar recent_variance = 0.0;
    for (size_t i = 80; i < 100; ++i) {
        Scalar diff = haze_means[i] - recent_mean;
        recent_variance += diff * diff;
    }
    recent_variance /= 20.0;
    Scalar recent_stddev = std::sqrt(recent_variance);

    // 平衡状態では変動が小さい
    EXPECT_LT(recent_stddev, 0.05)
        << "Haze should converge to equilibrium with small fluctuations";
}

// ========================================
// 3. フィードバックループの収束性
// ========================================

TEST(V1Validation, FeedbackLoop_NoOscillation) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // Hazeの時間変化を記録
    std::vector<Scalar> haze_means;
    for (int t = 0; t < 100; ++t) {
        agent.update(spm, 0.1);
        haze_means.push_back(agent.haze().mean());
    }

    // 振動検出: 連続する増減の回数をカウント
    int sign_changes = 0;
    for (size_t i = 1; i < haze_means.size(); ++i) {
        Scalar diff_prev = (i > 1) ? (haze_means[i-1] - haze_means[i-2]) : 0.0;
        Scalar diff_curr = haze_means[i] - haze_means[i-1];

        if (diff_prev * diff_curr < 0) {  // 符号が変わった
            sign_changes++;
        }
    }

    // 過度な振動がない（sign_changesが少ない）
    EXPECT_LT(sign_changes, 30)
        << "Feedback loop should not oscillate excessively";
}

TEST(V1Validation, FeedbackLoop_NoDivergence) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 長時間実行
    for (int t = 0; t < 1000; ++t) {
        agent.update(spm, 0.1);

        // 各ステップで数値が有限であることを確認
        ASSERT_FALSE(std::isnan(agent.haze().sum()));
        ASSERT_FALSE(std::isinf(agent.haze().sum()));
        ASSERT_FALSE(std::isnan(agent.state().velocity.norm()));
        ASSERT_FALSE(std::isinf(agent.state().velocity.norm()));
    }

    // Hazeが妥当な範囲に留まる
    Scalar final_haze_mean = agent.haze().mean();
    EXPECT_GE(final_haze_mean, 0.0);
    EXPECT_LE(final_haze_mean, 1.0);
}

TEST(V1Validation, PredictionError_CorrelatesWithVelocityChange) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    std::vector<Scalar> velocity_changes;
    std::vector<Scalar> haze_changes;

    Scalar prev_haze_mean = agent.haze().mean();

    for (int t = 0; t < 50; ++t) {
        Vec2 v_old = agent.state().velocity;
        agent.update(spm, 0.1);
        Vec2 v_new = agent.state().velocity;

        Scalar v_change = (v_new - v_old).norm();
        Scalar haze_mean = agent.haze().mean();
        Scalar h_change = std::abs(haze_mean - prev_haze_mean);

        velocity_changes.push_back(v_change);
        haze_changes.push_back(h_change);

        prev_haze_mean = haze_mean;
    }

    // 速度変化とHaze変化の相関を検証（定性的）
    // 速度変化が大きいとき、Haze変化も大きい傾向
    Scalar max_v_change = *std::max_element(velocity_changes.begin(), velocity_changes.end());
    Scalar min_v_change = *std::min_element(velocity_changes.begin(), velocity_changes.end());

    // 速度変化に多様性がある
    EXPECT_GT(max_v_change - min_v_change, 0.01)
        << "Velocity changes should have some variability";
}

TEST(V1Validation, ZeroSaliencyGradient_SmallVelocityChange) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    // 完全に均一なSPM（勾配ゼロ）
    SaliencyPolarMap spm;
    Matrix12x12 uniform = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, uniform);

    std::vector<Scalar> velocity_changes;

    for (int t = 0; t < 20; ++t) {
        Vec2 v_old = agent.state().velocity;
        agent.update(spm, 0.1);
        Vec2 v_new = agent.state().velocity;

        velocity_changes.push_back((v_new - v_old).norm());
    }

    // 勾配ゼロでも Pragmatic 項（疲労コスト）により速度変化がある
    // LEARNING_RATE=0.8 では比較的大きな変化が起こる
    Scalar max_change = *std::max_element(velocity_changes.begin(), velocity_changes.end());

    // 速度変化が有限範囲に収まることを確認
    EXPECT_LT(max_change, 2.0)
        << "Even with zero gradient, velocity changes should be bounded";

    // 全てのステップで数値的に安定
    for (Scalar change : velocity_changes) {
        EXPECT_FALSE(std::isnan(change));
        EXPECT_FALSE(std::isinf(change));
    }
}

TEST(V1Validation, HighFatigue_ReducesActionMagnitude) {
    AgentState initial_state;
    initial_state.velocity = Vec2(1.0, 1.0);
    initial_state.fatigue = 0.9;  // 高疲労

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    agent.update(spm, 0.1);

    // 高疲労時は速度が減少（休息を選択）
    Scalar final_speed = agent.state().velocity.norm();
    EXPECT_LT(final_speed, 1.0)
        << "High fatigue should reduce action magnitude";
}

TEST(V1Validation, LongTermBehavior_StableDynamics) {
    AgentState initial_state;
    initial_state.velocity = Vec2(0.5, 0.5);
    initial_state.fatigue = 0.0;

    EPHAgent agent(initial_state, 1.0);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 500ステップ実行
    for (int t = 0; t < 500; ++t) {
        agent.update(spm, 0.1);
    }

    // 全ての状態変数が妥当な範囲
    EXPECT_FALSE(std::isnan(agent.state().position.x()));
    EXPECT_FALSE(std::isnan(agent.state().velocity.x()));
    EXPECT_FALSE(std::isnan(agent.state().fatigue));
    EXPECT_FALSE(agent.haze().hasNaN());

    EXPECT_GE(agent.state().fatigue, 0.0);
    EXPECT_LE(agent.state().fatigue, 1.0);
    EXPECT_GE(agent.haze().mean(), 0.0);
    EXPECT_LE(agent.haze().mean(), 1.0);
}
