---
title: "EPH v2.1: カオスの縁における適応性最大化 — Broken Markov Blanket による群知能の臨界制御"
type: Research_Proposal
status: "🟢 Active"
version: 2.1.0
date_created: "2026-02-01"
date_modified: "2026-02-02"
author: "Hiroshi Igarashi"
institution: "Tokyo Denki University"
document_id: "EPH-2.1_main"
keywords:
  - Free Energy Principle
  - Active Inference
  - Markov Blanket
  - Broken Markov Blanket
  - Edge of Chaos
  - Criticality
  - Phase Transition
  - Adaptability Maximization
  - Susceptibility
  - Perceptual Haze (intrinsic)
  - Saliency Polar Map (SPM)
  - Swarm Intelligence
  - Environmental Adaptation
tags:
  - Research/Proposal
  - Topic/EPH
  - Topic/FEP
  - Topic/SwarmIntelligence
  - Status/Active
appendices:
  - id: "A"
    title: "Complete Mathematical Proofs"
    file: "appendix/EPH-2.1_appendix-A_proofs.md"
  - id: "B"
    title: "SPM Specification"
    file: "appendix/EPH-2.1_appendix-B_spm.md"
  - id: "C"
    title: "Numerical Validation Plan"
    file: "appendix/EPH-2.1_appendix-C_validation.md"
  - id: "D"
    title: "Related Work Survey"
    file: "appendix/EPH-2.1_appendix-D_related-work.md"
changelog:
  - version: 2.1.0
    date: "2026-02-02"
    changes:
      - "【重要】研究フォーカスを「カオスの縁における適応性最大化」に再定義"
      - "臨界点β_cでの応答関数χ(β)最大化を中心的主張に"
      - "2:6:2役割分布を「定理」から「創発的観察」に格下げ"
      - "Swarm Intelligenceの革新を「環境変化への高適応性」として明確化"
      - "関連研究との差別化を「臨界制御可能性」に焦点化"
  - version: 2.0.2
    date: "2026-02-01"
    changes:
      - "§2.6に予測器f_θの学習設計を追加"
      - "FEP準拠損失関数（変分下界最大化）を定式化"
      - "オフライン事前学習 + オンライン微調整の2フェーズ設計"
      - "時間スケール分離（τ_θ ≫ τ_h ≫ τ_u）を明記"
  - version: 2.0.1
    date: "2026-02-01"
    changes:
      - "§2.3.1に行為uの決定機序を詳細追記"
      - "世界モデル（予測器）を通じたu最適化フローを明確化"
      - "勾配∇_u ΔÔの物理的解釈を追加"
  - version: 2.0.0
    date: "2026-02-01"
    changes:
      - "メジャーバージョン更新: v8.1 → v2.0"
      - "2:6:2役割分布理論を追加（§5.5）"
      - "パレートの法則・働きアリ研究との接続"
      - "Related Workに最新論文（2023-2025）を追加"
      - "新規性5（2:6:2創発的役割分布）を追加"
      - "仮説H5（2:6:2分布の優位性）を追加"
      - "疲労モデルと動的役割切替を追加"
  - version: 8.1.0
    date: "2026-02-01"
    changes:
      - "Free Energyの明示的定義を追加（§2）"
      - "収束性・安定性の理論的証明を追加（§5）"
      - "βと結合強度の数学的関係式を明確化（§4）"
      - "相転移理論（Ising類似）との接続を追加（§4.4）"
      - "ベースライン比較設計（SFM/DWA）を追加（§6）"
      - "Related Workを拡充（§1.3）"
      - "出版戦略を追加（§8）"
---

# 研究提案書: EPH v2.1 — カオスの縁における適応性最大化

> [!ABSTRACT] 提案の概要（One-Liner Pitch）
>
> **Markov Blanket 破れ強度 β を制御パラメータとして、群知能システムを「カオスの縁」（臨界点 β_c）に配置することで、環境変化に対する適応性を最大化する。** 臨界点では応答関数 χ(β) が最大となり、群れは微小な環境変化に対して最大の感度を持つ。これが Swarm Intelligence の本質的革新である。

---

## 要旨（Abstract）

### 背景（Background）
従来の群知能制御は、(i) 反応則（Social Force Model等）または (ii) 報酬最適化（Deep RL）に依存しがちであり、**環境変化への適応性**を理論的に保証する枠組みが欠如している。Active Inferenceはこのギャップを埋めるポテンシャルを持つが、既存研究（Heins et al., 2024; Kanso et al., 2024）は群集行動の**創発**を記述するに留まり、「**どのように群れを設計すれば適応性が最大化されるか**」という規範的問いには答えていない。

### 目的（Objective）
本研究の目的は、**カオスの縁（Edge of Chaos）における適応性最大化**を理論的に定式化し、実証することである。

1. **臨界点 β_c の解析的導出**：MB破れ強度 β の臨界点を平均場理論から導出し、相転移の制御可能性を示す。
2. **応答関数 χ(β) の最大化**：臨界点で応答関数（環境変化への感度）が最大となることを証明する。
3. **適応速度の定量化**：環境変化後の再平衡化時間が臨界点近傍で最小となることを示す。
4. **Broken MB の能動的制御**：haze を介した勾配ゲーティングにより、β を能動的に調整可能であることを示す。

### 学術的新規性（Academic Novelty）
- **新規性1（中心的主張）：臨界点 β_c での適応性最大化**
  MB破れ強度 β の臨界点 β_c において、応答関数 χ(β) が最大化され、群れの環境適応性が最大となることを理論的に証明する。これは Swarm Intelligence の**設計原理**を提供する。
