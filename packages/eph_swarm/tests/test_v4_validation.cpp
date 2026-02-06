#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "eph_swarm/swarm_manager.hpp"
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;
using namespace eph::swarm;
using namespace eph::spm;

/**
 * @brief V4検証: 長時間安定性検証
 *
 * ## 検証項目
 * 1. 数値爆発の防止
 *    - 位置・速度・疲労・Hazeが有限範囲に留まる
 *    - NaN/Inf の発生無し
 *
 * 2. 定常状態への収束
 *    - エージェント動態が定常状態に到達
 *    - φ、χなどの観測量が安定
 *
 * 3. 長期挙動の一貫性
 *    - 10000ステップ以上の実行で破綻しない
 *    - 数値精度の劣化無し
 */

// ========================================
// 1. 数値爆発の防止
// ========================================

TEST(V4Validation, LongRun_NoNumericalExplosion) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.098;  // Critical point
    const Scalar DT = 0.1;
    const int N_STEPS = 10000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 10000ステップ実行
    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // 定期的にチェック（100ステップごと）
        if (t % 100 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                const auto& agent = swarm.get_agent(i);
                const auto& state = agent.state();
                const auto& haze = agent.haze();

                // 位置が有限
                ASSERT_FALSE(std::isnan(state.position.x()));
                ASSERT_FALSE(std::isinf(state.position.x()));
                ASSERT_FALSE(std::isnan(state.position.y()));
                ASSERT_FALSE(std::isinf(state.position.y()));

                // 速度が有限
                ASSERT_FALSE(std::isnan(state.velocity.x()));
                ASSERT_FALSE(std::isinf(state.velocity.x()));
                ASSERT_FALSE(std::isnan(state.velocity.y()));
                ASSERT_FALSE(std::isinf(state.velocity.y()));

                // 疲労が有限
                ASSERT_FALSE(std::isnan(state.fatigue));
                ASSERT_FALSE(std::isinf(state.fatigue));
                ASSERT_GE(state.fatigue, 0.0);
                ASSERT_LE(state.fatigue, 1.0);

                // Hazeが有限
                ASSERT_FALSE(haze.hasNaN());
            }
        }
    }

    // 最終状態が妥当
    for (size_t i = 0; i < swarm.size(); ++i) {
        const auto& agent = swarm.get_agent(i);
        EXPECT_LT(agent.state().velocity.norm(), 3.0);
        EXPECT_GE(agent.haze().mean(), 0.0);
        EXPECT_LE(agent.haze().mean(), 1.0);
    }
}

TEST(V4Validation, HighBeta_LongTermStability) {
    const size_t N_AGENTS = 15;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.2;  // High coupling
    const Scalar DT = 0.1;
    const int N_STEPS = 5000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);
    }

    // 高βでも収束
    auto haze_fields = swarm.get_all_haze_fields();
    for (const auto& haze : haze_fields) {
        EXPECT_FALSE(haze.hasNaN());
        EXPECT_GE(haze.mean(), 0.0);
        EXPECT_LE(haze.mean(), 1.0);
    }
}

TEST(V4Validation, LowBeta_LongTermStability) {
    const size_t N_AGENTS = 15;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.01;  // Low coupling (independent)
    const Scalar DT = 0.1;
    const int N_STEPS = 5000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);
    }

    // 低βでも安定
    for (size_t i = 0; i < swarm.size(); ++i) {
        const auto& agent = swarm.get_agent(i);
        EXPECT_FALSE(std::isnan(agent.state().velocity.norm()));
        EXPECT_LT(agent.state().velocity.norm(), 3.0);
    }
}

// ========================================
// 2. 定常状態への収束
// ========================================

