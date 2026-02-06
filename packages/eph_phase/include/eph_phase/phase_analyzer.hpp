#ifndef EPH_PHASE_PHASE_ANALYZER_HPP
#define EPH_PHASE_PHASE_ANALYZER_HPP

#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <Eigen/Core>
#include "eph_core/types.hpp"

namespace eph::phase {

/**
 * @brief 相転移解析クラス（Phase 3）
 *
 * MB破れ強度βに対する秩序パラメータφ(β)と応答関数χ(β)を計算し、
 * 臨界点β_cを検出します。
 *
 * ## 秩序パラメータ φ(β)
 * φ = (1/N) Σᵢ |h_i - h̄|
 * - β < β_c: φ ≈ 0（無秩序相、均一なhaze）
 * - β > β_c: φ > 0（秩序相、不均一なhaze）
 *
 * ## 応答関数 χ(β)
 * χ = N(⟨φ²⟩ - ⟨φ⟩²)
 * - β = β_c で最大値（発散的ピーク）
 *
 * ## 臨界点検出
 * dφ/dβの最大値を探索してβ_cを決定
 */
class PhaseAnalyzer {
public:
    using Scalar = eph::Scalar;
    using Matrix12x12 = eph::Matrix12x12;

    /**
     * @brief 秩序パラメータφの計算
     *
     * φ = (1/N) Σᵢ |h_i - h̄|
     *
     * 各エージェントのhaze空間平均h_iと、全エージェントの平均h̄との
     * 偏差の絶対値の平均を計算します。
     *
     * @param haze_fields 全エージェントのhazeフィールド（N×12×12）
     * @return 秩序パラメータφ [0, 1]
     */
    static auto compute_phi(const std::vector<Matrix12x12>& haze_fields) -> Scalar {
        if (haze_fields.empty()) {
            return 0.0;
        }

        size_t N = haze_fields.size();

        // 1. 各hazeフィールドの空間平均h_iを計算
        std::vector<Scalar> h_means(N);
        for (size_t i = 0; i < N; ++i) {
            h_means[i] = haze_fields[i].mean();
        }

        // 2. 全エージェントの平均h̄を計算
        Scalar h_bar = std::accumulate(h_means.begin(), h_means.end(), 0.0) / static_cast<Scalar>(N);

        // 3. φ = (1/N) Σᵢ |h_i - h̄|
        Scalar phi = 0.0;
        for (Scalar h_i : h_means) {
            phi += std::abs(h_i - h_bar);
        }
        phi /= static_cast<Scalar>(N);

        return phi;
    }

    /**
     * @brief 応答関数χの計算
     *
     * χ = N(⟨φ²⟩ - ⟨φ⟩²)
     *
     * 時系列のφサンプルから揺らぎを計算します。
     * 臨界点β_cでχは最大値をとります。
     *
     * @param phi_samples 時系列のφサンプル（測定ステップ数分）
     * @return 応答関数χ
     */
    static auto compute_chi(const std::vector<Scalar>& phi_samples) -> Scalar {
        if (phi_samples.size() < 2) {
            return 0.0;
        }

        size_t M = phi_samples.size();

        // 1. ⟨φ⟩の計算（時間平均）
        Scalar phi_mean = std::accumulate(phi_samples.begin(), phi_samples.end(), 0.0) / static_cast<Scalar>(M);

        // 2. ⟨φ²⟩の計算
        Scalar phi2_mean = 0.0;
        for (Scalar phi : phi_samples) {
            phi2_mean += phi * phi;
        }
        phi2_mean /= static_cast<Scalar>(M);

        // 3. χ = M(⟨φ²⟩ - ⟨φ⟩²)
        // 注: Nではなく測定サンプル数Mを使用（時間揺らぎを測定）
        Scalar chi = static_cast<Scalar>(M) * (phi2_mean - phi_mean * phi_mean);

        return chi;
    }

