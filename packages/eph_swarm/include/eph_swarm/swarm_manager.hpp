#ifndef EPH_SWARM_SWARM_MANAGER_HPP
#define EPH_SWARM_SWARM_MANAGER_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include <Eigen/Core>
#include "eph_core/types.hpp"
#include "eph_core/constants.hpp"
#include "eph_agent/eph_agent.hpp"
#include "eph_spm/saliency_polar_map.hpp"

namespace eph::swarm {

/**
 * @brief マルチエージェント群管理クラス（Phase 4完全版）
 *
 * N個のEPHAgentを管理し、動的更新とMB破れ（Markov Blanket Breaking）を適用します。
 * 各エージェントの行為選択、状態更新、近傍情報共有を統合管理します。
 *
 * ## Markov Blanket Breaking
 * h_eff,i = (1-β)h_i + β⟨h_j⟩_{j∈N_i}
 *
 * - β = 0: 完全分離（各エージェント独立）
 * - β = β_c ≈ 0.098: 臨界点（Edge of Chaos）
 * - β → 1: 完全情報共有（コンセンサス）
 */
class SwarmManager {
public:
    using Scalar = eph::Scalar;
    using Vec2 = eph::Vec2;
    using Matrix12x12 = eph::Matrix12x12;

    /**
     * @brief コンストラクタ
     * @param n_agents エージェント数（N=50推奨）
     * @param beta MB破れ強度 [0, 1]
     * @param avg_neighbors 平均近傍数（z=6推奨）
     */
    SwarmManager(size_t n_agents, Scalar beta, int avg_neighbors)
        : beta_(beta)
        , avg_neighbors_(avg_neighbors)
    {
        agents_.reserve(n_agents);
        positions_.resize(n_agents);

        // エージェント初期化（ランダム配置・ランダム速度）
        std::mt19937 rng(42);  // 再現性のためシード固定
        std::uniform_real_distribution<Scalar> pos_dist(-10.0, 10.0);
        std::uniform_real_distribution<Scalar> vel_mag_dist(0.3, 1.0);  // 初期速度大きさ
        std::uniform_real_distribution<Scalar> vel_angle_dist(0.0, 2.0 * constants::PI);  // 方向

        for (size_t i = 0; i < n_agents; ++i) {
            AgentState state;
            state.position = Vec2(pos_dist(rng), pos_dist(rng));

            // ランダム速度（対称性を破る）
            Scalar speed = vel_mag_dist(rng);
            Scalar angle = vel_angle_dist(rng);
            state.velocity = Vec2(speed * std::cos(angle), speed * std::sin(angle));

            state.kappa = 1.0;
            state.fatigue = 0.0;

            agents_.push_back(std::make_unique<agent::EPHAgent>(state, 1.0));
            positions_[i] = state.position;
        }
    }

    /**
     * @brief β値設定
     * @param beta MB破れ強度 [0, 1]
     */
    void set_beta(Scalar beta) {
        beta_ = beta;
    }

    /**
     * @brief β値取得
     * @return 現在のβ値
     */
    auto get_beta() const -> Scalar {
        return beta_;
    }

    /**
     * @brief 全エージェントの状態更新 + MB破れ適用（Phase 4完全版）
     *
     * ## アルゴリズム
     * 1. 各エージェントのupdate()を呼び出し（並列可能）
     * 2. エージェント位置を同期
     * 3. MB破れを適用（近傍hazeミキシング）
     *
     * Phase 4で予測誤差フィードバックループが閉じ、真の相転移が観測可能になります。
     *
     * @param spm 共通のSaliency Polar Map（全エージェント共有）
     * @param dt タイムステップ [s]（推奨: 0.1）
     */
    void update_all_agents(const spm::SaliencyPolarMap& spm, Scalar dt) {
        if (agents_.empty()) return;

        // Stage 1: 各エージェントの状態更新
        for (size_t i = 0; i < agents_.size(); ++i) {
            agents_[i]->update(spm, dt);

            // Stage 2: 位置同期
            positions_[i] = agents_[i]->state().position;
        }

        // Stage 3: MB破れ適用
        update_effective_haze();
    }

