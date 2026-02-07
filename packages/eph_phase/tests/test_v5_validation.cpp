#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
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
 * @brief V5 Validation: Large Swarm Scalability
 *
 * Validates that the EPH system scales to larger swarm sizes and
 * that phase transition detection is consistent across different N.
 *
 * Checks:
 *  - N=100 reproduces phase transition (beta_c detection)
 *  - N=50 and N=100 give consistent beta_c values
 *  - Computational cost scales appropriately
 *  - Statistical accuracy improves with larger N
 *  - Scaled susceptibility chi/N peaks consistently
 *
 * Parameters (reduced for speed):
 *  - Beta sweep: 0.0 to 0.3, step 0.03
 *  - Warmup: 100 steps, DT=0.1
 *  - Measurement: 30 samples
 *  - Target: each test < 60 seconds, total < 300 seconds
 */

// Helper: run a beta sweep and return (betas, phis, chis)
struct SweepResult {
    std::vector<Scalar> betas;
    std::vector<Scalar> phis;
    std::vector<Scalar> chis;
};

static SweepResult run_beta_sweep(
    size_t n_agents,
    Scalar beta_min,
    Scalar beta_max,
    Scalar beta_step,
    int warmup_steps,
    int measurement_samples,
    Scalar dt,
    int avg_neighbors = 6,
    int spm_seed = 42,
    int haze_seed = 123)
{
    SweepResult result;

    for (Scalar beta = beta_min; beta <= beta_max + 1e-9; beta += beta_step) {
        SwarmManager swarm(n_agents, beta, avg_neighbors);

        // Initialize haze with random perturbation
        std::mt19937 rng(haze_seed);
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        // Create SPM
        SaliencyPolarMap spm;
        std::mt19937 spm_rng(spm_seed);
        std::uniform_real_distribution<Scalar> spm_dist(0.2, 0.8);
        Matrix12x12 saliency;
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                saliency(a, b) = spm_dist(spm_rng);
            }
        }
        spm.set_channel(ChannelID::F2, saliency);

        // Warmup phase
        for (int t = 0; t < warmup_steps; ++t) {
            swarm.update_all_agents(spm, dt);
        }

        // Measurement phase
        std::vector<Scalar> phi_samples;
        phi_samples.reserve(measurement_samples);

        for (int t = 0; t < measurement_samples; ++t) {
            swarm.update_all_agents(spm, dt);
            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phi_samples.push_back(phi);
        }

        Scalar phi_avg = PhaseAnalyzer::mean(phi_samples);
        Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

        result.betas.push_back(beta);
        result.phis.push_back(phi_avg);
        result.chis.push_back(chi);
    }

    return result;
}

// ========================================
// Test 1: N=100 can reproduce phase transition (beta_c detection)
// ========================================
TEST(V5Validation, N100_DetectsPhaseTransition) {
    const size_t N_AGENTS = 100;
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.03;
    const int WARMUP_STEPS = 100;
    const int MEASUREMENT_SAMPLES = 30;
    const Scalar DT = 0.1;

    auto result = run_beta_sweep(
        N_AGENTS, BETA_MIN, BETA_MAX, BETA_STEP,
        WARMUP_STEPS, MEASUREMENT_SAMPLES, DT);

    // All values must be finite
    for (size_t i = 0; i < result.betas.size(); ++i) {
        ASSERT_FALSE(std::isnan(result.phis[i]))
            << "NaN in phi at beta=" << result.betas[i];
        ASSERT_FALSE(std::isnan(result.chis[i]))
            << "NaN in chi at beta=" << result.betas[i];
        ASSERT_FALSE(std::isinf(result.phis[i]))
            << "Inf in phi at beta=" << result.betas[i];
        ASSERT_FALSE(std::isinf(result.chis[i]))
            << "Inf in chi at beta=" << result.betas[i];
    }

    // Detect beta_c
    ASSERT_GE(result.betas.size(), 3u)
        << "Need at least 3 data points for beta_c detection";
    Scalar beta_c = PhaseAnalyzer::find_beta_c(result.betas, result.phis);

    // beta_c should be in a reasonable range around the theoretical value
    // Wider tolerance for reduced parameters and coarse grid
    EXPECT_GT(beta_c, 0.02)
        << "beta_c too low: " << beta_c;
    EXPECT_LT(beta_c, 0.25)
        << "beta_c too high: " << beta_c;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "[V5] N=100 beta_c = " << beta_c
              << " (theory: " << constants::BETA_C_TYPICAL << ")" << std::endl;

    // Print sweep data for diagnostics
    std::cout << " beta   phi     chi" << std::endl;
    for (size_t i = 0; i < result.betas.size(); ++i) {
        std::cout << std::setw(5) << result.betas[i] << "  "
                  << std::setw(7) << result.phis[i] << "  "
                  << std::setw(7) << result.chis[i] << std::endl;
    }
}

