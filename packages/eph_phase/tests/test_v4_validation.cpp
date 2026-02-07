#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <random>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"
#include "eph_swarm/swarm_manager.hpp"
#include "eph_phase/phase_analyzer.hpp"
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;
using namespace eph::swarm;
using namespace eph::phase;
using namespace eph::spm;

/**
 * @brief V4 Validation: Long-term Numerical Stability (Phase Analysis Integration)
 *
 * Validates that the full EPH pipeline (agents + swarm + phase analysis) remains
 * numerically stable over extended simulation runs.
 *
 * Checks:
 *  - No NaN/Inf in any agent state variable
 *  - Positions remain bounded (finite range)
 *  - Velocities stay within [V_MIN, V_MAX] neighborhood
 *  - Fatigue stays in [0, 1]
 *  - Haze values stay in [0, 1]
 *  - Order parameter phi and susceptibility chi converge to steady state
 *
 * Parameters: N=20 agents, DT=0.1, checked every 100-500 steps.
 * Each test targets < 30 seconds execution time.
 */

// Helper: create an SPM with realistic saliency content
static SaliencyPolarMap create_test_spm(int seed = 42) {
    SaliencyPolarMap spm;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<Scalar> dist(0.2, 0.8);

    Matrix12x12 saliency;
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            saliency(i, j) = dist(rng);
        }
    }
    spm.set_channel(ChannelID::F2, saliency);
    return spm;
}

// Helper: set initial haze with small random perturbation
static void init_random_haze(SwarmManager& swarm, int seed = 123) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
    for (size_t i = 0; i < swarm.size(); ++i) {
        Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
        swarm.get_agent(i).set_effective_haze(initial_haze);
    }
}

// ========================================
// Test 1: No NaN or Inf in any agent state variable over long run
// ========================================
TEST(V4Validation, NoNaNOrInf_LongRun) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int N_STEPS = 3000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // Check every 100 steps
        if (t % 100 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                const auto& state = swarm.get_agent(i).state();
                const auto& haze = swarm.get_agent(i).haze();

                // Position: no NaN/Inf
                ASSERT_FALSE(std::isnan(state.position.x()))
                    << "NaN in position.x at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isnan(state.position.y()))
                    << "NaN in position.y at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isinf(state.position.x()))
                    << "Inf in position.x at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isinf(state.position.y()))
                    << "Inf in position.y at step " << t << ", agent " << i;

                // Velocity: no NaN/Inf
                ASSERT_FALSE(std::isnan(state.velocity.x()))
                    << "NaN in velocity.x at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isnan(state.velocity.y()))
                    << "NaN in velocity.y at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isinf(state.velocity.x()))
                    << "Inf in velocity.x at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isinf(state.velocity.y()))
                    << "Inf in velocity.y at step " << t << ", agent " << i;

                // Fatigue: no NaN/Inf
                ASSERT_FALSE(std::isnan(state.fatigue))
                    << "NaN in fatigue at step " << t << ", agent " << i;
                ASSERT_FALSE(std::isinf(state.fatigue))
                    << "Inf in fatigue at step " << t << ", agent " << i;

                // Haze: no NaN
                ASSERT_FALSE(haze.hasNaN())
                    << "NaN in haze field at step " << t << ", agent " << i;
            }
        }
    }
}

// ========================================
// Test 2: Position remains bounded (no drift to infinity)
// ========================================
TEST(V4Validation, PositionBounded_LongRun) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int N_STEPS = 3000;
    // With V_MAX=2.0 and DT=0.1, max displacement per step = 0.2
    // Over 3000 steps, max theoretical drift ~ 600, but bounded in practice
    const Scalar POSITION_BOUND = 1000.0;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // Check every 500 steps
        if (t % 500 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                const auto& pos = swarm.get_agent(i).state().position;
                ASSERT_LT(std::abs(pos.x()), POSITION_BOUND)
                    << "Position x unbounded at step " << t << ", agent " << i;
                ASSERT_LT(std::abs(pos.y()), POSITION_BOUND)
                    << "Position y unbounded at step " << t << ", agent " << i;
            }
        }
    }
}