    /**
     * @brief MB破れ適用
     *
     * 全エージェントに対して効果的hazeを計算・適用します。
     * h_eff,i = (1-β)h_i + β⟨h_j⟩_{j∈N_i}
     *
     * ## アルゴリズム
     * 1. 各エージェントの近傍平均hazeを計算
     * 2. MB破れ式を適用して効果的hazeを設定
     *
     * stop-gradientにより、haze推定器の内部状態は汚染されません。
     */
    void update_effective_haze() {
        if (agents_.empty()) return;

        // Stage 1: 各エージェントの近傍平均hazeを計算
        std::vector<Matrix12x12> neighbor_avg(agents_.size());

        for (size_t i = 0; i < agents_.size(); ++i) {
            auto neighbors = find_neighbors(i);

            if (neighbors.empty()) {
                // 近傍がない場合は自分自身のhazeを使用
                neighbor_avg[i] = agents_[i]->haze();
                continue;
            }

            Matrix12x12 avg = Matrix12x12::Zero();
            for (size_t j : neighbors) {
                avg += agents_[j]->haze();
            }
            avg /= static_cast<Scalar>(neighbors.size());
            neighbor_avg[i] = avg;
        }

        // Stage 2: h_eff,i = (1-β)h_i + β⟨h_j⟩ を適用
        for (size_t i = 0; i < agents_.size(); ++i) {
            auto h_i = agents_[i]->haze();
            auto h_eff = (1.0 - beta_) * h_i + beta_ * neighbor_avg[i];
            agents_[i]->set_effective_haze(h_eff.eval());  // stop-gradient
        }
    }

    /**
     * @brief 近傍検索（k-NN）
     *
     * エージェントiの最近傍k個を距離順に返します（Phase 3ではO(N²)実装）。
     *
     * @param agent_id エージェントID
     * @return 近傍エージェントIDのリスト（距離順）
     */
    auto find_neighbors(size_t agent_id) const -> std::vector<size_t> {
        std::vector<size_t> neighbors;
        if (agents_.empty() || agent_id >= agents_.size()) {
            return neighbors;
        }

        Vec2 pos = positions_[agent_id];

        // 距離計算（自分自身を除く）
        std::vector<std::pair<Scalar, size_t>> distances;
        distances.reserve(agents_.size() - 1);

        for (size_t j = 0; j < agents_.size(); ++j) {
            if (j == agent_id) continue;
            Scalar dist = (positions_[j] - pos).norm();
            distances.push_back({dist, j});
        }

        // k-NN選択（partial_sortで効率化）
        int k = std::min(avg_neighbors_, static_cast<int>(distances.size()));
        if (k > 0) {
            std::partial_sort(
                distances.begin(),
                distances.begin() + k,
                distances.end(),
                [](const auto& a, const auto& b) {
                    return a.first < b.first;
                }
            );

            neighbors.reserve(k);
            for (int i = 0; i < k; ++i) {
                neighbors.push_back(distances[i].second);
            }
        }

        return neighbors;
    }

    /**
     * @brief エージェント取得（非const）
     * @param i エージェントID
     * @return エージェント参照
     */
    auto get_agent(size_t i) -> agent::EPHAgent& {
        return *agents_[i];
    }

    /**
     * @brief エージェント取得（const）
     * @param i エージェントID
     * @return エージェント参照（const）
     */
    auto get_agent(size_t i) const -> const agent::EPHAgent& {
        return *agents_[i];
    }

    /**
     * @brief エージェント数取得
     * @return エージェント数
     */
    auto size() const -> size_t {
        return agents_.size();
    }

    /**
     * @brief 全hazeフィールド取得
     *
     * Phase解析（PhaseAnalyzer）で使用します。
     *
     * @return 全エージェントのhazeフィールド
     */
    auto get_all_haze_fields() const -> std::vector<Matrix12x12> {
        std::vector<Matrix12x12> fields;
        fields.reserve(agents_.size());
        for (const auto& agent : agents_) {
            fields.push_back(agent->haze());
        }
        return fields;
    }

    /**
     * @brief エージェント位置更新
     *
     * Phase 4（行為決定実装後）で、エージェントの移動に応じて位置を更新します。
     * Phase 3では位置は固定です。
     *
     * @param agent_id エージェントID
     * @param new_position 新しい位置
     */
    void update_position(size_t agent_id, const Vec2& new_position) {
        if (agent_id < positions_.size()) {
            positions_[agent_id] = new_position;
        }
    }

private:
    std::vector<std::unique_ptr<agent::EPHAgent>> agents_;  // エージェント群
    std::vector<Vec2> positions_;                           // エージェント位置
    Scalar beta_;                                           // MB破れ強度
    int avg_neighbors_;                                     // 平均近傍数
};

}  // namespace eph::swarm

#endif  // EPH_SWARM_SWARM_MANAGER_HPP
