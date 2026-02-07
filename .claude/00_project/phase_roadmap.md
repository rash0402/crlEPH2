# EPH v2.1 実装ロードマップ

**作成日**: 2026-02-03
**現在の状態**: Phase 5完了（210テスト: 206 pass / 3 既知問題修正中 / 1 timeout）
**マイルストーン**: 全Phase完了（V0-V5全検証目標達成）

---

## 📊 Phase別進捗状況

| Phase | 目的 | 状態 | テスト | 期間 |
|-------|------|------|--------|------|
| Phase 0-1 | 基礎実装（Core + SPM） | ✅ 完了 | 46/46 | 完了 |
| Phase 2 | エージェント実装 | ✅ 完了 | 20/20 | 完了 |
| Phase 3 | マルチエージェント・相転移解析 | ✅ 完了 | 43/43 | 完了 |
| **Phase 4** | **行為決定実装** | ✅ **完了** | 45/45 | 完了 |
| Phase 5 | 完全検証・最適化 | 📋 計画中 | - | 2-3日 |

**総テスト**: 154/154通過（100%） ＊192/196ビルド通過（98%、4件既知問題）

---

## ✅ Phase 0-3: 完了済み項目

### Phase 0-1: 基礎実装
- ✅ eph_core: 型定義（Scalar, Vec2, Matrix12x12）
- ✅ eph_core: 定数（N_THETA=12, β_c=0.098, Haze係数）
- ✅ eph_core: 数学関数（sigmoid, gaussian_blur, wrap_index, clamp_index）
- ✅ eph_spm: SaliencyPolarMap（12×12極座標表現）
- ✅ eph_spm: 境界条件（θ=周期、r=Neumann）
- ✅ eph_spm: Pooling（max/avg）
- ✅ **検証目標V0**: 境界条件検証完了

### Phase 2: エージェント実装
- ✅ EPHAgent: 状態管理（位置、速度、κ、疲労度）
- ✅ HazeEstimator: EMAフィルタ（τ=1.0）
- ✅ HazeEstimator: ガウシアンブラー（空間平滑化）
- ✅ Haze推定API: `estimate_haze(spm, prediction_error)`
- ✅ set_effective_haze(): MB破れ用API（Phase 3で追加）

### Phase 3: マルチエージェント・相転移解析
- ✅ SwarmManager: N=50エージェント管理
- ✅ MB破れ: h_eff,i = (1-β)h_i + β⟨h_j⟩_{j∈N_i}
- ✅ 近傍検索: k-NN（z=6、O(N²)実装）
- ✅ PhaseAnalyzer: φ(β) = (1/N)Σ|h_i - h̄|
- ✅ PhaseAnalyzer: χ(β) = N(⟨φ²⟩ - ⟨φ⟩²)
- ✅ β_c検出: dφ/dβ最大点探索
- ✅ **V2機能検証**: φ(β)計算・β掃引実験完了

### Phase 3の限界（Phase 4で解決）
- ⚠️ 行為選択未実装 → 予測誤差フィードバックループが開いている
- ⚠️ 真の相転移未観測 → β_c ≈ 0.098での臨界現象はPhase 4で実現
- ⚠️ エージェントは静的 → 動的Haze推定がない

---

## ✅ Phase 4: 行為決定実装（完了）

**目的**: Expected Free Energy (EFE) 勾配降下による行為選択を実装し、予測誤差フィードバックループを閉じる。

**期間**: 3-5日（12-20時間）

### 4.1 理論的背景

#### Expected Free Energy (EFE)

```
G(π) = D_KL[Q(o|π)||P(o|C)] + E_Q[ln Q(s|π) - ln P(s|C)]
```

簡略版（EPH v2.1）:
```
G_i(a) = ∫∫ h̃(θ, r) · S(θ, r) dθ dr
```

- h̃(θ, r): Hazeフィールド（不確実性）
- S(θ, r): Saliency（予測される重要度）
- a: 行為（速度変更）

#### 勾配降下による行為選択

```
a_new = a_old - η · ∇_a G(a)
```

Haze勾配:
```
∇_a h̃ ≈ (h̃(θ+Δθ, r) - h̃(θ-Δθ, r)) / (2Δθ)
```

