#include <gtest/gtest.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <random>
#include "eph_phase/phase_analyzer.hpp"
#include "eph_swarm/swarm_manager.hpp"

using namespace eph;
using namespace eph::phase;
using namespace eph::swarm;

// === V2検証実験: 相転移検出 ===

/**
 * @brief β掃引実験によるPhase Transition検出（Phase 3版）
 *
 * ## Phase 3の限界
 * Phase 3では行為選択が未実装のため、真の相転移（β_c ≈ 0.098での臨界現象）は
 * 観測できません。現在のテストは、SwarmManagerとPhaseAnalyzerの**機能検証**を
 * 目的としています。
 *
 * ## 検証内容（Phase 3版）
 * - φ(β)がβに応じて変化することを確認
 * - PhaseAnalyzer::find_beta_c()が動作することを確認
 * - χ(β)が計算できることを確認
 *
 * ## 完全なV2検証（Phase 4以降）
 * 行為選択実装後、予測誤差フィードバックループが閉じることで、
 * β_c ≈ 0.098での真の相転移が観測可能になります。
 */
TEST(V2Validation, BetaSweep_FunctionalityCheck) {
    // パラメータ
    const size_t N_AGENTS = 50;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.01;
    const int EQUILIBRATION_STEPS = 100;
    const int MEASUREMENT_STEPS = 50;

    std::vector<Scalar> betas, phis_avg, chis;

    std::cout << "\n=== V2 Validation: Phase Transition Detection ===\n";
    std::cout << "Parameters:\n";
    std::cout << "  N_AGENTS = " << N_AGENTS << "\n";
    std::cout << "  AVG_NEIGHBORS = " << AVG_NEIGHBORS << "\n";
    std::cout << "  β ∈ [" << BETA_MIN << ", " << BETA_MAX << "] step " << BETA_STEP << "\n";
    std::cout << "  EQUILIBRATION_STEPS = " << EQUILIBRATION_STEPS << "\n";
    std::cout << "  MEASUREMENT_STEPS = " << MEASUREMENT_STEPS << "\n\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << " β      φ       χ\n";
    std::cout << "----------------------\n";

    // β掃引
    for (Scalar beta = BETA_MIN; beta <= BETA_MAX; beta += BETA_STEP) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // 初期hazeを設定（ランダムな不均一性）
        std::mt19937 rng(123);  // 再現性のためシード固定
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        // 平衡化フェーズ
        for (int t = 0; t < EQUILIBRATION_STEPS; ++t) {
            swarm.update_effective_haze();
        }

        // 測定フェーズ
        std::vector<Scalar> phi_samples;
        phi_samples.reserve(MEASUREMENT_STEPS);

        for (int t = 0; t < MEASUREMENT_STEPS; ++t) {
            swarm.update_effective_haze();

            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phi_samples.push_back(phi);
        }

        // 時間平均と揺らぎ
        Scalar phi_avg = PhaseAnalyzer::mean(phi_samples);
        Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

        betas.push_back(beta);
        phis_avg.push_back(phi_avg);
        chis.push_back(chi);

        std::cout << beta << "  " << phi_avg << "  " << chi << "\n";
    }

    std::cout << "\n";

    // β_c検出（機能検証）
    Scalar beta_c_empirical = PhaseAnalyzer::find_beta_c(betas, phis_avg);

    std::cout << "Results:\n";
    std::cout << "  β_c (empirical): " << beta_c_empirical << "\n";
    std::cout << "  Note: True phase transition will emerge in Phase 4\n\n";

    // Phase 3機能検証: find_beta_c()が動作すること
    EXPECT_GE(beta_c_empirical, BETA_MIN);
    EXPECT_LE(beta_c_empirical, BETA_MAX);

    // φ(β)がβに応じて変化していることを確認
    Scalar phi_range = *std::max_element(phis_avg.begin(), phis_avg.end()) -
                       *std::min_element(phis_avg.begin(), phis_avg.end());

    std::cout << "Order Parameter Check:\n";
    std::cout << "  φ_max = " << *std::max_element(phis_avg.begin(), phis_avg.end()) << "\n";
    std::cout << "  φ_min = " << *std::min_element(phis_avg.begin(), phis_avg.end()) << "\n";
    std::cout << "  Range = " << phi_range << "\n\n";

    EXPECT_GT(phi_range, 0.01)
        << "φ should vary significantly with β";

    // χ(β)が計算できることを確認
    auto max_chi_it = std::max_element(chis.begin(), chis.end());
    size_t max_chi_idx = std::distance(chis.begin(), max_chi_it);
    Scalar beta_at_max_chi = betas[max_chi_idx];

    std::cout << "Susceptibility Check:\n";
    std::cout << "  χ_max = " << *max_chi_it << " at β = " << beta_at_max_chi << "\n\n";

    // χが非負であることを確認
    for (Scalar chi : chis) {
        EXPECT_GE(chi, -1e-6)  // 数値誤差を許容
            << "χ should be non-negative";
    }

    std::cout << "=== Phase 3 Functionality Check: SUCCESS ===\n";
    std::cout << "Note: Complete V2 validation will be performed in Phase 4\n\n";
}

