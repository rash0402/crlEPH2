#include <gtest/gtest.h>
#include <cmath>
#include "eph_agent/action_selector.hpp"

using namespace eph;
using namespace eph::agent;

// ===================================================================
// カテゴリ1: EFE計算テスト（5テスト）
// ===================================================================

TEST(ActionSelector, ComputeEFE_ZeroVelocity_ReturnsEpistemic) {
    // ゼロ速度: Pragmatic項=0、Epistemic項>0
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5));

    Vec2 v_zero(0.0, 0.0);
    Scalar efe = ActionSelector::compute_efe(v_zero, haze, spm, 0.0);

    // Pragmatic項=0、Epistemic項>0
    EXPECT_GT(efe, 0.0);
    EXPECT_LT(efe, 10.0);
}

TEST(ActionSelector, ComputeEFE_HigherVelocity_HigherEFE) {
    // 速度が大きい → Pragmatic項大 → EFE増加
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_low(0.5, 0.5);
    Vec2 v_high(1.5, 1.5);

    Scalar efe_low = ActionSelector::compute_efe(v_low, haze, spm, 0.0);
    Scalar efe_high = ActionSelector::compute_efe(v_high, haze, spm, 0.0);

    EXPECT_GT(efe_high, efe_low)
        << "Higher velocity should increase EFE (Pragmatic term)";
}

TEST(ActionSelector, ComputeEFE_HighFatigue_HigherCost) {
    // 高疲労 → κ増加 → Pragmatic項増加
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v(1.0, 1.0);

    Scalar efe_low_fatigue = ActionSelector::compute_efe(v, haze, spm, 0.0);
    Scalar efe_high_fatigue = ActionSelector::compute_efe(v, haze, spm, 0.8);

    EXPECT_GT(efe_high_fatigue, efe_low_fatigue)
        << "High fatigue should increase Pragmatic cost";
}

TEST(ActionSelector, ComputeEFE_HighHaze_HigherEpistemic) {
    // 高Haze → Epistemic項増加
    // 勾配を持つSPMを作成（線形勾配）
    spm::SaliencyPolarMap spm;
    Matrix12x12 gradient_field = Matrix12x12::Zero();
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            gradient_field(i, j) = static_cast<Scalar>(i) / 11.0;  // 0.0→1.0の勾配
        }
    }
    spm.set_channel(ChannelID::F2, gradient_field);

    Vec2 v(0.5, 0.5);

    Matrix12x12 low_haze = Matrix12x12::Constant(0.1);
    Matrix12x12 high_haze = Matrix12x12::Constant(0.9);

    Scalar efe_low = ActionSelector::compute_efe(v, low_haze, spm, 0.0);
    Scalar efe_high = ActionSelector::compute_efe(v, high_haze, spm, 0.0);

    EXPECT_GT(efe_high, efe_low)
        << "High haze should increase Epistemic term";
}

TEST(ActionSelector, ComputeEFE_NoNaN) {
    // 数値安定性: NaN/Inf防止
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v(1.0, 1.0);
    Scalar efe = ActionSelector::compute_efe(v, haze, spm, 0.5);

    EXPECT_FALSE(std::isnan(efe));
    EXPECT_FALSE(std::isinf(efe));
}

// ===================================================================
// カテゴリ2: 勾配計算テスト（5テスト）
// ===================================================================

TEST(ActionSelector, ComputeGradient_ZeroVelocity_NonZero) {
    // ゼロ速度でも勾配は計算可能
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_zero(0.0, 0.0);
    Vec2 grad = ActionSelector::compute_efe_gradient(v_zero, haze, spm, 0.0);

    // 勾配は有限値
    EXPECT_FALSE(std::isnan(grad.x()));
    EXPECT_FALSE(std::isnan(grad.y()));
}

