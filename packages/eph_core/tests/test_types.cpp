#include <gtest/gtest.h>
#include "eph_core/types.hpp"

using namespace eph;

// AgentState構造体のテスト
TEST(Types, AgentState_DefaultConstructor_InitializesCorrectly) {
    AgentState state;

    EXPECT_DOUBLE_EQ(state.position.x(), 0.0);
    EXPECT_DOUBLE_EQ(state.position.y(), 0.0);
    EXPECT_DOUBLE_EQ(state.velocity.x(), 0.0);
    EXPECT_DOUBLE_EQ(state.velocity.y(), 0.0);
    EXPECT_DOUBLE_EQ(state.kappa, 1.0);
    EXPECT_DOUBLE_EQ(state.fatigue, 0.0);
}

TEST(Types, AgentState_ParameterizedConstructor_SetsValuesCorrectly) {
    Vec2 pos(1.0, 2.0);
    Vec2 vel(3.0, 4.0);
    AgentState state(pos, vel, 0.5, 0.7);

    EXPECT_DOUBLE_EQ(state.position.x(), 1.0);
    EXPECT_DOUBLE_EQ(state.position.y(), 2.0);
    EXPECT_DOUBLE_EQ(state.velocity.x(), 3.0);
    EXPECT_DOUBLE_EQ(state.velocity.y(), 4.0);
    EXPECT_DOUBLE_EQ(state.kappa, 0.5);
    EXPECT_DOUBLE_EQ(state.fatigue, 0.7);
}

// ChannelIDのテスト
TEST(Types, ChannelID_EnumValues_MatchSpecification) {
    EXPECT_EQ(static_cast<int>(ChannelID::T0), 0);
    EXPECT_EQ(static_cast<int>(ChannelID::R0), 1);
    EXPECT_EQ(static_cast<int>(ChannelID::R1), 2);
    EXPECT_EQ(static_cast<int>(ChannelID::F0), 3);
    EXPECT_EQ(static_cast<int>(ChannelID::F1), 4);
    EXPECT_EQ(static_cast<int>(ChannelID::F2), 5);
    EXPECT_EQ(static_cast<int>(ChannelID::F3), 6);
    EXPECT_EQ(static_cast<int>(ChannelID::F4), 7);
    EXPECT_EQ(static_cast<int>(ChannelID::F5), 8);
    EXPECT_EQ(static_cast<int>(ChannelID::M0), 9);
}

// Scalar型のテスト
TEST(Types, Scalar_IsDouble) {
    EXPECT_TRUE((std::is_same<Scalar, double>::value));
}

// Vec2型のテスト
TEST(Types, Vec2_IsEigenVector2d) {
    Vec2 v(1.0, 2.0);
    EXPECT_DOUBLE_EQ(v.x(), 1.0);
    EXPECT_DOUBLE_EQ(v.y(), 2.0);
    EXPECT_DOUBLE_EQ(v.norm(), std::sqrt(5.0));
}
