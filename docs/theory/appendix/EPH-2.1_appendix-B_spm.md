---
title: "EPH v2.1: Appendix B — Saliency Polar Map (SPM) Specification"
type: Technical_Spec
status: "🟢 Active"
version: 2.1.0
date_created: "2026-02-01"
date_modified: "2026-02-02"
author: "Hiroshi Igarashi"
institution: "Tokyo Denki University"
parent_document: "EPH-2.1_main.md"
appendix_id: "B"
scope:
  - "SPMの全チャネル仕様（PR＋PF＋Meta）"
  - "12×12の(θ,r)極座標格子"
  - "2:6:2役割分布に対応したκ分布モデル"
related_documents:
  - "proposal_EPH_v2.0.md"
keywords:
  - "Saliency Polar Map"
  - "SPM"
  - "Active Inference"
  - "EPH"
  - "2:6:2 Distribution"
  - "Role Heterogeneity"
notes:
  - "STAは本ドキュメントでは扱わない（SPMは共有可だが、予測ホライゾン拡張は別紙）"
  - "ch0は教師/観測の真値スロットとして扱い、学習時のみ参照する"
changelog:
  - version: 2.0.0
    date: "2026-02-01"
    changes:
      - "メジャーバージョン更新：EPH v2.0との整合性確保"
      - "§10に2:6:2役割分布に対応したκ分布モデルを追加"
      - "§10.3に疲労モデル対応のκ動的調整を追加"
      - "§12 APIに役割関連メソッドを追加"
  - version: 8.1.0
    date: "2026-02-01"
    changes:
      - "EPH v8.1との整合性確保"
      - "Free Energy定義との接続を§8に追加"
      - "βによるhaze共有メカニズムを§7.2に追加"
      - "収束性に関する注記を§7.3に追加"
  - version: 8.0.2
    date: "2026-01-31"
    changes:
      - "初版：10チャネル仕様の完全定義"
---

# SPM v2.0: Saliency Polar Map Channel Specification

> [!NOTE] 本書について
> 本書は、EPH (Emergent Perceptual Haze) のための知覚表現 **SPM (Saliency Polar Map)** を、実装・学習・可視化・評価が一貫するように**チャネル（ch）単位で完全仕様化**する。
> EPH v2.0（proposal_EPH_v2.0.md）と併せて参照のこと。

---

# 1. 目的と設計原理

EPH v2.0 の設計原理は次の5点である。

1. **PR最小主義**：予測（世界モデル推定）を要するチャネルは最小限とし、EPHでは**PR-R0のみを学習・予測対象**とする。
2. **PF/PR分離**：PF（prediction-free）は観測から直接構成でき、PR（prediction-required）は学習器が出力する。
3. **hazeは内在推定＋Stop-Gradient**：h(M0)は設計者付与ではなく、予測誤差・不確実性・観測不安定性から推定される「曖昧性の場」。行為更新で**uに関して微分しない**（崩壊防止）。
4. **FEP準拠**：変分Free Energy $F_{\text{EPH}}$およびExpected Free Energy $G_{\text{EPH}}$との理論的整合性を保つ（詳細はEPH v2.0 §2参照）。
5. **役割異質性**【v2.0追加】：2:6:2役割分布に対応したκ分布モデルにより、群れの機能的多様性を実現（詳細はEPH v2.0 §5参照）。

---

# 2. SPMの幾何とデータ構造

## 2.1 グリッド構造（12×12）

SPMは**自己中心極座標格子**で、インデックスを

- 角度 index：$a = 0..11$（周期）
- 距離 index：$b = 0..11$（中心→遠方、非周期）

とする。セルは $(a,b) \equiv (\theta_a, r_b)$ を表す。

- $\Delta\theta = 2\pi / 12 = 30°$
- $\Delta r$ は環境スケールに応じて定義（線形 or 対数分割）

### 角度方向の参照（wrap）
$a \pm 1$ は $\bmod 12$ で循環する。

### 半径方向の参照（Neumann推奨）
$b$ の端（0, 11）は反射境界（法線勾配0）を採用する（詳細は§7）。

## 2.2 テンソル形状

時刻$[k]$でのSPMテンソルを

$$
\text{SPM}[k] \in \mathbb{R}^{C \times 12 \times 12}
$$

