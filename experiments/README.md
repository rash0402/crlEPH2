# Experiments Directory

EPH v2.1 プロジェクトの実験管理ディレクトリ

## 📁 ディレクトリ構造

```
experiments/
├── configs/          # 実験設定ファイル（YAML）
│   ├── baseline.yaml
│   ├── template.yaml
│   └── README.md
│
├── protocols/        # 実験プロトコル（手順書）
│   ├── 01_phase_transition.md
│   ├── 02_spm_validation.md
│   └── README.md
│
├── data/             # 実験データ
│   ├── raw/          # 生データ（Git LFS管理）
│   │   ├── 2026-02-15_beta-sweep/
│   │   │   ├── run_001.h5
│   │   │   ├── run_002.h5
│   │   │   └── metadata.json
│   │   └── README.md
│   ├── processed/    # 前処理済みデータ
│   └── external/     # 外部データ・ベンチマーク
│
├── notebooks/        # Jupyter Notebook解析
│   ├── 01_exploratory_analysis.ipynb
│   ├── 02_phase_transition_analysis.ipynb
│   └── utils.py
│
└── results/          # 実験結果サマリー
    ├── 2026-02-15_beta-sweep/
    │   ├── summary.md
    │   ├── figures/
    │   │   └── phase_diagram.pdf
    │   └── tables/
    │       └── metrics.csv
    └── README.md
```

## 🎯 使用方法

### 1. 実験設定の作成

```bash
# テンプレートをコピー
cp experiments/configs/template.yaml experiments/configs/my_experiment.yaml

# 設定ファイルを編集してパラメータを調整
vim experiments/configs/my_experiment.yaml
```

### 2. 実験プロトコルの確認

```bash
# プロトコルを読む
cat experiments/protocols/01_phase_transition.md

# 手順に従って実験準備
```

### 3. 実験実行

```bash
# （実装予定）
python scripts/experiment/run_experiment.py \
  --config experiments/configs/my_experiment.yaml \
  --output experiments/data/raw/$(date +%Y-%m-%d)_my-experiment
```

### 4. データ解析

```bash
# Jupyter Notebook で解析
cd experiments/notebooks
jupyter lab

# または解析スクリプトで自動化
python analysis/statistics/analyze_phase_transition.py \
  --input experiments/data/raw/2026-02-15_beta-sweep \
  --output experiments/results/2026-02-15_beta-sweep
```

### 5. 結果の可視化

```bash
# 図表生成
python analysis/visualization/plot_phase_diagram.py \
  --input experiments/results/2026-02-15_beta-sweep/aggregated.csv \
  --output experiments/results/2026-02-15_beta-sweep/figures/
```

## 📋 命名規則

### ディレクトリ命名

```
YYYY-MM-DD_experiment-description/
```

**例:**
- `2026-02-15_beta-sweep/`
- `2026-02-20_high-density-N100/`
- `2026-03-01_spm-validation/`

### ファイル命名

**生データ:**
```
beta_0.098_trial_001.h5
run_001.h5
```

**結果図表:**
```
phase_diagram.pdf
phi_vs_beta.pdf
chi_timeseries.png
```

## 🔬 実験ワークフロー

```
1. 設定ファイル作成
   ↓
2. プロトコル確認
   ↓
3. 実験実行 → data/raw/
   ↓
4. データ前処理 → data/processed/
   ↓
5. 統計解析 → results/
   ↓
6. 可視化 → results/figures/
   ↓
7. サマリー作成 → results/summary.md
   ↓
8. Git コミット（設定、結果サマリー、図表）
```

## 📊 データ管理

### Git LFS 管理対象

- ✅ `*.h5` - HDF5データファイル
- ✅ `*.hdf5` - HDF5データファイル
- ✅ `data/raw/**/*.csv` - 大容量CSVファイル

### Git 通常管理対象

- ✅ `configs/*.yaml` - 実験設定
- ✅ `protocols/*.md` - 実験プロトコル
- ✅ `results/*/summary.md` - 結果サマリー
- ✅ `results/*/figures/*.pdf` - 論文図表（小サイズのみ）

### Git 除外対象（.gitignore）

- ❌ `data/raw/**/*.h5` - 生データ（LFS管理）
- ❌ `notebooks/.ipynb_checkpoints/` - Jupyter一時ファイル

## 🔄 再現性の担保

実験の再現性を保証するため、以下を実施：

1. **設定ファイルのバージョン管理**
   ```bash
   git add experiments/configs/my_experiment.yaml
   git commit -m "exp: Add configuration for phase transition study"
   ```

2. **メタデータの記録**
   - `metadata.json` に実験条件を自動保存
   - 乱数シードの固定

3. **Git タグで実験バージョンを記録**
   ```bash
   git tag -a exp/2026-02-15-beta-sweep -m "Beta sweep experiment data"
   ```

4. **論文投稿時のスナップショット**
   ```bash
   git tag -a v1.0-nature-submission -m "Code and config for Nature submission"
   ```

## 🔗 関連ドキュメント

- [実験ワークフロー完全例](../EXPERIMENT_WORKFLOW_EXAMPLE.md)
- [実験設定ガイド](configs/README.md)
- [データ解析ガイド](notebooks/README.md)
- [マイグレーションガイド](../MIGRATION_GUIDE.md)

---

**作成日:** 2026-02-11
**プロジェクト:** EPH v2.1 (crlEPH2)
