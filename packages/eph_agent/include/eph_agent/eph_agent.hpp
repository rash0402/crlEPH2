#ifndef EPH_AGENT_EPH_AGENT_HPP
#define EPH_AGENT_EPH_AGENT_HPP

#include <Eigen/Core>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"
#include "eph_spm/saliency_polar_map.hpp"
#include "eph_agent/haze_estimator.hpp"
#include "eph_agent/action_selector.hpp"

namespace eph::agent {

/**
 * @brief EPHエージェントクラス（Phase 4完全版）
 *
 * 単一エージェントの状態管理、Haze推定、行為決定を行います。
 * Expected Free Energy (EFE) 勾配降下による行為選択を実装。
 */
class EPHAgent {
public:
    using Scalar = eph::Scalar;
    using Vec2 = eph::Vec2;
    using Matrix12x12 = eph::Matrix12x12;

    /**
     * @brief コンストラクタ
     * @param initial_state 初期状態（位置・速度・κ・疲労度）
     * @param kappa Haze感度 [0.3-1.5]
     */
    EPHAgent(const AgentState& initial_state, Scalar kappa)
        : state_(initial_state)
        , haze_(Matrix12x12::Zero())
        , haze_estimator_(1.0)  // EMA時定数 τ=1.0
    {
        state_.kappa = kappa;
    }

    /**
     * @brief 状態更新（Phase 4: 完全実装）
     *
     * ## アルゴリズム
     * 1. Action Selection: EFE勾配降下で新しい速度を選択
     * 2. State Update: 位置・速度を更新
     * 3. Prediction Error: 速度変化から予測誤差を計算
     * 4. Haze Estimation: 予測誤差を用いてHazeを推定
     * 5. Fatigue Update: 疲労度を更新
     *
     * ## 予測誤差フィードバックループ
     * Action Selection → State Update → Prediction Error → Haze Estimation
     *       ↑                                                      ↓
     *       └──────────────────── Feedback ──────────────────────┘
     *
     * @param spm Saliency Polar Map
     * @param dt タイムステップ [s]
     */
    void update(const spm::SaliencyPolarMap& spm, Scalar dt) {
        using namespace eph::constants;
        using namespace eph::math;

        // 1. 行為選択（EFE勾配降下）
        Vec2 old_velocity = state_.velocity;
        Vec2 new_velocity = ActionSelector::select_action(
            old_velocity,
            haze_,
            spm,
            state_.fatigue
        );

        // 2. 状態更新
        state_.velocity = new_velocity;
        state_.position += state_.velocity * dt;

        // 3. 予測誤差計算（簡易版: 速度変化量）
        Scalar velocity_change = (new_velocity - old_velocity).norm();
        Scalar prediction_error = clamp(velocity_change / V_MAX, 0.0, 1.0);

        // 4. Haze推定
        haze_ = haze_estimator_.estimate(spm, prediction_error);

        // 5. 疲労度更新
        Scalar speed = state_.velocity.norm();
        if (speed > V_MIN) {
            // 移動中: 疲労蓄積
            state_.fatigue += FATIGUE_RATE * dt;
        } else {
            // 休息中: 疲労回復
            state_.fatigue -= RECOVERY_RATE * dt;
        }

        // 疲労度を[0, 1]にクリップ
        state_.fatigue = clamp(state_.fatigue, 0.0, 1.0);
    }

    /**
     * @brief Haze推定
     *
     * HazeEstimatorを使用してHazeフィールドを推定し、内部状態を更新します。
     *
     * @param spm Saliency Polar Map
     * @param prediction_error 予測誤差 [0, 1]
     * @return 推定されたHazeフィールド [0, 1]
     */
    auto estimate_haze(const spm::SaliencyPolarMap& spm, Scalar prediction_error) -> Matrix12x12 {
        haze_ = haze_estimator_.estimate(spm, prediction_error);
        return haze_;
    }

    /**
     * @brief エージェント状態取得
     * @return 現在の状態（const参照）
     */
    auto state() const -> const AgentState& {
        return state_;
    }

    /**
     * @brief Haze感度取得
     * @return κ値 [0.3-1.5]
     */
    auto kappa() const -> Scalar {
        return state_.kappa;
    }

    /**
     * @brief 現在のHazeフィールド取得
     * @return Hazeフィールド（const参照）[0, 1]
     */
    auto haze() const -> const Matrix12x12& {
        return haze_;
    }

    /**
     * @brief 効果的hazeの設定（MB破れ用）
     *
     * SwarmManagerからのMB破れ適用時に使用します。
     * h_eff,i = (1-β)h_i + β⟨h_j⟩_{j∈N_i}
     *
     * @param h_eff 効果的hazeフィールド [0, 1]
     */
    void set_effective_haze(const Matrix12x12& h_eff) {
        haze_ = h_eff;
    }

    /**
     * @brief Haze推定器をリセット
     *
     * EMAフィルタをリセットします。
     * シミュレーション再開時やエージェント再初期化時に使用。
     */
    void reset_haze_estimator() {
        haze_estimator_.reset();
        haze_ = Matrix12x12::Zero();
    }

private:
    AgentState state_;              // エージェント状態
    Matrix12x12 haze_;              // 現在のHazeフィールド
    HazeEstimator haze_estimator_;  // Haze推定器
};

}  // namespace eph::agent

#endif  // EPH_AGENT_EPH_AGENT_HPP
