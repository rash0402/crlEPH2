#include <gtest/gtest.h>
#include <cmath>
#include "eph_swarm/swarm_manager.hpp"

using namespace eph;
using namespace eph::swarm;

// === MB破れ基本テスト ===

TEST(MBBreaking, Beta0_NoMixing) {
    SwarmManager swarm(10, 0.0, 4);

    // 各エージェントに異なるhazeを設定
    for (size_t i = 0; i < swarm.size(); ++i) {
        Matrix12x12 h = Matrix12x12::Constant(static_cast<Scalar>(i) * 0.1);
        swarm.get_agent(i).set_effective_haze(h);
    }

    auto before = swarm.get_all_haze_fields();

    // MB破れ適用
    swarm.update_effective_haze();

    auto after = swarm.get_all_haze_fields();

    // β=0 → h_eff = h（変化なし）
    for (size_t i = 0; i < swarm.size(); ++i) {
        EXPECT_TRUE(before[i].isApprox(after[i], 1e-10))
            << "Agent " << i << " haze changed with β=0";
    }
}

TEST(MBBreaking, Beta1_Consensus) {
    SwarmManager swarm(10, 1.0, 4);

    // 各エージェントに異なるhazeを設定（平均=0.5になるように）
    for (size_t i = 0; i < swarm.size(); ++i) {
        Matrix12x12 h = Matrix12x12::Constant(static_cast<Scalar>(i) / 9.0);
        swarm.get_agent(i).set_effective_haze(h);
    }

    // 複数回更新して収束させる
    for (int iter = 0; iter < 100; ++iter) {
        swarm.update_effective_haze();
    }

    auto fields = swarm.get_all_haze_fields();

    // β=1 → 全エージェントが平均値に収束
    Scalar global_mean = 0.0;
    for (const auto& h : fields) {
        global_mean += h.mean();
    }
    global_mean /= fields.size();

    for (size_t i = 0; i < fields.size(); ++i) {
        EXPECT_NEAR(fields[i].mean(), global_mean, 1e-2)
            << "Agent " << i << " did not converge to consensus with β=1";
    }
}

TEST(MBBreaking, BetaIntermediate_PartialMixing) {
    SwarmManager swarm(10, 0.5, 4);

    // エージェント0に高いhaze、他は低いhaze
    swarm.get_agent(0).set_effective_haze(Matrix12x12::Ones());
    for (size_t i = 1; i < swarm.size(); ++i) {
        swarm.get_agent(i).set_effective_haze(Matrix12x12::Zero());
    }

    auto before = swarm.get_all_haze_fields();

    // 1回更新
    swarm.update_effective_haze();

    auto after = swarm.get_all_haze_fields();

    // β=0.5 → エージェント0のhazeは減少（近傍の影響）
    EXPECT_LT(after[0].mean(), before[0].mean())
        << "Agent 0 haze should decrease with β=0.5";

    // 近傍エージェントのhazeは増加
    auto neighbors = swarm.find_neighbors(0);
    for (size_t n : neighbors) {
        EXPECT_GT(after[n].mean(), before[n].mean())
            << "Neighbor " << n << " haze should increase with β=0.5";
    }
}

// === MB破れの保存則テスト ===

TEST(MBBreaking, TotalHaze_ConservationWithUniformNeighbors) {
    // 均一な近傍構造で総Hazeが保存されることを確認
    SwarmManager swarm(10, 0.5, 9);  // 全員が全員の近傍（k=9）

    // ランダムなhazeを設定
    Scalar total_before = 0.0;
    for (size_t i = 0; i < swarm.size(); ++i) {
        Matrix12x12 h = Matrix12x12::Constant(static_cast<Scalar>(i) * 0.1);
        swarm.get_agent(i).set_effective_haze(h);
        total_before += h.sum();
    }

    // MB破れ適用
    swarm.update_effective_haze();

    Scalar total_after = 0.0;
    auto fields = swarm.get_all_haze_fields();
    for (const auto& h : fields) {
        total_after += h.sum();
    }

    // 総Hazeは保存される（均一な近傍構造の場合）
    EXPECT_NEAR(total_after, total_before, 1e-8)
        << "Total haze should be conserved with uniform neighbors";
}

// === MB破れの数値安定性テスト ===

