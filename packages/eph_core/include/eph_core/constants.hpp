#ifndef EPH_CORE_CONSTANTS_HPP
#define EPH_CORE_CONSTANTS_HPP

#include "eph_core/types.hpp"

namespace eph {
namespace constants {

// 数学定数
constexpr Scalar PI = 3.14159265358979323846;

// SPM定数
constexpr int N_CHANNELS = 10;
constexpr int N_THETA = 12;
constexpr int N_R = 12;
constexpr Scalar DELTA_THETA = 2.0 * PI / N_THETA;
constexpr Scalar FIELD_OF_VIEW_DEGREES = 270.0;  // Default: 270° forward-facing view
constexpr Scalar FIELD_OF_VIEW_RADIANS = FIELD_OF_VIEW_DEGREES * PI / 180.0;

// EPH理論定数
constexpr Scalar BETA_C_TYPICAL = 0.098;  // 臨界点

// Haze推定パラメータ（§4.2推奨値）
constexpr Scalar HAZE_COEFF_A = 0.4;  // 予測誤差の重み
constexpr Scalar HAZE_COEFF_B = 0.3;  // 不確実性の重み
constexpr Scalar HAZE_COEFF_C = 0.2;  // 不可視性の重み
constexpr Scalar HAZE_COEFF_D = 0.1;  // 観測不安定性の重み

// 数値安定性定数
constexpr Scalar EPS = 1e-6;                  // ゼロ除算防止
constexpr Scalar SIGMOID_CLIP_MIN = -10.0;    // Sigmoid飽和防止
constexpr Scalar SIGMOID_CLIP_MAX = 10.0;

// Phase 4: 行為選択パラメータ
constexpr Scalar V_MIN = 0.1;           // 最小速度 [m/s]
constexpr Scalar V_MAX = 2.0;           // 最大速度 [m/s]
constexpr Scalar LEARNING_RATE = 0.8;   // 勾配降下学習率（tuning: 0.5→0.7→1.0→0.8 interpolation）
constexpr Scalar FATIGUE_RATE = 0.02;   // 疲労蓄積率 [1/s]
constexpr Scalar RECOVERY_RATE = 0.01;  // 疲労回復率 [1/s]

// EFE勾配計算
constexpr Scalar GRADIENT_EPSILON = 1e-4;  // 数値微分ステップ
constexpr Scalar ACTION_CLIP_MIN = -5.0;   // 行為空間クリッピング
constexpr Scalar ACTION_CLIP_MAX = 5.0;

}  // namespace constants
}  // namespace eph

#endif  // EPH_CORE_CONSTANTS_HPP