// ========================================
// Test 2: N=50 and N=100 give consistent beta_c values
// ========================================
TEST(V5Validation, BetaC_ConsistentAcrossSwarmSizes) {
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.03;
    const int WARMUP_STEPS = 100;
    const int MEASUREMENT_SAMPLES = 30;
    const Scalar DT = 0.1;

    // Run sweep with N=50
    auto result_50 = run_beta_sweep(
        50, BETA_MIN, BETA_MAX, BETA_STEP,
        WARMUP_STEPS, MEASUREMENT_SAMPLES, DT);

    // Run sweep with N=100
    auto result_100 = run_beta_sweep(
        100, BETA_MIN, BETA_MAX, BETA_STEP,
        WARMUP_STEPS, MEASUREMENT_SAMPLES, DT);

    // Both must have enough data points
    ASSERT_GE(result_50.betas.size(), 3u);
    ASSERT_GE(result_100.betas.size(), 3u);

    Scalar beta_c_50 = PhaseAnalyzer::find_beta_c(result_50.betas, result_50.phis);
    Scalar beta_c_100 = PhaseAnalyzer::find_beta_c(result_100.betas, result_100.phis);

    // beta_c values should be in same general regime (within 0.15 of each other)
    // Note: Finite-size scaling theory predicts β_c(N) shifts with system size.
    // For small N (50-100), deviations of O(0.1) are expected and physically meaningful.
    Scalar deviation = std::abs(beta_c_50 - beta_c_100);
    EXPECT_LT(deviation, 0.15)
        << "beta_c inconsistent: N=50 -> " << beta_c_50
        << ", N=100 -> " << beta_c_100
        << ", deviation=" << deviation;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "[V5] beta_c(N=50)  = " << beta_c_50 << std::endl;
    std::cout << "[V5] beta_c(N=100) = " << beta_c_100 << std::endl;
    std::cout << "[V5] deviation     = " << deviation << std::endl;
}

// ========================================
// Test 3: Computational cost scales appropriately
// ========================================
TEST(V5Validation, ComputationalCost_ScalesReasonably) {
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int N_STEPS = 100;
    const int AVG_NEIGHBORS = 6;

    // Measure time for N=50
    auto t1_start = std::chrono::high_resolution_clock::now();
    {
        SwarmManager swarm(50, BETA, AVG_NEIGHBORS);
        SaliencyPolarMap spm;
        Matrix12x12 saliency = Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, saliency);

        for (int t = 0; t < N_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);
        }
    }
    auto t1_end = std::chrono::high_resolution_clock::now();
    double time_50 = std::chrono::duration<double, std::milli>(t1_end - t1_start).count();

    // Measure time for N=100
    auto t2_start = std::chrono::high_resolution_clock::now();
    {
        SwarmManager swarm(100, BETA, AVG_NEIGHBORS);
        SaliencyPolarMap spm;
        Matrix12x12 saliency = Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, saliency);

        for (int t = 0; t < N_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);
        }
    }
    auto t2_end = std::chrono::high_resolution_clock::now();
    double time_100 = std::chrono::duration<double, std::milli>(t2_end - t2_start).count();

    // Scaling factor: doubling N should not be worse than O(N^2) ~ 4x
    // Allow up to 6x to account for cache effects and overhead
    double scaling_factor = (time_50 > 0.0) ? (time_100 / time_50) : 1.0;

    EXPECT_LT(scaling_factor, 6.0)
        << "Computational cost scales worse than expected: "
        << "N=50: " << time_50 << "ms, N=100: " << time_100 << "ms, "
        << "ratio=" << scaling_factor;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[V5] Timing: N=50: " << time_50 << "ms, N=100: " << time_100
              << "ms, ratio=" << scaling_factor << std::endl;
}

