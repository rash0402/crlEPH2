# EPH v2.1 用語集・記号一覧

**最終更新**: 2026-02-02

## このドキュメントの目的

EPHプロジェクトで使用される数学記号、略語、専門用語の統一的な定義を提供します。実装時の命名規則や理論式の読解に使用してください。

**対象読者**: 全開発者、AIエージェント

---

## 主要パラメータ

### β (ベータ)

- **正式名称**: Markov Blanket Breaking Strength
- **日本語**: マルコフブランケット破れ強度
- **定義域**: [0, 1]
- **物理的意味**: エージェント間でhazeフィールドをどれだけ共有するか
- **臨界値**: β_c ≈ 0.098（典型値）
- **実装変数名**: `beta` (C++), `beta` (Python)

**数式**:
```
h_eff,i = (1-β)h_i + β⟨h_j⟩_{j∈N_i}
```

**相図**:
- β < β_c: 無秩序相（個体独立）
- β = β_c: **臨界点（カオスの縁）**
- β > β_c: 秩序相（過同調）

---

### κ (カッパ)

- **正式名称**: Haze Sensitivity
- **日本語**: ヘイズ感度
- **定義域**: [0.1, 2.0]（実用範囲）
- **物理的意味**: エージェントがhazeの影響をどれだけ受けやすいか
- **実装変数名**: `kappa` (C++), `kappa` (Python)

**役割別の典型値** (2:6:2分布):

| 役割 | κ範囲 | 平均 | 行動特性 |
|------|-------|------|---------|
| **Leader** | 0.3 - 0.5 | 0.3 | hazeが高くても踏み込む（低感度） |
| **Follower** | 0.8 - 1.2 | 1.0 | 標準的な応答 |
| **Reserve** | 1.3 - 1.7 | 1.5 | hazeで強く抑制（高感度） |

**数式**:
```
∇_u^gated = (1 - κ·sg[h]) ⊙ ∇_u G_EPH
```
（sg = stop_gradient）

---

### h (エイチ)

- **正式名称**: Intrinsic Perceptual Haze
- **日本語**: 内在知覚ヘイズ、曖昧性フィールド
- **定義域**: [0, 1]
- **物理的意味**: 知覚の不確実性・曖昧性の場
- **実装変数名**: `haze` (C++), `haze` (Python)
- **SPMチャネル**: M0 (ch=9)

**推定式** (§4.2):
```
h̃ = σ(a·EMA(e) + b·R1 + c·(1-F4) + d·F5)
h = Gaussian_blur(h̃, σ=1)
```

推奨パラメータ: a=0.4, b=0.3, c=0.2, d=0.1

---

## 応答関数・秩序パラメータ

### χ(β) (カイ)

- **正式名称**: Susceptibility / Response Function
- **日本語**: 応答関数、帯磁率
- **定義域**: [0, +∞)
- **物理的意味**: 環境変化に対する群の感度
- **実装変数名**: `susceptibility` (C++), `chi` (Python)

**数式**:
```
χ(β) = N(⟨φ²⟩ - ⟨φ⟩²)
```

**臨界指数**: γ = 1（平均場近似）
```
χ(β) ~ |β - β_c|^(-γ)
```

**Theorem 1**: χ(β) は β = β_c で最大化される

---

### φ(β) (ファイ)

- **正式名称**: Order Parameter
- **日本語**: 秩序パラメータ
- **定義域**: [0, 1]
- **物理的意味**: hazeフィールドの不均一性の度合い
- **実装変数名**: `order_parameter` (C++), `phi` (Python)

**数式**:
```
φ(β) = (1/N)Σ_i |h_i - h̄|
```

**相転移の指標**:
- φ ≈ 0: 均一状態（β < β_c）
- φ > 0: パターン形成（β > β_c）

---

### ξ(β) (グザイ)

- **正式名称**: Correlation Length
- **日本語**: 相関長
- **定義域**: [0, +∞)
- **物理的意味**: haze場の空間相関が及ぶ距離
- **実装変数名**: `correlation_length` (C++), `xi` (Python)

**数式**:
```
ξ(β) = √[Σ_{i,j} |r_i - r_j|² · C_{ij} / Σ_{i,j} C_{ij}]
```
（C_{ij} = ⟨h_i h_j⟩ - ⟨h_i⟩⟨h_j⟩）

**臨界挙動**: ξ(β) ~ |β - β_c|^(-ν)（ν ≈ 1/2）

---

## Free Energy関連

### F_EPH

- **正式名称**: Variational Free Energy
- **日本語**: 変分自由エネルギー
- **実装変数名**: `free_energy` (C++), `F_EPH` (Python)

**数式** (§2.2):
```
F_EPH = (1/2σ²)Σ F4·(ΔO - ΔÔ)² + (λ/2)Σ Σ̂_ΔO
```

---

### G_EPH

- **正式名称**: Expected Free Energy
- **日本語**: 期待自由エネルギー
- **実装変数名**: `expected_free_energy` (C++), `G_EPH` (Python)

**数式** (§2.3):
```
G_EPH(u) = Σ F4·ΔÔ² + λ·Σ̂_ΔO
```

**行為更新**:
```
u ← u - η·∇_u G_EPH
```

---

## SPM関連

### SPM

