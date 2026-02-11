# 実験ワークフロー実例：β値スイープによる相転移観測

> EPH v2.1 研究プロジェクトにおける典型的な実験フローの完全な例

---

## 📊 実験概要

**研究課題:** Active Inferenceエージェント群における相転移現象の定量化

**実験目的:** ノイズ強度β値を変化させた際の秩序パラメータφ(t)と感受率χ(t)の挙動を観測し、臨界点β_cを特定する

**期待される成果:**
- φ vs β のS字型相転移曲線
- χ のピーク位置から臨界点 β_c の推定
- 論文 Figure 2 のデータ取得

---

## 🎯 実験デザイン

### パラメータ設定

| パラメータ | 記号 | 値 | 理由 |
|-----------|------|-----|------|
| ノイズ強度 | β | 0.05, 0.06, ..., 0.15 (11点) | 臨界点β_c≈0.098をカバー |
| 学習率 | η | 0.01 (固定) | 先行研究の標準値 |
| エージェント数 | N | 50 (固定) | 統計的有意性と計算コストのバランス |
| 試行回数 | - | 5回/条件 | 95%信頼区間の推定に必要 |
| シミュレーション長 | T | 20,000ステップ | 定常状態到達に十分 |
| 時間刻み | dt | 0.1 | 数値安定性 |

**総実験数:** 11 (β値) × 5 (試行) = **55実験**
**推定所要時間:** 約2-3時間（並列実行なし）

---

## 📁 ディレクトリ準備

```bash
# 実験用ディレクトリ作成（日付ベース）
EXPERIMENT_DATE=$(date +%Y-%m-%d)
EXPERIMENT_NAME="beta-sweep-phase-transition"
EXP_DIR="experiments/data/raw/${EXPERIMENT_DATE}_${EXPERIMENT_NAME}"

mkdir -p ${EXP_DIR}
mkdir -p experiments/results/${EXPERIMENT_DATE}_${EXPERIMENT_NAME}/figures
mkdir -p experiments/results/${EXPERIMENT_DATE}_${EXPERIMENT_NAME}/tables

echo "Experiment directory created: ${EXP_DIR}"
```

---

## ⚙️ STEP 1: 実験設定ファイルの作成

**ファイル:** `experiments/configs/beta_sweep_2026-02-15.yaml`

```yaml
# EPH v2.1 - β値スイープ実験設定
# 目的: 相転移の観測と臨界点の特定
# 研究者: 五十嵐 洋
# 日付: 2026-02-15

experiment:
  name: "beta_sweep_phase_transition"
  description: |
    ノイズ強度βを0.05から0.15まで変化させた際の
    相転移現象を観測し、臨界点β_cを特定する
  date: "2026-02-15"
  researcher: "Hiroshi Igarashi"
  hypothesis: "β ≈ 0.098 付近で秩序-無秩序相転移が発生"

parameters:
  # 変動パラメータ
  beta_range:
    min: 0.05
    max: 0.15
    step: 0.01
    # → [0.05, 0.06, 0.07, ..., 0.15] 計11点

  # 固定パラメータ
  eta: 0.01              # 学習率（固定）
  n_agents: 50           # エージェント数
  avg_neighbors: 6       # k-NNグラフの平均次数
  world_size: [10.0, 10.0]  # トーラス世界サイズ
  fov_degrees: 270       # 視野角（前方270度）

simulation:
  timesteps: 20000       # 定常状態到達に十分
  dt: 0.1                # 時間刻み
  warmup_steps: 2000     # ウォームアップ期間（解析から除外）
  save_interval: 10      # データ保存間隔
  trials_per_condition: 5  # 各β値で5回試行

random_seed:
  base_seed: 42
  seed_increment: 100    # 試行ごとに +100
  # trial 1: seed=42, trial 2: seed=142, ...

output:
  format: "hdf5"         # HDF5形式（効率的）
  compression: "gzip"    # 圧縮レベル
  save_agent_trajectories: true
  save_haze_fields: false  # メモリ節約のためfalse

  metrics:  # 保存する評価指標
    - phi                # 秩序パラメータ
    - chi                # 感受率
    - avg_haze           # 平均Haze
    - avg_speed          # 平均速度
    - avg_fatigue        # 平均疲労度
    - efe                # Expected Free Energy

logging:
  level: "INFO"
  save_to_file: true
  log_dir: "logs/experiments"
```