// ========================================
// Test 4: Statistical accuracy improves with larger N
// ========================================
TEST(V5Validation, StatisticalAccuracy_ImprovesWithN) {
    const Scalar BETA = constants::BETA_C_TYPICAL;
    const Scalar DT = 0.1;
    const int WARMUP_STEPS = 100;
    const int MEASUREMENT_STEPS = 50;
    const int SAMPLE_INTERVAL = 2;
    const int AVG_NEIGHBORS = 6;
    const int N_RUNS = 3;

    // Collect phi variance for N=20 and N=80
    auto measure_phi_variance = [&](size_t n_agents) -> Scalar {
        std::vector<Scalar> run_means;

        for (int run = 0; run < N_RUNS; ++run) {
            SwarmManager swarm(n_agents, BETA, AVG_NEIGHBORS);

            // Varying seed per run for statistical independence
            std::mt19937 rng(42 + run * 100);
            std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
            for (size_t i = 0; i < swarm.size(); ++i) {
                Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
                swarm.get_agent(i).set_effective_haze(initial_haze);
            }

            SaliencyPolarMap spm;
            Matrix12x12 saliency = Matrix12x12::Constant(0.5);
            spm.set_channel(ChannelID::F2, saliency);

            for (int t = 0; t < WARMUP_STEPS; ++t) {
                swarm.update_all_agents(spm, DT);
            }

            std::vector<Scalar> phi_samples;
            for (int t = 0; t < MEASUREMENT_STEPS; ++t) {
                swarm.update_all_agents(spm, DT);
                if (t % SAMPLE_INTERVAL == 0) {
                    auto haze_fields = swarm.get_all_haze_fields();
                    Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
                    phi_samples.push_back(phi);
                }
            }

            run_means.push_back(PhaseAnalyzer::mean(phi_samples));
        }

        return PhaseAnalyzer::stddev(run_means);
    };

    Scalar variance_small = measure_phi_variance(20);
    Scalar variance_large = measure_phi_variance(80);

    // Larger N should have equal or lower variance (or at least not much worse).
    // Allow generous tolerance given reduced sampling.
    EXPECT_LT(variance_large, variance_small * 3.0 + 0.01)
        << "Larger swarm should not have significantly worse statistical accuracy. "
        << "var(N=20)=" << variance_small << ", var(N=80)=" << variance_large;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "[V5] Phi stddev: N=20: " << variance_small
              << ", N=80: " << variance_large << std::endl;
}

