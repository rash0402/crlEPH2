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

// === トロイダルワールド関数 ===

/**
 * @brief 座標をトーラス境界でラッピング
 *
 * 座標を [min, max) の範囲に周期的にラッピングします。
 *
 * @param x 座標値
 * @param min 最小境界
 * @param max 最大境界
 * @return ラッピングされた座標 [min, max)
 */
inline Scalar wrap_coordinate(Scalar x, Scalar min, Scalar max) {
    Scalar size = max - min;
    // fmod を使用して効率的にラッピング
    Scalar wrapped = std::fmod(x - min, size);
    if (wrapped < 0.0) {
        wrapped += size;
    }
    return wrapped + min;
}

/**
 * @brief Vec2 の各成分をトーラス境界でラッピング
 *
 * @param pos 位置ベクトル
 * @param min 最小境界
 * @param max 最大境界
 * @return ラッピングされた位置ベクトル
 */
inline Vec2 wrap_position(const Vec2& pos, Scalar min, Scalar max) {
    return Vec2(
        wrap_coordinate(pos.x(), min, max),
        wrap_coordinate(pos.y(), min, max)
    );
}

/**
 * @brief トーラス距離（最短距離）の計算
 *
 * トーラス上での2点間の最短距離を計算します。
 * 各軸で、直接距離と境界を超えた距離のうち短い方を使用します。
 *
 * @param a 点A
 * @param b 点B
 * @param world_size 世界サイズ（各軸）
 * @return トーラス距離
 */
inline Scalar torus_distance(const Vec2& a, const Vec2& b, Scalar world_size) {
    // 各軸で最短距離を計算
    Scalar dx = std::abs(a.x() - b.x());
    Scalar dy = std::abs(a.y() - b.y());

    // トーラスでは境界を超えた距離も考慮
    dx = std::min(dx, world_size - dx);
    dy = std::min(dy, world_size - dy);

    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief トーラス上での最短変位ベクトル
 *
 * 点 a から点 b への最短変位ベクトル（境界を考慮）を計算します。
 *
 * @param a 始点
 * @param b 終点
 * @param world_size 世界サイズ（各軸）
 * @return 最短変位ベクトル
 */
inline Vec2 torus_displacement(const Vec2& a, const Vec2& b, Scalar world_size) {
    Vec2 delta = b - a;

    // 各軸で最短経路を選択
    if (std::abs(delta.x()) > world_size / 2.0) {
        delta.x() = delta.x() > 0 ? delta.x() - world_size : delta.x() + world_size;
    }
    if (std::abs(delta.y()) > world_size / 2.0) {
        delta.y() = delta.y() > 0 ? delta.y() - world_size : delta.y() + world_size;
    }

    return delta;
}

}  // namespace math
}  // namespace eph

#endif  // EPH_CORE_MATH_UTILS_HPP