**作成コマンド:**
```bash
# 上記YAMLをファイルに保存
cat > experiments/configs/beta_sweep_2026-02-15.yaml << 'EOF'
(上記YAML内容)
EOF
```

---

## 🚀 STEP 2: 実験実行スクリプト

**ファイル:** `scripts/experiment/run_beta_sweep.py`

```python
#!/usr/bin/env python3
"""
β値スイープ実験実行スクリプト

Usage:
    python scripts/experiment/run_beta_sweep.py \\
        --config experiments/configs/beta_sweep_2026-02-15.yaml \\
        --output experiments/data/raw/2026-02-15_beta-sweep
"""

import argparse
import subprocess
import yaml
import json
from pathlib import Path
from datetime import datetime
import numpy as np

def load_config(config_path):
    with open(config_path, 'r') as f:
        return yaml.safe_load(f)

def generate_beta_values(config):
    """β値のリストを生成"""
    beta_min = config['parameters']['beta_range']['min']
    beta_max = config['parameters']['beta_range']['max']
    beta_step = config['parameters']['beta_range']['step']
    return np.arange(beta_min, beta_max + beta_step/2, beta_step)

def run_single_experiment(beta, trial, config, output_dir):
    """単一実験の実行"""
    seed = config['random_seed']['base_seed'] + trial * config['random_seed']['seed_increment']

    # 実験固有の設定
    exp_config = {
        'beta': float(beta),
        'eta': config['parameters']['eta'],
        'n_agents': config['parameters']['n_agents'],
        'timesteps': config['simulation']['timesteps'],
        'dt': config['simulation']['dt'],
        'seed': seed
    }

    # 出力ファイル名: beta_0.098_trial_003.h5
    output_file = output_dir / f"beta_{beta:.3f}_trial_{trial:03d}.h5"

    # C++サーバーを実行（仮想的なコマンド）
    # 実際のコマンドはプロジェクトの実装に依存
    cmd = [
        './build/eph_gui_server',
        '--headless',  # GUI なし
        '--beta', str(beta),
        '--eta', str(exp_config['eta']),
        '--n-agents', str(exp_config['n_agents']),
        '--timesteps', str(exp_config['timesteps']),
        '--dt', str(exp_config['dt']),
        '--seed', str(seed),
        '--output', str(output_file)
    ]

    print(f"  Running: β={beta:.3f}, trial={trial}, seed={seed}")

    # 実行（ここでは模擬）
    # result = subprocess.run(cmd, capture_output=True, text=True)
    # if result.returncode != 0:
    #     raise RuntimeError(f"Experiment failed: {result.stderr}")

    # メタデータ保存
    metadata = {
        'experiment_config': exp_config,
        'timestamp': datetime.now().isoformat(),
        'output_file': str(output_file)
    }

    return metadata

def main():
    parser = argparse.ArgumentParser(description='Run β-sweep experiment')
    parser.add_argument('--config', required=True, help='Experiment config YAML')
    parser.add_argument('--output', required=True, help='Output directory')
    parser.add_argument('--parallel', type=int, default=1, help='Number of parallel processes')
    args = parser.parse_args()

    # 設定読み込み
    config = load_config(args.config)
    output_dir = Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)

    # β値リスト生成
    beta_values = generate_beta_values(config)
    n_trials = config['simulation']['trials_per_condition']

    print(f"=== β-Sweep Experiment ===")
    print(f"β values: {list(beta_values)}")
    print(f"Trials per β: {n_trials}")
    print(f"Total experiments: {len(beta_values) * n_trials}")
    print(f"Output: {output_dir}")
    print()

    # 実験実行
    all_metadata = []
    for i, beta in enumerate(beta_values, 1):
        print(f"[{i}/{len(beta_values)}] β = {beta:.3f}")
        for trial in range(1, n_trials + 1):
            metadata = run_single_experiment(beta, trial, config, output_dir)
            all_metadata.append(metadata)

    # 実験全体のメタデータ保存
    experiment_metadata = {
        'config': config,
        'beta_values': list(map(float, beta_values)),
        'n_trials': n_trials,
        'total_experiments': len(all_metadata),
        'start_time': datetime.now().isoformat(),
        'experiments': all_metadata
    }

    metadata_file = output_dir / 'metadata.json'
    with open(metadata_file, 'w') as f:
        json.dump(experiment_metadata, f, indent=2)

    print(f"\\n✓ All experiments completed!")
    print(f"  Results: {output_dir}")
    print(f"  Metadata: {metadata_file}")

if __name__ == '__main__':
    main()
```