- **新規性2：β_c の解析的導出**
  平均場理論を適用し、臨界点を閉形式で表現：$\beta_c = (1/\tau_h - g'(\bar{h})\alpha_e) / 2(z-1)$。既存研究（Kanso et al.）の数値的発見を超えた**定量的予測**。
- **新規性3：Broken MB の能動的制御**
  haze を介した勾配ゲーティングにより、β を**受動的な環境依存パラメータ**ではなく、**能動的に制御可能な設計パラメータ**として扱う。
- **新規性4：Haze = Precision Attenuation**
  Hazeを Friston の precision weighting 理論における「precision の低下」として再解釈し、認知科学・神経科学との理論的接続を明確化する。

### 副次的観察（Secondary Observations）
- **観察1：創発的役割分化**
  臨界点近傍で運用される群れにおいて、κ（haze感度）の分布が生物学的観察（2:6:2、働きアリ研究）と類似するパターンに収束しうる。これは理論的予測ではなく、創発的結果として観察される可能性がある。

### 手法（Methods）
- **2次系ダイナミクス**（離散時間）
  $$
  \mathbf{x}_i[k+1] = \mathbf{x}_i[k] + \Delta t\, \mathbf{v}_i[k],\quad
  \mathbf{v}_i[k+1] = \mathbf{v}_i[k] + \Delta t\,\frac{1}{m}\Big(\mathbf{u}_i[k] - c_d\|\mathbf{v}_i[k]\|\mathbf{v}_i[k]\Big)
  $$
- **MB破れ制御**：$h_{\text{eff},i} = (1-\beta)h_i + \beta\langle h_j \rangle_{N_i}$ により β を制御
- **応答関数測定**：$\chi(\beta) = N(\langle \phi^2 \rangle - \langle \phi \rangle^2)$、$\phi = N^{-1}\sum_i |h_i - \bar{h}|$
- **適応速度測定**：環境変化後の再平衡化時間 $\tau_{\text{adapt}}(\beta)$

### 検証目標（Validation Goals）
1. **χ(β) の山型確認**：応答関数が β = β_c で最大となることを数値的に検証
2. **β_c 予測精度**：理論値 β_c ≈ 0.098 と数値実験の一致（±10%以内）
3. **適応速度**：$\tau_{\text{adapt}}(\beta_c) < \tau_{\text{adapt}}(\beta \neq \beta_c)$ を確認
4. **相関長発散**：$\xi(\beta) \to \infty$ as $\beta \to \beta_c$ を確認

### 理論的限界（Theoretical Limitations）

本研究の理論的主張には以下の限界があり、解釈に注意を要する：

1. **平衡近似の限界**：Landau理論に基づくχ(β)解析は熱平衡を仮定。実際の群システムはエネルギー注入を伴う非平衡定常状態（NESS）であり、相転移の性質が異なる可能性がある。

2. **平均場近似の妥当性**：臨界指数（γ=1, ν=1/2）は平均場近似に基づく。2次元スウォームでは上部臨界次元（d_c=4）を下回るため、2D Ising的な指数（γ=7/4, ν=1）が適用される可能性がある。有限サイズスケーリング解析による検証が必要。

3. **予測器の理想化**：Lipschitz定数 L_f の一様有界性を仮定。実際のニューラルネット予測器では局所的にこれが破れる可能性があり、収束保証に影響する。

4. **同期更新の仮定**：全エージェントの同期更新を仮定。非同期更新では臨界点がシフトする可能性がある。

これらの限界を踏まえ、数値検証では有限サイズスケーリング解析 χ_L(β) = L^(γ/ν) χ̃(L^(1/ν)(β - β_c)) による指数推定を行い、平均場近似の妥当性を実験的に確認する。

### 結論と意義（Conclusion / Significance）
EPH v2.1 は、**群知能システムをカオスの縁に配置することで環境適応性を最大化する**という設計原理を提供する。これは既存研究（Heins, Kanso）が示した「群集行動の創発」を超えて、「**どのように群れを設計すべきか**」という規範的問いに答えるものである。臨界点 β_c を解析的に導出し、能動的に制御可能であることを示すことで、理論と工学の橋渡しを行う。

---

## Contributions（主要貢献）

本研究の主要貢献は以下の3点である：

> **C1. 適応性最大化定理（Theorem 1）**
> Markov Blanket破れ強度 β の臨界点 β_c において、応答関数 χ(β) が最大化され、群の環境適応性が最大となることを Landau 理論を用いて厳密に証明した。これは「カオスの縁で適応性が最大」という直感を**定量的に裏付ける初の理論的結果**である。

> **C2. 臨界点の解析的導出（Theorem 2）**
> 平均場理論を適用し、臨界点を閉形式で表現：
> $$\beta_c = \frac{1/\tau_{\text{haze}} - \bar{h}(1-\bar{h})\kappa\eta L_f^2}{2(z-1)}$$
> 既存研究（Kanso et al., 2024）の数値的発見を超えた**定量的予測**を提供し、システムパラメータとの関係を明示した。

> **C3. 能動的臨界制御メカニズム**
> Intrinsic Haze を介した勾配ゲーティングにより、β を**受動的な環境依存パラメータ**ではなく、**能動的に制御可能な設計パラメータ**として扱う方法を提案した。これにより「カオスの縁」への配置が工学的に実現可能となる。

**既存研究との差別化**：Langton (1990), Kauffman (1993) による「Edge of Chaos」の概念は主にセルラーオートマトン（離散系）で議論されてきた。本研究は (i) 連続系での厳密な数学的定式化、(ii) β_c の閉形式解の導出、(iii) 能動的制御可能性の証明、の3点で本質的に新しい。

---

## 1. 序論（Introduction）

### 1.1 "壊すべき"マルコフブランケット（MB）とは何か

マルコフブランケット（MB）は、内部状態（belief / latent）と外界を、感覚状態と能動状態で隔てる境界である（Pearl, 1988; Friston, 2013）。Active Inferenceでは、通常この境界が**局所性**と**安定性**を保証する。

しかし群システムでは、境界を完全に守ると「局所最適の渋滞」「役割固定」「環境変化への硬直」が起きる。本提案では、MB破れを「**勾配漏れ/遮断を条件付きに許す**」という操作的定義で捉え、適応・相転移の源泉として扱う。

### 1.2 EPH v2.0 の設計方針

- **SPMは12×12、PF/PRを分離**し、学習・予測はPR-ch1のみ（+1ホライゾンのΔ占有）。
- **PR-ch2は不確実性**（分散・アンサンブル不一致など）として、学習の副産物として扱う。
- **haze（Meta-ch）は内在推定**であり、$\mathbf{u}$で微分しない（勾配遮断ゲートとして使い、崩壊を防ぐ）。
- **Free Energyを明示的に定義**し、FEP/Active Inferenceとの理論的整合性を保証する。
- **2:6:2役割分布**を導入し、κ（haze感度）の異質性で群のレジリエンスを実現する。【v2.0追加】

### 1.3 関連研究（Related Work）

#### 1.3.1 Free Energy PrincipleとActive Inference

Free Energy Principle（FEP）は、生物システムが変分自由エネルギーを最小化することで環境に適応するという統一理論である（Friston, 2010）。Active Inferenceはこの原理に基づく行動決定の枠組みであり、知覚（状態推定）と行動（能動的推論）を同一の目的関数で統合する（Friston et al., 2017）。

近年の重要研究：

- **Friston et al. (2024)**："Designing Ecosystems of Intelligence from First Principles"（Collective Intelligence）。マルチエージェントActive Inferenceの理論的基盤。Nested Intelligenceの概念を提案。
- **Maisto et al. (2025)**："What the Flock Knows That the Birds Do Not"。群レベルのMB創発とシナジー情報を示す。
- **Wu et al. (2026)**："Think How Your Teammates Think"（AAAI 2026）。分散Active Inferenceにおけるチームメイトモデリング。
- **Palacios et al. (2020)**："On Markov blankets and hierarchical self-organisation"。MBの階層的自己組織化。

#### 1.3.2 群行動モデル

- **Social Force Model (SFM)**（Helbing & Molnár, 1995）：歩行者の相互作用を力場としてモデル化。反応的であり、予測能力を持たない。
- **Dynamic Window Approach (DWA)**（Fox et al., 1997）：ロボットの局所経路計画。明示的なコスト関数を最適化するが、群協調の理論的基盤は弱い。
- **Vicsekモデル**（Vicsek et al., 1995）：自己推進粒子系の群行動。秩序-無秩序相転移を示すが、認知的要素（予測、不確実性）を欠く。

#### 1.3.3 Edge of Chaos / 臨界性の古典研究【v2.1追加】

「カオスの縁（Edge of Chaos）」における適応性最大化は、複雑系科学の中心的テーマである。本研究はこれらの古典的知見を**連続系マルチエージェントシステム**に拡張する。

- **Langton (1990)**："Computation at the Edge of Chaos"。セルラーオートマトンにおいて、秩序相とカオス相の境界で計算能力が最大化されることを示した。**限界**: 離散系のみで議論、閉形式の臨界点導出なし。
- **Kauffman (1993)**：*The Origins of Order*。ブール・ネットワークにおける臨界性と進化的適応性の関係を議論。「生命はカオスの縁で進化する」という仮説を提唱。**限界**: 理論的枠組みが定性的、定量的予測が困難。
- **Bak, Tang, Wiesenfeld (1987)**："Self-Organized Criticality"。砂山モデルを通じて、多くの系が臨界点に自発的に到達する「自己組織化臨界」を提案。**限界**: 受動的な自己組織化であり、能動的制御の視点なし。
- **Beggs & Plenz (2003)**："Neuronal Avalanches"。脳神経活動において臨界性が観察されることを報告。認知機能と臨界性の関係を示唆。

**EPHとの差別化**：
| 観点 | 古典研究 | EPH v2.1 |
|-----|---------|----------|
| 対象系 | 離散系（CA, Boolean Network） | 連続系（微分方程式） |
| 臨界点 | 数値発見、定性的議論 | **解析的導出（閉形式）** |
| 制御性 | 受動的（自己組織化） | **能動的（haze経由）** |
| 適応性 | 定性的主張 | **定量的証明（Theorem 1）** |

#### 1.3.5 働きアリ研究と2:6:2分布【v2.0追加】

- **Hasegawa (長谷川英祐) et al.**：北海道大学。アリのコロニーにおける「働かないアリ」の存在とその進化的意義を研究。
  - 観察：集団の約20%が高パフォーマー、60%が中パフォーマー、20%が低パフォーマー（「働かないアリ」）
  - 発見：働かないアリを除去しても、残りの集団から新たな「働かないアリ」が出現
  - 解釈：**Reserve（予備力）としての戦略的待機**。疲労した働きアリのリリーフ、緊急時の予備戦力。

#### 1.3.6 本研究の位置づけ

| 既存研究 | 限界 | EPH v2.0での解決 |
|----------|------|-----------------|
| SFM/DWA | 反応的、予測なし | PR（+1ホライゾン予測）を導入 |
| 1次系Active Inference | 慣性・遅れを扱えない | 2次系ダイナミクスに拡張 |
| Vicsek/Couzin | 認知的要素なし | haze（曖昧性の場）で認知を表現 |
| Deep RL | 理論的一貫性なし | FEP準拠のFree Energy定義 |
| 従来群モデル | 役割均質性 | **2:6:2異質的役割分布** |

---

## 2. 理論的基盤（Theoretical Foundation）

### 2.1 Free Energy Principleの概説

Free Energy Principle（FEP）によれば、適応的システムは変分自由エネルギー$F$を最小化する（Friston, 2010）：

$$
F = D_{\text{KL}}\big[q(\mathbf{s}) \| p(\mathbf{s}|\mathbf{o})\big] - \log p(\mathbf{o})
$$

ここで：
- $q(\mathbf{s})$：隠れ状態$\mathbf{s}$に対する近似事後分布
- $p(\mathbf{s}|\mathbf{o})$：観測$\mathbf{o}$が与えられたときの真の事後分布
- $p(\mathbf{o})$：観測のモデルエビデンス（対数周辺尤度）

### 2.2 EPHのための変分Free Energy定義

EPHの文脈では、隠れ状態$\mathbf{s}$を「次ステップの占有変化$\Delta O[k+1]$」とし、観測$\mathbf{o}$を「現在のPF情報$\text{PF}[k]$および制御入力$\mathbf{u}[k]$」とする。

ガウス仮定を置くと、変分Free Energyは次のように書ける：

$$
F_{\text{EPH}}[k] = \underbrace{\frac{1}{2\sigma^2}\sum_{\theta,r}\text{F4}[k](\theta,r)\cdot\big(\Delta O[k+1] - \widehat{\Delta O}[k+1]\big)^2}_{\text{Prediction Error}} + \underbrace{\frac{\lambda}{2}\sum_{\theta,r}\widehat{\Sigma}_{\Delta O}[k+1]}_{\text{Expected Uncertainty}}
$$

### 2.3 Expected Free Energyと行為更新

Active Inferenceでは、行為$\mathbf{u}$はExpected Free Energy $G$を最小化するように選ばれる：

$$
G_{\text{EPH}}(\mathbf{u}[k]) = \sum_{\theta,r}\text{F4}[k]\cdot\widehat{\Delta O}[k+1;\mathbf{u}]^2 + \lambda\,\widehat{\Sigma}_{\Delta O}[k+1;\mathbf{u}]
$$

**勾配降下による行為更新**：

$$
\mathbf{u}[k] \leftarrow \mathbf{u}[k] - \eta\,\nabla_{\mathbf{u}}G_{\text{EPH}}(\mathbf{u}[k])
$$

### 2.3.1 行為 $\mathbf{u}$ の決定機序【詳細】

#### $\mathbf{u}$ の物理的定義

$\mathbf{u} \in \mathbb{R}^2$ は**力ベクトル** [N] であり、2次系ダイナミクス（§2.5）の制御入力として作用する。

#### 世界モデル（予測器）の役割

**予測器** $f_\theta$ は「この行為 $\mathbf{u}$ を実行したら、次ステップで周囲の占有がどう変化するか」を予測する：

$$
\widehat{\Delta O}[k+1; \mathbf{u}] = f_\theta\big(\text{PF}[k],\, \mathbf{u}\big)
$$

ここで：
- **入力 PF[k]**: 現在の観測（F0: occupancy, F1: motion pressure, F2: saliency, F3: TTC, F4: visibility, F5: stability）
- **入力 $\mathbf{u}$**: 仮定する行為（力ベクトル）
- **出力 $\widehat{\Delta O}$**: 12×12 の占有変化予測（PR-R0）

**予測器が $\mathbf{u}$ を使う理由**：自己運動（ego-motion）による視点変化を考慮するため。例えば、前方への力 $\mathbf{u}$ を仮定すると、前方の物体が接近する（$\Delta O > 0$）と予測される。

#### 行為決定のフロー

```
1. 初期化: u^(0) ← u[k-1] または 0
2. 反復 (n = 0, 1, ..., N_iter-1):
   a. 予測:      ΔÔ = f_θ(PF[k], u^(n))
   b. G計算:     G = Σ F4·ΔÔ² + λ·Σ̂
   c. 勾配計算:  ∇_u G = ∂G/∂u（予測器を通じて）
   d. ゲート適用: ∇_u^gated = (1-R1)·F4·(1-κ·sg[h]) ⊙ ∇_u ΔÔ
   e. 更新:      u^(n+1) ← u^(n) - η·∇_u^gated G
3. 出力: u[k] ← u^(N_iter)
```

#### 勾配 $\nabla_{\mathbf{u}}\widehat{\Delta O}$ の解釈

予測器がニューラルネットワークの場合、自動微分で計算：

$$
\nabla_{\mathbf{u}}\widehat{\Delta O} = \frac{\partial f_\theta}{\partial \mathbf{u}}
$$

**物理的意味**: 「行為を少し変えたとき、予測される占有変化がどう変わるか」

- 正の勾配方向に力を増やす → 占有変化が増える → $G$ が増える
- **勾配降下で $G$ を下げる** = 占有変化が小さくなる方向に $\mathbf{u}$ を調整 = 安定状態を維持

#### 推奨パラメータ

| パラメータ | 記号 | 推奨値 | 意味 |
|-----------|------|--------|------|
| 反復回数 | $N_{\text{iter}}$ | 1-3 | 計算コストとのバランス |
| 学習率 | $\eta$ | 0.1 | 行為更新のステップサイズ |
| 不確実性重み | $\lambda$ | 0.1 | 探索と活用のバランス |

### 2.4 Hazeによるゲーティング（Stop-Gradient）

Haze $h[k]$は**Stop-Gradient演算子** $\text{sg}[\cdot]$ を適用した**非微分ゲート**として使う：

$$
\mathbf{u}[k] \leftarrow \mathbf{u}[k] - \eta\,\underbrace{\big(1 - \text{R1}[k]\big)}_{\text{uncertainty}} \cdot \underbrace{\text{F4}[k]}_{\text{visibility}} \cdot \underbrace{\big(1 - \kappa_i\,\text{sg}[h[k]]\big)}_{\text{haze gate (κ依存)}} \odot \nabla_{\mathbf{u}}\widehat{\Delta O}[k+1]
$$

ここで$\kappa_i$は**エージェント$i$のhaze感度**（2:6:2分布に従う）。

#### 2.4.1 Precision理論との数学的対応【v2.1追加】

Friston の Active Inference における **precision weighting** （Friston, 2019; Parr & Friston, 2019）との対応を明示する。

**Precision の定義**：
予測分布 $p(\mathbf{o}|\mathbf{s})$ の精度（precision）は分散の逆数：
$$
\pi = \frac{1}{\sigma^2}
$$

高精度（$\pi$ 大）= 予測の確信度が高い → 予測に従う
低精度（$\pi$ 小）= 予測の不確実性が高い → 感覚に従う

**Haze = Precision Attenuation**：
EPH における haze $h$ は、正規化された精度減衰として解釈できる：
$$
h = 1 - \frac{\pi}{\pi_0} = 1 - \frac{\sigma_0^2}{\sigma^2}
$$

ここで $\pi_0$ はベースライン精度、$\sigma_0^2$ はベースライン分散。

- $h = 0$: 精度最大（$\pi = \pi_0$）、予測を完全に信頼
- $h = 1$: 精度ゼロ（$\pi \to 0$）、予測を無視

**ゲーティングの解釈**：
勾配更新のゲーティング項 $(1 - \kappa h)$ は精度重み付けに対応：
$$
(1 - \kappa h) = 1 - \kappa\left(1 - \frac{\pi}{\pi_0}\right) = (1-\kappa) + \kappa\frac{\pi}{\pi_0} \propto \pi
$$

すなわち、**精度が低い（haze が高い）領域では勾配更新を抑制**する。これは Active Inference の標準的な処方：「不確実な予測には従わない」と一致する。

**神経科学的対応**：
- Haze高 ≈ Attention lapse / Mind wandering（注意散漫状態）
- Haze低 ≈ Focused attention（集中状態）
- κ ≈ 注意資源の個人差（trait attention）

**引用**：Friston, K. (2019). A free energy principle for a particular physics. *arXiv:1906.10184*.

### 2.5 2次系への拡張

EPHでは慣性を考慮した2次系を採用する：

$$
\mathbf{x}_i[k+1] = \mathbf{x}_i[k] + \Delta t\, \mathbf{v}_i[k]
$$
$$
\mathbf{v}_i[k+1] = \mathbf{v}_i[k] + \Delta t\,\frac{1}{m}\Big(\mathbf{u}_i[k] - c_d\|\mathbf{v}_i[k]\|\mathbf{v}_i[k]\Big)
$$

### 2.6 予測器 $f_\theta$ の学習【FEP準拠】

#### 2.6.1 学習の目的

世界モデル（予測器）$f_\theta$ は、「行為 $\mathbf{u}$ を実行したとき、次ステップの占有変化 $\Delta O$ がどうなるか」を予測する。Active Inference の枠組みでは、これは**生成モデル**に相当する。

$$
f_\theta: (\text{PF}[k], \mathbf{u}[k]) \mapsto (\widehat{\Delta O}[k+1], \widehat{\Sigma}[k+1])
$$

ここで $\widehat{\Sigma}$ は予測の不確実性（分散）。

#### 2.6.2 FEP準拠損失関数

Free Energy Principle に基づき、予測器は**変分下界（ELBO）を最大化**するように学習する。これは以下の損失関数を最小化することに等価：

$$
\mathcal{L}_{\text{FEP}}(\theta) = \mathbb{E}\Big[\underbrace{\frac{1}{2}\sum_{\theta,r}\text{F4}[k]\cdot\frac{(\Delta O^{\text{true}} - \widehat{\Delta O})^2}{\widehat{\Sigma}}}_{\text{正規化予測誤差}} + \underbrace{\frac{1}{2}\sum_{\theta,r}\text{F4}[k]\cdot\log\widehat{\Sigma}}_{\text{不確実性ペナルティ}}\Big]
$$

**解釈**：
- 第1項: 予測誤差を分散で正規化（不確実な領域の誤差は許容）
- 第2項: 分散を無限大にする逃げを防止

**教師データ**:
$$
\Delta O^{\text{true}}[k+1] = O^{\text{obs}}[k+1] - O^{\text{obs}}[k]
$$

T0チャネル（観測occupancy）の差分から生成。

#### 2.6.3 学習フェーズ

**Phase 1: オフライン事前学習（シミュレータ）**

1. ランダム方策 $\pi_{\text{rand}}$ でデータ収集：$\{(\text{PF}^{(i)}, \mathbf{u}^{(i)}, \Delta O^{(i)})\}_{i=1}^{N}$
2. ミニバッチ勾配降下で $\theta$ を更新：
   $$
   \theta \leftarrow \theta - \alpha_\theta\,\nabla_\theta\mathcal{L}_{\text{FEP}}
   $$
3. 検証セットで予測精度を評価、収束まで繰り返し

**Phase 2: オンライン微調整（実環境）**

1. EPH方策で行動しながら経験を収集
2. 経験再生バッファ $\mathcal{B}$ に蓄積
3. 定期的に（$N_{\text{update}}$ ステップごと）$f_\theta$ を更新：
   $$
   \theta \leftarrow \theta - \alpha_{\theta}^{\text{fine}}\,\nabla_\theta\mathcal{L}_{\text{FEP}}
   $$
4. 学習率は小さく設定（$\alpha_{\theta}^{\text{fine}} \ll \alpha_\theta$）

#### 2.6.4 時間スケール分離

安定性のため、以下の時間スケール階層を維持：

$$
\tau_\theta \gg \tau_h \gg \tau_u
$$

| 更新対象 | 時間スケール | 頻度 |
|---------|-------------|------|
| 予測器 $f_\theta$ | 遅い（$\tau_\theta$） | 100ステップごと |
| haze $h$ | 中間（$\tau_h$） | EMA（1秒程度） |
| 行為 $\mathbf{u}$ | 速い（$\tau_u$） | 毎ステップ |

#### 2.6.5 学習パラメータ

| パラメータ | 記号 | 推奨値 | 意味 |
|-----------|------|--------|------|
| 事前学習率 | $\alpha_\theta$ | $10^{-3}$ | Phase 1 の学習率 |
| 微調整学習率 | $\alpha_\theta^{\text{fine}}$ | $10^{-4}$ | Phase 2 の学習率 |
| バッファサイズ | $|\mathcal{B}|$ | 10000 | 経験再生容量 |
| 更新頻度 | $N_{\text{update}}$ | 100 | オンライン更新間隔 |
| バッチサイズ | $B$ | 64 | ミニバッチサイズ |

---

## 3. SPM v2.0 仕様（12×12×10ch）

### 3.1 チャネル一覧

| ch | 種別 | ID | 名称 | 生成方法 | 学習対象 | $\mathbf{u}$微分 |
|---:|:---:|:---:|---|---|:---:|:---:|
| 0 | Truth | T0 | obs occupancy (teacher) | 観測 | × | × |
| 1 | PR | R0 | **Δoccupancy (+1)** | 学習器 | **○** | **○** |
| 2 | PR | R1 | uncertainty of R0 | 予測副産物 | × | × |
| 3 | PF | F0 | occupancy (current) | 観測 | × | × |
| 4 | PF | F1 | **motion pressure** | 観測 | × | × |
| 5 | PF | F2 | saliency | 観測 | × | × |
| 6 | PF | F3 | TTC proxy | 観測 | × | × |
| 7 | PF | F4 | visibility | 観測 | × | × |
| 8 | PF | F5 | observation stability | 観測 | × | × |
| 9 | Meta | M0 | **haze field** | 推定 | × | ×（Stop-Grad） |

### 3.2 グリッド構造

- **形状**：$C \times 12 \times 12$（$C=10$チャネル）
- **角度**：$a = 0..11$（周期、$\Delta\theta = 2\pi/12$）
- **距離**：$b = 0..11$（非周期、Neumann境界）
- **値域**：原則$[0,1]$、Δ占有のみ$[-1,1]$

---

## 4. Intrinsic Haze推定と破れたマルコフブランケット

### 4.1 Hazeの観測量（推定材料）

Hazeは単一の誤差ではなく、**誤差の"構造"**から推定する：

1. **予測残差**：$e_{\Delta O}[k] = |\Delta O[k] - \widehat{\Delta O}[k]|$
2. **観測不安定性**：$\text{F5}[k] = \text{Std}(O^{\text{obs}}[k-w+1:k])$
3. **予測不確実性**：$\text{R1}[k] = \widehat{\Sigma}_{\Delta O}[k+1]$
4. **不可視性**：$1 - \text{F4}[k]$

### 4.2 Hazeフィールド更新

$$
\widetilde{h}[k] = \sigma\Big(
a\,\text{EMA}(e_{\Delta O}) + b\,\text{R1}[k] + c\,(1-\text{F4}[k]) + d\,\text{F5}[k]
\Big)
$$

$$
h[k] = \text{Smooth}_{\rho,\theta}(\widetilde{h}[k])
$$

### 4.3 βと結合強度の数学的関係式

MB破れの強度$\beta$を、**近傍hazeの影響度**として定式化する：

$$
h_{\text{eff},i}[k] = (1-\beta)\,h_i[k] + \beta\,\langle h_j[k] \rangle_{j \in N_i}
$$

### 4.4 相転移理論との接続（Isingモデル類似）

**平均場近似**による臨界点：
$$
\beta_c = \frac{1}{z\,\bar{J}}
$$

**相図の予測**：

| 領域 | $\beta$ | 状態 | 群行動 |
|------|---------|------|--------|
| 無秩序相 | $\beta < \beta_c$ | 低結合 | 個体独立、渋滞、局所最適 |
| 臨界相（Edge of Chaos） | $\beta \approx \beta_c$ | 臨界 | 創発的秩序、高適応性 |
| 秩序相 | $\beta > \beta_c$ | 高結合 | 過同調、探索死、崩壊 |

### 4.5 臨界点における適応性最大化【v2.1 中心的主張】

#### 4.5.1 秩序パラメータと応答関数

**秩序パラメータ**（haze場の不均一性）：
$$
\phi(\beta) = \frac{1}{N} \sum_{i=1}^N |h_i - \bar{h}|
$$

**応答関数（帯磁率に相当）**：
$$
\chi(\beta) = N \left( \langle \phi^2 \rangle - \langle \phi \rangle^2 \right)
$$

**臨界点の特性**：
- $\beta < \beta_c$：$\phi \approx 0$（均一状態）、$\chi$ 小
- $\beta = \beta_c$：$\chi$ **最大**（臨界揺らぎ）
- $\beta > \beta_c$：$\phi > 0$（パターン形成）、$\chi$ 減少

#### 4.5.2 適応性の定義と最大化

**環境変化への適応性** $A(\beta)$ を以下で定義：

$$
A(\beta) = \frac{\partial \langle \mathbf{u} \rangle}{\partial \mathbf{s}_{\text{env}}} \bigg|_{\mathbf{s}_{\text{env}} = \mathbf{s}_0}
$$

ここで $\mathbf{s}_{\text{env}}$ は環境状態（障害物配置、目標位置など）。

**主張（中心定理）**：
> 適応性 $A(\beta)$ は臨界点 $\beta = \beta_c$ で最大化される。

**直感的説明**：
- $\beta < \beta_c$：個体が独立 → 環境変化に個別対応 → 協調的適応が遅い
- $\beta = \beta_c$：臨界揺らぎ → 微小な環境変化が群全体に伝播 → **最大感度**
- $\beta > \beta_c$：過同調 → 過剰反応・不安定 → 適応精度が低い

#### 4.5.3 相関長と情報伝播

**相関長**（haze場の空間相関）：
$$
\xi(\beta) = \sqrt{\frac{\sum_{i,j} |\mathbf{r}_i - \mathbf{r}_j|^2 \cdot C_{ij}}{\sum_{i,j} C_{ij}}}
$$

ここで $C_{ij} = \langle h_i h_j \rangle - \langle h_i \rangle \langle h_j \rangle$。

**臨界点での発散**：
$$
\xi(\beta) \sim |\beta - \beta_c|^{-\nu}, \quad \nu > 0
$$

**意味**：臨界点で情報（haze変動）が群全体に伝播する。

#### 4.5.4 適応速度（再平衡化時間）

環境変化後の**再平衡化時間** $\tau_{\text{adapt}}(\beta)$：

$$
\tau_{\text{adapt}}(\beta) = \int_0^\infty \left| \frac{V(t) - V^*}{V(0) - V^*} \right| dt
$$

**臨界スローイング・ダウンの修正**：
- 標準的な臨界現象では $\tau \to \infty$（臨界スローイング・ダウン）
- しかし、能動的系（Active Inference）では異なる：
  - 臨界点で**感度が最大**だが、**応答は有限時間で収束**
  - これは「能動的臨界性（Active Criticality）」の特徴

> **定義（能動的臨界性 / Active Criticality）【v2.1追加】**
>
> 能動的エージェント系における臨界現象を**能動的臨界性（Active Criticality）**と呼び、以下の3条件で特徴づける：
>
> **(AC1) 応答関数の最大化**：
> $$\chi(\beta_c) = \max_\beta \chi(\beta)$$
>
> **(AC2) 有限の緩和時間**：
> $$\tau_{\text{adapt}}(\beta_c) < \infty$$
> （通常の臨界現象における $\tau \to \infty$ とは対照的）
>
> **(AC3) 適応効率の最大化**：
> $$E(\beta) \equiv \frac{A(\beta)}{\tau_{\text{adapt}}(\beta)}, \quad E(\beta_c) = \max_\beta E(\beta)$$
>
> **物理的解釈**：
> - **(AC1)** は線形応答理論から導かれる標準的結果
> - **(AC2)** は能動的推論（Active Inference）による持続的なエネルギー注入が、臨界スローイング・ダウンを打ち消すことに起因
> - **(AC3)** は「最大感度 × 有限応答時間」の組み合わせが、適応効率を最大化することを意味
>
> **従来の臨界現象との違い**：
> | 特性 | 平衡系臨界現象 | 能動的臨界性（EPH） |
> |------|--------------|-------------------|
> | 緩和時間 $\tau$ | $\tau \to \infty$ | $\tau < \infty$ |
> | エネルギー注入 | なし（閉じた系） | あり（Active Inference） |
> | 適応効率 $E$ | 定義困難 | 最大化 |

**予測**：$\tau_{\text{adapt}}(\beta_c)$ は最小ではないが、$A(\beta_c) / \tau_{\text{adapt}}(\beta_c)$（適応効率）は最大。

#### 4.5.5 Heins/Kanso との差別化（再掲・強化）

| 観点 | Heins et al. (2024) | Kanso et al. (2024) | EPH v2.1 |
|-----|---------------------|---------------------|----------|
| 主題 | 群集行動の創発 | 閉じ込め下の相転移 | **臨界点での適応性最大化** |
| 臨界点 | パラメータ探索 | 数値発見 | **解析的導出** |
| 適応性 | 言及なし | 言及なし | **中心的主張** |
| β の制御 | 受動的 | 環境依存 | **能動的（haze経由）** |
| 応用 | 記述的 | 記述的 | **規範的（設計原理）** |

---

## 5. 創発的役割分化【v2.1 副次的観察】

> [!NOTE] 位置づけの変更（v2.1）
> 2:6:2 役割分布は、v2.0 では中心的定理として扱ったが、v2.1 では**臨界点近傍で観察されうる創発的現象**として位置づけを変更する。中心的主張は「§4.5 臨界点における適応性最大化」に移動。

### 5.1 背景：パレートの法則と生物学的観察

**生物学的観察（働きアリ研究）**：
- 集団の約20%が高パフォーマー
- 60%が中パフォーマー
- 20%が低パフォーマー（「働かないアリ」）

**驚くべき発見**（長谷川英祐, 北海道大学）：
- 「働かないアリ」を除去しても、残りの中から新たな「働かないアリ」が出現
- これは**群の適応戦略**：疲労した働きアリの代替として待機

### 5.2 EPHへの2:6:2導入

**κ（haze感度）による役割定義**：

| 役割 | 割合 | κ値 | 行動特性 |
|------|------|-----|----------|
| **Leader（開拓者）** | 20% | $\kappa < 0.7$ | haze高でも踏み込む |
| **Follower（追従者）** | 60% | $0.7 \leq \kappa \leq 1.3$ | haze勾配に従う |
| **Reserve（待機者）** | 20% | $\kappa > 1.3$ | 戦略的待機（ロバスト性担保） |

**κ分布の数学的モデル**（混合ガウス分布）：

$$
p(\kappa) = 0.2\,\mathcal{N}(\kappa; 0.5, \sigma^2) + 0.6\,\mathcal{N}(\kappa; 1.0, \sigma^2) + 0.2\,\mathcal{N}(\kappa; 1.5, \sigma^2)
$$

### 5.3 2:6:2とβ相図の相互作用

**仮説**：2:6:2分布とβパラメータの相互作用が群の適応性を決定する。

| β領域 | 2:6:2の効果 |
|-------|-------------|
| $\beta < \beta_c$（低結合） | Leaderのみ活動、Reserveは完全停止 |
| $\beta \approx \beta_c$（臨界） | **最適機能**：Leader→Follower→Reserveの連鎖的役割転換 |
| $\beta > \beta_c$（高結合） | 2:6:2が崩れ、全員同調 |

### 5.4 疲労モデルと動的役割切替

**疲労蓄積**：
$$
\text{fatigue}_i[k+1] = \text{fatigue}_i[k] + \alpha\,\|\mathbf{u}_i[k]\| - \beta_{\text{rest}}\,\mathbb{1}[\|\mathbf{u}_i[k]\| < \epsilon]
$$

**κの動的更新**：
$$
\kappa_i[k+1] = \kappa_i[k] + \gamma\,(\text{fatigue}_i[k] - \text{threshold})^+
$$

疲労が閾値を超えるとκが上昇し、ReserveへMIGRATE。

**リリーフ効果**：
Reserveは疲労回復後にκが低下し、Leader/Followerに復帰可能。

### 5.5 2:6:2の進化的・認知的意義

**進化的意義**（働きアリ研究より）：
- **ロバスト性**：疲労・故障への耐性
- **予備力**：緊急時の即応能力
- **長期生存**：コロニー全体の持続可能性

**認知的意義**（人間群行動より）：
- **リーダーシップの分散**：常に20%がリスクを取る
- **フォロワーの安定流**：60%が効率的移動を形成
- **躊躇者の安全弁**：20%が過度の競争を抑制

### 5.6 2:6:2が創発する条件

**作業仮説**：適切なβとhaze構造の下で、初期均一分布から2:6:2が**創発**する。

**メカニズム**：
1. 高haze領域でκが上昇（躊躇→Reserve化）
2. 低haze領域でκが低下（踏み込み→Leader化）
3. 平均的haze領域でκが安定（Follower維持）

**検証実験**：初期均一κからのκ分布の時間発展を追跡

---

## 6. 収束性と安定性の解析

### 6.1 仮定

**仮定1（リプシッツ連続性）**：
$$
\|\nabla_{\mathbf{u}}f_{\text{EPH}}\| \leq L_f
$$

**仮定2（EMA時定数の下限）**：
$$
\tau_{\text{EMA}} > \tau_{\text{crit}} = \frac{1}{\eta\,L_f^2}
$$

**仮定3（学習率の上限）**：
$$
\eta < \eta_{\max} = \frac{2}{L_f^2 + \lambda_{\max}(\nabla^2 G_{\text{EPH}})}
$$

### 6.2 定理（局所収束）

仮定1-3の下、行為更新はExpected Free Energyの局所的停留点に収束する：
$$
\lim_{k\to\infty}\|\nabla_{\mathbf{u}}G_{\text{EPH}}[k]\| = 0
$$

---

## 7. 実験設計

### 7.1 シナリオ設計

**シナリオ1：Corridor（対向流）**
- 幅10m、長さ50m。各方向20エージェント。

**シナリオ2：Scramble（四方向交差）**
- 10m×10m交差点。各方向10エージェント。

**シナリオ3：Bottleneck（狭路通過）**
- 幅2m狭路。40エージェント一方向。

### 7.2 ベースライン

- Social Force Model (SFM)
- Dynamic Window Approach (DWA)
- 均一κ分布のEPH（2:6:2との比較用）

### 7.3 評価指標

**群行動評価**：

| 指標 | 良い方向 |
|------|----------|
| 衝突回数 $N_{\text{coll}}$ | 低↓ |
| 到達率 $R_{\text{goal}}$ | 高↑ |
| 平均速度 $\bar{v}$ | 高↑ |
| 流れ効率 $\eta_{\text{flow}}$ | 高↑ |
| 回復時間 $T_{\text{rec}}$ | 短↓ |

**2:6:2固有指標**【v2.0】：

| 指標 | 定義 | 意味 |
|------|------|------|
| 役割分布エントロピー | $H(\text{role})$ | 役割の多様性 |
| リリーフ成功率 | Reserve→Leader転換の成功率 | 予備力の有効性 |
| κ分布の収束度 | 初期→定常の変化 | 2:6:2創発の強さ |

### 7.4 β×2:6:2相図実験

**手順**：
1. $\beta \in \{0.0, 0.1, \ldots, 1.0\}$を掃引
2. 各$\beta$で「均一κ」と「2:6:2 κ」を比較
3. 適応性能$A(\beta, \text{distribution})$を計算

---

## 8. 期待される結果

### 8.1 仮説

**H1（PR最小主義の妥当性）**：
PR-ch1（+1 Δ占有）のみの学習でも、衝突回避が成立する。

**H2（Intrinsic Hazeの有効性）**：
内在推定されたhazeは、固定危険マップより適応的である。

**H3（β相図の山型）**：
適応性能$A(\beta)$は$\beta = \beta_c$付近で最大となる。

**H4（ベースライン優位性）**：
EPHはSFM/DWAより高い流れ効率と低い衝突回数を達成する。

**H5（2:6:2分布の優位性）**【v2.0追加】：
2:6:2分布は均一分布より適応性能$A$が高い：
$$
A_{\text{2:6:2}} > A_{\text{uniform}}
$$

特に、擾乱後の回復時間$T_{\text{rec}}$で優位性が顕著。

### 8.2 予測される相図

```
A(β)
  │     ── 2:6:2分布（高い）
  │    ╱╲
  │   ╱  ╲── 均一分布
  │  ╱    ╲
  │ ╱      ╲
  │╱        ╲
  └────────────────→ β
    0    β_c      1
```

---

## 9. 出版戦略（Publication Roadmap）

### 9.1 段階的出版計画

| Phase | 時期 | ターゲット | 内容 |
|-------|------|-----------|------|
| 1 | 2026 Q3 | ICRA/IROS 2027 | EPH基本設計 + SFM/DWA比較 |
| 2 | 2026 Q4 | NeurIPS Workshop + arXiv | 理論的貢献（FEP、相転移、2:6:2） |
| 3 | 2027 Q3 | IEEE T-RO | 完全版（拡張実験、2:6:2検証） |
| 4 | 2027 Q4- | GitHub + ROS 2 | オープンソース公開 |

---

## 参考文献

### Free Energy Principle / Active Inference
- Friston, K. (2010). The free-energy principle: a unified brain theory? *Nature Reviews Neuroscience*, 11(2), 127-138.
- Friston, K., et al. (2024). Designing ecosystems of intelligence from first principles. *Collective Intelligence*, 3(1), 1-19.
- Parr, T., & Friston, K. J. (2019). Generalised free energy and active inference. *Biological Cybernetics*, 113(5), 495-513.
- Palacios, E. R., et al. (2020). On Markov blankets and hierarchical self-organisation. *Journal of Theoretical Biology*, 486, 110089.
- Maisto, D., Nuzzi, D., & Pezzulo, G. (2025). What the flock knows that the birds do not. *arXiv:2511.10835*.
- Wu, H., et al. (2026). Think How Your Teammates Think. *AAAI 2026*.

### 群行動モデル
- Helbing, D., & Molnár, P. (1995). Social force model for pedestrian dynamics. *Physical Review E*, 51(5), 4282.
- Fox, D., et al. (1997). The dynamic window approach to collision avoidance. *IEEE RAM*, 4(1), 23-33.
- Vicsek, T., et al. (1995). Novel type of phase transition in a system of self-driven particles. *PRL*, 75(6), 1226.

### 働きアリ研究・2:6:2
- Hasegawa, E. (長谷川英祐) et al. 働かないアリに関する研究。北海道大学。
- 「働かないアリに意義がある」（2016）。

### 相転移・複雑系（Edge of Chaos / Criticality）
- Langton, C. G. (1990). Computation at the edge of chaos: Phase transitions and emergent computation. *Physica D*, 42(1-3), 12-37.
- Kauffman, S. A. (1993). *The Origins of Order: Self-Organization and Selection in Evolution*. Oxford University Press.
- Bak, P., Tang, C., & Wiesenfeld, K. (1987). Self-organized criticality: An explanation of the 1/f noise. *Physical Review Letters*, 59(4), 381.
- Beggs, J. M., & Plenz, D. (2003). Neuronal avalanches in neocortical circuits. *Journal of Neuroscience*, 23(35), 11167-11177.
- Bar-Yam, Y. (2004). *Dynamics of complex systems*. Westview Press.

---

## Appendices（付録）

本文書の詳細は以下の付録に記載：

| Appendix | タイトル | ファイル | 内容 |
|----------|---------|----------|------|
| **A** | Complete Mathematical Proofs | `appendix/EPH-2.1_appendix-A_proofs.md` | 定理1-3、命題1の完全証明 |
| **B** | SPM Specification | `appendix/EPH-2.1_appendix-B_spm.md` | SPM 12×12×10ch 完全仕様、API |
| **C** | Numerical Validation Plan | `appendix/EPH-2.1_appendix-C_validation.md` | V1-V5検証計画、実装構造 |
| **D** | Related Work Survey | `appendix/EPH-2.1_appendix-D_related-work.md` | 関連論文8件の詳細解説 |

### 実装チェックリスト（Quick Reference）

- [ ] SPM形状：10×12×12（float32）
- [ ] θ wrap / r Neumann の差分実装
- [ ] PR-R0のみ $\mathbf{u}$ 微分
- [ ] F4で損失マスク
- [ ] 2:6:2 κ分布の初期化
- [ ] 疲労モデルと動的κ更新
- [ ] β掃引ログ（相図用）
- [ ] 均一 vs 2:6:2 比較実験

---

*Last updated: 2026-02-02*
*Version: 2.1.0*
*Document ID: EPH-2.1_main*
