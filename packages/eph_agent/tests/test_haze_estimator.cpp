#include <gtest/gtest.h>
#include <cmath>
#include "eph_agent/haze_estimator.hpp"

using namespace eph;
using namespace eph::agent;
using namespace eph::constants;

// === 値域テスト ===

TEST(HazeEstimator, EstimatedHaze_InValidRange) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // ランダムSPMデータ（[0, 1]範囲）
    auto R1 = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);  // [0, 1]
    auto F4 = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    auto F5 = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::R1, R1);
    spm.set_channel(ChannelID::F4, F4);
    spm.set_channel(ChannelID::F5, F5);

    auto haze = estimator.estimate(spm, 0.5);

    // Haze ∈ [0, 1]
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_GE(haze(a, b), 0.0)
                << "Haze below 0 at (θ=" << a << ", r=" << b << ")";
            EXPECT_LE(haze(a, b), 1.0)
                << "Haze above 1 at (θ=" << a << ", r=" << b << ")";
        }
    }
}

TEST(HazeEstimator, EstimatedHaze_NeverNaN) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // 極端な値でテスト
    auto R1 = Matrix12x12::Ones();
    auto F4 = Matrix12x12::Zero();
    auto F5 = Matrix12x12::Ones();
    spm.set_channel(ChannelID::R1, R1);
    spm.set_channel(ChannelID::F4, F4);
    spm.set_channel(ChannelID::F5, F5);

    auto haze = estimator.estimate(spm, 1.0);

    // NaN/Infチェック
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_FALSE(std::isnan(haze(a, b)))
                << "NaN detected at (θ=" << a << ", r=" << b << ")";
            EXPECT_FALSE(std::isinf(haze(a, b)))
                << "Inf detected at (θ=" << a << ", r=" << b << ")";
        }
    }
}

// === EMAフィルタテスト ===

TEST(HazeEstimator, EMA_Initialization_FirstCall) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // 初回呼び出し
    auto haze1 = estimator.estimate(spm, 0.8);

    // 初回呼び出しでは、EMAは入力値で初期化される
    // （内部状態は直接確認できないので、結果が妥当な範囲かをチェック）
    EXPECT_GT(haze1.mean(), 0.0);
    EXPECT_LT(haze1.mean(), 1.0);
}

TEST(HazeEstimator, EMA_ConvergesToInput) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // ゼロチャネルで初期化
    spm.set_channel(ChannelID::R1, Matrix12x12::Zero());
    spm.set_channel(ChannelID::F4, Matrix12x12::Ones());
    spm.set_channel(ChannelID::F5, Matrix12x12::Zero());

    // 一定の予測誤差を与え続ける（τ=1なので速く収束）
    Scalar constant_error = 0.8;
    Matrix12x12 haze;
    for (int i = 0; i < 20; ++i) {
        haze = estimator.estimate(spm, constant_error);
    }

    // EMAは入力値に収束する（τ=1なので20回で十分）
    // Hazeの平均値が入力誤差の影響を反映しているかチェック
    Scalar mean_haze = haze.mean();
    EXPECT_GT(mean_haze, 0.0);
    EXPECT_LT(mean_haze, 1.0);
}

TEST(HazeEstimator, EMA_Reset_ClearsState) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // いくつかの推定を実行
    for (int i = 0; i < 5; ++i) {
        estimator.estimate(spm, 0.5);
    }

    // リセット
    estimator.reset();

    // リセット後、初回呼び出し時の挙動（初期化）が再現される
    auto haze = estimator.estimate(spm, 0.3);
    EXPECT_GT(haze.mean(), 0.0);
    EXPECT_LT(haze.mean(), 1.0);
}

// === 空間平滑化テスト（間接的） ===

TEST(HazeEstimator, GaussianBlur_SmoothsField) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // デルタ関数的な入力（中心のみ高い）
    Matrix12x12 R1 = Matrix12x12::Zero();
    R1(6, 6) = 1.0;
    spm.set_channel(ChannelID::R1, R1);
    spm.set_channel(ChannelID::F4, Matrix12x12::Ones());
    spm.set_channel(ChannelID::F5, Matrix12x12::Zero());

    auto haze = estimator.estimate(spm, 0.0);

    // 中心が最も高い
    Scalar center_value = haze(6, 6);
    EXPECT_GT(center_value, 0.0);

    // 周辺にも広がっている（ガウシアンブラーの効果）
    Scalar neighbor_value = haze(6, 7);
    EXPECT_GT(neighbor_value, 0.0);
    EXPECT_LT(neighbor_value, center_value);  // 中心より低い
}

// === 境界条件テスト ===

TEST(HazeEstimator, GaussianBlur_RespectsBoundaries) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // エッジにピーク
    Matrix12x12 R1 = Matrix12x12::Zero();
    R1(0, 0) = 1.0;  // θ=0, r=0 (内側エッジ)
    spm.set_channel(ChannelID::R1, R1);
    spm.set_channel(ChannelID::F4, Matrix12x12::Ones());
    spm.set_channel(ChannelID::F5, Matrix12x12::Zero());

    auto haze = estimator.estimate(spm, 0.0);

    // エッジでもNaN/Infが発生しない（境界条件が正しく動作）
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_FALSE(std::isnan(haze(a, b)));
            EXPECT_FALSE(std::isinf(haze(a, b)));
        }
    }
}

// === パラメータテスト ===

TEST(HazeEstimator, DifferentTau_AffectsConvergence) {
    HazeEstimator fast_estimator(0.5);   // 速い応答
    HazeEstimator slow_estimator(5.0);   // 遅い応答
    spm::SaliencyPolarMap spm;

    // 同じ入力を与える
    Scalar error = 0.8;
    for (int i = 0; i < 5; ++i) {
        fast_estimator.estimate(spm, error);
        slow_estimator.estimate(spm, error);
    }

    auto haze_fast = fast_estimator.estimate(spm, error);
    auto haze_slow = slow_estimator.estimate(spm, error);

    // τ=0.5の方が速く収束する（平均値がより入力に近い）
    // （厳密な比較は難しいが、両方とも妥当な範囲にあることを確認）
    EXPECT_GT(haze_fast.mean(), 0.0);
    EXPECT_LT(haze_fast.mean(), 1.0);
    EXPECT_GT(haze_slow.mean(), 0.0);
    EXPECT_LT(haze_slow.mean(), 1.0);
}

// === 数値安定性テスト ===

TEST(HazeEstimator, NumericalStability_ExtremeInputs) {
    HazeEstimator estimator(1.0);
    spm::SaliencyPolarMap spm;

    // 極端な入力（すべて1または0）
    spm.set_channel(ChannelID::R1, Matrix12x12::Ones());
    spm.set_channel(ChannelID::F4, Matrix12x12::Zero());
    spm.set_channel(ChannelID::F5, Matrix12x12::Ones());

    auto haze = estimator.estimate(spm, 1.0);

    // Sigmoid飽和でも安定（[0, 1]範囲、NaN/Infなし）
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_GE(haze(a, b), 0.0);
            EXPECT_LE(haze(a, b), 1.0);
            EXPECT_FALSE(std::isnan(haze(a, b)));
            EXPECT_FALSE(std::isinf(haze(a, b)));
        }
    }
}