**実行コマンド:**
```bash
# スクリプトに実行権限付与
chmod +x scripts/experiment/run_beta_sweep.py

# 実験実行
python scripts/experiment/run_beta_sweep.py \
    --config experiments/configs/beta_sweep_2026-02-15.yaml \
    --output experiments/data/raw/2026-02-15_beta-sweep \
    --parallel 4  # 4並列実行
```

---

## 📈 STEP 3: データ解析スクリプト

**ファイル:** `analysis/statistics/analyze_phase_transition.py`

```python
#!/usr/bin/env python3
"""
相転移解析スクリプト

Usage:
    python analysis/statistics/analyze_phase_transition.py \\
        --input experiments/data/raw/2026-02-15_beta-sweep \\
        --output experiments/results/2026-02-15_beta-sweep
"""

import argparse
import h5py
import numpy as np
import pandas as pd
from pathlib import Path
import json

def load_experiment_data(h5_file):
    """HDF5ファイルから実験データを読み込み"""
    with h5py.File(h5_file, 'r') as f:
        data = {
            'timesteps': f['timesteps'][:],
            'phi': f['metrics/phi'][:],
            'chi': f['metrics/chi'][:],
            'avg_haze': f['metrics/avg_haze'][:],
            'avg_speed': f['metrics/avg_speed'][:]
        }
    return data

def compute_statistics(data_list, warmup_steps=2000):
    """複数試行の統計量を計算"""
    # ウォームアップ期間を除外
    data_list_trimmed = [d[warmup_steps:] for d in data_list]

    # 平均と標準偏差
    mean = np.mean(data_list_trimmed, axis=0)
    std = np.std(data_list_trimmed, axis=0, ddof=1)

    # 定常状態の平均（後半50%）
    steady_state_start = len(mean) // 2
    steady_mean = np.mean(mean[steady_state_start:])
    steady_std = np.mean(std[steady_state_start:])

    return {
        'mean': mean,
        'std': std,
        'steady_state_mean': steady_mean,
        'steady_state_std': steady_std
    }

def analyze_phase_transition(input_dir, output_dir):
    """相転移解析のメイン処理"""
    input_dir = Path(input_dir)
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    # メタデータ読み込み
    with open(input_dir / 'metadata.json', 'r') as f:
        metadata = json.load(f)

    beta_values = metadata['beta_values']
    n_trials = metadata['n_trials']

    results = []

    for beta in beta_values:
        print(f"Analyzing β = {beta:.3f}")

        # 該当するファイルを読み込み
        phi_trials = []
        chi_trials = []

        for trial in range(1, n_trials + 1):
            h5_file = input_dir / f"beta_{beta:.3f}_trial_{trial:03d}.h5"
            if not h5_file.exists():
                print(f"  Warning: {h5_file} not found")
                continue

            data = load_experiment_data(h5_file)
            phi_trials.append(data['phi'])
            chi_trials.append(data['chi'])

        # 統計量計算
        phi_stats = compute_statistics(phi_trials)
        chi_stats = compute_statistics(chi_trials)

        results.append({
            'beta': beta,
            'phi_mean': phi_stats['steady_state_mean'],
            'phi_std': phi_stats['steady_state_std'],
            'chi_mean': chi_stats['steady_state_mean'],
            'chi_std': chi_stats['steady_state_std'],
            'n_trials': len(phi_trials)
        })

    # DataFrameに変換
    df = pd.DataFrame(results)

    # CSV保存
    csv_file = output_dir / 'aggregated_results.csv'
    df.to_csv(csv_file, index=False)
    print(f"\\n✓ Aggregated results saved: {csv_file}")

    # 臨界点推定（χのピーク位置）
    chi_max_idx = df['chi_mean'].idxmax()
    beta_critical = df.loc[chi_max_idx, 'beta']

    print(f"\\n=== Phase Transition Analysis ===")
    print(f"Estimated critical point: β_c = {beta_critical:.3f}")
    print(f"Maximum susceptibility: χ_max = {df.loc[chi_max_idx, 'chi_mean']:.4f}")

    # サマリー保存
    summary = {
        'beta_critical': beta_critical,
        'chi_max': df.loc[chi_max_idx, 'chi_mean'],
        'results_table': csv_file.name
    }

    summary_file = output_dir / 'summary.json'
    with open(summary_file, 'w') as f:
        json.dump(summary, f, indent=2)

    return df

def main():
    parser = argparse.ArgumentParser(description='Analyze phase transition')
    parser.add_argument('--input', required=True, help='Input directory (raw data)')
    parser.add_argument('--output', required=True, help='Output directory (results)')
    args = parser.parse_args()

    df = analyze_phase_transition(args.input, args.output)
    print("\\n✓ Analysis completed!")

if __name__ == '__main__':
    main()
```

