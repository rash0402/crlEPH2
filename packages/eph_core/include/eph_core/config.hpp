#ifndef EPH_CORE_CONFIG_HPP
#define EPH_CORE_CONFIG_HPP

#include "eph_core/types.hpp"

namespace eph {

// エージェント個別設定
struct AgentConfig {
    Scalar kappa = 1.0;              // haze sensitivity [0.3-1.5]
    Scalar mass = 1.0;               // [kg]
    Scalar drag_coeff = 0.1;         // drag coefficient
    Scalar eta = 0.1;                // learning rate
    Scalar tau_haze = 1.0;           // haze time constant [s]
};

// 群れ全体設定
struct SwarmConfig {
    size_t n_agents = 100;           // number of agents
    Scalar beta = 0.098;             // MB breaking strength
    Scalar dt = 0.1;                 // time step [s]
    int neighbor_count = 6;          // z-1 (neighbors per agent)
    bool use_role_distribution = true;  // 2:6:2 distribution
};

// シミュレーション設定
struct SimulationConfig {
    int max_steps = 10000;           // maximum simulation steps
    Scalar max_time = 1000.0;        // maximum simulation time [s]
    int log_interval = 100;          // logging every N steps
    bool enable_visualization = false;
};

}  // namespace eph

#endif  // EPH_CORE_CONFIG_HPP
