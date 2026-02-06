#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>
#include <random>
#include "eph_phase/phase_analyzer.hpp"
#include "eph_swarm/swarm_manager.hpp"

using namespace eph;
using namespace eph::phase;
using namespace eph::swarm;

/**
 * @brief V2完全検証: 相転移検出（Phase 4完全版）
 *
 * ## Phase 4の目標
 * 行為選択実装により予測誤差フィードバックループが閉じ、
 * β_c ≈ 0.098での真の相転移が観測可能になります。
 *
 * ## V2検証目標
 * - β_c^empirical ∈ [0.088, 0.108]（理論値±10%）
 * - χ(β)がβ_c付近でピーク
 * - 数値安定性（NaN/Inf無し）
 *
 * ## 実験設計
 * - N = 50 agents
 * - z = 6 neighbors
 * - β ∈ [0, 0.3], step = 0.01
 * - Equilibration: 2000 steps (extended for full thermalization)
 * - Measurement: 200 steps
 * - dt = 0.1
 */
TEST(V2Complete, BetaSweep_DetectsCriticalPoint) {
    // パラメータ
    const size_t N_AGENTS = 50;
    const int AVG_NEIGHBORS = 6;
    const Scalar DT = 0.1;
    const int EQUILIBRATION_STEPS = 2000;  // Extended from 500 to ensure full equilibration
    const int MEASUREMENT_STEPS = 200;
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.01;

    std::vector<Scalar> betas, phis, chis;

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  V2 Complete Validation (Phase 4)\n";
    std::cout << "========================================\n";
    std::cout << "Parameters:\n";
    std::cout << "  N = " << N_AGENTS << "\n";
    std::cout << "  z = " << AVG_NEIGHBORS << "\n";
    std::cout << "  β ∈ [" << BETA_MIN << ", " << BETA_MAX << "] step " << BETA_STEP << "\n";
    std::cout << "  dt = " << DT << "\n";
    std::cout << "  Equilibration: " << EQUILIBRATION_STEPS << " steps\n";
    std::cout << "  Measurement: " << MEASUREMENT_STEPS << " steps\n";
    std::cout << "\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << " β      φ       χ\n";
    std::cout << "----------------------\n";

    // β掃引: 0.0 → 0.3（ステップ0.01）
    for (Scalar beta = BETA_MIN; beta <= BETA_MAX; beta += BETA_STEP) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // 初期hazeを設定（ランダムな不均一性）
        // Note: SwarmManagerは既に初期速度をランダム化しているため対称性は破れている
        std::mt19937 rng(123);  // 再現性のためシード固定
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        // 動的SPM（時間変化する勾配）
        spm::SaliencyPolarMap spm;
        Matrix12x12 dynamic_saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, dynamic_saliency);

        // 平衡化フェーズ
        for (int t = 0; t < EQUILIBRATION_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);
        }

        // 測定フェーズ
        std::vector<Scalar> phi_samples;
        phi_samples.reserve(MEASUREMENT_STEPS);

        for (int t = 0; t < MEASUREMENT_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);

            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phi_samples.push_back(phi);
        }

        // 時間平均と揺らぎ
        Scalar phi_avg = PhaseAnalyzer::mean(phi_samples);
        Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

        betas.push_back(beta);
        phis.push_back(phi_avg);
        chis.push_back(chi);

        std::cout << beta << "  " << phi_avg << "  " << chi << "\n";
    }

    std::cout << "\n";

    // β_c検出
    Scalar beta_c_empirical = PhaseAnalyzer::find_beta_c(betas, phis);
    Scalar beta_c_theory = 0.098;

    std::cout << "========================================\n";
    std::cout << "  Results\n";
    std::cout << "========================================\n";
    std::cout << "  β_c (theory):    " << beta_c_theory << "\n";
    std::cout << "  β_c (empirical): " << beta_c_empirical << "\n";
    std::cout << "  Deviation:       " << std::abs(beta_c_empirical - beta_c_theory) << "\n";
    std::cout << "  Tolerance (±10%): " << beta_c_theory * 0.1 << "\n";
    std::cout << "\n";

    // V2成功基準: β_c^empirical ∈ [0.088, 0.108]
    EXPECT_NEAR(beta_c_empirical, beta_c_theory, beta_c_theory * 0.1)
        << "CRITICAL: V2 validation failed - phase transition not detected at β_c ≈ 0.098";

    // χ(β)がβ_c付近で最大
    auto max_chi_it = std::max_element(chis.begin(), chis.end());
    size_t max_chi_idx = std::distance(chis.begin(), max_chi_it);
    Scalar beta_at_max_chi = betas[max_chi_idx];

    std::cout << "Susceptibility Check:\n";
    std::cout << "  χ_max = " << *max_chi_it << " at β = " << beta_at_max_chi << "\n";
    std::cout << "  Distance from β_c: " << std::abs(beta_at_max_chi - beta_c_empirical) << "\n";
    std::cout << "\n";

    EXPECT_NEAR(beta_at_max_chi, beta_c_empirical, 0.02)
        << "Susceptibility peak should be near critical point";

    // 数値安定性: φとχが有限
    for (Scalar phi : phis) {
        EXPECT_FALSE(std::isnan(phi));
        EXPECT_FALSE(std::isinf(phi));
    }
    for (Scalar chi : chis) {
        EXPECT_FALSE(std::isnan(chi));
        EXPECT_FALSE(std::isinf(chi));
    }

    std::cout << "========================================\n";
    std::cout << "  V2 Complete Validation: ";
    if (std::abs(beta_c_empirical - beta_c_theory) < beta_c_theory * 0.1) {
        std::cout << "SUCCESS ✓\n";
    } else {
        std::cout << "FAILED ✗\n";
    }
    std::cout << "========================================\n";
    std::cout << "\n";
}

