# EPH v2.1: Emergent Phenomena in Human-swarm interaction

> **Free Energy Principle に基づくマルチエージェントシミュレーション研究プロジェクト**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)

---

## 📚 論文・引用

**関連論文:**
- Igarashi, H. et al. (2026). "Phase Transitions in Active Inference Swarms" *(投稿準備中)*
- 先行研究: [EPH v2.1 Research Proposal](docs/theory/EPH-2.1_main.md)

**引用方法:**
```bibtex
@software{igarashi2026eph,
  author = {Igarashi, Hiroshi},
  title = {EPH v2.1: Emergent Phenomena in Human-swarm interaction},
  year = {2026},
  url = {https://github.com/rash0402/crlEPH2}
}
```

---

## 🎯 研究目的

Active Inference理論に基づくマルチエージェントシステムにおける相転移現象の解明：

1. **Markov Blanket Breaking** 現象の観測と定量化
2. β（ノイズ強度）による秩序-無秩序相転移の解析
3. Saliency Polar Map (SPM) による認知的注意メカニズムの実装
4. 群知能における創発現象のリアルタイム可視化

---

## 🚀 クイックスタート

### 環境構築（初回のみ）

```bash
# 1. リポジトリクローン
git clone https://github.com/rash0402/crlEPH2.git
cd crlEPH2

# 2. Git LFS有効化（大容量データ管理）
git lfs install
git lfs pull

# 3. Python環境構築
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

# 4. C++サーバービルド
./scripts/build/build_cpp_server.sh

# 5. 動作確認
./scripts/experiment/run_experiment.py --config experiments/configs/baseline.yaml --dry-run
```

### GUIの起動

```bash
# 自動実験スクリプト（C++サーバー + Python GUI）
./run.sh

# または手動起動
# Terminal 1: C++ simulation server
./build/eph_gui_server

# Terminal 2: Python GUI
cd gui && python main.py
```

---

## 📂 プロジェクト構造

```
crlEPH2/
├── packages/           # 再利用可能なC++ライブラリ（eph_core, eph_agent, eph_swarm...）
├── src/                # アプリケーションコード（cpp_server, gui）
├── experiments/        # 実験管理（configs, data, protocols, results）
├── analysis/           # データ解析・可視化スクリプト
├── tests/              # テストコード（unit, integration, validation）
├── docs/               # ドキュメント（理論、API、論文）
└── scripts/            # ビルド・実験実行スクリプト
```

詳細: [プロジェクト構造ガイド](docs/user_guide/project_structure.md)

---

## 🧪 実験の実行方法

### 1. 単一実験の実行

```bash
# 設定ファイルを指定して実験実行
python scripts/experiment/run_experiment.py \
  --config experiments/configs/beta_sweep.yaml \
  --output experiments/data/raw/2026-02-15_beta-sweep
```

### 2. パラメータスイープ

```bash
# β値を0.05から0.15まで0.01刻みでスイープ
python scripts/experiment/run_parameter_sweep.py \
  --param beta \
  --range 0.05 0.15 0.01 \
  --base-config experiments/configs/baseline.yaml
```

### 3. バッチ実験

```bash
# 複数設定を一括実行（夜間バッチ推奨）
./scripts/experiment/batch_experiments.sh experiments/configs/batch_list.txt
```

**実験プロトコル:** [experiments/protocols/README.md](experiments/protocols/README.md)

---

## 📊 データ解析

### Jupyter Notebookでの解析

```bash
cd experiments/notebooks
jupyter lab
# → 01_exploratory_analysis.ipynb を開く
```

### スクリプトによる図表生成

```bash
# 相転移図の生成
python analysis/visualization/plot_phase_diagram.py \
  --input experiments/data/processed/2026-02-15_beta-sweep/aggregated.csv \
  --output experiments/results/2026-02-15_beta-sweep/figures/phase_diagram.pdf

# SPMヒートマップ生成
python analysis/visualization/plot_spm_heatmap.py \
  --agent-id 5 \
  --timestep 1000 \
  --input experiments/data/raw/2026-02-15_beta-sweep/run_001.h5
```

---

## 🧬 システムアーキテクチャ

### コンポーネント構成

```
┌─────────────────────────────────────────────────────┐
│  PyQt6 GUI (Python)                                 │
│  - Global View (matplotlib)                         │
│  - SPM Visualization                                │
│  - Parameter Panel                                  │
└───────────────┬─────────────────────────────────────┘
                │ UDP (Binary Protocol)
                │ Port 5555: State data (C++ → Python)
                │ Port 5556: Commands (Python → C++)
┌───────────────▼─────────────────────────────────────┐
│  C++ Simulation Server                              │
│  - SwarmManager (N agents)                          │
│  - PhaseAnalyzer (φ, χ calculation)                 │
│  - SaliencyPolarMap (12×12 grid)                    │
└─────────────────────────────────────────────────────┘
```

詳細: [docs/architecture/system_overview.md](docs/architecture/system_overview.md)

### 主要パラメータ

| パラメータ | 記号 | デフォルト値 | 範囲 | 説明 |
|-----------|------|-------------|------|------|
| ノイズ強度 | β | 0.098 | 0.01-0.20 | 相転移の制御パラメータ |
| 学習率 | η | 0.01 | 0.001-0.1 | Haze場の更新速度 |
| エージェント数 | N | 10 | 5-200 | 群れサイズ |
| 視野角 | FOV | 270° | - | 前方270度（後方90度は盲点） |
| 近傍数 | k | 6 | 3-10 | 平均近傍エージェント数 |