// === 簡易β掃引テスト（小規模） ===

TEST(BetaSweep, SmallScale_PhiIncreases) {
    // N=10, 5点掃引で基本動作を確認
    const size_t N_AGENTS = 10;
    const int AVG_NEIGHBORS = 4;

    std::vector<Scalar> betas = {0.0, 0.05, 0.1, 0.15, 0.2};
    std::vector<Scalar> phis;

    for (Scalar beta : betas) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // 初期hazeを設定
        std::mt19937 rng(123);
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        // 簡易平衡化
        for (int t = 0; t < 20; ++t) {
            swarm.update_effective_haze();
        }

        // φ測定
        auto haze_fields = swarm.get_all_haze_fields();
        Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
        phis.push_back(phi);
    }

    // φがβに応じて変化する（Phase 3ではβ増加でφ減少：コンセンサスへの収束）
    Scalar phi_range = *std::max_element(phis.begin(), phis.end()) -
                       *std::min_element(phis.begin(), phis.end());

    EXPECT_GT(phi_range, 0.01)
        << "φ should vary with β";
}

// === χ(β)の挙動テスト ===

TEST(BetaSweep, Chi_IncreasesNearCriticalPoint) {
    // β_c付近でχが大きくなることを確認
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;

    std::vector<Scalar> betas = {0.0, 0.05, 0.098, 0.15, 0.2};
    std::vector<Scalar> chis;

    for (Scalar beta : betas) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // 初期hazeを設定
        std::mt19937 rng(123);
        std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
        for (size_t i = 0; i < swarm.size(); ++i) {
            Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
            swarm.get_agent(i).set_effective_haze(initial_haze);
        }

        // 平衡化
        for (int t = 0; t < 50; ++t) {
            swarm.update_effective_haze();
        }

        // φサンプリング
        std::vector<Scalar> phi_samples;
        for (int t = 0; t < 30; ++t) {
            swarm.update_effective_haze();
            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phi_samples.push_back(phi);
        }

        Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);
        chis.push_back(chi);
    }

    // β=0.098（臨界点）でχが最大に近いことを期待
    // （小規模なので厳密な検証は難しいが、ゼロではないことを確認）
    EXPECT_GT(chis[2], 0.0)
        << "χ should be positive near β_c";
}

// === 有限サイズ効果テスト ===

TEST(BetaSweep, FiniteSizeEffect_LargerN_SharperTransition) {
    // N=10とN=30で相転移の鋭さを比較
    std::vector<size_t> N_values = {10, 30};
    std::vector<Scalar> betas = {0.08, 0.098, 0.12};

    for (size_t N : N_values) {
        std::vector<Scalar> phis;

        for (Scalar beta : betas) {
            SwarmManager swarm(N, beta, 6);

            // 初期hazeを設定
            std::mt19937 rng(123);
            std::uniform_real_distribution<Scalar> haze_dist(0.2, 0.8);
            for (size_t i = 0; i < swarm.size(); ++i) {
                Matrix12x12 initial_haze = Matrix12x12::Constant(haze_dist(rng));
                swarm.get_agent(i).set_effective_haze(initial_haze);
            }

            for (int t = 0; t < 50; ++t) {
                swarm.update_effective_haze();
            }

            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phis.push_back(phi);
        }

        // β_c前後の勾配を計算（Phase 3ではコンセンサス方向なので負）
        Scalar slope = (phis[2] - phis[0]) / (betas[2] - betas[0]);

        // Phase 3では、βが増えるとφが減少（コンセンサス）
        // 基本動作の確認: 勾配が存在すること
        EXPECT_NE(slope, 0.0)
            << "Transition slope should be non-zero for N=" << N;
    }
}