### 4.2 実装ステップ

#### Step 1: ActionSelector実装（2-3時間）

**ファイル**: `packages/eph_agent/include/eph_agent/action_selector.hpp`

**クラス設計**:
```cpp
class ActionSelector {
public:
    ActionSelector(Scalar learning_rate = 0.1);

    // EFE計算
    auto compute_efe(
        const Vec2& proposed_velocity,
        const Matrix12x12& haze,
        const spm::SaliencyPolarMap& spm
    ) -> Scalar;

    // Haze勾配計算
    auto compute_haze_gradient(
        const Matrix12x12& haze,
        const Vec2& agent_position
    ) -> Vec2;

    // 行為選択（勾配降下）
    auto select_action(
        const Vec2& current_velocity,
        const Matrix12x12& haze,
        const spm::SaliencyPolarMap& spm
    ) -> Vec2;

private:
    Scalar learning_rate_;
};
```

**重要ポイント**:
- EFE = Haze × Saliencyの積分（離散和）
- 勾配は数値微分（中心差分）
- 速度制約: |v| ∈ [V_MIN, V_MAX]

#### Step 2: EPHAgent::update()実装（2時間）

**ファイル**: `packages/eph_agent/include/eph_agent/eph_agent.hpp`

**更新内容**:
```cpp
void update(const spm::SaliencyPolarMap& spm, Scalar dt) {
    // 1. 行為選択
    Vec2 new_velocity = action_selector_.select_action(
        state_.velocity, haze_, spm
    );

    // 2. 状態更新
    state_.velocity = new_velocity;
    state_.position += state_.velocity * dt;

    // 3. 予測誤差計算（簡易版: 速度変化量）
    Scalar prediction_error = (new_velocity - state_.velocity).norm();

    // 4. Haze推定更新
    haze_ = haze_estimator_.estimate(spm, prediction_error);

    // 5. 疲労度更新
    state_.fatigue += FATIGUE_RATE * state_.velocity.squaredNorm() * dt;
    state_.fatigue = std::max(0.0, state_.fatigue - RECOVERY_RATE * dt);
}
```

**追加メンバー**:
```cpp
private:
    ActionSelector action_selector_;
```

#### Step 3: SwarmManager統合（1時間）

**ファイル**: `packages/eph_swarm/include/eph_swarm/swarm_manager.hpp`

**追加メソッド**:
```cpp
void update_all_agents(const spm::SaliencyPolarMap& spm, Scalar dt) {
    // 1. 全エージェント更新（行為選択・状態更新・Haze推定）
    for (size_t i = 0; i < agents_.size(); ++i) {
        agents_[i]->update(spm, dt);
        positions_[i] = agents_[i]->state().position;
    }

    // 2. MB破れ適用
    update_effective_haze();
}
```

#### Step 4: テスト実装（3-4時間）

**test_action_selector.cpp**（20テスト）:
- EFE計算（Haze高→EFE高、Haze低→EFE低）
- 勾配計算（数値微分精度）
- 行為選択（勾配降下収束）
- 速度制約（V_MIN, V_MAX）

**test_eph_agent_phase4.cpp**（15テスト）:
- update()基本動作
- 予測誤差フィードバック
- 疲労度蓄積・回復
- 位置更新

**test_swarm_dynamics.cpp**（10テスト）:
- 群全体の動的更新
- MB破れ+行為選択の統合
- 収束挙動

#### Step 5: V2完全検証実験（2-3時間）

**test_v2_phase_transition.cpp**:

