#ifndef EPH_SPM_SALIENCY_POLAR_MAP_HPP
#define EPH_SPM_SALIENCY_POLAR_MAP_HPP

#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <cmath>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"
#include "eph_core/math_utils.hpp"

namespace eph::spm {

/**
 * @brief Saliency Polar Map with configurable field of view
 *
 * θ index interpretation (when FIELD_OF_VIEW_DEGREES = 270):
 *   - θ_idx = 0   → -135° (left edge of FOV)
 *   - θ_idx = 6   → 0° (forward, agent heading)
 *   - θ_idx = 11  → +135° (right edge of FOV)
 *
 * For 360° FOV: θ ∈ [0°, 360°)
 * For 270° FOV: θ ∈ [-135°, +135°] centered on heading
 */
class SaliencyPolarMap {
public:
    using Scalar = eph::Scalar;
    using Tensor3 = eph::Tensor3;
    using Matrix12x12 = eph::Matrix12x12;

    // コンストラクタ
    SaliencyPolarMap() : data_(10, 12, 12) {
        data_.setZero();
    }

    // チャネルアクセス
    auto get_channel(eph::ChannelID id) const -> Matrix12x12 {
        return channel_to_matrix(static_cast<int>(id));
    }

    void set_channel(eph::ChannelID id, const Matrix12x12& mat) {
        matrix_to_channel(static_cast<int>(id), mat);
    }

    // 境界条件を満たす勾配計算

    // θ方向勾配（周期境界）
    auto gradient_theta(eph::ChannelID id) const -> Matrix12x12 {
        using namespace eph::constants;
        using namespace eph::math;

        auto channel = get_channel(id);
        Matrix12x12 grad;

        for (int a = 0; a < N_THETA; ++a) {
            for (int b = 0; b < N_R; ++b) {
                // θ方向: 周期境界（a=0 と a=11 が隣接）
                int a_plus = wrap_index(a + 1, N_THETA);
                int a_minus = wrap_index(a - 1, N_THETA);

                // 中心差分
                grad(a, b) = (channel(a_plus, b) - channel(a_minus, b)) / (2.0 * DELTA_THETA);
            }
        }
        return grad;
    }

    // r方向勾配（Neumann境界）
    auto gradient_r(eph::ChannelID id) const -> Matrix12x12 {
        using namespace eph::constants;
        using namespace eph::math;

        auto channel = get_channel(id);
        Matrix12x12 grad;

        for (int a = 0; a < N_THETA; ++a) {
            for (int b = 0; b < N_R; ++b) {
                if (b == 0 || b == N_R - 1) {
                    // r方向: Neumann境界（端でゼロ勾配）
                    grad(a, b) = 0.0;
                } else {
                    // 内部: 中心差分
                    int b_plus = clamp_index(b + 1, N_R);
                    int b_minus = clamp_index(b - 1, N_R);
                    grad(a, b) = (channel(a, b_plus) - channel(a, b_minus)) / 2.0;
                }
            }
        }
        return grad;
    }

    // 勾配の大きさ
    auto gradient_magnitude(eph::ChannelID id) const -> Matrix12x12 {
        auto grad_theta = gradient_theta(id);
        auto grad_r = gradient_r(id);

        Matrix12x12 mag;
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                mag(a, b) = std::sqrt(grad_theta(a, b) * grad_theta(a, b) +
                                       grad_r(a, b) * grad_r(a, b));
            }
        }
        return mag;
    }

    // ユーティリティ
    void zero_all() {
        data_.setZero();
    }

    auto channel_count() const -> int { return 10; }
    auto theta_count() const -> int { return 12; }
    auto r_count() const -> int { return 12; }

private:
    Tensor3 data_;  // (10, 12, 12) = (C, θ, r)

    // 内部ヘルパー
    auto channel_to_matrix(int channel_idx) const -> Matrix12x12 {
        Matrix12x12 mat;
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                mat(a, b) = data_(channel_idx, a, b);
            }
        }
        return mat;
    }

    void matrix_to_channel(int channel_idx, const Matrix12x12& mat) {
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                data_(channel_idx, a, b) = mat(a, b);
            }
        }
    }
};

}  // namespace eph::spm

#endif  // EPH_SPM_SALIENCY_POLAR_MAP_HPP