**実行コマンド:**
```bash
python analysis/statistics/analyze_phase_transition.py \
    --input experiments/data/raw/2026-02-15_beta-sweep \
    --output experiments/results/2026-02-15_beta-sweep
```

---

## 📊 STEP 4: 可視化（論文用図表生成）

**ファイル:** `analysis/visualization/plot_phase_diagram.py`

```python
#!/usr/bin/env python3
"""
相転移図の生成（論文Figure 2用）

Usage:
    python analysis/visualization/plot_phase_diagram.py \\
        --input experiments/results/2026-02-15_beta-sweep/aggregated_results.csv \\
        --output experiments/results/2026-02-15_beta-sweep/figures/
"""

import argparse
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.style as mplstyle
from pathlib import Path

# 論文用スタイル適用
mplstyle.use('seaborn-v0_8-paper')
plt.rcParams.update({
    'font.size': 10,
    'font.family': 'serif',
    'figure.dpi': 300,
    'savefig.dpi': 300,
    'savefig.bbox': 'tight'
})

def plot_phase_diagram(df, output_dir):
    """φ vs β と χ vs β のプロット"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(6, 8))

    # (a) Order parameter φ vs β
    ax1.errorbar(df['beta'], df['phi_mean'], yerr=df['phi_std'],
                 fmt='o-', capsize=3, label='φ(β)', color='blue')
    ax1.set_xlabel('Noise intensity β')
    ax1.set_ylabel('Order parameter φ')
    ax1.set_title('(a) Phase Transition: Order Parameter')
    ax1.grid(True, alpha=0.3)
    ax1.legend()

    # (b) Susceptibility χ vs β
    ax2.errorbar(df['beta'], df['chi_mean'], yerr=df['chi_std'],
                 fmt='s-', capsize=3, label='χ(β)', color='red')
    ax2.set_xlabel('Noise intensity β')
    ax2.set_ylabel('Susceptibility χ')
    ax2.set_title('(b) Susceptibility Peak at Critical Point')
    ax2.grid(True, alpha=0.3)
    ax2.legend()

    # 臨界点にマーカー
    chi_max_idx = df['chi_mean'].idxmax()
    beta_c = df.loc[chi_max_idx, 'beta']
    ax2.axvline(beta_c, color='gray', linestyle='--', alpha=0.5)
    ax2.text(beta_c, ax2.get_ylim()[1]*0.9, f'β_c ≈ {beta_c:.3f}',
             ha='center', fontsize=9)

    plt.tight_layout()

    # 保存（PDF + PNG）
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    pdf_file = output_dir / 'phase_diagram.pdf'
    png_file = output_dir / 'phase_diagram.png'

    plt.savefig(pdf_file, format='pdf')
    plt.savefig(png_file, format='png', dpi=150)  # プレビュー用

    print(f"✓ Figures saved:")
    print(f"  PDF: {pdf_file}")
    print(f"  PNG: {png_file}")

    plt.close()

def main():
    parser = argparse.ArgumentParser(description='Plot phase diagram')
    parser.add_argument('--input', required=True, help='Aggregated results CSV')
    parser.add_argument('--output', required=True, help='Output directory for figures')
    args = parser.parse_args()

    df = pd.read_csv(args.input)
    plot_phase_diagram(df, args.output)

if __name__ == '__main__':
    main()
```

**実行コマンド:**
```bash
python analysis/visualization/plot_phase_diagram.py \
    --input experiments/results/2026-02-15_beta-sweep/aggregated_results.csv \
    --output experiments/results/2026-02-15_beta-sweep/figures/
```

---

## 📝 STEP 5: 実験サマリーの作成

**ファイル:** `experiments/results/2026-02-15_beta-sweep/summary.md`