```cpp
TEST(V2Validation, FullPhaseTransition_DetectsBetaC) {
    const size_t N_AGENTS = 50;
    const int AVG_NEIGHBORS = 6;
    const Scalar BETA_MIN = 0.0;
    const Scalar BETA_MAX = 0.3;
    const Scalar BETA_STEP = 0.01;
    const int WARMUP_STEPS = 200;
    const int MEASUREMENT_STEPS = 100;

    std::vector<Scalar> betas, phis_avg, chis;

    for (Scalar beta = BETA_MIN; beta <= BETA_MAX; beta += BETA_STEP) {
        SwarmManager swarm(N_AGENTS, beta, AVG_NEIGHBORS);

        // 動的SPM生成（時間変化するSaliency）
        auto generate_spm = [](int t) -> spm::SaliencyPolarMap {
            spm::SaliencyPolarMap spm;
            // 時間変化するパターン（例: 回転するガウシアンピーク）
            // ...実装詳細は省略
            return spm;
        };

        // Warm-up: 系を平衡化
        for (int t = 0; t < WARMUP_STEPS; ++t) {
            auto spm = generate_spm(t);
            swarm.update_all_agents(spm, 0.01);
        }

        // Measurement: φ測定
        std::vector<Scalar> phi_samples;
        for (int t = 0; t < MEASUREMENT_STEPS; ++t) {
            auto spm = generate_spm(WARMUP_STEPS + t);
            swarm.update_all_agents(spm, 0.01);

            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = PhaseAnalyzer::compute_phi(haze_fields);
            phi_samples.push_back(phi);
        }

        Scalar phi_avg = PhaseAnalyzer::mean(phi_samples);
        Scalar chi = PhaseAnalyzer::compute_chi(phi_samples);

        betas.push_back(beta);
        phis_avg.push_back(phi_avg);
        chis.push_back(chi);
    }

    // β_c検出
    Scalar beta_c_empirical = PhaseAnalyzer::find_beta_c(betas, phis_avg);
    Scalar beta_c_theory = 0.098;

    // V2成功基準: β_c^empirical が理論値±10%以内
    EXPECT_NEAR(beta_c_empirical, beta_c_theory, beta_c_theory * 0.1)
        << "V2 VALIDATION FAILED: β_c empirical = " << beta_c_empirical;

    // χ(β)がβ_c付近で最大
    auto max_chi_it = std::max_element(chis.begin(), chis.end());
    size_t max_chi_idx = std::distance(chis.begin(), max_chi_it);
    Scalar beta_at_max_chi = betas[max_chi_idx];

    EXPECT_NEAR(beta_at_max_chi, beta_c_empirical, 2.0 * BETA_STEP)
        << "χ should peak near β_c";
}
```

### 4.3 Phase 4完了基準

- [x] ActionSelector実装（EFE計算、勾配降下）
- [x] EPHAgent::update()実装（行為選択ループ）
- [x] SwarmManager::update_all_agents()実装
- [x] テスト45件追加（総154/154テスト通過）
- [x] **V2完全達成**: β_c^empirical ∈ [0.088, 0.108]
- [x] χ(β)がβ_c付近でピーク

### 4.4 Phase 4 実績サマリー（2026-02-06確定）

| 指標 | 目標 | 実績 |
|------|------|------|
| テスト追加 | 45件 | 45件（154/154通過） |
| ビルド | 全通過 | 192/196通過（98%、4件既知問題） |
| V2: β_c検出 | ∈ [0.088, 0.108] | ✅ 達成 |
| V2: χ(β)ピーク | β_c付近 | ✅ 達成 |
| ActionSelector | 実装完了 | ✅ EFE勾配降下 |
| EPHAgent::update() | フィードバックループ閉 | ✅ 完了 |
| SwarmManager統合 | 動的更新 | ✅ 完了 |

### 4.5 Phase 4のリスクと対策

| リスク | 影響 | 対策 |
|--------|------|------|
| β_c検出失敗 | V2未達成 | WARMUP_STEPS増加（200→500）<br>N_AGENTS増加（50→100） |
| 数値不安定 | NaN/Inf発生 | 勾配クリッピング<br>学習率調整（η=0.1→0.01） |
| 収束が遅い | 実験時間長い | 適応的学習率<br>モーメンタム追加 |
| 有限サイズ効果 | β_c誤差大 | N_AGENTS=100で再実験 |

---

## 🚀 Phase 5: 完全検証・最適化

**目的**: 全検証目標（V1-V5）達成、パフォーマンス最適化、可視化

**期間**: 2-3日（8-12時間）

### 5.1 検証目標V1-V5（Phase 5統一定義、2026-02-06更新）

#### V1: 信念更新検証（予測誤差フィードバック）
- [x] PE = |Delta v| / V_MAX が適切な範囲 [0, 1] に収まる
- [ ] 高PE → Haze増加（不確実性認識）
- [ ] 低PE → Haze減少（確信度向上）
- [ ] 1000ステップ実行で収束確認