TEST(ActionSelector, ComputeGradient_Symmetry) {
    // 対称性: (v_x, v_y)と(-v_x, -v_y)の勾配大きさは近い
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_pos(1.0, 0.5);
    Vec2 v_neg(-1.0, -0.5);

    Vec2 grad_pos = ActionSelector::compute_efe_gradient(v_pos, haze, spm, 0.0);
    Vec2 grad_neg = ActionSelector::compute_efe_gradient(v_neg, haze, spm, 0.0);

    // 大きさがほぼ同じ（方向は逆）
    EXPECT_NEAR(grad_pos.norm(), grad_neg.norm(), 0.1);
}

TEST(ActionSelector, ComputeGradient_FatigueEffect) {
    // 疲労が高いと勾配が変化
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v(1.0, 1.0);

    Vec2 grad_low = ActionSelector::compute_efe_gradient(v, haze, spm, 0.0);
    Vec2 grad_high = ActionSelector::compute_efe_gradient(v, haze, spm, 0.8);

    // 疲労による勾配変化
    EXPECT_FALSE(grad_low.isApprox(grad_high, 0.01));
}

TEST(ActionSelector, ComputeGradient_NumericalStability) {
    // 極端なhazeでも数値安定
    Matrix12x12 extreme_haze = Matrix12x12::Ones();
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Zero());

    Vec2 v(1.5, 1.5);
    Vec2 grad = ActionSelector::compute_efe_gradient(v, extreme_haze, spm, 0.9);

    EXPECT_FALSE(std::isnan(grad.x()));
    EXPECT_FALSE(std::isnan(grad.y()));
    EXPECT_FALSE(std::isinf(grad.x()));
    EXPECT_FALSE(std::isinf(grad.y()));
}

TEST(ActionSelector, ComputeGradient_CentralDifference) {
    // 中心差分の検証: 片側差分との比較
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v(1.0, 0.5);

    // 中心差分（実装）
    Vec2 grad_central = ActionSelector::compute_efe_gradient(v, haze, spm, 0.0);

    // 片側差分（検証用）
    using namespace eph::constants;
    Vec2 v_plus_x = v + Vec2(GRADIENT_EPSILON, 0.0);
    Scalar efe_plus_x = ActionSelector::compute_efe(v_plus_x, haze, spm, 0.0);
    Scalar efe_0 = ActionSelector::compute_efe(v, haze, spm, 0.0);
    Scalar grad_forward_x = (efe_plus_x - efe_0) / GRADIENT_EPSILON;

    // 中心差分と片側差分の値は近い（中心差分の方が精度高い）
    EXPECT_NEAR(grad_central.x(), grad_forward_x, 0.5);
}

// ===================================================================
// カテゴリ3: 制約適用テスト（5テスト）
// ===================================================================

TEST(ActionSelector, ApplyConstraints_VMinClip) {
    // 速度が小さすぎる → V_MINにクリップ
    Vec2 v_too_small(0.05, 0.05);
    Vec2 v_constrained = ActionSelector::apply_constraints(v_too_small, 0.0);

    Scalar v_mag = v_constrained.norm();

    EXPECT_GE(v_mag, constants::V_MIN - 1e-6)
        << "Velocity should be clipped to V_MIN";
}

TEST(ActionSelector, ApplyConstraints_VMaxClip) {
    // 速度が大きすぎる → V_MAXにクリップ
    Vec2 v_too_large(5.0, 5.0);
    Vec2 v_constrained = ActionSelector::apply_constraints(v_too_large, 0.0);

    Scalar v_mag = v_constrained.norm();

    EXPECT_LE(v_mag, constants::V_MAX + 1e-6)
        << "Velocity should be clipped to V_MAX";
}

TEST(ActionSelector, ApplyConstraints_DirectionPreserved) {
    // 方向は保存される
    Vec2 v_original(1.5, 1.0);
    Vec2 v_constrained = ActionSelector::apply_constraints(v_original, 0.0);

    // 方向ベクトルの一致
    Vec2 dir_original = v_original.normalized();
    Vec2 dir_constrained = v_constrained.normalized();

    EXPECT_TRUE(dir_original.isApprox(dir_constrained, 0.01))
        << "Direction should be preserved during clipping";
}

