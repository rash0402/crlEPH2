#ifndef EPH_AGENT_ACTION_SELECTOR_HPP
#define EPH_AGENT_ACTION_SELECTOR_HPP

#include <Eigen/Core>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"
#include "eph_core/math_utils.hpp"
#include "eph_spm/saliency_polar_map.hpp"

namespace eph::agent {

/**
 * @brief 行為選択クラス（Phase 4: Expected Free Energy勾配降下）
 *
 * ## Expected Free Energy (EFE)
 * G(v) = ⟨h⟩ · ⟨|∇SPM|⟩ + κ(fatigue) · |v|
 *        ↑ Epistemic      ↑ Pragmatic
 *
 * - **Epistemic項**: Haze × 環境勾配（不確実性駆動探索）
 * - **Pragmatic項**: 疲労 × 速度（エネルギーコスト）
 *
 * ## 勾配降下
 * v_new = v_old - η · ∇_v G(v)
 *
 * - η = LEARNING_RATE
 * - ∇_v G: 中心差分による数値微分
 * - 制約: |v| ∈ [V_MIN, V_MAX]
 */
class ActionSelector {
public:
    using Scalar = eph::Scalar;
    using Vec2 = eph::Vec2;
    using Matrix12x12 = eph::Matrix12x12;

    /**
     * @brief 行為選択（EFE勾配降下）
     *
     * @param current_velocity 現在の速度 [m/s]
     * @param haze 現在のHazeフィールド [0, 1]
     * @param spm Saliency Polar Map
     * @param fatigue 疲労度 [0, 1]
     * @return 新しい速度 [m/s]（[V_MIN, V_MAX]にクリップ済み）
     */
    static auto select_action(
        const Vec2& current_velocity,
        const Matrix12x12& haze,
        const spm::SaliencyPolarMap& spm,
        Scalar fatigue
    ) -> Vec2;

    // === 以下のメソッドはテスト可能性のためpublic ===
    /**
     * @brief EFE計算: G(v) = ⟨h⟩·⟨|∇SPM|⟩ + κ·|v|
     *
     * @param velocity 速度ベクトル
     * @param haze Hazeフィールド
     * @param spm Saliency Polar Map
     * @param fatigue 疲労度 [0, 1]
     * @return Expected Free Energy
     */
    static auto compute_efe(
        const Vec2& velocity,
        const Matrix12x12& haze,
        const spm::SaliencyPolarMap& spm,
        Scalar fatigue
    ) -> Scalar;

    /**
     * @brief EFE勾配計算（中心差分）
     *
     * ∇_v G = [(G(v+εx) - G(v-εx))/(2ε), (G(v+εy) - G(v-εy))/(2ε)]
     *
     * @param velocity 現在の速度
     * @param haze Hazeフィールド
     * @param spm Saliency Polar Map
     * @param fatigue 疲労度
     * @return EFE勾配ベクトル
     */
    static auto compute_efe_gradient(
        const Vec2& velocity,
        const Matrix12x12& haze,
        const spm::SaliencyPolarMap& spm,
        Scalar fatigue
    ) -> Vec2;

    /**
     * @brief 速度制約適用
     *
     * - 高疲労（>0.8）→ 強制休息（v=0）
     * - ゼロ速度 → デフォルト（V_MIN, 0）
     * - 速度制約: [V_MIN, V_MAX]
     * - 方向保存
     *
     * @param velocity 制約前の速度
     * @param fatigue 疲労度
     * @return 制約適用後の速度
     */
    static auto apply_constraints(const Vec2& velocity, Scalar fatigue) -> Vec2;
};

// === 実装（ヘッダーオンリー） ===

inline auto ActionSelector::compute_efe(
    const Vec2& velocity,
    const Matrix12x12& haze,
    const spm::SaliencyPolarMap& spm,
    Scalar fatigue
) -> Scalar {
    using namespace eph::constants;

    // Epistemic項: ⟨h⟩ · ⟨|∇SPM|⟩
    Scalar avg_haze = haze.mean();
    auto grad_mag = spm.gradient_magnitude(eph::ChannelID::F2);  // F2 = Saliency
    Scalar avg_grad = grad_mag.mean();
    Scalar epistemic = avg_haze * avg_grad;

    // Pragmatic項: κ(fatigue) · |v|
    Scalar kappa_fatigue = 1.0 + 5.0 * fatigue;  // 疲労が高い → コスト増
    Scalar pragmatic = kappa_fatigue * velocity.norm();

    return epistemic + pragmatic;
}

inline auto ActionSelector::compute_efe_gradient(
    const Vec2& velocity,
    const Matrix12x12& haze,
    const spm::SaliencyPolarMap& spm,
    Scalar fatigue
) -> Vec2 {
    using namespace eph::constants;

    Vec2 gradient;

    // x方向の微分（中心差分）
    Vec2 v_plus_x = velocity + Vec2(GRADIENT_EPSILON, 0.0);
    Vec2 v_minus_x = velocity - Vec2(GRADIENT_EPSILON, 0.0);
    Scalar efe_plus_x = compute_efe(v_plus_x, haze, spm, fatigue);
    Scalar efe_minus_x = compute_efe(v_minus_x, haze, spm, fatigue);
    gradient.x() = (efe_plus_x - efe_minus_x) / (2.0 * GRADIENT_EPSILON);

    // y方向の微分
    Vec2 v_plus_y = velocity + Vec2(0.0, GRADIENT_EPSILON);
    Vec2 v_minus_y = velocity - Vec2(0.0, GRADIENT_EPSILON);
    Scalar efe_plus_y = compute_efe(v_plus_y, haze, spm, fatigue);
    Scalar efe_minus_y = compute_efe(v_minus_y, haze, spm, fatigue);
    gradient.y() = (efe_plus_y - efe_minus_y) / (2.0 * GRADIENT_EPSILON);

    return gradient;
}

inline auto ActionSelector::apply_constraints(
    const Vec2& velocity,
    Scalar fatigue
) -> Vec2 {
    using namespace eph::constants;
    using namespace eph::math;

    // 高疲労 → 強制休息
    if (fatigue > 0.8) {
        return Vec2::Zero();
    }

    Scalar v_mag = velocity.norm();

    // ゼロ速度の場合のデフォルト
    if (v_mag < EPS) {
        return Vec2(V_MIN, 0.0);
    }

    // 速度制約: [V_MIN, V_MAX]
    Scalar v_clamped = clamp(v_mag, V_MIN, V_MAX);

    // 方向を保持、大きさを調整
    return velocity * (v_clamped / v_mag);
}

inline auto ActionSelector::select_action(
    const Vec2& current_velocity,
    const Matrix12x12& haze,
    const spm::SaliencyPolarMap& spm,
    Scalar fatigue
) -> Vec2 {
    using namespace eph::constants;

    // 1. EFE勾配計算
    Vec2 grad = compute_efe_gradient(current_velocity, haze, spm, fatigue);

    // 2. 勾配降下: v_new = v_old - η∇G
    Vec2 new_velocity = current_velocity - LEARNING_RATE * grad;

    // 3. 制約適用
    new_velocity = apply_constraints(new_velocity, fatigue);

    return new_velocity;
}

}  // namespace eph::agent

#endif  // EPH_AGENT_ACTION_SELECTOR_HPP