// ========================================
// Test 3: Velocity stays within expected range
// ========================================
TEST(V4Validation, VelocityInRange_LongRun) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int N_STEPS = 3000;
    // Allow some tolerance above V_MAX due to transient dynamics
    const Scalar V_UPPER_TOLERANCE = constants::V_MAX * 1.5;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // Check every 200 steps
        if (t % 200 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                Scalar speed = swarm.get_agent(i).state().velocity.norm();
                ASSERT_FALSE(std::isnan(speed))
                    << "NaN speed at step " << t << ", agent " << i;
                EXPECT_LT(speed, V_UPPER_TOLERANCE)
                    << "Velocity exceeded upper bound at step " << t
                    << ", agent " << i << ", speed=" << speed;
            }
        }
    }
}

// ========================================
// Test 4: Fatigue stays in [0, 1]
// ========================================
TEST(V4Validation, FatigueInRange_LongRun) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int N_STEPS = 3000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // Check every 100 steps
        if (t % 100 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                Scalar fatigue = swarm.get_agent(i).state().fatigue;
                ASSERT_GE(fatigue, 0.0)
                    << "Fatigue below 0 at step " << t << ", agent " << i;
                ASSERT_LE(fatigue, 1.0)
                    << "Fatigue above 1 at step " << t << ", agent " << i;
            }
        }
    }
}

// ========================================
// Test 5: Haze values stay in [0, 1]
// ========================================
TEST(V4Validation, HazeInRange_LongRun) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int N_STEPS = 3000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // Check every 200 steps
        if (t % 200 == 0) {
            auto haze_fields = swarm.get_all_haze_fields();
            for (size_t i = 0; i < haze_fields.size(); ++i) {
                Scalar haze_min = haze_fields[i].minCoeff();
                Scalar haze_max = haze_fields[i].maxCoeff();

                ASSERT_GE(haze_min, -0.01)
                    << "Haze below 0 at step " << t << ", agent " << i
                    << ", min=" << haze_min;
                ASSERT_LE(haze_max, 1.01)
                    << "Haze above 1 at step " << t << ", agent " << i
                    << ", max=" << haze_max;
                ASSERT_FALSE(haze_fields[i].hasNaN())
                    << "NaN in haze at step " << t << ", agent " << i;
            }
        }
    }
}

// ========================================
// Test 6: Order parameter phi converges to steady state
// ========================================
TEST(V4Validation, PhiConvergesToSteadyState) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int EQUILIBRATION_STEPS = 1000;
    const int MEASUREMENT_STEPS = 1000;
    const int SAMPLE_INTERVAL = 10;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    // Equilibration phase
    for (int t = 0; t < EQUILIBRATION_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);
    }

    // Measurement phase: collect phi samples
    std::vector<Scalar> phi_samples;
    phi_samples.reserve(MEASUREMENT_STEPS / SAMPLE_INTERVAL);

    for (int t = 0; t < MEASUREMENT_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        if (t % SAMPLE_INTERVAL == 0) {
            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phi_samples.push_back(phi);

            // Phi must be finite
            ASSERT_FALSE(std::isnan(phi));
            ASSERT_FALSE(std::isinf(phi));
        }
    }

    // Compute mean and stddev of phi
    Scalar phi_mean = PhaseAnalyzer::mean(phi_samples);
    Scalar phi_std = PhaseAnalyzer::stddev(phi_samples);

    // Steady state: coefficient of variation should be moderate
    // (phi_std / phi_mean < 1.0, or phi_std < 0.1 if phi_mean is very small)
    if (phi_mean > 0.01) {
        EXPECT_LT(phi_std / phi_mean, 1.0)
            << "Order parameter phi has not converged to steady state. "
            << "mean=" << phi_mean << ", std=" << phi_std;
    } else {
        EXPECT_LT(phi_std, 0.1)
            << "Order parameter phi fluctuation too large. "
            << "mean=" << phi_mean << ", std=" << phi_std;
    }

    std::cout << "[V4] Phi steady state: mean=" << phi_mean
              << ", std=" << phi_std << std::endl;
}