とする（$C$はチャネル数）。EPH v2.0では

- **T0..M0の10スロット**を定義

とし、実装上はfloat32を推奨する。

## 2.3 値域・正規化（推奨）

原則として**各チャネルは$[0,1]$に正規化**する（学習・可視化・ゲートの安定化のため）。

例外：
- **PR-R0（Δ占有）**：符号付き$[-1,1]$（増加=+、減少=-）
- **不確実性・haze**：$[0,1]$

---

# 3. チャネル一覧（EPH v2.0）

## 3.0 チャネルID体系（論文表記）

- Truth: **T0**
- PR: **R0 (Δoccupancy +1)**, **R1 (uncertainty)**
- PF: **F0..F5**
- Meta: **M0 (haze)**

## 3.1 全チャネル表

| ch | 区分 | ID | 名称 | 形状 | 生成 | 学習対象 | $\mathbf{u}$微分 | 主用途 |
|---:|:---:|:---:|:---|:---|:---|:---:|:---:|:---|
| 0 | Truth | T0 | obs occupancy (teacher) | 12×12 | 観測 | × | × | 教師（Δ真値生成） |
| 1 | PR | R0 | **Δoccupancy (+1)** | 12×12 | 学習器 | **○** | **○** | 行為勾配の核 |
| 2 | PR | R1 | uncertainty of R0 | 12×12 | 予測副産物 | × | × | ゲート／haze入力 |
| 3 | PF | F0 | occupancy (current) | 12×12 | 観測 | × | × | 入力／マスク |
| 4 | PF | F1 | **motion pressure** | 12×12 | 観測 | × | × | 躊躇境界／PF制約 |
| 5 | PF | F2 | saliency | 12×12 | 観測 | × | × | 入力（注意） |
| 6 | PF | F3 | TTC proxy | 12×12 | 観測 | × | × | PF安全制約 |
| 7 | PF | F4 | visibility | 12×12 | 観測 | × | × | 損失マスク／ゲート |
| 8 | PF | F5 | observation stability | 12×12 | 観測 | × | × | haze入力（不安定性） |
| 9 | Meta | M0 | **haze field** | 12×12 | 推定 | × | ×（Stop-Grad） | 勾配ゲート／相図 |

> **設計原理**：EPH v2.0では、**PR-R0のみ**を「学習・予測」する。
> PR-R1は学習器の副産物（分散・アンサンブルなど）として定義し、損失で直接教師付けしない。

---

# 4. チャネル詳細仕様（T0〜R1）

## T0: obs occupancy (teacher)

### 意味
観測から得られる「現在の占有/存在」真値。学習時の教師生成に用いる。

### 定義（例）
- LiDAR/Depth/Segmentation等からoccupancyを推定し、$(θ,r)$格子に射影
- 値域：0（空）〜1（占有）

### 備考
- 直接行為に使わない（学習の基準）
- 観測欠損はF4で表し、T0は「未観測=0」ではなく、別途マスクで扱う

---

## R0: PR Δoccupancy (+1) **(学習・予測対象)**

### 意味
次ステップにおける占有変化（Δ）を予測する。**+1ホライゾン固定**。

### 教師（真値）
$$
\Delta O^{\text{true}}[k+1] = O^{\text{obs}}[k+1] - O^{\text{obs}}[k]
$$

### 予測
$$
\widehat{\Delta O}[k+1] = f_{\text{EPH}}(\text{PF}[k],\ \mathbf{u}[k])
$$

- 入力PF：F0〜F5（必要なら簡易履歴を付与）
- 出力：12×12

### 値域
- 推奨：clipして$[-1,1]$
- あるいはtanhにより安定化

### 行為微分
- **$\mathbf{u}$に関して微分するのはR0のみ**（EPHの核）

### Free Energyとの接続
EPH v2.0 §2.2で定義された変分Free Energy：
$$
F_{\text{EPH}}[k] = \frac{1}{2\sigma^2}\sum_{\theta,r}\text{F4}[k]\cdot\big(\Delta O[k+1] - \widehat{\Delta O}[k+1]\big)^2 + \frac{\lambda}{2}\sum_{\theta,r}\widehat{\Sigma}_{\Delta O}[k+1]
$$

において、第1項の予測誤差はR0が担う。

---