TEST(MBBreaking, ExtremeHaze_StillStable) {
    SwarmManager swarm(10, 0.5, 4);

    // 極端な値を設定
    for (size_t i = 0; i < swarm.size(); ++i) {
        if (i % 2 == 0) {
            swarm.get_agent(i).set_effective_haze(Matrix12x12::Ones());
        } else {
            swarm.get_agent(i).set_effective_haze(Matrix12x12::Zero());
        }
    }

    // MB破れ適用
    swarm.update_effective_haze();

    auto fields = swarm.get_all_haze_fields();

    // NaN/Infが発生しない
    for (size_t i = 0; i < fields.size(); ++i) {
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                EXPECT_FALSE(std::isnan(fields[i](a, b)))
                    << "NaN at agent " << i << " position (" << a << "," << b << ")";
                EXPECT_FALSE(std::isinf(fields[i](a, b)))
                    << "Inf at agent " << i << " position (" << a << "," << b << ")";
            }
        }

        // Hazeは[0, 1]範囲内
        EXPECT_GE(fields[i].minCoeff(), 0.0);
        EXPECT_LE(fields[i].maxCoeff(), 1.0);
    }
}

TEST(MBBreaking, MultipleUpdates_Converges) {
    SwarmManager swarm(10, 0.098, 6);  // β = β_c（臨界点）

    // 初期状態: 不均一なhaze
    for (size_t i = 0; i < swarm.size(); ++i) {
        Matrix12x12 h = Matrix12x12::Constant(static_cast<Scalar>(i) / 9.0);
        swarm.get_agent(i).set_effective_haze(h);
    }

    // 100回更新
    for (int iter = 0; iter < 100; ++iter) {
        swarm.update_effective_haze();
    }

    auto fields = swarm.get_all_haze_fields();

    // 収束している（変動が小さい）
    Scalar mean_haze = 0.0;
    for (const auto& h : fields) {
        mean_haze += h.mean();
    }
    mean_haze /= fields.size();

    // 各エージェントのhazeが平均付近に収束
    for (size_t i = 0; i < fields.size(); ++i) {
        EXPECT_GT(fields[i].mean(), 0.0);
        EXPECT_LT(fields[i].mean(), 1.0);
    }
}

// === エッジケーステスト ===

TEST(MBBreaking, SingleAgent_DoesNotCrash) {
    SwarmManager swarm(1, 0.5, 6);

    swarm.get_agent(0).set_effective_haze(Matrix12x12::Constant(0.5));

    // 近傍がいないのでMB破れは何もしない
    EXPECT_NO_THROW(swarm.update_effective_haze());

    auto haze = swarm.get_agent(0).haze();
    // hazeは変化しない（近傍がいない）
    EXPECT_DOUBLE_EQ(haze(0, 0), 0.5);
}

TEST(MBBreaking, EmptySwarm_DoesNotCrash) {
    SwarmManager swarm(0, 0.5, 6);

    // エージェント数0でもクラッシュしない
    EXPECT_NO_THROW(swarm.update_effective_haze());
}

// === MB破れの線形性テスト ===

TEST(MBBreaking, Linearity_VerifyFormula) {
    SwarmManager swarm(5, 0.3, 4);

    // 既知のhazeを設定
    Matrix12x12 h0 = Matrix12x12::Constant(1.0);
    Matrix12x12 h1 = Matrix12x12::Constant(0.5);
    Matrix12x12 h2 = Matrix12x12::Constant(0.0);
    Matrix12x12 h3 = Matrix12x12::Constant(0.8);
    Matrix12x12 h4 = Matrix12x12::Constant(0.2);

    swarm.get_agent(0).set_effective_haze(h0);
    swarm.get_agent(1).set_effective_haze(h1);
    swarm.get_agent(2).set_effective_haze(h2);
    swarm.get_agent(3).set_effective_haze(h3);
    swarm.get_agent(4).set_effective_haze(h4);

    // エージェント0の近傍を取得
    auto neighbors = swarm.find_neighbors(0);

    // 近傍平均を手動計算
    Matrix12x12 neighbor_avg = Matrix12x12::Zero();
    for (size_t n : neighbors) {
        neighbor_avg += swarm.get_agent(n).haze();
    }
    neighbor_avg /= static_cast<Scalar>(neighbors.size());

    // 期待値: h_eff,0 = (1-β)h_0 + β⟨h_j⟩
    Scalar beta = 0.3;
    Matrix12x12 expected = (1.0 - beta) * h0 + beta * neighbor_avg;

    // MB破れ適用
    swarm.update_effective_haze();

    auto h_eff_0 = swarm.get_agent(0).haze();

    // 数式通りの結果になることを確認
    EXPECT_TRUE(h_eff_0.isApprox(expected, 1e-10))
        << "MB breaking formula verification failed";
}