// ========================================
// Test 7: Susceptibility chi converges to steady state
// ========================================
TEST(V4Validation, ChiConvergesToSteadyState) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int EQUILIBRATION_STEPS = 1000;
    const int WINDOW_SIZE = 200;   // Steps per chi measurement window
    const int N_CHI_WINDOWS = 4;   // Number of chi measurements
    const int SAMPLE_INTERVAL = 5;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    init_random_haze(swarm);
    auto spm = create_test_spm();

    // Equilibration phase
    for (int t = 0; t < EQUILIBRATION_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);
    }

    // Collect chi values over consecutive windows
    std::vector<Scalar> chi_values;

    for (int w = 0; w < N_CHI_WINDOWS; ++w) {
        std::vector<Scalar> phi_window;
        phi_window.reserve(WINDOW_SIZE / SAMPLE_INTERVAL);

        for (int t = 0; t < WINDOW_SIZE; ++t) {
            swarm.update_all_agents(spm, DT);

            if (t % SAMPLE_INTERVAL == 0) {
                auto haze_fields = swarm.get_all_haze_fields();
                Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
                phi_window.push_back(phi);
            }
        }

        Scalar chi = PhaseAnalyzer::compute_chi(phi_window);
        chi_values.push_back(chi);

        // Chi must be finite and non-negative
        ASSERT_FALSE(std::isnan(chi))
            << "NaN in chi at window " << w;
        ASSERT_FALSE(std::isinf(chi))
            << "Inf in chi at window " << w;
        ASSERT_GE(chi, 0.0)
            << "Negative chi at window " << w;
    }

    // Chi values across windows should be of similar magnitude
    Scalar chi_mean = PhaseAnalyzer::mean(chi_values);
    Scalar chi_std = PhaseAnalyzer::stddev(chi_values);

    // Steady state: chi should not diverge across windows
    if (chi_mean > 1e-6) {
        EXPECT_LT(chi_std / chi_mean, 2.0)
            << "Chi is not converging. mean=" << chi_mean
            << ", std=" << chi_std;
    }

    std::cout << "[V4] Chi steady state: mean=" << chi_mean
              << ", std=" << chi_std << std::endl;
}

// ========================================
// Test 8: Multiple beta values remain stable over long runs
// ========================================
TEST(V4Validation, MultipleBeta_LongRunStability) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar DT = 0.1;
    const int N_STEPS = 1000;

    // Test stability at several beta values spanning the phase transition
    std::vector<Scalar> beta_values = {0.0, 0.05, 0.098, 0.15, 0.25};

    for (Scalar beta : beta_values) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);
        init_random_haze(swarm);
        auto spm = create_test_spm();

        for (int t = 0; t < N_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);
        }

        // After long run: all agents must have finite, bounded state
        for (size_t i = 0; i < swarm.size(); ++i) {
            const auto& state = swarm.get_agent(i).state();
            const auto& haze = swarm.get_agent(i).haze();

            EXPECT_FALSE(std::isnan(state.position.x()))
                << "NaN position at beta=" << beta << ", agent " << i;
            EXPECT_FALSE(std::isnan(state.velocity.x()))
                << "NaN velocity at beta=" << beta << ", agent " << i;
            EXPECT_FALSE(std::isnan(state.fatigue))
                << "NaN fatigue at beta=" << beta << ", agent " << i;
            EXPECT_FALSE(haze.hasNaN())
                << "NaN haze at beta=" << beta << ", agent " << i;

            EXPECT_GE(state.fatigue, 0.0);
            EXPECT_LE(state.fatigue, 1.0);
            EXPECT_GE(haze.minCoeff(), -0.01);
            EXPECT_LE(haze.maxCoeff(), 1.01);
        }

        // Phi must be computable and finite
        auto haze_fields = swarm.get_all_haze_fields();
        Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
        EXPECT_FALSE(std::isnan(phi))
            << "NaN phi at beta=" << beta;
        EXPECT_FALSE(std::isinf(phi))
            << "Inf phi at beta=" << beta;

        std::cout << "[V4] beta=" << beta << " -> phi=" << phi << " (stable)" << std::endl;
    }
}