## R1: PR uncertainty of R0 **(副産物)**

### 意味
R0の予測不確実性（予測分布の分散等）。行為更新では**Stop-Gradient**。

### 推定例（アンサンブル）
$$
\text{R1}[k](\theta,r) = \text{Var}\big(\widehat{\Delta O}^{(m)}[k+1](\theta,r)\big)
$$

ここで$m = 1..M$はアンサンブルメンバー。

### 推定例（MC-Dropout）
$$
\text{R1}[k](\theta,r) = \text{Var}\big(\widehat{\Delta O}^{(\text{drop})}[k+1](\theta,r)\big)
$$

### 正規化
- 分散を$[0,1]$にマッピング（例：min-max or tanh）
- **大=不確実**とする

### 用途
- ゲート：$(1 - \text{R1})$
- haze推定入力
- Free Energy第2項（EPH v2.0 §2.2）

---

# 5. チャネル詳細仕様（F0〜F3：身体性PF）

## F0: PF occupancy (current)

### 意味
T0と同種だが、「現在入力として用いる観測占有」。

### 値域
- $[0,1]$

### 用途
- PR予測器の入力
- 物理的マスク（あり得ない領域の除外）

---

## F1: PF motion pressure（運動圧）

### 意味
「相対運動がどれだけ切迫しているか」を、予測なしに推定するスカラー場。
**"踏み込みを躊躇する境界"**の主要因。

### 入力
- 相対速度$\mathbf{v}[k]$（自己中心）
- 視線方向単位ベクトル$\hat{\mathbf{e}}_r$
- 距離$r$
- 角度$\theta$

### 推奨定義

接近成分（closing speed）：
$$
v_{\text{in}}[k] = \max(0, -\mathbf{v}[k]\cdot \hat{\mathbf{e}}_r)
$$

距離・視野重み：
$$
\text{F1}[k](\theta,r) = \tanh\Big(
\alpha\ v_{\text{in}}[k]\ \frac{\max(0,\cos\theta)}{r+\epsilon}
\Big)
$$

- 近距離・正面・接近ほど大
- 値域：$[0,1)$（tanh）
- 推奨：$\alpha = 1.0$、$\epsilon = 0.1$ [m]

### 可視化（推奨）
- 前方扇形（±90°）を強調
- 青→黄→赤（危険系カラーマップ）
- 残像（指数平滑）を入れて「ヒヤッと感」を表現

### 群表示（推奨）
- 自己F1を主表示（高彩度）
- 他者はshadow（blur＋低彩度）として重ね、足し算で個体性を潰さない

---

## F2: PF saliency（顕著性）

### 意味
「目立つ／注意を引く」領域の強度。タスク報酬ではなく、感覚の強さ。

### 例（構成案）
- コントラスト（深度勾配）
- 物体カテゴリ重要度（例：動的=高）
- 近接強調

### 値域
- $[0,1]$

### 用途
- PR予測器の入力
- 可視化で直感を作る

---

## F3: PF TTC proxy（Time-to-Collision 近似）

### 意味
衝突までの時間の近似（予測不要で計算可能なproxy）。安全制約に使う。

### 推奨定義（例）
$$
\text{TTC} \approx \frac{r}{v_{\text{in}}+\epsilon}
$$

これを危険度に変換：
$$
\text{F3} = \text{clip}\left(\frac{1}{\text{TTC}+1}\right)
$$

- TTCが短いほどF3が大

### 用途
- PF安全集合への射影
- 速度制限・旋回制限等の局所制約
- Fallback制御（EPH v2.0 §6参照）

---

# 6. チャネル詳細仕様（F4〜F5：観測性PF）

## F4: PF visibility（可視性）

### 意味
セルが「観測されているか」の重み。遮蔽・欠損を明示する。

### 値域
- $[0,1]$（1=完全可視、0=不可視）

### 用途
- 損失マスク：F4が低い領域は教師誤差を数えない
- ゲート：F4が低い領域は行為勾配を通さない
- Free Energy計算：$F_{\text{EPH}}$の第1項でF4を重みとして使用

---

## F5: PF observation stability（観測安定性）

### 意味
観測が時間的に不安定（ちらつき・欠損揺れ）な領域を表す。
haze推定の重要入力。

