# Experiment Configurations

実験設定ファイル（YAML形式）を管理するディレクトリ

## 📁 ファイル一覧

- **`baseline.yaml`** - ベースライン設定（標準パラメータ）
- **`template.yaml`** - 新規実験用テンプレート

## 🎯 使用方法

### 新規実験設定の作成

```bash
# 1. テンプレートをコピー
cp experiments/configs/template.yaml experiments/configs/my_experiment.yaml

# 2. エディタで編集
vim experiments/configs/my_experiment.yaml

# 3. パラメータを調整
#   - beta: ノイズ強度
#   - eta: 学習率
#   - n_agents: エージェント数
#   など

# 4. 実験実行（実装予定）
# python scripts/experiment/run_experiment.py --config experiments/configs/my_experiment.yaml
```

### ベースライン設定の参照

```bash
# 標準パラメータで実験実行
# python scripts/experiment/run_experiment.py --config experiments/configs/baseline.yaml
```

## 📋 設定ファイルの構造

```yaml
experiment:
  name: "実験名"
  description: "実験の目的・仮説"
  date: "YYYY-MM-DD"
  researcher: "研究者名"

parameters:
  beta: 0.098          # ノイズ強度
  eta: 0.01            # 学習率
  n_agents: 10         # エージェント数
  # ... 他のパラメータ

simulation:
  timesteps: 10000     # ステップ数
  dt: 0.1              # 時間刻み
  # ... 他の設定

output:
  format: "hdf5"       # 出力形式
  metrics: [phi, chi]  # 保存する評価指標
```

## 🔬 主要パラメータの説明

| パラメータ | 記号 | 標準値 | 範囲 | 説明 |
|-----------|------|--------|------|------|
| ノイズ強度 | β | 0.098 | 0.01-0.20 | 相転移の制御パラメータ |
| 学習率 | η | 0.01 | 0.001-0.1 | Haze場の更新速度 |
| エージェント数 | N | 10 | 5-200 | 群れサイズ |
| 平均近傍数 | k | 6 | 3-10 | k-NNグラフの次数 |
| 視野角 | FOV | 270° | 180-360 | 前方視野（270°は後方90°盲点） |

### β（ノイズ強度）の効果

- **β < 0.098**: 高秩序（整列運動、φ ≈ 0.8-0.9）
- **β ≈ 0.098**: 相転移点（φ ≈ 0.5）
- **β > 0.098**: 無秩序（ランダム運動、φ ≈ 0.1-0.2）

## 📝 命名規則

設定ファイル名は以下の形式を推奨：

- `baseline.yaml` - ベースライン（標準パラメータ）
- `beta_sweep.yaml` - パラメータスイープ実験
- `high_density.yaml` - 高密度条件（N=100など）
- `long_timescale.yaml` - 長時間シミュレーション
- `YYYY-MM-DD_experiment_name.yaml` - 日付付き（論文用）

## 🔄 バージョン管理

- ✅ **すべての設定ファイルを Git でバージョン管理**
- ✅ 実験実行前にコミット（再現性担保）
- ✅ 論文投稿時は Git tag を作成

```bash
# 設定ファイルをコミット
git add experiments/configs/my_experiment.yaml
git commit -m "exp: Add configuration for phase transition study"

# 論文投稿時
git tag -a v1.0-nature-submission -m "Configuration for Nature submission"
```

## 📊 実験実行との連携

```bash
# （将来実装予定）

# 単一実験
python scripts/experiment/run_experiment.py \
  --config experiments/configs/baseline.yaml \
  --output experiments/data/raw/2026-02-15_baseline

# パラメータスイープ
python scripts/experiment/run_parameter_sweep.py \
  --param beta \
  --range 0.05 0.15 0.01 \
  --base-config experiments/configs/baseline.yaml

# バッチ実験
./scripts/experiment/batch_experiments.sh experiments/configs/batch_list.txt
```

## 🔗 関連ドキュメント

- [実験プロトコル](../protocols/README.md)
- [実験ワークフロー例](../../EXPERIMENT_WORKFLOW_EXAMPLE.md)
- [データ解析ガイド](../notebooks/README.md)

---

**作成日:** 2026-02-11
**プロジェクト:** EPH v2.1 (crlEPH2)