- **正式名称**: Saliency Polar Map
- **日本語**: 顕著性極座標マップ
- **次元**: 10 × 12 × 12（C × θ × r）
- **実装変数名**: `spm` (C++), `spm` (Python)
- **型**: `Eigen::Tensor<double, 3>` (C++), `np.ndarray[float64]` (Python)

**チャネル構成**:

| ch | ID | 名称 | 役割 | 微分対象 |
|----|----|----|------|---------|
| 0 | T0 | Obs Occupancy (Teacher) | 教師信号 | × |
| 1 | R0 | **Δoccupancy (+1)** | 予測（学習対象） | ✅ |
| 2 | R1 | Uncertainty of R0 | 不確実性 | × |
| 3 | F0 | Occupancy (Current) | 現在占有 | × |
| 4 | F1 | **Motion Pressure** | 運動圧 | × |
| 5 | F2 | Saliency | 顕著性 | × |
| 6 | F3 | TTC Proxy | 衝突時間 | × |
| 7 | F4 | **Visibility** | 可視性（マスク） | × |
| 8 | F5 | Observation Stability | 観測安定性 | × |
| 9 | M0 | **Haze Field** | ヘイズ場 | × (Stop-Grad) |

**境界条件**:
- θ方向（ch軸2）: 周期境界（N_THETA = 12）
- r方向（ch軸3）: Neumann境界（N_R = 12）

---

### ΔÔ (デルタオーハット)

- **正式名称**: Predicted Occupancy Change
- **日本語**: 予測占有変化
- **定義域**: [-1, 1]
- **実装変数名**: `delta_occupancy_pred` (C++), `delta_O_hat` (Python)

**数式**:
```
ΔÔ[k+1] = f_θ(PF[k], u[k])
```

**教師信号**:
```
ΔO^true[k+1] = O^obs[k+1] - O^obs[k]
```

---

## 略語・専門用語

### MB

- **正式名称**: Markov Blanket
- **日本語**: マルコフブランケット
- **意味**: 内部状態と外界を隔てる境界（感覚状態と能動状態）

**EPHでの拡張**: Broken MB（破れたマルコフブランケット）

---

### EFE

- **正式名称**: Expected Free Energy
- **日本語**: 期待自由エネルギー
- **同義**: G_EPH

---

### FEP

- **正式名称**: Free Energy Principle
- **日本語**: 自由エネルギー原理
- **提唱**: Karl Friston (2010)

---

### Active Inference

- **日本語**: 能動的推論
- **意味**: FEPに基づく行動決定の枠組み

---

### PR / PF / Meta

- **PR**: Prediction-Required（予測必須チャネル）
- **PF**: Prediction-Free（予測不要チャネル）
- **Meta**: メタ情報チャネル（haze等）

---

### 2:6:2 Distribution

- **日本語**: 2:6:2役割分布
- **意味**: Leader 20% : Follower 60% : Reserve 20%
- **生物学的類似**: 働きアリ研究（長谷川英祐）

---

## 物理量の単位

| 記号 | 物理量 | 単位 | 備考 |
|------|--------|------|------|
| t | 時間 | s（秒） | 離散時刻kでの実装 |
| Δt | 時間刻み | 0.01 - 0.1 s | 推奨: 0.1s |
| τ_haze | Haze時定数 | 1.0 s | EMA時定数 |
| τ_EMA | EMA時定数 | 1.0 s | τ_haze と同等 |
| η | 学習率 | 無次元 | 推奨: 0.1 |
| r | 距離 | m（メートル） | SPMの半径座標 |
| θ | 角度 | rad（ラジアン） | SPMの角度座標 |
| u | 力 | N（ニュートン） | 制御入力 |
| m | 質量 | kg | エージェント質量 |
| c_d | 抗力係数 | kg/m | 速度減衰 |

---

## 臨界指数（Critical Exponents）

平均場近似での値:

| 指数 | 記号 | 値 | 定義 |
|------|------|------|------|
| 帯磁率 | γ | 1 | χ ~ \|β - β_c\|^(-γ) |
| 相関長 | ν | 1/2 | ξ ~ \|β - β_c\|^(-ν) |
| 秩序パラメータ | β_exp | 1/2 | φ ~ (β - β_c)^(β_exp) |

**注意**: 2次元系では上部臨界次元（d_c=4）を下回るため、2D Ising的指数（γ=7/4, ν=1）に修正される可能性あり（有限サイズスケーリング解析で検証）。

---

## C++/Python型対応

| 概念 | C++型 | Python型 |
|------|-------|----------|
| Scalar | `double` | `float64` |
| Vec2 | `Eigen::Vector2d` | `np.ndarray[2]` |
| Tensor3 | `Eigen::Tensor<double, 3>` | `np.ndarray[C,θ,r]` |
| Matrix12x12 | `Eigen::Matrix<double,12,12>` | `np.ndarray[12,12]` |
| AgentState | `struct AgentState` | `dataclass AgentState` |

---

## 関連ドキュメント

- **実装変数名の詳細**: `.claude/01_development_docs/cpp_guidelines/coding_style.md`
- **SPM完全仕様**: `doc/appendix/EPH-2.1_appendix-B_spm.md`
- **理論的定義**: `doc/EPH-2.1_main.md`
- **数学的証明**: `doc/appendix/EPH-2.1_appendix-A_proofs.md`

---

*この用語集は実装時の命名とドキュメント理解の統一基準として使用してください。*