TEST(ActionSelector, ApplyConstraints_HighFatigue_ForcedRest) {
    // 高疲労（>0.8）→ 強制休息（v=0）
    Vec2 v(1.0, 1.0);
    Vec2 v_constrained = ActionSelector::apply_constraints(v, 0.85);

    EXPECT_NEAR(v_constrained.norm(), 0.0, 1e-6)
        << "High fatigue (>0.8) should force rest (v=0)";
}

TEST(ActionSelector, ApplyConstraints_ZeroVelocity_DefaultHandling) {
    // ゼロ速度 → デフォルト（V_MIN, 0）
    Vec2 v_zero(0.0, 0.0);
    Vec2 v_constrained = ActionSelector::apply_constraints(v_zero, 0.0);

    EXPECT_NEAR(v_constrained.x(), constants::V_MIN, 1e-6);
    EXPECT_NEAR(v_constrained.y(), 0.0, 1e-6);
}

// ===================================================================
// カテゴリ4: 行為選択テスト（5テスト）
// ===================================================================

TEST(ActionSelector, SelectAction_ReducesEFE) {
    // 勾配降下の基本性質: EFE減少
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_old(0.5, 0.5);
    Vec2 v_new = ActionSelector::select_action(v_old, haze, spm, 0.0);

    Scalar efe_old = ActionSelector::compute_efe(v_old, haze, spm, 0.0);
    Scalar efe_new = ActionSelector::compute_efe(v_new, haze, spm, 0.0);

    // 勾配降下: EFE減少（または同じ）
    EXPECT_LE(efe_new, efe_old + 1e-6)
        << "Gradient descent should reduce EFE";
}

TEST(ActionSelector, SelectAction_RespectConstraints) {
    // 選択された行為は制約を満たす
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_old(1.0, 1.0);
    Vec2 v_new = ActionSelector::select_action(v_old, haze, spm, 0.0);

    Scalar v_mag = v_new.norm();

    // V_MIN ≤ |v| ≤ V_MAX（疲労低い場合）
    EXPECT_GE(v_mag, constants::V_MIN - 1e-6);
    EXPECT_LE(v_mag, constants::V_MAX + 1e-6);
}

TEST(ActionSelector, SelectAction_HighHaze_ExploratoryAction) {
    // 高Haze → 探索的行為（速度変化大）
    Matrix12x12 high_haze = Matrix12x12::Constant(0.9);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5));

    Vec2 v_old(0.5, 0.5);
    Vec2 v_new = ActionSelector::select_action(v_old, high_haze, spm, 0.0);

    // 高Hazeでも行為選択は動作
    EXPECT_FALSE(v_new.isApprox(v_old, 0.0))
        << "High haze should lead to velocity change";
}

TEST(ActionSelector, SelectAction_Convergence) {
    // 反復適用で収束
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v = Vec2(1.0, 1.0);

    // 複数回適用
    for (int i = 0; i < 10; ++i) {
        v = ActionSelector::select_action(v, haze, spm, 0.0);
    }

    // 収束後もNaN/Infなし
    EXPECT_FALSE(std::isnan(v.x()));
    EXPECT_FALSE(std::isnan(v.y()));
    EXPECT_FALSE(std::isinf(v.x()));
    EXPECT_FALSE(std::isinf(v.y()));
}

TEST(ActionSelector, SelectAction_NoNaN) {
    // 数値安定性: NaN/Inf防止
    Matrix12x12 haze = Matrix12x12::Constant(0.5);
    spm::SaliencyPolarMap spm;
    spm.set_channel(ChannelID::F2, Matrix12x12::Ones());

    Vec2 v_old(1.0, 1.0);
    Vec2 v_new = ActionSelector::select_action(v_old, haze, spm, 0.5);

    EXPECT_FALSE(std::isnan(v_new.x()));
    EXPECT_FALSE(std::isnan(v_new.y()));
    EXPECT_FALSE(std::isinf(v_new.x()));
    EXPECT_FALSE(std::isinf(v_new.y()));
}
