#ifndef EPH_CORE_MATH_UTILS_HPP
#define EPH_CORE_MATH_UTILS_HPP

#include <algorithm>
#include <cmath>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"

namespace eph {
namespace math {

// 値域クリッピング
inline Scalar clamp(Scalar value, Scalar min_val, Scalar max_val) {
    return std::max(min_val, std::min(value, max_val));
}

// 角度正規化 [-π, π)
inline Scalar wrap_angle(Scalar angle) {
    angle = std::fmod(angle + constants::PI, 2.0 * constants::PI);
    if (angle < 0.0) {
        angle += 2.0 * constants::PI;
    }
    return angle - constants::PI;
}

// インデックスラップ（θ方向: 周期境界）
// 重要: C++の%演算子は負の値で負を返すため、明示的に処理
inline int wrap_index(int index, int size) {
    int result = index % size;
    return (result < 0) ? result + size : result;
}

// インデックスクリップ（r方向: Neumann境界）
inline int clamp_index(int index, int size) {
    return std::max(0, std::min(index, size - 1));
}

// 数値安定Sigmoid
inline Scalar sigmoid(Scalar x) {
    x = clamp(x, constants::SIGMOID_CLIP_MIN, constants::SIGMOID_CLIP_MAX);
    return 1.0 / (1.0 + std::exp(-x));
}

// 線形補間
inline Scalar lerp(Scalar a, Scalar b, Scalar t) {
    return a + t * (b - a);
}

// 2乗
inline Scalar square(Scalar x) {
    return x * x;
}

// 距離（2D）
inline Scalar distance(const Vec2& a, const Vec2& b) {
    return (a - b).norm();
}

}  // namespace math
}  // namespace eph

#endif  // EPH_CORE_MATH_UTILS_HPP
