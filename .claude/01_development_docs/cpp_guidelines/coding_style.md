# C++ コーディング規約

**最終更新**: 2026-02-02

## このドキュメントの目的

EPHプロジェクトのC++コードにおける命名規則、フォーマット、コーディングスタイルの統一基準を提供します。全C++パッケージ（eph_core～eph_world）で共通して適用されます。

**対象読者**: C++開発者、AIエージェント

**参考**: Google C++ Style Guide、LLVM Coding Standards

---

## 目次

1. [命名規則](#1-命名規則)
2. [ファイル構成](#2-ファイル構成)
3. [フォーマット（clang-format）](#3-フォーマットclang-format)
4. [コメント](#4-コメント)
5. [ヘッダーファイル](#5-ヘッダーファイル)
6. [名前空間](#6-名前空間)
7. [クラス設計](#7-クラス設計)
8. [エラーハンドリング](#8-エラーハンドリング)
9. [テストコード](#9-テストコード)

---

## 1. 命名規則

### 1.1 クラス・構造体

**形式**: PascalCase（各単語の先頭大文字）

```cpp
// ✅ Good
class EPHAgent { ... };
class SaliencyPolarMap { ... };
struct AgentState { ... };
class PhaseAnalyzer { ... };

// ❌ Bad
class eph_agent { ... };
class Saliency_Polar_Map { ... };
class agentState { ... };
```

**略語の扱い**: 2文字略語は大文字、3文字以上は先頭のみ大文字

```cpp
// ✅ Good
class SPM { ... };        // 2文字略語
class EMAFilter { ... };  // 3文字以上
class HTTPClient { ... };

// ❌ Bad
class Spm { ... };
class EMaFilter { ... };
class HttpClient { ... };
```

---

### 1.2 関数・メソッド

**形式**: snake_case（小文字 + アンダースコア）

```cpp
// ✅ Good
void compute_action(const SPM& spm);
auto update_haze(double dt) -> void;
Vec2 get_position() const;
void set_kappa(double kappa);

// ❌ Bad
void ComputeAction(const SPM& spm);
void UpdateHaze(double dt);
Vec2 GetPosition() const;
```

**動詞から始める**: 関数名は動作を表す動詞で開始

```cpp
// ✅ Good
compute_gradient();
update_state();
check_collision();
is_valid();
has_converged();

// ❌ Bad
gradient();       // 名詞のみ
state_update();   // 順序が逆
valid();          // 述語の場合は is_ を付ける
```

---

### 1.3 変数

**形式**: snake_case

```cpp
// ✅ Good
double haze_value = 0.5;
int agent_count = 100;
const Vec2 initial_position(0, 0);

// ❌ Bad
double hazeValue;
int AgentCount;
const Vec2 InitialPosition;
```

**メンバ変数**: 末尾にアンダースコア `_`

```cpp
class EPHAgent {
private:
    AgentState state_;          // ✅ Good
    SaliencyPolarMap spm_;      // ✅ Good
    double kappa_;              // ✅ Good

    // ❌ Bad
    AgentState state;
    AgentState m_state;
    AgentState _state;
};
```

**ポインタ・参照の型修飾子**: 変数名側に付ける

```cpp
// ✅ Good
int* ptr;
const Vec2& ref;

// ❌ Bad（ただし許容される）
int *ptr;
const Vec2 &ref;
```

---

### 1.4 定数

**形式**: UPPER_SNAKE_CASE

```cpp
// ✅ Good
constexpr int N_CHANNELS = 10;
constexpr int N_THETA = 12;
constexpr int N_R = 12;
constexpr double BETA_C = 0.098;

// ❌ Bad
constexpr int nChannels = 10;
constexpr int n_channels = 10;
const double betaC = 0.098;
```

**constexpr vs const**:
- コンパイル時定数 → `constexpr`
- 実行時定数 → `const`

```cpp
constexpr double PI = 3.14159265358979;  // ✅ コンパイル時定数
const double runtime_value = compute();   // ✅ 実行時定数
```

---

### 1.5 Enum

**Enum class（推奨）**: PascalCase、メンバーもPascalCase

```cpp
// ✅ Good
enum class ChannelID {
    T0,  // Obs Occupancy
    R0,  // Δoccupancy
    R1,  // Uncertainty
    F0,  // Occupancy (Current)
    F1,  // Motion Pressure
    F2,  // Saliency
    F3,  // TTC Proxy
    F4,  // Visibility
    F5,  // Observation Stability
    M0   // Haze Field
};

// ❌ Bad
enum ChannelID {  // C-style enum（避ける）
    CHANNEL_T0,
    CHANNEL_R0,
    ...
};
```

---

### 1.6 名前空間

**形式**: snake_case

```cpp
// ✅ Good
namespace eph {
    namespace spm {
        class SaliencyPolarMap { ... };
    }
    namespace agent {
        class EPHAgent { ... };
    }
}

// ❌ Bad
namespace EPH {
    namespace SPM { ... }
}
```

**ネストは2階層まで**:
```cpp
eph::spm::SaliencyPolarMap  // ✅ Good（2階層）
eph::core::types::Vec2      // ⚠️ 可（3階層、必要な場合のみ）
```

---

### 1.7 テンプレートパラメータ

**形式**: PascalCase

```cpp
// ✅ Good
template <typename T, int Dim>
class Tensor { ... };

template <typename Scalar>
auto compute(Scalar value) -> Scalar;

// ❌ Bad
template <typename t, int dim>
class Tensor { ... };
```

---

## 2. ファイル構成

### 2.1 ファイル名

**形式**: snake_case

```
✅ Good
eph_core/include/eph_core/types.hpp
eph_spm/src/saliency_polar_map.cpp
eph_agent/include/eph_agent/haze_estimator.hpp

❌ Bad
eph_core/include/eph_core/Types.hpp
eph_spm/src/SaliencyPolarMap.cpp
eph_agent/include/eph_agent/HazeEstimator.hpp
```

**拡張子**:
- ヘッダー: `.hpp`（`.h` は C互換用のみ）
- 実装: `.cpp`
- テスト: `test_*.cpp`

---

### 2.2 ディレクトリ構造

```
packages/eph_<package>/
├── CMakeLists.txt
├── include/
│   └── eph_<package>/
│       ├── <class_name>.hpp       # 1クラス = 1ファイル
│       └── <another_class>.hpp
├── src/
│   ├── <class_name>.cpp
│   └── <another_class>.cpp
└── tests/
    ├── test_<feature>.cpp
    └── fixtures/
        └── test_data.hpp
```

---

### 2.3 ヘッダーファイルのテンプレート

```cpp
// eph_core/include/eph_core/types.hpp

#ifndef EPH_CORE_TYPES_HPP
#define EPH_CORE_TYPES_HPP

#include <Eigen/Core>

namespace eph {

// 型定義
using Scalar = double;
using Vec2 = Eigen::Vector2d;

// 構造体
struct AgentState {
    Vec2 position;
    Vec2 velocity;
    Scalar kappa;
    Scalar fatigue;
};

}  // namespace eph

#endif  // EPH_CORE_TYPES_HPP
```

---

### 2.4 実装ファイルのテンプレート

```cpp
// eph_agent/src/agent.cpp

#include "eph_agent/agent.hpp"

#include <cassert>
#include <cmath>

#include "eph_spm/spm.hpp"

namespace eph::agent {

EPHAgent::EPHAgent(const Config& config)
    : state_{},
      spm_{},
      kappa_{config.kappa} {
    // 初期化処理
}

void EPHAgent::step(Scalar dt) {
    update_haze(dt);
    auto action = compute_action(spm_);
    update_dynamics(action, dt);
}

}  // namespace eph::agent
```

---

## 3. フォーマット（clang-format）

### 3.1 設定ファイル（.clang-format）

プロジェクトルートに配置:

```yaml
# .clang-format
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
DerivePointerAlignment: false
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: Never
AllowShortLoopsOnASingleLine: false
BreakBeforeBraces: Attach
IncludeBlocks: Regroup
```

### 3.2 自動フォーマット

```bash
# 全ファイルをフォーマット
find packages -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# 特定ファイルのみ
clang-format -i packages/eph_core/src/types.cpp
```

---

### 3.3 主要なフォーマット規則

**インデント**: 4スペース（タブ不可）

```cpp
// ✅ Good
void function() {
    if (condition) {
        do_something();
    }
}

// ❌ Bad（タブ使用）
void function() {
	if (condition) {
		do_something();
	}
}
```

**行長**: 最大100文字

```cpp
// ✅ Good（99文字）
auto result = compute_very_long_function_name(
    argument1, argument2, argument3);

// ❌ Bad（120文字超）
auto result = compute_very_long_function_name(argument1, argument2, argument3, argument4, argument5);
```

**波括弧**: K&Rスタイル（開き括弧は同じ行）

```cpp
// ✅ Good
if (condition) {
    ...
}

class MyClass {
    ...
};

// ❌ Bad（Allman style）
if (condition)
{
    ...
}
```

---

## 4. コメント

### 4.1 ファイルヘッダー

```cpp
// eph_agent/include/eph_agent/agent.hpp

/**
 * @file agent.hpp
 * @brief Single agent dynamics with haze estimation
 *
 * Implements EPHAgent class representing a single agent in the swarm.
 * Handles haze estimation, action computation, and 2nd-order dynamics.
 */
```

### 4.2 クラス・関数のドキュメント

```cpp
/**
 * @brief Represents a single agent in the EPH swarm
 *
 * EPHAgent encapsulates the dynamics of a single agent, including:
 * - Haze field estimation (§4.2)
 * - Action computation via EFE gradient descent (§2.3)
 * - 2nd-order dynamics with inertia
 */
class EPHAgent {
public:
    /**
     * @brief Compute action via Expected Free Energy gradient
     * @param spm Saliency Polar Map input
     * @return Control action (force vector)
     *
     * Implements §2.3 of EPH-2.1_main.md:
     * u ← u - η·∇_u G_EPH
     *
     * CRITICAL: Haze must be stop-gradient (use .eval())
     */
    Vec2 compute_action(const SPM& spm);
};
```

### 4.3 インラインコメント

```cpp
// ✅ Good: 理由を説明
// Clip input to prevent sigmoid saturation (numerical stability)
input = input.cwiseMax(-10.0).cwiseMin(10.0);

// θ direction: periodic boundary (wrap with mod 12)
int theta_plus = (theta + 1) % N_THETA;

// ❌ Bad: 自明なコメント
// Add 1 to theta
int theta_plus = theta + 1;
```

### 4.4 TODO/FIXME/NOTE

```cpp
// TODO(username): Implement adaptive learning rate
// FIXME: This breaks for β > 1.0
// NOTE: §4.2 recommends a=0.4, b=0.3, c=0.2, d=0.1
```

---

## 5. ヘッダーファイル

### 5.1 ヘッダーガード

**形式**: `EPH_<PACKAGE>_<FILE>_HPP`

```cpp
// eph_agent/include/eph_agent/haze_estimator.hpp

#ifndef EPH_AGENT_HAZE_ESTIMATOR_HPP
#define EPH_AGENT_HAZE_ESTIMATOR_HPP

// ... 内容 ...

#endif  // EPH_AGENT_HAZE_ESTIMATOR_HPP
```

**`#pragma once` は使わない**（移植性のため）

---

### 5.2 include順序

1. 対応するヘッダー（.cpp の場合）
2. C標準ライブラリ
3. C++標準ライブラリ
4. サードパーティライブラリ（Eigen等）
5. 自プロジェクトのヘッダー

各グループ内はアルファベット順。

```cpp
// eph_agent/src/agent.cpp

#include "eph_agent/agent.hpp"  // 1. 対応するヘッダー

#include <cassert>              // 2. C標準ライブラリ
#include <cmath>

#include <iostream>             // 3. C++標準ライブラリ
#include <vector>

#include <Eigen/Core>           // 4. サードパーティ

#include "eph_core/types.hpp"   // 5. 自プロジェクト
#include "eph_spm/spm.hpp"
```

---

## 6. 名前空間

### 6.1 宣言

```cpp
// ✅ Good: 各階層を個別に開く
namespace eph {
namespace agent {

class EPHAgent { ... };

}  // namespace agent
}  // namespace eph

// ✅ Good: C++17のネスト記法
namespace eph::agent {

class EPHAgent { ... };

}  // namespace eph::agent
```

### 6.2 using宣言

**ヘッダーファイルでは避ける**:
```cpp
// ❌ Bad（ヘッダーファイルで）
using namespace std;
using namespace Eigen;

// ✅ Good（.cppファイルで、関数内のみ）
void compute() {
    using Eigen::Matrix;
    Matrix m;
}
```

---

## 7. クラス設計

### 7.1 メンバー順序

```cpp
class EPHAgent {
public:
    // 1. 型定義
    using Config = AgentConfig;

    // 2. コンストラクタ・デストラクタ
    EPHAgent(const Config& config);
    ~EPHAgent() = default;

    // 3. コピー・ムーブ（明示的に削除 or デフォルト）
    EPHAgent(const EPHAgent&) = delete;
    EPHAgent& operator=(const EPHAgent&) = delete;

    // 4. 公開メソッド
    void step(Scalar dt);
    Vec2 compute_action(const SPM& spm);

    // 5. Getter/Setter
    auto state() const -> const AgentState& { return state_; }
    void set_kappa(Scalar kappa) { kappa_ = kappa; }

private:
    // 6. プライベートメソッド
    void update_haze(Scalar dt);
    void update_dynamics(const Vec2& action, Scalar dt);

    // 7. メンバ変数
    AgentState state_;
    SaliencyPolarMap spm_;
    Scalar kappa_;
};
```

---

### 7.2 const correctness

```cpp
// ✅ Good: const参照で受け取る
void process(const SPM& spm);

// ✅ Good: constメソッド
auto get_position() const -> Vec2;

// ❌ Bad: 不要なコピー
void process(SPM spm);  // 巨大なオブジェクトをコピー

// ❌ Bad: const忘れ
Vec2 get_position();  // 状態を変更しないのにconstでない
```

---

## 8. エラーハンドリング

### 8.1 アサーション

**開発時**: アサーションで前提条件をチェック

```cpp
void set_kappa(Scalar kappa) {
    assert(kappa >= 0.1 && kappa <= 2.0 && "kappa out of range");
    kappa_ = kappa;
}

void update_haze(const SPM& spm) {
    auto h = compute_haze(spm);
    assert(h.minCoeff() >= 0.0 && h.maxCoeff() <= 1.0);
    assert(h.allFinite() && "Haze contains NaN or Inf");
    haze_ = h;
}
```

**リリース時**: アサーションは無効化される（-DNDEBUG）

---

### 8.2 例外（慎重に使用）

```cpp
// ✅ Good: 回復不能なエラーのみ例外を投げる
void load_config(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config: " + path);
    }
}

// ❌ Bad: 制御フローに例外を使わない
try {
    auto agent = find_agent(id);
    agent.update();
} catch (const AgentNotFound&) {
    // 例外で制御フロー → アンチパターン
}
```

---

## 9. テストコード

### 9.1 テストファイル命名

```
tests/test_<feature>.cpp
```

例:
```
eph_core/tests/test_types.cpp
eph_spm/tests/test_boundary.cpp
eph_agent/tests/test_haze_dynamics.cpp
```

### 9.2 テストケース命名

```cpp
TEST(TestSuiteName, TestCaseName) {
    // Arrange
    ...
    // Act
    ...
    // Assert
    ...
}
```

**形式**: `TEST(クラス名, 機能_条件_期待結果)`

```cpp
// ✅ Good
TEST(HazeEstimator, Compute_ValidInput_ReturnsNormalizedHaze) {
    HazeEstimator estimator;
    auto haze = estimator.compute(mock_spm);
    EXPECT_TRUE((haze.array() >= 0.0).all());
    EXPECT_TRUE((haze.array() <= 1.0).all());
}

TEST(SaliencyPolarMap, GradientTheta_PeriodicBoundary_Wraps) {
    SPM spm;
    auto grad = spm.gradient_theta(ChannelID::M0);
    // θ=0 と θ=11 が連続することを確認
}
```

---

## 10. clang-tidy（静的解析）

### 10.1 推奨チェック

```yaml
# .clang-tidy
Checks: >
  bugprone-*,
  modernize-*,
  performance-*,
  readability-*,
  -modernize-use-trailing-return-type
```

### 10.2 実行

```bash
clang-tidy packages/eph_core/src/*.cpp -- -I packages/eph_core/include
```

---

## まとめ

この規約に従うことで:
- ✅ コードの一貫性が保たれる
- ✅ AIエージェント（Claude Code）が迷わず実装できる
- ✅ レビュー負荷が下がる
- ✅ バグが減少する

**自動チェック**:
```bash
# フォーマット
clang-format -i <file>

# 静的解析
clang-tidy <file>

# テスト
ctest --output-on-failure
```

---

**関連ドキュメント**:
- Eigenの使用法: `.claude/01_development_docs/cpp_guidelines/eigen_patterns.md`
- テスト戦略: `.claude/01_development_docs/cpp_guidelines/testing_strategy.md`
- 数値安定性: `.claude/01_development_docs/cpp_guidelines/numerical_stability.md`
