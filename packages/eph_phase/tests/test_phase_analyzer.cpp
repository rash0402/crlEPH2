#include <gtest/gtest.h>
#include <cmath>
#include <fstream>
#include <string>
#include "eph_phase/phase_analyzer.hpp"

using namespace eph;
using namespace eph::phase;

// === 秩序パラメータφテスト ===

TEST(PhaseAnalyzer, ComputePhi_HomogeneousHaze_ReturnsZero) {
    // 全エージェントのhazeが同じ → φ = 0
    std::vector<Matrix12x12> haze_fields(10, Matrix12x12::Constant(0.5));

    Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);

    EXPECT_NEAR(phi, 0.0, 1e-10)
        << "Homogeneous haze should give φ = 0";
}

TEST(PhaseAnalyzer, ComputePhi_HeterogeneousHaze_ReturnsPositive) {
    // エージェント間でhazeが異なる → φ > 0
    std::vector<Matrix12x12> haze_fields(10);
    for (size_t i = 0; i < haze_fields.size(); ++i) {
        haze_fields[i] = Matrix12x12::Constant(static_cast<Scalar>(i) / 10.0);
    }

    Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);

    EXPECT_GT(phi, 0.01)
        << "Heterogeneous haze should give φ > 0";
}

TEST(PhaseAnalyzer, ComputePhi_BinaryHaze_MaximumPhi) {
    // 半分が0、半分が1 → φが最大
    std::vector<Matrix12x12> haze_fields(10);
    for (size_t i = 0; i < 5; ++i) {
        haze_fields[i] = Matrix12x12::Zero();
    }
    for (size_t i = 5; i < 10; ++i) {
        haze_fields[i] = Matrix12x12::Ones();
    }

    Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);

    // h̄ = 0.5, φ = (1/10)(5×0.5 + 5×0.5) = 0.5
    EXPECT_NEAR(phi, 0.5, 1e-6)
        << "Binary haze should give φ = 0.5";
}

TEST(PhaseAnalyzer, ComputePhi_EmptyInput_ReturnsZero) {
    std::vector<Matrix12x12> empty_fields;

    Scalar phi = PhaseAnalyzer::compute_phi(empty_fields);

    EXPECT_DOUBLE_EQ(phi, 0.0);
}

// === 応答関数χテスト ===

TEST(PhaseAnalyzer, ComputeChi_ConstantPhi_ReturnsZero) {
    // φが一定 → 揺らぎなし → χ = 0
    std::vector<Scalar> phi_samples(100, 0.5);

    Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

    EXPECT_NEAR(chi, 0.0, 1e-10)
        << "Constant φ should give χ = 0";
}

TEST(PhaseAnalyzer, ComputeChi_FluctuatingPhi_ReturnsPositive) {
    // φが揺らぐ → χ > 0
    std::vector<Scalar> phi_samples;
    for (int i = 0; i < 100; ++i) {
        phi_samples.push_back(0.5 + 0.1 * std::sin(i * 0.1));
    }

    Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

    EXPECT_GT(chi, 0.0)
        << "Fluctuating φ should give χ > 0";
}

TEST(PhaseAnalyzer, ComputeChi_BinaryFluctuation_LargeChi) {
    // φが0と1を交互に取る → 大きな揺らぎ → χが大きい
    std::vector<Scalar> phi_samples;
    for (int i = 0; i < 100; ++i) {
        phi_samples.push_back((i % 2 == 0) ? 0.0 : 1.0);
    }

    Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

    // ⟨φ⟩ = 0.5, ⟨φ²⟩ = 0.5, χ = M(0.5 - 0.25) = 25M
    // しかし実際の数値を確認
    EXPECT_GT(chi, 10.0)
        << "Binary fluctuation should give large χ";
}

TEST(PhaseAnalyzer, ComputeChi_InsufficientSamples_ReturnsZero) {
    std::vector<Scalar> phi_samples = {0.5};

    Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

    EXPECT_DOUBLE_EQ(chi, 0.0)
        << "Single sample should give χ = 0";
}

// === β_c検出テスト ===

TEST(PhaseAnalyzer, FindBetaC_SyntheticTransition_DetectsCorrectly) {
    // Sigmoid型の合成相転移
    std::vector<Scalar> betas, phis;

    for (Scalar beta = 0.0; beta <= 0.3; beta += 0.01) {
        betas.push_back(beta);
        // φ(β) = 1 / (1 + exp(-50(β - 0.1)))
        Scalar phi = 1.0 / (1.0 + std::exp(-50.0 * (beta - 0.1)));
        phis.push_back(phi);
    }

    Scalar beta_c = PhaseAnalyzer::find_beta_c(betas, phis);

    // 理論値0.1付近を検出
    EXPECT_NEAR(beta_c, 0.1, 0.02)
        << "Synthetic transition at β=0.1 should be detected";
}

TEST(PhaseAnalyzer, FindBetaC_LinearTransition_DetectsMiddle) {
    // 線形相転移: φ(β) = β
    std::vector<Scalar> betas, phis;

    for (Scalar beta = 0.0; beta <= 1.0; beta += 0.1) {
        betas.push_back(beta);
        phis.push_back(beta);
    }

    Scalar beta_c = PhaseAnalyzer::find_beta_c(betas, phis);

    // 線形なので勾配は一定、中央付近を返す（実装依存）
    EXPECT_GE(beta_c, 0.0);
    EXPECT_LE(beta_c, 1.0);
}

