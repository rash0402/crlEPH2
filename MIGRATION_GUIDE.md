# EPH v2.1 プロジェクト構造マイグレーションガイド

> 現在の構造から研究最適化構造への移行手順

---

## 📋 移行の目的

- ✅ テストコードと実験ログの明確な分離
- ✅ 実験データの体系的管理（Git LFS活用）
- ✅ 論文執筆時のファイルアクセス性向上
- ✅ 学生・共同研究者との引き継ぎ容易化
- ✅ 再現可能性の保証

---

## 🚀 マイグレーション手順（段階的実施）

### Phase 1: バックアップと準備（30分）

```bash
# 1. 現在の状態をバックアップ
git checkout -b backup/pre-restructure-2026-02-11
git push origin backup/pre-restructure-2026-02-11

# 2. 新しいブランチで作業
git checkout main
git checkout -b refactor/project-structure-research

# 3. Git LFS セットアップ
git lfs install
git lfs track "*.h5"
git lfs track "*.hdf5"
git lfs track "experiments/data/raw/**/*.csv"
git add .gitattributes
git commit -m "setup: Add Git LFS configuration for large data files"
```

### Phase 2: ディレクトリ構造の作成（15分）

```bash
# 必要なディレクトリを一括作成
mkdir -p experiments/{configs,protocols,data/{raw,processed,external},notebooks,results}
mkdir -p analysis/{preprocessing,statistics,visualization/figure_templates}
mkdir -p tests/{unit,integration,validation,fixtures}
mkdir -p scripts/{setup,build,experiment,utils}
mkdir -p docs/{user_guide,api,theory,architecture,publications}
mkdir -p logs/{experiments,server,gui}
mkdir -p archive/{old_experiments,deprecated_code}

# .gitkeep で空ディレクトリをコミット可能に
touch experiments/data/raw/.gitkeep
touch experiments/data/processed/.gitkeep
touch experiments/results/.gitkeep
touch logs/.gitkeep

git add experiments/ analysis/ tests/ scripts/ docs/ logs/ archive/
git commit -m "structure: Create research-optimized directory structure"
```

### Phase 3: 既存ファイルの移動（1時間）

#### 3-1. ソースコードの整理

```bash
# cpp_server → src/cpp_server
git mv cpp_server src/cpp_server

# gui → src/gui (すでに gui/ として存在)
# packages/ はそのまま（ライブラリコンポーネント）

git commit -m "refactor: Move application code to src/"
```

#### 3-2. テストファイルの統合

```bash
# GUI テストファイルを tests/integration/ へ移動
git mv gui/test_phase2_integration.py tests/integration/
git mv gui/test_toolbar_integration.py tests/integration/

# 手動テストは tests/unit/ へ
git mv gui/test_global_view_manual.py tests/unit/test_global_view.py
git mv gui/test_parameter_panel_manual.py tests/unit/test_parameter_panel.py
git mv gui/test_playback_toolbar_manual.py tests/unit/test_playback_toolbar.py

# 修正後のテストは削除（統合済み）
rm gui/test_global_view_fixes.py

git commit -m "refactor: Consolidate test files into tests/ directory"
```

#### 3-3. スクリプトの分類

```bash
# ビルドスクリプト
git mv scripts/build_gui_server.sh scripts/build/
git mv scripts/kill_processes.sh scripts/utils/

# テストスクリプトは削除（pytest に統合）
rm scripts/test_udp_communication.sh

# 可視化スクリプトは analysis/visualization/ へ
git mv scripts/plot_agent_trajectories.py analysis/visualization/
git mv scripts/plot_haze_field.py analysis/visualization/
git mv scripts/plot_phase_transition.py analysis/visualization/

git commit -m "refactor: Organize scripts by purpose"
```

#### 3-4. ドキュメントの統合

```bash
# doc/ → docs/theory/
git mv doc/EPH-2.1_main.md docs/theory/
git mv doc/appendix docs/theory/appendix
git mv doc/proposals docs/theory/proposals

# 既存の docs/ と統合
# (docs/plans/, docs/phase2_completion_notes.md はそのまま)

# README を統合
mv README_RESEARCH_TEMPLATE.md README.md.new
# 手動で内容をマージ後
mv README.md.new README.md

git commit -m "docs: Consolidate documentation into docs/"
```

#### 3-5. 実験データ・ログの整理

```bash
# demo_output/ → experiments/results/demo/
mkdir -p experiments/results/demo/figures
git mv demo_output/*.pdf experiments/results/demo/figures/
git mv demo_output/*.png experiments/results/demo/figures/
rmdir demo_output/

# /tmp/eph_server.log は logs/ で管理するよう run.sh を修正
# （後述）

git commit -m "refactor: Move demo outputs to experiments/results/"
```

### Phase 4: 設定ファイルの更新（30分）

#### 4-1. .gitignore の更新