TEST(V4Validation, OrderParameter_ConvergesToSteadyState) {
    const size_t N_AGENTS = 20;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.098;
    const Scalar DT = 0.1;
    const int EQUILIBRATION = 2000;
    const int MEASUREMENT = 1000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 平衡化
    for (int t = 0; t < EQUILIBRATION; ++t) {
        swarm.update_all_agents(spm, DT);
    }

    // 測定
    std::vector<Scalar> phi_samples;
    phi_samples.reserve(MEASUREMENT);

    for (int t = 0; t < MEASUREMENT; ++t) {
        swarm.update_all_agents(spm, DT);

        auto haze_fields = swarm.get_all_haze_fields();
        Scalar phi = 0.0;
        Scalar haze_mean = 0.0;
        for (const auto& haze : haze_fields) {
            haze_mean += haze.mean();
        }
        haze_mean /= haze_fields.size();

        for (const auto& haze : haze_fields) {
            phi += std::abs(haze.mean() - haze_mean);
        }
        phi /= haze_fields.size();

        phi_samples.push_back(phi);
    }

    // 定常状態での揺らぎが小さい
    Scalar phi_mean = 0.0;
    for (Scalar phi : phi_samples) {
        phi_mean += phi;
    }
    phi_mean /= phi_samples.size();

    Scalar phi_variance = 0.0;
    for (Scalar phi : phi_samples) {
        Scalar diff = phi - phi_mean;
        phi_variance += diff * diff;
    }
    phi_variance /= phi_samples.size();
    Scalar phi_stddev = std::sqrt(phi_variance);

    // 標準偏差が平均の50%以下（定常状態）
    EXPECT_LT(phi_stddev, phi_mean * 0.5 + 0.01)
        << "Order parameter should stabilize in steady state";
}

TEST(V4Validation, VelocityDistribution_RemainsFinite) {
    const size_t N_AGENTS = 25;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.098;
    const Scalar DT = 0.1;
    const int N_STEPS = 8000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Random() * 0.5 + Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 速度分布の推移を記録
    std::vector<Scalar> max_velocities;
    std::vector<Scalar> avg_velocities;

    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        if (t % 200 == 0) {
            Scalar max_v = 0.0;
            Scalar avg_v = 0.0;

            for (size_t i = 0; i < swarm.size(); ++i) {
                Scalar v = swarm.get_agent(i).state().velocity.norm();
                max_v = std::max(max_v, v);
                avg_v += v;
            }
            avg_v /= swarm.size();

            max_velocities.push_back(max_v);
            avg_velocities.push_back(avg_v);
        }
    }

    // 全ての記録時点で速度が有限範囲
    for (Scalar max_v : max_velocities) {
        EXPECT_LT(max_v, 3.0);
        EXPECT_GT(max_v, 0.0);
    }

    for (Scalar avg_v : avg_velocities) {
        EXPECT_LT(avg_v, 2.0);
        EXPECT_GT(avg_v, 0.0);
    }

    // 後半での平均が安定（前半と大きく変わらない）
    size_t half = avg_velocities.size() / 2;
    Scalar avg_first_half = 0.0;
    Scalar avg_second_half = 0.0;

    for (size_t i = 0; i < half; ++i) {
        avg_first_half += avg_velocities[i];
    }
    avg_first_half /= half;

    for (size_t i = half; i < avg_velocities.size(); ++i) {
        avg_second_half += avg_velocities[i];
    }
    avg_second_half /= (avg_velocities.size() - half);

    // 前半と後半で大きく変わらない（定常状態）
    EXPECT_LT(std::abs(avg_first_half - avg_second_half), avg_first_half * 0.5)
        << "Average velocity should stabilize over time";
}

// ========================================
// 3. 長期挙動の一貫性
// ========================================

TEST(V4Validation, ExtremeLongRun_MaintainsConsistency) {
    const size_t N_AGENTS = 10;  // Smaller swarm for faster test
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.098;
    const Scalar DT = 0.1;
    const int N_STEPS = 15000;  // Very long run

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    SaliencyPolarMap spm;
    Matrix12x12 saliency = Matrix12x12::Constant(0.5);
    spm.set_channel(ChannelID::F2, saliency);

    // 15000ステップ実行（25分相当）
    for (int t = 0; t < N_STEPS; ++t) {
        swarm.update_all_agents(spm, DT);

        // 1000ステップごとにチェック
        if (t % 1000 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                const auto& agent = swarm.get_agent(i);
                ASSERT_FALSE(std::isnan(agent.state().velocity.x()));
                ASSERT_FALSE(agent.haze().hasNaN());
            }
        }
    }

    // 最終状態が妥当
    for (size_t i = 0; i < swarm.size(); ++i) {
        const auto& agent = swarm.get_agent(i);
        EXPECT_LT(agent.state().velocity.norm(), 3.0);
        EXPECT_GE(agent.state().fatigue, 0.0);
        EXPECT_LE(agent.state().fatigue, 1.0);
    }
}