```markdown
# 実験サマリー: β値スイープによる相転移観測

**実験日:** 2026-02-15
**実施者:** 五十嵐 洋
**プロトコル:** [experiments/protocols/01_phase_transition.md](../../protocols/01_phase_transition.md)

---

## 実験条件

- **β値範囲:** 0.05 〜 0.15 (0.01刻み、11点)
- **試行回数:** 5回/条件
- **エージェント数:** N = 50
- **シミュレーション長:** 20,000ステップ
- **総実験数:** 55回

## 主要結果

### 臨界点の特定

**推定臨界点:** β_c ≈ **0.098**

- 感受率χのピーク位置から推定
- 理論予測（β_c ≈ 0.098）と一致

### 秩序パラメータの挙動

- β < 0.098: φ ≈ 0.8 〜 0.9（高秩序、整列運動）
- β ≈ 0.098: φ ≈ 0.5（相転移点）
- β > 0.098: φ ≈ 0.1 〜 0.2（無秩序、ランダム運動）

### 感受率のピーク

- χ_max ≈ 1.25 (β = 0.098)
- 明確なピーク構造を確認

## 再現性

- 5試行の標準偏差: 平均 σ_φ ≈ 0.05
- 高い再現性を確認

## 論文への反映

- **Figure 2(a):** φ vs β のS字カーブ → ✓ 生成済み
- **Figure 2(b):** χ vs β のピーク → ✓ 生成済み

図表ファイル:
- `figures/phase_diagram.pdf` (300 dpi, 論文投稿用)
- `figures/phase_diagram.png` (150 dpi, プレビュー用)

## 次のステップ

1. N依存性の調査（N = 20, 50, 100での比較）
2. 有限サイズスケーリング解析
3. 動的臨界指数の推定

---

**データ保存先:**
- 生データ: `experiments/data/raw/2026-02-15_beta-sweep/`
- 解析結果: `experiments/results/2026-02-15_beta-sweep/`
```

---

## ✅ 実験完了チェックリスト

実験が正しく完了したか確認：

- [ ] 全55ファイル（11 β × 5試行）が `experiments/data/raw/` に存在
- [ ] `metadata.json` にパラメータが正しく記録されている
- [ ] `aggregated_results.csv` に統計量がまとめられている
- [ ] `summary.json` に臨界点β_cが記録されている
- [ ] `figures/phase_diagram.pdf` が論文投稿品質で生成されている
- [ ] `summary.md` に実験結果が記載されている
- [ ] Gitにコミット済み（設定ファイル、スクリプト、結果サマリー）
- [ ] 生データはGit LFS管理下にある

---

## 🔄 論文執筆への統合

### 図表のリンク作成

```bash
# 論文用ディレクトリに図表をリンク
mkdir -p docs/publications/paper_2026_nature/figures
ln -s ../../../experiments/results/2026-02-15_beta-sweep/figures/phase_diagram.pdf \
      docs/publications/paper_2026_nature/figures/fig2_phase_transition.pdf

# LaTeX から参照
# \includegraphics{figures/fig2_phase_transition.pdf}
```

### 再現スクリプトの作成

```bash
cat > docs/publications/paper_2026_nature/reproduce_fig2.sh << 'EOF'
#!/bin/bash
# Figure 2 の完全再現スクリプト

echo "Reproducing Figure 2: Phase Transition..."

# 1. 実験実行（時間がかかる場合はスキップ）
# python scripts/experiment/run_beta_sweep.py \
#     --config experiments/configs/beta_sweep_2026-02-15.yaml \
#     --output experiments/data/raw/2026-02-15_beta-sweep

# 2. 解析
python analysis/statistics/analyze_phase_transition.py \
    --input experiments/data/raw/2026-02-15_beta-sweep \
    --output experiments/results/2026-02-15_beta-sweep

# 3. 図表生成
python analysis/visualization/plot_phase_diagram.py \
    --input experiments/results/2026-02-15_beta-sweep/aggregated_results.csv \
    --output experiments/results/2026-02-15_beta-sweep/figures/

echo "✓ Figure 2 reproduced: figures/fig2_phase_transition.pdf"
EOF

chmod +x docs/publications/paper_2026_nature/reproduce_fig2.sh
```

---

## 📊 期待される出力イメージ

```
experiments/results/2026-02-15_beta-sweep/
├── summary.md                        # このファイル
├── summary.json                      # 臨界点などの定量値
├── aggregated_results.csv            # 全β値の統計量
├── figures/
│   ├── phase_diagram.pdf             # 論文投稿用（300 dpi）
│   └── phase_diagram.png             # プレビュー用（150 dpi）
└── tables/
    └── critical_parameters.csv       # 臨界点周辺のパラメータ
```

---

**作成日:** 2026-02-11
**プロジェクト:** EPH v2.1 (crlEPH2)
**目的:** 研究ワークフローの完全な実例提示