```bash
# 提案した .gitignore に置き換え
cat > .gitignore << 'EOF'
# （前述の .gitignore 内容をコピー）
EOF

git add .gitignore
git commit -m "config: Update .gitignore for research project"
```

#### 4-2. CMakeLists.txt の更新

```cmake
# CMakeLists.txt の修正（src/cpp_server のパス変更）
# 手動で編集が必要

# Before:
# add_subdirectory(cpp_server)

# After:
# add_subdirectory(src/cpp_server)
```

#### 4-3. run.sh の更新

```bash
# run.sh のログ出力先を logs/ に変更

# Before:
# LOG_FILE="/tmp/eph_server.log"

# After:
# LOG_FILE="logs/server/eph_server_$(date +%Y-%m-%d_%H%M%S).log"
# ln -sf "eph_server_$(date +%Y-%m-%d_%H%M%S).log" logs/server/eph_server_latest.log

git add run.sh
git commit -m "config: Update run.sh to use logs/ directory"
```

### Phase 5: 新規ファイルの作成（1時間）

#### 5-1. 実験設定テンプレート

```bash
cat > experiments/configs/template.yaml << 'EOF'
# EPH v2.1 実験設定テンプレート
experiment:
  name: "experiment_name_here"
  description: "実験の目的・仮説"
  date: "YYYY-MM-DD"
  researcher: "Your Name"

parameters:
  beta: 0.098          # ノイズ強度
  eta: 0.01            # 学習率
  n_agents: 10         # エージェント数
  avg_neighbors: 6     # 平均近傍数
  world_size: [10.0, 10.0]  # ワールドサイズ

simulation:
  timesteps: 10000     # シミュレーションステップ数
  dt: 0.1              # 時間刻み
  seed: 42             # 乱数シード（再現性）

output:
  format: "hdf5"       # hdf5 | csv
  save_interval: 10    # 保存間隔（ステップ）
  metrics:
    - phi              # 秩序パラメータ
    - chi              # 感受率
    - avg_haze         # 平均Haze
    - avg_speed        # 平均速度
EOF

git add experiments/configs/
git commit -m "exp: Add experiment configuration template"
```

#### 5-2. 実験プロトコル

```bash
cat > experiments/protocols/01_phase_transition.md << 'EOF'
# 実験プロトコル: 相転移観測

## 目的
β値の変化による秩序-無秩序相転移の観測と臨界点の特定

## 仮説
β ≈ 0.098 付近で相転移が発生し、感受率χがピークを示す

## 手順

### 1. 事前準備
- [ ] C++サーバーのビルド確認
- [ ] Python環境の依存関係確認
- [ ] ディスク空き容量確認（最低10GB）

### 2. パラメータ設定
- β: 0.05 から 0.15 まで 0.01 刻み（11点）
- N: 50 エージェント（固定）
- timesteps: 20000（安定化に十分）
- 各条件で5回試行（統計的有意性）

### 3. 実験実行

```bash
python scripts/experiment/run_parameter_sweep.py \
  --param beta \
  --range 0.05 0.15 0.01 \
  --trials 5 \
  --base-config experiments/configs/baseline.yaml \
  --output experiments/data/raw/$(date +%Y-%m-%d)_phase-transition
```

### 4. データチェック
- [ ] 全11×5=55ファイルが生成されたか確認
- [ ] 各ファイルのサイズが妥当か確認（> 1MB）
- [ ] metadata.json にパラメータが正しく記録されているか確認

### 5. 解析
```bash
python analysis/statistics/phase_transition_detection.py \
  --input experiments/data/raw/YYYY-MM-DD_phase-transition \
  --output experiments/results/YYYY-MM-DD_phase-transition
```

### 6. 可視化
```bash
python analysis/visualization/plot_phase_diagram.py \
  --input experiments/results/YYYY-MM-DD_phase-transition/aggregated.csv \
  --output experiments/results/YYYY-MM-DD_phase-transition/figures/
```

## 期待される結果
- φ vs β のS字カーブ
- χ のピークが β_c ≈ 0.098 付近に出現
- 5試行のエラーバーが小さい（再現性高い）

## データ保存先
- 生データ: `experiments/data/raw/YYYY-MM-DD_phase-transition/`
- 解析結果: `experiments/results/YYYY-MM-DD_phase-transition/`
- 論文図表: `docs/publications/paper_2026/figures/fig2_phase_transition.pdf`

## 実施記録
- 実施日: YYYY-MM-DD
- 実施者: Your Name
- 所要時間: X hours
- 特記事項: （問題があれば記載）
EOF

git add experiments/protocols/
git commit -m "exp: Add phase transition experiment protocol"
```

#### 5-3. Jupyter Notebook テンプレート

