# EPH v2.1 クイックスタートガイド

**最終更新**: 2026-02-02

## このドキュメントの目的

新規参加者が30分以内に開発環境を構築し、最初のテストを実行できるようにするセットアップガイド。

**対象読者**: 新規参加者、環境構築を行う開発者

**前提知識**: C++とPythonの基本的な開発経験

---

## 目次

1. [必要なツール](#1-必要なツール)
2. [環境構築手順](#2-環境構築手順)
3. [プロジェクトのクローン](#3-プロジェクトのクローン)
4. [C++ビルド（Phase 1以降）](#4-cビルドphase-1以降)
5. [Pythonパッケージのインストール（Phase 4以降）](#5-pythonパッケージのインストールphase-4以降)
6. [動作確認](#6-動作確認)
7. [トラブルシューティング](#7-トラブルシューティング)

---

## 1. 必要なツール

### 1.1 C++開発環境

| ツール | 最低バージョン | 推奨バージョン | 用途 |
|--------|---------------|---------------|------|
| **CMake** | 3.20 | 3.25+ | ビルドシステム |
| **C++コンパイラ** | C++17対応 | C++20対応 | コンパイル |
| - GCC | 9+ | 11+ | Linux推奨 |
| - Clang | 10+ | 14+ | macOS推奨 |
| - MSVC | 2019+ | 2022+ | Windows |
| **Eigen** | 3.4.0 | 3.4.0+ | テンソル/行列演算 |
| **pybind11** | 2.11.0 | 2.11.1+ | Python連携 |
| **GoogleTest** | 1.12.0 | 1.14+ | C++単体テスト |

### 1.2 Python環境

| ツール | 最低バージョン | 推奨バージョン | 用途 |
|--------|---------------|---------------|------|
| **Python** | 3.9 | 3.11+ | スクリプト実行 |
| **NumPy** | 1.21 | 1.24+ | 数値計算 |
| **PyQt6** | 6.4 | 6.6+ | GUI |
| **Matplotlib** | 3.5 | 3.8+ | 可視化 |
| **pytest** | 7.0 | 7.4+ | Python単体テスト |
| **SciPy** | 1.7 | 1.11+ | 科学計算 |

### 1.3 オプション（推奨）

- **clang-format** (14+): コードフォーマット
- **clangd**: C++ LSP（エディタ補完）
- **ccache**: ビルドキャッシュ（高速化）

---

## 2. 環境構築手順

### 2.1 macOS（推奨: Homebrew）

```bash
# Homebrewのインストール（未インストールの場合）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 必要パッケージのインストール
brew install cmake eigen pybind11 googletest

# Python環境（推奨: pyenv + venv）
brew install pyenv
pyenv install 3.11.7
pyenv global 3.11.7

# Pythonパッケージ
pip install numpy scipy matplotlib pyqt6 pytest
```

### 2.2 Ubuntu/Debian Linux

```bash
# システムパッケージ
sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    libeigen3-dev \
    pybind11-dev \
    libgtest-dev \
    python3.11 \
    python3.11-venv \
    python3-pip

# Pythonパッケージ
python3.11 -m venv venv
source venv/bin/activate
pip install numpy scipy matplotlib pyqt6 pytest
```

### 2.3 Windows（WSL2推奨）

**推奨**: WSL2 + Ubuntu 22.04を使用し、上記Linuxの手順に従う。

**ネイティブWindows**の場合:
1. Visual Studio 2022（C++デスクトップ開発）をインストール
2. CMake 3.25+をインストール
3. vcpkgでライブラリをインストール:
   ```powershell
   vcpkg install eigen3:x64-windows pybind11:x64-windows gtest:x64-windows
   ```
4. Python 3.11+をインストール（公式サイトから）
5. `pip install numpy scipy matplotlib pyqt6 pytest`

---

## 3. プロジェクトのクローン

```bash
# プロジェクトディレクトリに移動（既存の場合）
cd /Users/igarashi/local/project_workspace/crlEPH2

# または新規クローン（将来的にGitHubにホストした場合）
# git clone https://github.com/your-org/eph-project.git
# cd eph-project
```

**ディレクトリ構造の確認**:
```bash
ls -la
# 出力例:
# .claude/          ← ドキュメント体系
# doc/              ← 研究提案書
# packages/         ← C++パッケージ（Phase 1以降に作成）
# CLAUDE.md         ← プロジェクト司令塔
```

---

## 4. C++ビルド（Phase 1以降）

### 4.1 初回ビルド

```bash
# プロジェクトルートで実行
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# ビルド
cmake --build build -j$(nproc)  # Linux/macOS
# または
cmake --build build -j%NUMBER_OF_PROCESSORS%  # Windows
```

**オプション**:
- `-DCMAKE_BUILD_TYPE=Release`: 最適化ビルド（Phase 6でのベンチマーク時）
- `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`: clangd用（エディタ補完）

### 4.2 個別パッケージのビルド

Phase 1で `eph_core` のみビルドする場合:

```bash
cd packages/eph_core
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

### 4.3 ビルドの確認

```bash
# ビルドディレクトリに移動
cd build

# テスト実行
ctest --output-on-failure

# 期待される出力（Phase 1完了時）:
# Test project /path/to/eph-project/build
#     Start 1: eph_core_types_test
# 1/2 Test #1: eph_core_types_test ..........   Passed    0.01 sec
#     Start 2: eph_spm_boundary_test
# 2/2 Test #2: eph_spm_boundary_test ........   Passed    0.02 sec
#
# 100% tests passed, 0 tests failed out of 2
```

---

## 5. Pythonパッケージのインストール（Phase 4以降）

### 5.1 仮想環境の作成（推奨）

```bash
# プロジェクトルートで実行
python3 -m venv venv
source venv/bin/activate  # Linux/macOS
# または
venv\Scripts\activate  # Windows
```

### 5.2 開発モードでインストール

```bash
# C++ビルドが完了していることを確認
cd /Users/igarashi/local/project_workspace/crlEPH2

# Pythonパッケージのインストール（Phase 4以降）
pip install -e packages/eph_gui
pip install -e packages/eph_analysis
pip install -e packages/eph_app
```

### 5.3 インストールの確認

```python
# Pythonインタプリタで確認
python
>>> import eph_native  # pybind11バインディング
>>> import eph_gui
>>> import eph_analysis
>>> import eph_app
>>> exit()
```

---

## 6. 動作確認

### 6.1 Phase 1完了時の確認

```bash
# C++単体テストの実行
cd build
ctest --output-on-failure

# 個別テストの実行
./packages/eph_core/tests/test_types
./packages/eph_spm/tests/test_boundary
```

### 6.2 Phase 3完了時の確認（相転移検出）

```bash
# β sweep実行（Python経由）
python scripts/run_phase_sweep.py --beta-min 0.0 --beta-max 0.3 --n-points 30

# 出力: β-φ.png, β-χ.png が生成される
# 期待: β ≈ 0.098 でχ(β)がピーク
```

### 6.3 Phase 4完了時の確認（GUI起動）

```bash
# GUIアプリ起動
python -m eph_app gui

# 期待: PyQt6ウィンドウが開き、β-φダイアグラムが表示される
```

---

## 7. トラブルシューティング

### 7.1 CMakeがEigenを見つけられない

**症状**:
```
CMake Error: Could not find package Eigen3
```

**解決策（macOS Homebrew）**:
```bash
# Eigenのパスを確認
brew --prefix eigen

# CMakeに明示的に指定
cmake -B build -S . -DEigen3_DIR=$(brew --prefix eigen)/share/eigen3/cmake
```

**解決策（Ubuntu）**:
```bash
# eigen3-configが/usr/lib/cmake/eigen3/にあることを確認
ls /usr/lib/cmake/eigen3/

# なければ再インストール
sudo apt install --reinstall libeigen3-dev
```

---

### 7.2 pybind11バインディングのビルドエラー

**症状**:
```
fatal error: 'pybind11/pybind11.h' file not found
```

**解決策**:
```bash
# pybind11のインストール確認
python -c "import pybind11; print(pybind11.get_cmake_dir())"

# CMakeに明示的に指定
cmake -B build -S . -Dpybind11_DIR=$(python -c "import pybind11; print(pybind11.get_cmake_dir())")
```

---

### 7.3 Pythonインポートエラー（eph_native not found）

**症状**:
```python
ModuleNotFoundError: No module named 'eph_native'
```

**原因**: C++ビルドが完了していない、またはPYTHONPATHが設定されていない

**解決策**:
```bash
# C++ビルドの確認
ls build/packages/eph_bridge/eph_native*.so  # Linux/macOS
ls build/packages/eph_bridge/eph_native*.pyd  # Windows

# PYTHONPATHに追加（一時的）
export PYTHONPATH=$PYTHONPATH:$(pwd)/build/packages/eph_bridge

# または、ビルド後にインストール
cd build/packages/eph_bridge
pip install .
```

---

### 7.4 テストがすべて失敗する

**症状**:
```
0% tests passed, 10 tests failed out of 10
```

**診断**:
```bash
# 詳細出力でテスト実行
ctest --verbose --output-on-failure

# 個別テストを直接実行
./packages/eph_core/tests/test_types --gtest_filter=*
```

**よくある原因**:
- リンクエラー（ライブラリパスの問題）
- 実行時エラー（アサーション失敗、NaN発生）

詳細は `.claude/05_troubleshooting/common_errors.md` を参照。

---

## 8. 次のステップ

環境構築が完了したら:

1. **プロジェクトツアー**: `.claude/04_onboarding/project_tour.md`（Phase 6以降に作成）
2. **最初の実装**: `.claude/01_development_docs/package_specs/eph_core.md` を読んでPhase 1開始
3. **コーディング規約の確認**: `.claude/01_development_docs/cpp_guidelines/coding_style.md`

---

## 9. 開発コマンドのクイックリファレンス

```bash
# ビルド（全体）
cmake -B build -S . && cmake --build build -j$(nproc)

# ビルド（個別パッケージ）
cmake --build build --target eph_core -j$(nproc)

# テスト実行
cd build && ctest --output-on-failure

# クリーンビルド
rm -rf build && cmake -B build -S . && cmake --build build -j$(nproc)

# フォーマット（clang-format）
find packages -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Pythonテスト
pytest packages/eph_gui/tests/
pytest packages/eph_analysis/tests/

# GUI起動（Phase 4以降）
python -m eph_app gui

# β sweep実行（Phase 3以降）
python scripts/run_phase_sweep.py
```

---

## 10. ヘルプ・サポート

- **トラブルシューティング**: `.claude/05_troubleshooting/`
- **FAQ**: `.claude/04_onboarding/faq.md`（Phase 6以降に作成）
- **Issue報告**: プロジェクトのIssue Tracker（将来的にGitHubで）

---

**セットアップ完了の確認**:
```bash
# 以下がすべて成功すればOK
cmake --version          # ≥ 3.20
python --version         # ≥ 3.9
python -c "import numpy; print(numpy.__version__)"  # ≥ 1.21
python -c "import eigen; import pybind11"  # エラーなし
cd build && ctest        # すべてのテストがパス（Phase 1以降）
```

---

*環境構築に成功したら、次は `.claude/00_project/overview.md` でプロジェクト全体像を把握してください。*