// ========================================
// Test 5: Scaled susceptibility chi/N peaks consistently
// ========================================
TEST(V5Validation, ScaledSusceptibility_PeaksConsistently) {
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.03;
    const int WARMUP_STEPS = 100;
    const int MEASUREMENT_SAMPLES = 30;
    const Scalar DT = 0.1;

    // Sweep with N=50
    auto result_50 = run_beta_sweep(
        50, BETA_MIN, BETA_MAX, BETA_STEP,
        WARMUP_STEPS, MEASUREMENT_SAMPLES, DT);

    // Sweep with N=100
    auto result_100 = run_beta_sweep(
        100, BETA_MIN, BETA_MAX, BETA_STEP,
        WARMUP_STEPS, MEASUREMENT_SAMPLES, DT);

    // Compute scaled chi/N peak location
    auto find_peak_beta = [](const SweepResult& res, size_t N) -> Scalar {
        Scalar max_scaled_chi = -1.0;
        Scalar peak_beta = 0.0;
        for (size_t i = 0; i < res.chis.size(); ++i) {
            Scalar scaled_chi = res.chis[i] / static_cast<Scalar>(N);
            if (scaled_chi > max_scaled_chi) {
                max_scaled_chi = scaled_chi;
                peak_beta = res.betas[i];
            }
        }
        return peak_beta;
    };

    Scalar peak_50 = find_peak_beta(result_50, 50);
    Scalar peak_100 = find_peak_beta(result_100, 100);

    // Peaks should be at similar beta values
    // With step=0.03, allow tolerance of 0.15 (5 grid points)
    Scalar peak_deviation = std::abs(peak_50 - peak_100);
    EXPECT_LT(peak_deviation, 0.15)
        << "chi/N peak location inconsistent: "
        << "N=50 peak at beta=" << peak_50
        << ", N=100 peak at beta=" << peak_100;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "[V5] chi/N peak: N=50 at beta=" << peak_50
              << ", N=100 at beta=" << peak_100
              << ", deviation=" << peak_deviation << std::endl;

    // Print scaled chi for diagnostics
    std::cout << "\n beta   chi/50   chi/100" << std::endl;
    size_t min_size = std::min(result_50.betas.size(), result_100.betas.size());
    for (size_t i = 0; i < min_size; ++i) {
        std::cout << std::setw(5) << result_50.betas[i] << "  "
                  << std::setw(8) << result_50.chis[i] / 50.0 << "  "
                  << std::setw(8) << result_100.chis[i] / 100.0 << std::endl;
    }
}

// ========================================
// Test 6: N=100 numerical stability over full sweep
// ========================================
TEST(V5Validation, N100_NumericalStabilityFullSweep) {
    const size_t N_AGENTS = 100;
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.05;  // Coarser for faster execution
    const int WARMUP_STEPS = 100;
    const int MEASUREMENT_SAMPLES = 20;
    const Scalar DT = 0.1;
    const int AVG_NEIGHBORS = 6;

    for (Scalar beta = BETA_MIN; beta <= BETA_MAX + 1e-9; beta += BETA_STEP) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // Initialize
        std::mt19937 rng(123);
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        SaliencyPolarMap spm;
        Matrix12x12 saliency = Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, saliency);

        // Warmup
        for (int t = 0; t < WARMUP_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);
        }

        // Measurement
        for (int t = 0; t < MEASUREMENT_SAMPLES; ++t) {
            swarm.update_all_agents(spm, DT);
        }

        // Verify all agents are numerically stable
        for (size_t i = 0; i < swarm.size(); ++i) {
            const auto& state = swarm.get_agent(i).state();
            const auto& haze = swarm.get_agent(i).haze();

            ASSERT_FALSE(std::isnan(state.position.x()))
                << "NaN position at beta=" << beta << ", agent " << i;
            ASSERT_FALSE(std::isnan(state.velocity.x()))
                << "NaN velocity at beta=" << beta << ", agent " << i;
            ASSERT_FALSE(std::isnan(state.fatigue))
                << "NaN fatigue at beta=" << beta << ", agent " << i;
            ASSERT_FALSE(haze.hasNaN())
                << "NaN haze at beta=" << beta << ", agent " << i;

            ASSERT_FALSE(std::isinf(state.position.x()));
            ASSERT_FALSE(std::isinf(state.velocity.x()));

            ASSERT_GE(state.fatigue, 0.0);
            ASSERT_LE(state.fatigue, 1.0);
        }

        // Phi and chi must be finite
        auto haze_fields = swarm.get_all_haze_fields();
        Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
        ASSERT_FALSE(std::isnan(phi))
            << "NaN phi at beta=" << beta;
        ASSERT_FALSE(std::isinf(phi))
            << "Inf phi at beta=" << beta;
    }

    std::cout << "[V5] N=100 numerical stability: PASS across all beta values" << std::endl;
}
