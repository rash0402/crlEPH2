#include <gtest/gtest.h>
#include "eph_core/config.hpp"

using namespace eph;

// AgentConfig テスト
TEST(Config, AgentConfig_DefaultValues) {
    AgentConfig config;

    EXPECT_DOUBLE_EQ(config.kappa, 1.0);
    EXPECT_DOUBLE_EQ(config.mass, 1.0);
    EXPECT_DOUBLE_EQ(config.drag_coeff, 0.1);
    EXPECT_DOUBLE_EQ(config.eta, 0.1);
    EXPECT_DOUBLE_EQ(config.tau_haze, 1.0);
}

// SwarmConfig テスト
TEST(Config, SwarmConfig_DefaultValues) {
    SwarmConfig config;

    EXPECT_EQ(config.n_agents, 100);
    EXPECT_DOUBLE_EQ(config.beta, 0.098);
    EXPECT_DOUBLE_EQ(config.dt, 0.1);
    EXPECT_EQ(config.neighbor_count, 6);
    EXPECT_TRUE(config.use_role_distribution);
}

// SimulationConfig テスト
TEST(Config, SimulationConfig_DefaultValues) {
    SimulationConfig config;

    EXPECT_EQ(config.max_steps, 10000);
    EXPECT_DOUBLE_EQ(config.max_time, 1000.0);
    EXPECT_EQ(config.log_interval, 100);
    EXPECT_FALSE(config.enable_visualization);
}