TEST(PhaseAnalyzer, FindBetaC_InsufficientData_Throws) {
    std::vector<Scalar> betas = {0.0, 0.1};
    std::vector<Scalar> phis = {0.0, 0.5};

    // 3点未満でエラー
    EXPECT_THROW(PhaseAnalyzer::find_beta_c(betas, phis), std::invalid_argument);
}

TEST(PhaseAnalyzer, FindBetaC_MismatchedSize_Throws) {
    std::vector<Scalar> betas = {0.0, 0.1, 0.2};
    std::vector<Scalar> phis = {0.0, 0.5};

    // サイズ不一致でエラー
    EXPECT_THROW(PhaseAnalyzer::find_beta_c(betas, phis), std::invalid_argument);
}

// === 統計ヘルパーテスト ===

TEST(PhaseAnalyzer, Mean_CalculatesCorrectly) {
    std::vector<Scalar> values = {1.0, 2.0, 3.0, 4.0, 5.0};

    Scalar m = PhaseAnalyzer::mean(values);

    EXPECT_DOUBLE_EQ(m, 3.0);
}

TEST(PhaseAnalyzer, Mean_EmptyInput_ReturnsZero) {
    std::vector<Scalar> empty;

    Scalar m = PhaseAnalyzer::mean(empty);

    EXPECT_DOUBLE_EQ(m, 0.0);
}

TEST(PhaseAnalyzer, Stddev_CalculatesCorrectly) {
    std::vector<Scalar> values = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    Scalar s = PhaseAnalyzer::stddev(values);

    // 標準偏差の理論値（不偏分散）
    EXPECT_NEAR(s, 2.138, 0.01);
}

TEST(PhaseAnalyzer, Stddev_ConstantValues_ReturnsZero) {
    std::vector<Scalar> values(10, 5.0);

    Scalar s = PhaseAnalyzer::stddev(values);

    EXPECT_NEAR(s, 0.0, 1e-10);
}

TEST(PhaseAnalyzer, Stddev_InsufficientData_ReturnsZero) {
    std::vector<Scalar> values = {5.0};

    Scalar s = PhaseAnalyzer::stddev(values);

    EXPECT_DOUBLE_EQ(s, 0.0);
}

// === 統合テスト ===

TEST(PhaseAnalyzer, Integration_PhiAndChi_Workflow) {
    // φとχの典型的な使用フロー
    std::vector<Scalar> phi_samples;

    // 時系列のφサンプルを生成（β_c付近で揺らぎが大きい想定）
    for (int t = 0; t < 50; ++t) {
        // 合成データ: φ ≈ 0.3 ± 0.1
        Scalar phi = 0.3 + 0.1 * std::sin(t * 0.2);
        phi_samples.push_back(phi);
    }

    Scalar phi_mean = PhaseAnalyzer::mean(phi_samples);
    Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

    EXPECT_NEAR(phi_mean, 0.3, 0.05);
    EXPECT_GT(chi, 0.0);
}

// === CSV出力テスト ===

TEST(PhaseAnalyzer, ExportCSV_ValidData_CreatesFile) {
    // 合成β掃引データ
    std::vector<Scalar> betas = {0.00, 0.05, 0.10, 0.15, 0.20};
    std::vector<Scalar> phis = {0.05, 0.12, 0.35, 0.52, 0.61};
    std::vector<Scalar> chis = {1.2, 3.5, 8.7, 5.2, 2.1};

    // CSV出力
    std::string filename = "/private/tmp/claude-502/-Users-igarashi-local-project-workspace-crlEPH2/dae4bfa4-c0a3-4385-b282-1f7370c36ce1/scratchpad/test_phase_export.csv";
    bool success = PhaseAnalyzer::export_csv(filename, betas, phis, chis);

    EXPECT_TRUE(success) << "CSV export should succeed";

    // ファイルが存在することを確認
    std::ifstream file(filename);
    EXPECT_TRUE(file.is_open()) << "CSV file should exist";

    if (file.is_open()) {
        // ヘッダー行を読む
        std::string header;
        std::getline(file, header);
        EXPECT_EQ(header, "beta,phi,chi");

        // データ行の数を確認
        int line_count = 0;
        std::string line;
        while (std::getline(file, line)) {
            line_count++;
        }
        EXPECT_EQ(line_count, 5);

        file.close();
    }
}

TEST(PhaseAnalyzer, ExportCSV_EmptyData_ReturnsFalse) {
    std::vector<Scalar> empty_betas, empty_phis, empty_chis;

    std::string filename = "/private/tmp/claude-502/-Users-igarashi-local-project-workspace-crlEPH2/dae4bfa4-c0a3-4385-b282-1f7370c36ce1/scratchpad/test_empty_export.csv";
    bool success = PhaseAnalyzer::export_csv(filename, empty_betas, empty_phis, empty_chis);

    EXPECT_FALSE(success) << "Export should fail for empty data";
}

TEST(PhaseAnalyzer, ExportCSV_MismatchedSizes_ThrowsException) {
    std::vector<Scalar> betas = {0.0, 0.1};
    std::vector<Scalar> phis = {0.1, 0.2, 0.3};  // Different size
    std::vector<Scalar> chis = {1.0, 2.0};

    std::string filename = "/private/tmp/claude-502/-Users-igarashi-local-project-workspace-crlEPH2/dae4bfa4-c0a3-4385-b282-1f7370c36ce1/scratchpad/test_mismatched_export.csv";

    EXPECT_THROW(
        PhaseAnalyzer::export_csv(filename, betas, phis, chis),
        std::invalid_argument
    );
}