---

## 📈 評価指標

### 秩序パラメータ（Order Parameter）

**φ(t)**: 速度場の秩序度
```
φ(t) = |⟨v̂ᵢ⟩| ∈ [0, 1]
```
- φ ≈ 1: 高秩序（整列運動）
- φ ≈ 0: 無秩序（ランダム運動）

### 感受率（Susceptibility）

**χ(t)**: 秩序パラメータの揺らぎ
```
χ = N · Var(φ)
```
- 相転移点でピークを示す

### Expected Free Energy (EFE)

**G**: エージェントの行動選択コスト
```
G = KL[Q(o|π) || P(o|C)] + E_Q[ln Q(s|π) - ln P(s|o,π)]
```

---

## 🧪 テスト

### 単体テスト実行

```bash
# C++単体テスト
cd build && ctest --output-on-failure

# Python単体テスト
pytest tests/unit -v
```

### 統合テスト

```bash
pytest tests/integration -v
```

### 科学的検証テスト

```bash
# Vicsekモデルとの比較検証
pytest tests/validation/test_vicsek_convergence.py -v

# 物理保存則チェック（運動量保存など）
pytest tests/validation/test_conservation_laws.py -v
```

---

## 📚 ドキュメント

- **理論背景**: [docs/theory/EPH-2.1_main.md](docs/theory/EPH-2.1_main.md)
- **API仕様**: [docs/api/](docs/api/)
- **ユーザーガイド**: [docs/user_guide/](docs/user_guide/)
- **実験プロトコル**: [experiments/protocols/](experiments/protocols/)

---

## 🤝 共同研究者向けガイド

### 学生・新規参加者のオンボーディング

1. **背景理論の学習**
   - [docs/theory/free_energy_principle.md](docs/theory/free_energy_principle.md) を読む
   - Active Inference の基礎を理解（推奨論文リスト付き）

2. **環境構築**
   - [docs/user_guide/installation.md](docs/user_guide/installation.md) に従ってセットアップ

3. **初回実験**
   - `experiments/configs/baseline.yaml` を使って簡単な実験を実行
   - 結果を `experiments/notebooks/00_tutorial.ipynb` で可視化

4. **コード理解**
   - `packages/eph_agent/` のコメントを読む
   - 単体テストを見て動作を確認

### ブランチ戦略

```
main              # 安定版（論文投稿時のスナップショット）
├── develop       # 開発版（日常的な作業）
│   ├── feature/spm-optimization
│   └── experiment/beta-sweep-extended
└── paper/nature-2026  # 論文用ブランチ（再現性担保）
```

### データ共有ルール

- **生データ（raw）**: Git LFS で管理、10GB上限
- **処理済みデータ（processed）**: 小さければコミット、大きければ外部ストレージ
- **結果（results）**: 図表とサマリーのみコミット、元データは除外

---

## 📝 論文執筆ワークフロー

### 1. 実験実施

```bash
# 再現性のため実験設定をコミット
git add experiments/configs/paper_experiment_v1.yaml
git commit -m "exp: Add configuration for Nature submission"
```

### 2. 結果の整理

```bash
# 論文用ディレクトリを作成
mkdir -p docs/publications/paper_2026_nature/figures

# シンボリックリンクで図表を参照
ln -s ../../../experiments/results/2026-02-15_beta-sweep/figures/phase_diagram.pdf \
      docs/publications/paper_2026_nature/figures/fig1_phase_diagram.pdf
```

### 3. 再現可能なスクリプト

```bash
# 論文の全図表を再生成するスクリプト
./docs/publications/paper_2026_nature/reproduce_figures.sh
```

### 4. コード公開準備

```bash
# 論文投稿時のコードスナップショット
git tag -a v1.0-nature-submission -m "Code snapshot for Nature submission"
git push origin v1.0-nature-submission

# Zenodo連携でDOI取得
# → GitHub Releases にタグをアップロード
```

---

## 🛠️ トラブルシューティング

### UDP通信が確立しない

```bash
# ポート使用状況確認
lsof -i :5555
lsof -i :5556

# プロセス強制終了
./scripts/utils/kill_processes.sh
```

### C++サーバーがクラッシュする

```bash
# デバッグビルド
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# gdbで実行
gdb ./eph_gui_server
(gdb) run
```

### Python環境の依存関係エラー

```bash
# 環境をクリーンに再構築
rm -rf .venv
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
```

---

## 📄 ライセンス

MIT License - 詳細は [LICENSE](LICENSE) を参照

学術研究での利用を歓迎します。論文で使用する場合は上記の引用情報を使用してください。

---

## 👤 著者・連絡先

**五十嵐 洋（Hiroshi Igarashi）**
- 所属: 東京電機大学 工学部 電子システム工学科
- 専門: AI-DLC, 身体性AI, Free Energy Principle, 群知能
- Email: igarashi@example.ac.jp
- Lab: [https://example.ac.jp/igarashi-lab](https://example.ac.jp/igarashi-lab)

**共同研究者:**
- （学生名・ポスドク名など）

---

## 🙏 謝辞

本研究は科研費（課題番号: XXXXXXXX）の支援を受けて実施されました。

---

**Last updated:** 2026-02-11
**Version:** 2.1.0