### 推奨定義（例）
短窓でのoccupancyの変動：
$$
\text{F5}[k] = \text{Std}\big(O^{\text{obs}}[k-w+1..k]\big)
$$

正規化して$[0,1]$。推奨：$w = 5$ステップ。

### 用途
- haze推定：不安定領域でhazeが立ちやすい

---

# 7. Metaチャネル：haze と ∇haze

## 7.1 M0: Meta haze field（曖昧性の場）

### 意味
設計者付与の危険マップではない。
**予測誤差構造・不確実性・観測不安定性・不可視性から推定される「知覚的曖昧性のフィールド」**。

### 推奨推定式

誤差（可視領域のみ）：
$$
e[k] = \text{F4}[k]\ \big|\widehat{\Delta O}[k+1]-\Delta O^{\text{true}}[k+1]\big|
$$

haze（重み付き合成）：
$$
\widetilde{h}[k] = \sigma\Big(
a\,\text{EMA}(e[k]) + b\,\text{R1}[k] + c\,(1-\text{F4}[k]) + d\,\text{F5}[k]
\Big)
$$

空間平滑化：
$$
h[k] = \text{Smooth}_{\rho,\theta}(\widetilde{h}[k])
$$

### 推奨パラメータ
| パラメータ | 推奨値 | 意味 |
|-----------|--------|------|
| $a$ | 0.4 | 予測誤差の重み |
| $b$ | 0.3 | 予測不確実性の重み |
| $c$ | 0.2 | 不可視性の重み |
| $d$ | 0.1 | 観測不安定性の重み |
| $\tau_{\text{EMA}}$ | 1.0 s | EMA時定数 |
| $\sigma_{\text{smooth}}$ | 1セル | ガウシアン平滑化幅 |

### 値域
- $[0,1]$

### Stop-Gradient
- **haze(M0)は$\mathbf{u}$で微分しない**（崩壊防止）
- 実装上はdetach / stop_gradientを適用

## 7.2 βによるhaze共有メカニズム

マルチエージェント環境では、MB破れ強度$\beta$によりhazeを共有する：

$$
h_{\text{eff},i}[k] = (1-\beta)\,h_i[k] + \beta\,\langle h_j[k] \rangle_{j \in N_i}
$$

ここで：
- $h_i[k]$：エージェント$i$の局所haze場
- $\langle h_j \rangle_{j \in N_i}$：近傍エージェントのhaze場の平均
- $N_i$：エージェント$i$の近傍集合（通信範囲内）

**βの解釈**：
- $\beta = 0$：完全MB（自己hazeのみ）
- $0 < \beta < 1$：部分的MB破れ
- $\beta = 1$：完全共有

## 7.3 収束性に関する注記

EPH v2.0 §6で示された収束性を保証するため、EMA時定数は以下を満たす必要がある：

$$
\tau_{\text{EMA}} > \tau_{\text{crit}} = \frac{1}{\eta\,L_f^2}
$$

ここで$\eta$は学習率、$L_f$は予測器のリプシッツ定数。

**推奨**：$\tau_{\text{EMA}} \geq 1.0$ [s]（$\Delta t = 0.1$ sで10ステップ分）

---

## 7.4 ∇haze（派生量）

hazeの境界を検出するため、勾配を計算する。
θ方向は周期（wrap）、r方向は反射Neumann。

### 角度差分
$$
\partial_\theta h[a,b] = \frac{h[(a+1)\bmod 12,b]-h[(a-1)\bmod 12,b]}{2\,\Delta\theta}
$$

### 半径差分
$$
\partial_r h[a,b] =
\begin{cases}
\frac{h[a,b+1]-h[a,b-1]}{2\,\Delta r} & (1\le b\le 10)\\
0 & (b\in\{0,11\})\ \text{(Neumann)}
\end{cases}
$$

### 極座標補正（角度成分）
$$
\nabla h[a,b] =
\Big(
\partial_r h[a,b],\ \frac{1}{r_b+\epsilon}\partial_\theta h[a,b]
\Big)
$$

### 勾配の大きさ
$$
\|\nabla h[a,b]\| =
\sqrt{(\partial_r h)^2+\left(\frac{1}{r_b+\epsilon}\partial_\theta h\right)^2}
$$

---

# 8. EPHでの行為更新への接続

## 8.1 Expected Free Energyとの接続