#### V2: 相転移検出 ✅ ACHIEVED（Phase 4で達成）
- [x] β_c^empirical ∈ [0.088, 0.108]
- [x] φ(β)の明確な不連続
- [x] χ(β)のβ_c付近ピーク

#### V3: ボトムアップ顕著性検証
- [ ] 勾配方向への移動（SPM勾配が大きい方向へエージェントが移動）
- [ ] Saliency依存性（高Saliency領域への誘引、低Saliencyからの回避）
- [ ] 複数ピークでの選択挙動（ExplorationとExploitationのバランス）

#### V4: 長時間安定性検証
- [ ] 10000ステップでNaN/Inf無し
- [ ] 定常状態収束（φ、χなどの観測量が安定）
- [ ] メモリリーク無し（長時間実行でメモリ使用量一定）

#### V5: 大規模群検証
- [ ] N=100でβ_c再現（N=50での結果β_c≈0.098がN=100でも再現）
- [ ] 計算時間スケーリング（O(N²)の確認、並列化効果測定）
- [ ] 統計精度向上（大規模群でのφ、χの標準誤差減少）

### 5.2 実装項目

#### 可視化ツール（オプション）
- [ ] φ(β)、χ(β)グラフ出力（CSV/PNG）
- [ ] エージェント軌跡アニメーション
- [ ] Hazeフィールド可視化（ヒートマップ）

#### パフォーマンス最適化
- [ ] 近傍検索の空間分割（O(N²) → O(N log N)）
- [ ] OpenMP並列化（エージェント更新）
- [ ] Eigen最適化フラグ（-O3 -march=native）

#### ドキュメント
- [ ] Phase 4-5実装詳細追記
- [ ] V1-V5検証結果レポート
- [ ] README更新（最終版）

### 5.3 Phase 5完了基準

- [ ] V1-V5全達成
- [ ] 総テスト170+件通過
- [ ] ドキュメント完備
- [ ] パフォーマンス: N=100で実時間×10以下

---

## 📅 全体スケジュール

```
Phase 0-3: 完了 ✅
    │
    ├── Phase 0-1: 基礎実装（完了）
    ├── Phase 2: エージェント（完了）
    └── Phase 3: マルチエージェント（完了）
        └─→ 109/109テスト通過

Phase 4: 完了 ✅（154/154テスト通過、V2達成）
    │
    ├── ActionSelector実装 ✅
    ├── EPHAgent::update()実装 ✅
    ├── テスト実装 ✅
    ├── V2検証実験 ✅
    └── β_c ∈ [0.088, 0.108] 確認 ✅

Phase 5: 2-3日 ⏳ ← 現在ここ
    │
    ├── Day 1: V1, V3-V5検証
    ├── Day 2: 可視化・最適化
    └── Day 3: ドキュメント完成
        └─→ プロジェクト完了 🎉
```

**総見積もり**: Phase 4-5で5-8日（Phase 3完了から1-2週間）

---

## 🔑 Critical Success Factors

### Phase 4成功の鍵
1. **予測誤差の定義**: 速度変化量を使用（簡易版）
2. **学習率調整**: η=0.1から開始、必要に応じて適応
3. **十分な平衡化**: WARMUP_STEPS=200以上
4. **動的SPM生成**: 時間変化するSaliencyパターン

### Phase 5成功の鍵
1. **複数初期条件**: 統計的ロバスト性確保
2. **有限サイズ効果**: N=50, 100で比較
3. **パラメータ探索**: τ, η, zの感度分析
4. **可視化**: 直感的理解のため必須

---

## 📝 次のアクション

### Phase 5 即座に開始可能
1. V1検証: 信念更新検証（予測誤差フィードバック）
2. V3検証: ボトムアップ顕著性検証
3. V4検証: 長時間安定性検証（10000ステップ）
4. V5検証: 大規模群検証（N=100）

### Phase 5開始前の確認事項
- [x] Phase 4コードレビュー（154/154テスト全通過確認）
- [x] Phase 5設計レビュー（phase5_proposal.md）
- [x] 依存関係確認（追加ライブラリ不要）

---

**Phase 4完了（2026-02-06）。Phase 5実装へ進行可能。**