TEST(V4Validation, DynamicEnvironment_LongTermAdaptation) {
    const size_t N_AGENTS = 15;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.098;
    const Scalar DT = 0.1;
    const int N_STEPS = 6000;

    SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);

    // SPMを動的に変化させる
    for (int t = 0; t < N_STEPS; ++t) {
        SaliencyPolarMap spm;

        // 時間変化するSaliency（周期的）
        Scalar phase = static_cast<Scalar>(t) / 1000.0;
        Matrix12x12 dynamic_saliency = Matrix12x12::Zero();
        for (int i = 0; i < 12; ++i) {
            for (int j = 0; j < 12; ++j) {
                Scalar theta = static_cast<Scalar>(i) * constants::DELTA_THETA;
                dynamic_saliency(i, j) = 0.5 + 0.3 * std::sin(phase + theta);
            }
        }
        spm.set_channel(ChannelID::F2, dynamic_saliency);

        swarm.update_all_agents(spm, DT);

        // 定期的にチェック
        if (t % 500 == 0) {
            for (size_t i = 0; i < swarm.size(); ++i) {
                const auto& agent = swarm.get_agent(i);
                ASSERT_FALSE(std::isnan(agent.state().velocity.norm()));
                ASSERT_LT(agent.state().velocity.norm(), 3.0);
            }
        }
    }

    // 動的環境でも最終的に安定
    for (size_t i = 0; i < swarm.size(); ++i) {
        EXPECT_LT(swarm.get_agent(i).state().velocity.norm(), 3.0);
    }
}

TEST(V4Validation, RepeatedRuns_ProduceConsistentBehavior) {
    const size_t N_AGENTS = 10;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA = 0.098;
    const Scalar DT = 0.1;
    const int N_STEPS = 3000;

    // 同じ条件で2回実行（決定的システムなので同じ結果になるはず）
    std::vector<Scalar> phi_run1;
    {
        SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
        SaliencyPolarMap spm;
        Matrix12x12 saliency = Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, saliency);

        for (int t = 0; t < N_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);

            if (t % 100 == 0) {
                auto haze_fields = swarm.get_all_haze_fields();
                Scalar haze_mean = 0.0;
                for (const auto& haze : haze_fields) {
                    haze_mean += haze.mean();
                }
                haze_mean /= haze_fields.size();
                phi_run1.push_back(haze_mean);
            }
        }
    }

    std::vector<Scalar> phi_run2;
    {
        SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
        SaliencyPolarMap spm;
        Matrix12x12 saliency = Matrix12x12::Constant(0.5);
        spm.set_channel(ChannelID::F2, saliency);

        for (int t = 0; t < N_STEPS; ++t) {
            swarm.update_all_agents(spm, DT);

            if (t % 100 == 0) {
                auto haze_fields = swarm.get_all_haze_fields();
                Scalar haze_mean = 0.0;
                for (const auto& haze : haze_fields) {
                    haze_mean += haze.mean();
                }
                haze_mean /= haze_fields.size();
                phi_run2.push_back(haze_mean);
            }
        }
    }

    // 2回の実行で類似の挙動（完全一致は求めないが、傾向は同じ）
    ASSERT_EQ(phi_run1.size(), phi_run2.size());

    Scalar max_diff = 0.0;
    for (size_t i = 0; i < phi_run1.size(); ++i) {
        Scalar diff = std::abs(phi_run1[i] - phi_run2[i]);
        max_diff = std::max(max_diff, diff);
    }

    // ランダム初期化があるので完全一致はしないが、大きくは違わない
    EXPECT_LT(max_diff, 0.5)
        << "Repeated runs should show consistent behavior patterns";
}