    /**
     * @brief 臨界点β_cの検出
     *
     * dφ/dβが最大となるβを返します（中心差分による数値微分）。
     *
     * @param betas β値のリスト（昇順）
     * @param phis 対応するφ値のリスト
     * @return 臨界点β_c
     * @throws std::invalid_argument betasとphisのサイズが異なる、または3未満の場合
     */
    static auto find_beta_c(const std::vector<Scalar>& betas,
                            const std::vector<Scalar>& phis) -> Scalar {
        if (betas.size() != phis.size()) {
            throw std::invalid_argument("betas and phis must have the same size");
        }
        if (betas.size() < 3) {
            throw std::invalid_argument("Need at least 3 data points for derivative estimation");
        }

        // 数値微分 dφ/dβ（中心差分）
        std::vector<Scalar> derivatives;
        derivatives.reserve(betas.size() - 2);

        for (size_t i = 1; i < betas.size() - 1; ++i) {
            Scalar dPhi = phis[i + 1] - phis[i - 1];
            Scalar dBeta = betas[i + 1] - betas[i - 1];

            if (dBeta > 1e-12) {  // ゼロ除算回避
                derivatives.push_back(dPhi / dBeta);
            } else {
                derivatives.push_back(0.0);
            }
        }

        // 最大勾配点を探索
        auto max_it = std::max_element(derivatives.begin(), derivatives.end());
        size_t max_idx = std::distance(derivatives.begin(), max_it);

        // 対応するβを返す（index調整: derivatives[0] = betas[1]に対応）
        return betas[max_idx + 1];
    }

    /**
     * @brief 統計ヘルパー: 平均値計算
     */
    static auto mean(const std::vector<Scalar>& values) -> Scalar {
        if (values.empty()) return 0.0;
        return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<Scalar>(values.size());
    }

    /**
     * @brief 統計ヘルパー: 標準偏差計算
     */
    static auto stddev(const std::vector<Scalar>& values) -> Scalar {
        if (values.size() < 2) return 0.0;

        Scalar m = mean(values);
        Scalar variance = 0.0;
        for (Scalar v : values) {
            variance += (v - m) * (v - m);
        }
        variance /= static_cast<Scalar>(values.size() - 1);

        return std::sqrt(variance);
    }

    /**
     * @brief β掃引データをCSVファイルに出力
     *
     * ## 出力形式
     * ```
     * beta,phi,chi
     * 0.000,0.123,0.456
     * 0.010,0.145,0.523
     * ...
     * ```
     *
     * 可視化ツール（Pythonスクリプト等）で使用可能なCSV形式で出力します。
     *
     * @param filename 出力ファイル名（例: "phase_transition.csv"）
     * @param betas β値のリスト
     * @param phis φ値のリスト
     * @param chis χ値のリスト
     * @return true: 成功, false: 失敗
     * @throws std::invalid_argument サイズが一致しない場合
     */
    static auto export_csv(
        const std::string& filename,
        const std::vector<Scalar>& betas,
        const std::vector<Scalar>& phis,
        const std::vector<Scalar>& chis
    ) -> bool {
        // サイズチェック
        if (betas.size() != phis.size() || betas.size() != chis.size()) {
            throw std::invalid_argument("betas, phis, and chis must have the same size");
        }

        if (betas.empty()) {
            std::cerr << "Warning: No data to export\n";
            return false;
        }

        // ファイルを開く
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing\n";
            return false;
        }

        // ヘッダー行
        file << "beta,phi,chi\n";

        // データ行（高精度出力: 小数点以下6桁）
        file << std::fixed << std::setprecision(6);
        for (size_t i = 0; i < betas.size(); ++i) {
            file << betas[i] << "," << phis[i] << "," << chis[i] << "\n";
        }

        file.close();

        std::cout << "CSV exported successfully: " << filename << " (" << betas.size() << " data points)\n";
        return true;
    }
};

}  // namespace eph::phase

#endif  // EPH_PHASE_PHASE_ANALYZER_HPP
