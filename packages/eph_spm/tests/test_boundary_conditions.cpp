#include <gtest/gtest.h>
#include <cmath>
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;
using namespace eph::spm;
using namespace eph::constants;

// === θ方向: 周期境界検証 ===

TEST(BoundaryConditions, GradientTheta_PeriodicWrap_WrapIndexWorks) {
    SaliencyPolarMap spm;

    // 定数フィールド（勾配=0を期待）
    Matrix12x12 test_channel = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::M0, test_channel);

    auto grad = spm.gradient_theta(ChannelID::M0);

    // 定数フィールドなので全ての位置で勾配=0（wrap_indexが正しく動作していることを確認）
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_NEAR(grad(a, b), 0.0, 1e-10)
                << "Non-zero gradient for constant field at (θ=" << a << ", r=" << b << ")";
        }
    }
}

TEST(BoundaryConditions, GradientTheta_SinWave_MatchesTheory) {
    SaliencyPolarMap spm;

    // sin波（周期12）を設定: f(a) = sin(2π * a / 12)
    // 勾配: df/dθ = cos(2π * a / 12) * (2π / 12) / Δθ
    // ただしΔθ = 2π/12なので、df/da = cos(2π * a / 12)
    Matrix12x12 test_channel;
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            test_channel(a, b) = std::sin(2.0 * PI * a / 12.0);
        }
    }
    spm.set_channel(ChannelID::M0, test_channel);

    auto grad = spm.gradient_theta(ChannelID::M0);

    // 理論値との比較
    // gradient_thetaは (f(a+1) - f(a-1)) / (2 * Δθ) を計算
    // f(a) = sin(2π*a/12) なので、f(a+1) - f(a-1) = sin(2π*(a+1)/12) - sin(2π*(a-1)/12)
    // 中心差分の近似値と比較
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            // 数値微分の理論値（中心差分）
            double f_plus = std::sin(2.0 * PI * (a + 1) / 12.0);
            double f_minus = std::sin(2.0 * PI * (a - 1) / 12.0);
            double expected = (f_plus - f_minus) / (2.0 * DELTA_THETA);

            EXPECT_NEAR(grad(a, b), expected, 1e-10)
                << "Gradient mismatch at (θ=" << a << ", r=" << b << ")";
        }
    }
}

// === r方向: Neumann境界検証 ===

TEST(BoundaryConditions, GradientR_NeumannZero_AtEdges) {
    SaliencyPolarMap spm;

    // r方向に線形増加
    Matrix12x12 test_channel;
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            test_channel(a, b) = static_cast<double>(b) / 11.0;
        }
    }
    spm.set_channel(ChannelID::M0, test_channel);

    auto grad = spm.gradient_r(ChannelID::M0);

    // 境界（b=0, b=11）でゼロ勾配
    for (int a = 0; a < 12; ++a) {
        EXPECT_DOUBLE_EQ(grad(a, 0), 0.0)
            << "Neumann boundary violated at inner edge (θ=" << a << ")";
        EXPECT_DOUBLE_EQ(grad(a, 11), 0.0)
            << "Neumann boundary violated at outer edge (θ=" << a << ")";
    }
}

TEST(BoundaryConditions, GradientR_LinearField_ConstantGradient) {
    SaliencyPolarMap spm;

    // r方向に線形増加: f(b) = b/11
    // 勾配: df/db = 1/11
    // 中心差分: (f(b+1) - f(b-1))/2 = ((b+1)/11 - (b-1)/11)/2 = (2/11)/2 = 1/11
    Matrix12x12 test_channel;
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            test_channel(a, b) = static_cast<double>(b) / 11.0;
        }
    }
    spm.set_channel(ChannelID::M0, test_channel);

    auto grad = spm.gradient_r(ChannelID::M0);

    // 内部（b=1..10）で一定勾配: 1/11
    for (int a = 0; a < 12; ++a) {
        for (int b = 1; b <= 10; ++b) {
            double expected = 1.0 / 11.0;  // 修正: /2.0 を削除
            EXPECT_NEAR(grad(a, b), expected, 1e-6)
                << "Gradient mismatch at (θ=" << a << ", r=" << b << ")";
        }
    }
}

// === 勾配の大きさ検証 ===

TEST(BoundaryConditions, GradientMagnitude_ConstantField_ZeroGradient) {
    SaliencyPolarMap spm;

    // 定数フィールド
    Matrix12x12 test_channel = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F0, test_channel);

    auto grad_mag = spm.gradient_magnitude(ChannelID::F0);

    // 定数フィールド→勾配ゼロ
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_NEAR(grad_mag(a, b), 0.0, 1e-10)
                << "Non-zero gradient for constant field at (" << a << ", " << b << ")";
        }
    }
}

TEST(BoundaryConditions, GradientMagnitude_PositiveValues) {
    SaliencyPolarMap spm;

    // ランダムフィールド
    Matrix12x12 test_channel = Matrix12x12::Random();
    spm.set_channel(ChannelID::F0, test_channel);

    auto grad_mag = spm.gradient_magnitude(ChannelID::F0);

    // 勾配の大きさは常に非負
    for (int a = 0; a < 12; ++a) {
        for (int b = 0; b < 12; ++b) {
            EXPECT_GE(grad_mag(a, b), 0.0)
                << "Negative gradient magnitude at (" << a << ", " << b << ")";
        }
    }
}