```bash
# 簡易的なノートブック作成（実際はJupyterで作成推奨）
cat > experiments/notebooks/README.md << 'EOF'
# 実験解析ノートブック

## 使用方法

```bash
cd experiments/notebooks
jupyter lab
```

## ノートブック一覧

- `00_tutorial.ipynb`: 初学者向けチュートリアル
- `01_exploratory_analysis.ipynb`: 探索的データ解析
- `02_phase_transition_analysis.ipynb`: 相転移解析
- `03_spm_visualization.ipynb`: SPM可視化

## 共通関数

`utils.py` に共通の読み込み・プロット関数を実装。

```python
from utils import load_experiment_data, plot_timeseries

data = load_experiment_data("../data/raw/2026-02-15_beta-sweep/run_001.h5")
plot_timeseries(data, metrics=["phi", "chi"])
```
EOF

git add experiments/notebooks/
git commit -m "exp: Add notebook directory and README"
```

### Phase 6: テストの更新と実行（30分）

```bash
# インポートパスの修正が必要なテストを更新
# 例: src/cpp_server への移動に伴う変更

# テスト実行
pytest tests/ -v

# 問題があれば修正
git add tests/
git commit -m "test: Update tests for new project structure"
```

### Phase 7: ビルドとCI/CDの確認（30分）

```bash
# ビルド確認
./scripts/build/build_cpp_server.sh

# 統合テスト実行
pytest tests/integration/ -v

# 動作確認
./run.sh
# → GUI が起動し、エージェントが表示されることを確認

git add .
git commit -m "chore: Verify build and integration tests"
```

### Phase 8: マージと公開（15分）

```bash
# develop ブランチにマージ
git checkout develop
git merge refactor/project-structure-research

# main にマージ（慎重に）
git checkout main
git merge develop

# リモートへプッシュ
git push origin main develop

# タグ付け
git tag -a v2.1.0-restructured -m "Project structure optimized for research"
git push origin v2.1.0-restructured
```

---

## ✅ マイグレーション完了チェックリスト

### ディレクトリ構造
- [ ] `experiments/` ディレクトリが存在し、configs, data, protocols, results を含む
- [ ] `analysis/` ディレクトリが存在し、スクリプトが分類されている
- [ ] `tests/` ディレクトリにすべてのテストが統合されている
- [ ] `docs/` に理論・API・ガイドが整理されている
- [ ] `src/` にアプリケーションコード（cpp_server, gui）が配置されている

### Git設定
- [ ] `.gitignore` が更新され、logs/, build/, .venv/ が除外されている
- [ ] `.gitattributes` でGit LFSが設定されている（*.h5, *.hdf5）
- [ ] Git LFS が有効化されている（`git lfs install`）

### ビルド・実行
- [ ] `./scripts/build/build_cpp_server.sh` でビルド成功
- [ ] `./run.sh` でGUIが起動する
- [ ] `pytest tests/` で全テストがパスする

### ドキュメント
- [ ] `README.md` が研究プロジェクト向けに更新されている
- [ ] `CITATION.cff` が作成されている（論文引用情報）
- [ ] 実験プロトコルが `experiments/protocols/` に存在する

### 実験管理
- [ ] `experiments/configs/template.yaml` が存在する
- [ ] ログファイルが `logs/` に出力されるよう設定されている
- [ ] 実験結果のディレクトリ命名規則が決まっている

---

## 🔄 ロールバック手順（問題が発生した場合）

```bash
# バックアップブランチに戻る
git checkout backup/pre-restructure-2026-02-11

# または特定のコミットに戻る
git log --oneline  # コミットハッシュを確認
git checkout <commit-hash>

# 新しいブランチとして保存
git checkout -b rollback/YYYY-MM-DD
```

---

## 📚 移行後の推奨作業

1. **実験設定の作成**
   - `experiments/configs/` にプロジェクト固有の設定を追加

2. **既存データの整理**
   - 過去の実験データを `experiments/data/raw/` に移動
   - メタデータ（metadata.json）を追加

3. **CI/CDの設定**
   - GitHub Actions で自動テスト
   - 論文投稿時の自動アーカイブ

4. **学生向けドキュメント**
   - オンボーディングガイドの作成
   - Jupyter Notebook チュートリアルの充実

---

## ❓ FAQ

**Q: 既存の実験データはどうすれば？**
A: `archive/old_experiments/` に一時保存し、後で `experiments/data/raw/` に整理してください。

**Q: Git LFS の容量制限は？**
A: GitHubの無料プランは1GB、有料プランは最大5TB。大容量データは外部ストレージ推奨。

**Q: 学生がこの構造を理解できるか？**
A: `docs/user_guide/project_structure.md` に詳細な説明を記載し、オンボーディング時に説明してください。

**Q: 論文投稿時のコード公開はどうすれば？**
A: Git tag を作成し、Zenodo と連携してDOIを取得。`CITATION.cff` に記載。

---

**作成日:** 2026-02-11
**対象プロジェクト:** EPH v2.1 (crlEPH2)
**推定所要時間:** 4-6時間（段階的実施を推奨）