EPH v2.0 §2.3で定義されたExpected Free Energy：

$$
G_{\text{EPH}}(\mathbf{u}[k]) = \sum_{\theta,r}\text{F4}[k]\cdot\widehat{\Delta O}[k+1;\mathbf{u}]^2 + \lambda\,\widehat{\Sigma}_{\Delta O}[k+1;\mathbf{u}]
$$

を最小化するように行為を更新する。

## 8.2 勾配ゲート

行為更新はR0のみ微分対象とし、R1/F4/M0はゲートとして使う。

$$
G_{\text{EPH}}[k] = \underbrace{(1-\text{R1}[k])}_{\text{uncertainty}} \cdot \underbrace{\text{F4}[k]}_{\text{visibility}} \cdot \underbrace{(1-\kappa\,\text{sg}[h[k]])}_{\text{haze}}
$$

**v2.0追加**：κは個体ごとに異なる値を取り、2:6:2役割分布に従う（§10参照）。

## 8.3 行為更新式

$$
\mathbf{u}[k] = \mathbf{u}[k-1] - \eta\ \sum_{\theta,r}
G_{\text{EPH}}[k](\theta,r)\ \nabla_{\mathbf{u}} \widehat{\Delta O}[k+1](\theta,r)
$$

---

# 9. "踏み込みを躊躇する境界"の可視化指標

EPHでは、躊躇境界を

$$
\text{Hesitation Boundary} = \text{F1} \times \|\nabla h\|
$$

で表す。直感的には、

- F1（運動圧）が高い場所で
- hazeが急に濃くなる（$\|\nabla h\|$が大）方向

に対して踏み込みが抑制される。

### 可視化推奨
- 躊躇境界を等高線として描画
- F1をベースカラー、$\|\nabla h\|$をオーバーレイ
- 時間変化をアニメーション

---

# 10. 群拡張：役割分布とκ異質性【v2.0拡充】

## 10.1 κ分布モデル（2:6:2法則）

EPH v2.0 §5で導入された2:6:2役割分布に対応し、κを混合ガウス分布でモデル化する：

$$
\kappa_i \sim 0.2\,\mathcal{N}(\kappa_L, \sigma_L^2) + 0.6\,\mathcal{N}(\kappa_F, \sigma_F^2) + 0.2\,\mathcal{N}(\kappa_R, \sigma_R^2)
$$

### 役割パラメータ（推奨値）

| 役割 | 割合 | κ平均 | κ標準偏差 | 効果 |
|------|-----|-------|----------|------|
| Leader (L) | 20% | 0.3 | 0.1 | haze遮断を緩め、積極的に踏み込む |
| Follower (F) | 60% | 1.0 | 0.2 | 標準的なhaze感度 |
| Reserve (R) | 20% | 1.5 | 0.15 | haze遮断を強め、安全側で待機 |

### 有効haze感度

$$
\kappa_{\text{eff},i}[k] = \kappa_i \cdot (1 + \gamma \cdot \text{fatigue}_i[k])
$$

ここで$\gamma$は疲労感度係数（推奨：$\gamma = 0.3$）。

## 10.2 役割縮約スコア

各エージェント$i$の局所場を、役割判定のためにスカラーへ縮約する。
縮約は**行為更新には使わず、役割切替判定のみに使う**。

### 推奨：前方扇形×TopP平均

$$
S_i[k] = \text{Reduce}\Big(
\text{F1}_i[k]\cdot \|\nabla h_i[k]\|\cdot |\Delta h_i[k]|
\Big)
$$

- Reduceの例：前方±90°内の上位10%セル平均
- ヒステリシス／クールダウンでチャタリングを抑制

## 10.3 疲労モデルと動的役割切替【v2.0追加】

EPH v2.0 §5.3で定義された疲労モデル：

$$
\text{fatigue}_i[k+1] = \alpha_{\text{fat}}\,\text{fatigue}_i[k] + (1-\alpha_{\text{fat}})\,\mathbb{1}[\text{role}_i[k] = L]
$$

ここで$\alpha_{\text{fat}} = 0.99$（約100ステップで飽和）。

### 役割切替トリガー

$$
P(\text{role switch}_i) = \sigma\Big(\tau^{-1}\big(\text{fatigue}_i[k] - \theta_{\text{fat}}\big)\Big)
$$