/**
 * @brief V2補助検証: φ(β)の単調性（Phase 4版）
 *
 * Phase 4では行為選択により、β増加でφが増加または非単調（相転移）を示すはず。
 * Phase 3のようなコンセンサス（φ単調減少）ではない。
 */
TEST(V2Complete, PhiIncreases_OrNonMonotonic) {
    // 簡易β掃引
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar DT = 0.1;

    std::vector<Scalar> betas = {0.0, 0.05, 0.098, 0.15, 0.2};
    std::vector<Scalar> phis;

    for (Scalar beta : betas) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // 初期hazeを設定（ランダムな不均一性）
        std::mt19937 rng(123);
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        spm::SaliencyPolarMap spm;
        Matrix12x12 dynamic_saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, dynamic_saliency);

        // 平衡化
        for (int t = 0; t < 200; ++t) {
            swarm.update_all_agents(spm, DT);
        }

        // φ測定
        auto haze_fields = swarm.get_all_haze_fields();
        Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
        phis.push_back(phi);
    }

    // φ(β)が変化（単調減少ではない）
    Scalar phi_range = *std::max_element(phis.begin(), phis.end()) -
                       *std::min_element(phis.begin(), phis.end());

    EXPECT_GT(phi_range, 0.01)
        << "φ should vary with β (not constant)";

    // Phase 3ではφ単調減少だったが、Phase 4では非単調または増加のはず
    // ここでは変化することのみ確認
}

/**
 * @brief V2補助検証: 数値安定性（長時間実行）
 *
 * β_c付近で長時間実行しても数値爆発しないことを確認。
 */
TEST(V2Complete, NumericalStability_LongRun) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA_C = 0.098;
    const Scalar DT = 0.1;

    SwarmManager swarm(N_AGENTS, BETA_C, AVG_NEIGHBORS);

    spm::SaliencyPolarMap spm;
    Matrix12x12 dynamic_saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, dynamic_saliency);

    // 1000ステップ（100秒相当）
    for (int t = 0; t < 1000; ++t) {
        swarm.update_all_agents(spm, DT);
    }

    // 全エージェントの状態が有限
    for (size_t i = 0; i < swarm.size(); ++i) {
        auto state = swarm.get_agent(i).state();
        auto haze = swarm.get_agent(i).haze();

        EXPECT_FALSE(std::isnan(state.position.x()));
        EXPECT_FALSE(std::isnan(state.velocity.x()));
        EXPECT_FALSE(std::isnan(state.fatigue));
        EXPECT_FALSE(haze.hasNaN());

        EXPECT_FALSE(std::isinf(state.position.x()));
        EXPECT_FALSE(std::isinf(state.velocity.x()));
        EXPECT_FALSE(std::isinf(state.fatigue));
    }
}
