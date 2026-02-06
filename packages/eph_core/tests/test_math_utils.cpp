#include <gtest/gtest.h>
#include <cmath>
#include "eph_core/math_utils.hpp"
#include "eph_core/constants.hpp"

using namespace eph;
using namespace eph::math;
using namespace eph::constants;

// === θ方向: 周期境界テスト ===

TEST(MathUtils, WrapIndex_Negative_WrapsAround) {
    EXPECT_EQ(wrap_index(-1, 12), 11);
    EXPECT_EQ(wrap_index(-2, 12), 10);
    EXPECT_EQ(wrap_index(-13, 12), 11);  // -13 % 12 = -1 → 11
}

TEST(MathUtils, WrapIndex_Positive_WrapsAround) {
    EXPECT_EQ(wrap_index(12, 12), 0);
    EXPECT_EQ(wrap_index(13, 12), 1);
    EXPECT_EQ(wrap_index(24, 12), 0);
}

TEST(MathUtils, WrapIndex_InRange_ReturnsAsIs) {
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(wrap_index(i, 12), i);
    }
}

// === r方向: Neumann境界テスト ===

TEST(MathUtils, ClampIndex_Negative_ReturnsZero) {
    EXPECT_EQ(clamp_index(-1, 12), 0);
    EXPECT_EQ(clamp_index(-100, 12), 0);
}

TEST(MathUtils, ClampIndex_TooLarge_ReturnsMaxIndex) {
    EXPECT_EQ(clamp_index(12, 12), 11);
    EXPECT_EQ(clamp_index(15, 12), 11);
    EXPECT_EQ(clamp_index(100, 12), 11);
}

TEST(MathUtils, ClampIndex_InRange_ReturnsAsIs) {
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(clamp_index(i, 12), i);
    }
}

// === Sigmoid テスト ===

TEST(MathUtils, Sigmoid_Zero_ReturnsHalf) {
    EXPECT_DOUBLE_EQ(sigmoid(0.0), 0.5);
}

TEST(MathUtils, Sigmoid_LargePositive_ApproachesOne) {
    EXPECT_NEAR(sigmoid(10.0), 1.0, 1e-4);
    EXPECT_NEAR(sigmoid(100.0), 1.0, 1e-4);  // クリッピングされる
}

TEST(MathUtils, Sigmoid_LargeNegative_ApproachesZero) {
    EXPECT_NEAR(sigmoid(-10.0), 0.0, 1e-4);
    EXPECT_NEAR(sigmoid(-100.0), 0.0, 1e-4);  // クリッピングされる
}

TEST(MathUtils, Sigmoid_MonotonicallyIncreasing) {
    EXPECT_LT(sigmoid(-1.0), sigmoid(0.0));
    EXPECT_LT(sigmoid(0.0), sigmoid(1.0));
}

// === Clamp テスト ===

TEST(MathUtils, Clamp_BelowMin_ReturnsMin) {
    EXPECT_DOUBLE_EQ(clamp(-5.0, 0.0, 10.0), 0.0);
}

TEST(MathUtils, Clamp_AboveMax_ReturnsMax) {
    EXPECT_DOUBLE_EQ(clamp(15.0, 0.0, 10.0), 10.0);
}

TEST(MathUtils, Clamp_InRange_ReturnsValue) {
    EXPECT_DOUBLE_EQ(clamp(5.0, 0.0, 10.0), 5.0);
}

// === WrapAngle テスト ===

TEST(MathUtils, WrapAngle_Zero_ReturnsZero) {
    EXPECT_DOUBLE_EQ(wrap_angle(0.0), 0.0);
}

TEST(MathUtils, WrapAngle_PI_ReturnsNegativePI) {
    // π は -π にラップされる（[-π, π)の範囲）
    EXPECT_NEAR(wrap_angle(PI), -PI, 1e-10);
}

TEST(MathUtils, WrapAngle_LargePositive_WrapsToRange) {
    Scalar angle = 3.0 * PI;
    Scalar wrapped = wrap_angle(angle);
    EXPECT_GE(wrapped, -PI);  // -π を含む
    EXPECT_LT(wrapped, PI);
}

TEST(MathUtils, WrapAngle_LargeNegative_WrapsToRange) {
    Scalar angle = -3.0 * PI;
    Scalar wrapped = wrap_angle(angle);
    EXPECT_GE(wrapped, -PI);  // -π を含む
    EXPECT_LT(wrapped, PI);
}

// === Lerp テスト ===

TEST(MathUtils, Lerp_AtZero_ReturnsA) {
    EXPECT_DOUBLE_EQ(lerp(10.0, 20.0, 0.0), 10.0);
}

TEST(MathUtils, Lerp_AtOne_ReturnsB) {
    EXPECT_DOUBLE_EQ(lerp(10.0, 20.0, 1.0), 20.0);
}

TEST(MathUtils, Lerp_AtHalf_ReturnsMidpoint) {
    EXPECT_DOUBLE_EQ(lerp(10.0, 20.0, 0.5), 15.0);
}

// === Distance テスト ===

TEST(MathUtils, Distance_SamePoint_ReturnsZero) {
    Vec2 a(1.0, 2.0);
    Vec2 b(1.0, 2.0);
    EXPECT_DOUBLE_EQ(distance(a, b), 0.0);
}

TEST(MathUtils, Distance_UnitVector_ReturnsOne) {
    Vec2 a(0.0, 0.0);
    Vec2 b(1.0, 0.0);
    EXPECT_DOUBLE_EQ(distance(a, b), 1.0);
}

TEST(MathUtils, Distance_Pythagorean_ReturnsCorrect) {
    Vec2 a(0.0, 0.0);
    Vec2 b(3.0, 4.0);
    EXPECT_DOUBLE_EQ(distance(a, b), 5.0);
}

// === Square テスト ===

TEST(MathUtils, Square_Zero_ReturnsZero) {
    EXPECT_DOUBLE_EQ(square(0.0), 0.0);
}

TEST(MathUtils, Square_Positive_ReturnsSquare) {
    EXPECT_DOUBLE_EQ(square(3.0), 9.0);
}

TEST(MathUtils, Square_Negative_ReturnsPositiveSquare) {
    EXPECT_DOUBLE_EQ(square(-4.0), 16.0);
}