- $\theta_{\text{fat}} = 0.7$：疲労閾値
- $\tau = 0.1$：温度パラメータ

### 切替時のκ更新

役割切替時、新しい役割に対応するκ分布から再サンプリング：
$$
\kappa_i^{\text{new}} \sim p(\kappa | \text{role}_i^{\text{new}})
$$

## 10.4 役割分布のモニタリング

群全体の役割分布を監視し、2:6:2からの逸脱を検出：

$$
D_{\text{role}}[k] = D_{\text{KL}}\Big(p_{\text{actual}}(\text{role})\,\|\,p_{\text{target}}\Big)
$$

ここで$p_{\text{target}} = (0.2, 0.6, 0.2)$。

---

# 11. 実装チェックリスト（EPH v2.0準拠）

## 必須項目
- [ ] SPM形状：10×12×12（float32）
- [ ] θ wrap / r Neumann の差分実装
- [ ] R0のみ$\mathbf{u}$微分（R1/F4/M0はstop-gradient）
- [ ] F4で損失マスク（未観測セルを誤差計算しない）
- [ ] EMA時定数$\tau_{\text{EMA}} \geq 1.0$ s（収束性保証）
- [ ] κの2:6:2分布に基づく初期化【v2.0】
- [ ] 疲労モデルの実装（$\alpha_{\text{fat}} = 0.99$）【v2.0】

## 可視化
- [ ] F1を扇形ヒートマップで可視化（自己主・他者shadow）
- [ ] hazeと∇hazeを同一格子で可視化（境界が見える）
- [ ] 躊躇境界（F1×∇haze）の等高線
- [ ] 役割分布ヒストグラム（2:6:2の確認）【v2.0】
- [ ] κ分布の時間変化（役割切替の追跡）【v2.0】

## 実験
- [ ] β掃引ログ（$\beta = 0.0, 0.1, ..., 1.0$）
- [ ] 各チャネルの値域チェック（正規化確認）
- [ ] 役割切替時のhaze跳ね（ΔH）記録
- [ ] 役割分布のKLダイバージェンス監視【v2.0】
- [ ] 疲労ダイナミクスの可視化【v2.0】

---

# 12. API仕様（参考）

