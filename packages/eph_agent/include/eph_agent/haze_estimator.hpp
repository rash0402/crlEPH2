#ifndef EPH_AGENT_HAZE_ESTIMATOR_HPP
#define EPH_AGENT_HAZE_ESTIMATOR_HPP

#include <Eigen/Core>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"
#include "eph_core/math_utils.hpp"
#include "eph_spm/saliency_polar_map.hpp"

namespace eph::agent {

/**
 * @brief Haze推定クラス（§4.2）
 *
 * 予測誤差・不確実性・可視性・観測安定性からHazeフィールドを推定します。
 * EMAフィルタと空間平滑化を使用して数値的に安定な推定を行います。
 */
class HazeEstimator {
public:
    using Scalar = eph::Scalar;
    using Matrix12x12 = eph::Matrix12x12;

    /**
     * @brief コンストラクタ
     * @param tau EMA時定数（デフォルト: 1.0）
     */
    explicit HazeEstimator(Scalar tau = 1.0)
        : tau_(tau)
        , ema_error_(Matrix12x12::Zero())
        , initialized_(false)
    {}

    /**
     * @brief Haze推定（§4.2の式）
     *
     * h̃ = σ(a·EMA(e) + b·R1 + c·(1-F4) + d·F5)
     *
     * @param spm Saliency Polar Map
     * @param prediction_error 予測誤差 [0, 1]
     * @return Hazeフィールド [0, 1]
     */
    auto estimate(
        const spm::SaliencyPolarMap& spm,
        Scalar prediction_error
    ) -> Matrix12x12 {
        using namespace eph::constants;
        using namespace eph::math;

        // EMA更新
        if (!initialized_) {
            ema_error_ = Matrix12x12::Constant(prediction_error);
            initialized_ = true;
        } else {
            Scalar alpha = 1.0 / tau_;
            ema_error_ = alpha * Matrix12x12::Constant(prediction_error) +
                         (1.0 - alpha) * ema_error_;
        }

        // チャネル取得
        auto R1 = spm.get_channel(ChannelID::R1);  // 不確実性
        auto F4 = spm.get_channel(ChannelID::F4);  // 可視性
        auto F5 = spm.get_channel(ChannelID::F5);  // 観測安定性

        // 入力構成（§4.2の式）
        Matrix12x12 input = HAZE_COEFF_A * ema_error_ +
                            HAZE_COEFF_B * R1 +
                            HAZE_COEFF_C * (Matrix12x12::Ones() - F4) +
                            HAZE_COEFF_D * F5;

        // 数値安定性: 入力クリッピング
        input = input.cwiseMax(SIGMOID_CLIP_MIN).cwiseMin(SIGMOID_CLIP_MAX).eval();

        // Sigmoid適用
        Matrix12x12 h_tilde;
        for (int a = 0; a < N_THETA; ++a) {
            for (int b = 0; b < N_R; ++b) {
                h_tilde(a, b) = sigmoid(input(a, b));
            }
        }

        // 空間平滑化
        return gaussian_blur(h_tilde, 1.0);
    }

    /**
     * @brief EMAフィルタリセット
     */
    void reset() {
        ema_error_ = Matrix12x12::Zero();
        initialized_ = false;
    }

private:
    Scalar tau_;                  // EMA時定数
    Matrix12x12 ema_error_;       // 予測誤差のEMA
    bool initialized_;

    /**
     * @brief ガウシアンブラー（空間平滑化）
     *
     * 簡易的な3×3ガウシアンカーネル（σ≈1）を使用:
     * [1 2 1]
     * [2 4 2] / 16
     * [1 2 1]
     *
     * @param input 入力フィールド
     * @param sigma 標準偏差（未使用、拡張用）
     * @return 平滑化されたフィールド
     */
    auto gaussian_blur(const Matrix12x12& input, Scalar sigma) const -> Matrix12x12 {
        using namespace eph::constants;
        using namespace eph::math;

        Matrix12x12 output = Matrix12x12::Zero();

        for (int a = 0; a < N_THETA; ++a) {
            for (int b = 0; b < N_R; ++b) {
                Scalar sum = 0.0;
                Scalar weight_sum = 0.0;

                for (int da = -1; da <= 1; ++da) {
                    for (int db = -1; db <= 1; ++db) {
                        int na = wrap_index(a + da, N_THETA);  // θ方向: 周期境界
                        int nb = clamp_index(b + db, N_R);     // r方向: Neumann境界

                        Scalar weight = (da == 0 && db == 0) ? 4.0 :
                                       (da == 0 || db == 0) ? 2.0 : 1.0;

                        sum += weight * input(na, nb);
                        weight_sum += weight;
                    }
                }
                output(a, b) = sum / weight_sum;
            }
        }
        return output;
    }
};

}  // namespace eph::agent

#endif  // EPH_AGENT_HAZE_ESTIMATOR_HPP