```python
class SPM:
    """Saliency Polar Map for EPH v2.0"""

    def __init__(self, n_theta: int = 12, n_r: int = 12):
        self.shape = (10, n_theta, n_r)  # C x θ x r
        self.channels = {
            'T0': 0,  # obs occupancy (teacher)
            'R0': 1,  # Δoccupancy (+1) - LEARNABLE
            'R1': 2,  # uncertainty of R0
            'F0': 3,  # occupancy (current)
            'F1': 4,  # motion pressure
            'F2': 5,  # saliency
            'F3': 6,  # TTC proxy
            'F4': 7,  # visibility
            'F5': 8,  # observation stability
            'M0': 9,  # haze field
        }

    def compute_pf(self, observations: dict) -> torch.Tensor:
        """Compute Prediction-Free channels from observations."""
        pass

    def predict_pr(self, pf: torch.Tensor, u: torch.Tensor) -> torch.Tensor:
        """Predict PR-R0 (Δoccupancy) using learned model."""
        pass

    def estimate_haze(self,
                      prediction_error: torch.Tensor,
                      uncertainty: torch.Tensor,
                      visibility: torch.Tensor,
                      stability: torch.Tensor,
                      tau_ema: float = 1.0) -> torch.Tensor:
        """Estimate haze field from error structure."""
        pass

    def compute_gradient_gate(self,
                               r1: torch.Tensor,
                               f4: torch.Tensor,
                               haze: torch.Tensor,
                               kappa: float = 1.0) -> torch.Tensor:
        """Compute EPH gradient gate (with stop-gradient on haze)."""
        haze_sg = haze.detach()  # Stop-Gradient
        return (1 - r1) * f4 * (1 - kappa * haze_sg)

    def share_haze(self,
                   local_haze: torch.Tensor,
                   neighbor_hazes: List[torch.Tensor],
                   beta: float) -> torch.Tensor:
        """Share haze with neighbors (MB breaking)."""
        if len(neighbor_hazes) == 0:
            return local_haze
        neighbor_mean = torch.stack(neighbor_hazes).mean(dim=0)
        return (1 - beta) * local_haze + beta * neighbor_mean


class RoleDistribution:
    """2:6:2 Role Distribution Manager for EPH v2.0"""

    def __init__(self,
                 n_agents: int,
                 kappa_L: float = 0.3,
                 kappa_F: float = 1.0,
                 kappa_R: float = 1.5,
                 sigma_L: float = 0.1,
                 sigma_F: float = 0.2,
                 sigma_R: float = 0.15):
        self.n_agents = n_agents
        self.role_params = {
            'Leader': {'mean': kappa_L, 'std': sigma_L, 'ratio': 0.2},
            'Follower': {'mean': kappa_F, 'std': sigma_F, 'ratio': 0.6},
            'Reserve': {'mean': kappa_R, 'std': sigma_R, 'ratio': 0.2},
        }
        self.roles = self._initialize_roles()
        self.kappas = self._sample_kappas()
        self.fatigue = torch.zeros(n_agents)

    def _initialize_roles(self) -> List[str]:
        """Initialize roles according to 2:6:2 distribution."""
        n_L = int(self.n_agents * 0.2)
        n_R = int(self.n_agents * 0.2)
        n_F = self.n_agents - n_L - n_R
        roles = ['Leader'] * n_L + ['Follower'] * n_F + ['Reserve'] * n_R
        random.shuffle(roles)
        return roles

    def _sample_kappas(self) -> torch.Tensor:
        """Sample kappa values based on current roles."""
        kappas = torch.zeros(self.n_agents)
        for i, role in enumerate(self.roles):
            params = self.role_params[role]
            kappas[i] = torch.normal(
                torch.tensor(params['mean']),
                torch.tensor(params['std'])
            ).clamp(min=0.1, max=2.0)
        return kappas

    def update_fatigue(self, alpha_fat: float = 0.99) -> None:
        """Update fatigue based on current roles."""
        is_leader = torch.tensor([r == 'Leader' for r in self.roles], dtype=torch.float32)
        self.fatigue = alpha_fat * self.fatigue + (1 - alpha_fat) * is_leader

    def check_role_switch(self,
                          theta_fat: float = 0.7,
                          tau: float = 0.1) -> List[int]:
        """Check which agents should switch roles due to fatigue."""
        switch_prob = torch.sigmoid((self.fatigue - theta_fat) / tau)
        switches = torch.rand(self.n_agents) < switch_prob
        return switches.nonzero().flatten().tolist()

    def execute_role_switch(self, agent_idx: int) -> None:
        """Execute role switch for a fatigued agent."""
        old_role = self.roles[agent_idx]
        if old_role == 'Leader':
            # Leader becomes Reserve, random Follower becomes Leader
            self.roles[agent_idx] = 'Reserve'
            followers = [i for i, r in enumerate(self.roles) if r == 'Follower']
            if followers:
                new_leader = random.choice(followers)
                self.roles[new_leader] = 'Leader'
                self.kappas[new_leader] = torch.normal(
                    torch.tensor(self.role_params['Leader']['mean']),
                    torch.tensor(self.role_params['Leader']['std'])
                ).clamp(min=0.1, max=2.0)
            self.fatigue[agent_idx] = 0.0  # Reset fatigue
            self.kappas[agent_idx] = torch.normal(
                torch.tensor(self.role_params['Reserve']['mean']),
                torch.tensor(self.role_params['Reserve']['std'])
            ).clamp(min=0.1, max=2.0)

    def get_effective_kappa(self, gamma: float = 0.3) -> torch.Tensor:
        """Get effective kappa considering fatigue."""
        return self.kappas * (1 + gamma * self.fatigue)

    def get_role_distribution_kl(self) -> float:
        """Compute KL divergence from target 2:6:2 distribution."""
        actual = torch.tensor([
            sum(1 for r in self.roles if r == 'Leader') / self.n_agents,
            sum(1 for r in self.roles if r == 'Follower') / self.n_agents,
            sum(1 for r in self.roles if r == 'Reserve') / self.n_agents,
        ])
        target = torch.tensor([0.2, 0.6, 0.2])
        return F.kl_div(actual.log(), target, reduction='sum').item()
```

---

*Last updated: 2026-02-01*
*Related: proposal_EPH_v2.0.md*
