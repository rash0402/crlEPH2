# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

EPH (Expected-Free-Energy-based Perceptual Haze) v2.1 is a theoretical research framework for swarm intelligence control based on the Free Energy Principle (FEP) and Active Inference. The central thesis is that **swarm adaptability is maximized at the critical point (Edge of Chaos)** of the Markov Blanket breaking strength parameter β.

## Architecture Strategy

This project follows a **hierarchical modular package architecture**:
- **Core simulation**: C++ (performance-critical agent dynamics, multi-agent coordination)
- **GUI & Analysis**: Python/PyQt6 (visualization, metrics, validation)
- **Integration**: pybind11 (C++↔Python bridge)

Development approach: **Vertical slicing** - implement one validation target (V1-V5) completely before moving to the next, enabling incremental paper figure generation.

## Document Structure

```
doc/
├── EPH-2.1_main.md           # Main research proposal (v2.1)
└── appendix/
    ├── EPH-2.1_appendix-A_proofs.md      # Mathematical proofs (Theorems 1-3, Proposition 1)
    ├── EPH-2.1_appendix-B_spm.md         # SPM (Saliency Polar Map) specification
    ├── EPH-2.1_appendix-C_validation.md  # Numerical validation plan
    └── EPH-2.1_appendix-D_related-work.md # Literature survey
```

## Core Theoretical Concepts

### Key Parameters
- **β (MB breaking strength)**: Controls coupling between agents' haze fields. Critical point β_c ≈ 0.098
- **κ (haze sensitivity)**: Individual agent parameter following 2:6:2 distribution (Leader/Follower/Reserve)
- **h (haze)**: Intrinsic perceptual uncertainty field, estimated from prediction errors

### Central Claims
1. **Theorem 1**: Susceptibility χ(β) is maximized at critical point β_c → maximum adaptability
2. **Theorem 2**: β_c = (1/τ_haze - h̄(1-h̄)κηL_f²) / 2(z-1) (closed-form critical point)
3. **Theorem 3**: Linear convergence with rate ρ ≈ 0.02

### SPM (Saliency Polar Map) Channels
10-channel egocentric polar grid (12×12 in θ×r):
- T0: Ground truth occupancy (teacher signal)
- R0: Predicted Δoccupancy (+1 step) — **only learnable channel**
- R1: Prediction uncertainty
- F0-F5: Prediction-free observations (occupancy, motion pressure, saliency, TTC, visibility, stability)
- M0: Haze field (stop-gradient, not differentiable w.r.t. action u)

## Implementation Stack

```
C++ Core:
  - Eigen 3.4+        Tensor/matrix operations
  - pybind11 2.11+    Python bindings
  - CMake 3.20+       Build system
  - GoogleTest        Unit testing

Python Layer:
  - PyQt6             GUI framework
  - NumPy/SciPy       Numerical analysis
  - Matplotlib        Plotting
  - pytest            Testing
```

## Package Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                    eph_app (統合アプリ)                      │
│                    CLI + Configuration                       │
└──────────────────────────┬──────────────────────────────────┘
                           ↓
        ┌──────────────────┼──────────────────┐
        ↓                  ↓                  ↓
┌───────────────┐  ┌─────────────────┐  ┌──────────────┐
│   eph_gui     │  │  eph_analysis   │  │  eph_config  │
│   PyQt6 UI    │  │  Metrics/Valid. │  │  YAML/Param  │
└───────┬───────┘  └────────┬────────┘  └──────────────┘
        └─────────────┬─────┘
                      ↓
        ┌─────────────────────┐
        │    eph_bridge       │  ← pybind11 bindings
        └─────────────────────┘
                      ↓
┌─────────────────────┴────────────────────────────┐
│              C++ Core Packages                   │
├──────────────┬──────────────┬────────────────────┤
│  eph_swarm   │  eph_phase   │   eph_world        │
│ Multi-Agent  │ Phase Trans. │ Environment/Obs    │
└──────┬───────┴──────┬───────┴────────┬───────────┘
       └──────────────┼────────────────┘
                      ↓
            ┌─────────────────┐
            │   eph_agent     │  Single Agent Dynamics
            └────────┬────────┘
                     ↓
            ┌─────────────────┐
            │    eph_spm      │  SPM Structure
            └────────┬────────┘
                     ↓
            ┌─────────────────┐
            │   eph_core      │  Math Primitives
            └─────────────────┘
```

## Repository Structure

```
eph-project/
├── packages/
│   ├── eph_core/          # Layer 0: Math primitives
│   │   ├── CMakeLists.txt
│   │   ├── include/eph_core/
│   │   │   ├── types.hpp        # AgentState, Vec2, Tensor3
│   │   │   └── config.hpp       # Parameter structs
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── eph_spm/           # Layer 1: SPM 10×12×12
│   │   ├── CMakeLists.txt
│   │   ├── include/eph_spm/
│   │   │   ├── spm.hpp          # SaliencyPolarMap class
│   │   │   ├── channels.hpp     # ChannelID enum
│   │   │   └── operators.hpp    # Gradient, boundary ops
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── eph_agent/         # Layer 2: Single agent
│   │   ├── CMakeLists.txt
│   │   ├── include/eph_agent/
│   │   │   ├── agent.hpp        # EPHAgent class
│   │   │   ├── haze.hpp         # HazeEstimator
│   │   │   └── dynamics.hpp     # 2nd order dynamics
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── eph_swarm/         # Layer 3a: Multi-agent
│   │   ├── CMakeLists.txt
│   │   ├── include/eph_swarm/
│   │   │   ├── swarm.hpp        # Swarm class
│   │   │   ├── roles.hpp        # 2:6:2 RoleManager
│   │   │   └── neighbors.hpp    # NeighborGraph
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── eph_phase/         # Layer 3b: Phase analysis
│   │   ├── CMakeLists.txt
│   │   ├── include/eph_phase/
│   │   │   └── analysis.hpp     # PhaseAnalyzer
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── eph_world/         # Layer 3c: Environment
│   │   ├── CMakeLists.txt
│   │   ├── include/eph_world/
│   │   │   ├── world.hpp        # World scenarios
│   │   │   └── obstacles.hpp    # Collision detection
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── eph_bridge/        # Layer 4: Python bindings
│   │   ├── CMakeLists.txt
│   │   └── src/
│   │       └── bindings.cpp     # pybind11 module
│   │
│   ├── eph_gui/           # Layer 5a: GUI
│   │   ├── pyproject.toml
│   │   └── src/eph_gui/
│   │       ├── main_window.py   # Main window
│   │       ├── spm_viewer.py    # SPM heatmap
│   │       ├── phase_plot.py    # β-φ diagram
│   │       └── swarm_view.py    # 2D visualization
│   │
│   ├── eph_analysis/      # Layer 5b: Analysis
│   │   ├── pyproject.toml
│   │   └── src/eph_analysis/
│   │       ├── validators.py    # V1-V5 runners
│   │       └── figures.py       # Paper figures
│   │
│   └── eph_app/           # Layer 6: Integration
│       ├── pyproject.toml
│       └── src/eph_app/
│           └── cli.py           # Command-line interface
│
├── docs/                  # Research documents (current doc/)
├── CMakeLists.txt         # Top-level C++ build
├── pyproject.toml         # Python workspace
└── CLAUDE.md
```

## Package Interfaces & Key APIs

### eph_core::types.hpp
```cpp
namespace eph {
    using Scalar = double;
    using Vec2 = Eigen::Vector2d;
    using Tensor3 = Eigen::Tensor<Scalar, 3>;  // C×θ×r

    struct AgentState {
        Vec2 position;
        Vec2 velocity;
        Scalar kappa;      // haze sensitivity (0.3-1.5)
        Scalar fatigue;    // fatigue level (0-1)
    };
}
```

### eph_spm::spm.hpp
```cpp
class SaliencyPolarMap {
    static constexpr int N_CHANNELS = 10;
    static constexpr int N_THETA = 12;
    static constexpr int N_R = 12;

    // Channel access (returns Eigen::Map)
    auto channel(ChannelID id) -> Eigen::Map<Eigen::Matrix<Scalar, 12, 12>>;

    // Boundary-aware gradients
    auto gradient_theta(int ch) -> Eigen::Matrix<Scalar, 12, 12>;  // periodic
    auto gradient_r(int ch) -> Eigen::Matrix<Scalar, 12, 12>;      // Neumann
};

enum class ChannelID { T0, R0, R1, F0, F1, F2, F3, F4, F5, M0 };
```

### eph_agent::agent.hpp
```cpp
class EPHAgent {
    void step(Scalar dt);  // Full update (haze + action + dynamics)
    void update_haze(const SaliencyPolarMap& obs);
    Vec2 compute_action(const SaliencyPolarMap& spm);

    auto state() const -> const AgentState&;
    auto spm() const -> const SaliencyPolarMap&;
};
```

### eph_swarm::swarm.hpp
```cpp
class Swarm {
    Swarm(size_t n_agents, const SwarmConfig& config);

    void step(Scalar dt);  // Collective step
    void propagate_haze(Scalar beta);  // MB breaking: h_eff = (1-β)h + β<h>
    void update_roles();  // 2:6:2 management

    auto role_distribution() const -> std::array<Scalar, 3>;  // [w_L, w_F, w_R]
};
```

### eph_phase::analysis.hpp
```cpp
struct PhaseMetrics {
    Scalar order_parameter;    // φ(β) = ⟨|h_i - h̄|⟩
    Scalar susceptibility;     // χ(β) = N(⟨φ²⟩ - ⟨φ⟩²)
    Scalar correlation_length; // ξ(β)
};

class PhaseAnalyzer {
    auto compute_metrics(const Swarm& swarm) -> PhaseMetrics;
    auto sweep_beta(Scalar beta_min, Scalar beta_max, int n_points)
        -> std::vector<std::pair<Scalar, PhaseMetrics>>;
    auto estimate_beta_c(const std::vector<...>& data) -> Scalar;
};
```

## Development Order (AI-DLC Optimized)

### Phase 1: Foundation (Week 1-2)
**Target**: Build & test eph_core + eph_spm independently

```bash
# Commands
cd packages/eph_core && mkdir build && cd build
cmake .. && make && ctest

cd packages/eph_spm && mkdir build && cd build
cmake .. && make && ctest
```

**Validation**:
- eph_core: Type definitions compile, basic math tests pass
- eph_spm: Gradient operators respect boundary conditions (periodic θ, Neumann r)

### Phase 2: Single Agent (Week 3)
**Target**: eph_agent with haze dynamics only (no action update yet)

**Key Implementation**: Haze estimator (§4.2 in main doc)
```cpp
h_tilde = sigmoid(a·EMA(e) + b·R1 + c·(1-F4) + d·F5)
h = spatial_smooth(h_tilde)
```

**Validation**: Single agent haze converges to stable distribution

### Phase 3: Phase Transition (Week 4)
**Target**: V2 validation - phase transition detection

**Implement**: eph_swarm (haze propagation only) + eph_phase

**Key Experiment**:
```cpp
for (beta : 0.0 -> 0.3 step 0.01) {
    swarm.set_beta(beta);
    run_for(1000_steps);
    metrics = analyzer.compute_metrics(swarm);
    record(beta, metrics.order_parameter);
}
plot(beta, phi);  // Should show discontinuity at β_c ≈ 0.098
```

### Phase 4: Python Integration (Week 5)
**Target**: eph_bridge + minimal eph_gui for visualization

**First Visualization**: Real-time β-φ phase diagram

### Phase 5: Action Update (Week 6-7)
**Target**: Complete eph_agent with action computation, V4/V5 validation

**Key Implementation**: EFE gradient descent (§2.3)
```cpp
Vec2 compute_action(const SPM& spm) {
    auto dO_hat = predictor.forward(spm.PF(), u_prev);
    auto gate = (1 - spm.R1()) * spm.F4() * (1 - kappa * stop_grad(spm.M0()));
    Vec2 grad = compute_gradient(dO_hat, gate);
    return u_prev - eta * grad;
}
```

### Phase 6: Complete System (Week 8-10)
**Target**: eph_world + all V1-V5 validations + paper figures

## AI-DLC Development Guidelines

### Per-Package Development Pattern

When working on a specific package (e.g., `eph_spm`):

1. **Context Setup**: Read the package's interface contract from this CLAUDE.md
2. **Theory Reference**: Consult the relevant section in `docs/EPH-2.1_main.md` or appendices
3. **Implementation**: Write C++ code following the interface
4. **Unit Tests**: Write tests that validate theoretical properties
5. **Integration Tests**: Test against dependent packages

### Independent Testing Strategy

Each package must be testable **without running the full simulation**:

- **eph_core**: Math property tests (vector ops, tensor shapes)
- **eph_spm**: Boundary condition tests (wrap θ, Neumann r)
- **eph_agent**: Mock SPM input → verify haze dynamics
- **eph_swarm**: Mock agents → verify haze propagation formula
- **eph_phase**: Synthetic data → verify metric computation

### Build Commands

```bash
# Build all C++ packages
cmake -B build -S . && cmake --build build

# Build specific package
cmake -B packages/eph_core/build -S packages/eph_core
cmake --build packages/eph_core/build

# Run tests
cd build && ctest

# Install Python packages (after C++ build)
pip install -e packages/eph_gui
pip install -e packages/eph_analysis
pip install -e packages/eph_app
```

### Typical Workflow Example

**Task**: "Implement haze gradient computation in eph_spm"

1. Read `eph_spm::spm.hpp` interface (this file)
2. Read boundary conditions from docs (§7.4 in Appendix B)
3. Implement in `eph_spm/src/spm.cpp`
4. Write test in `eph_spm/tests/test_gradients.cpp`
5. Verify: `cd packages/eph_spm/build && ctest`

## Critical Implementation Notes

### Gradient Flow
- **R0 only**: Only PR-R0 (predicted Δoccupancy) should be differentiated w.r.t. action u
- **Stop-gradient on haze**: M0 (haze) must use `stop_gradient`/`detach` to prevent collapse
- **Gate formulation**: `(1 - R1) * F4 * (1 - κ * sg[h])` where sg = stop_gradient

### Boundary Conditions
- **θ direction**: Periodic (wrap with mod 12)
- **r direction**: Neumann (zero-gradient at boundaries b=0, b=11)

### Time Scale Separation
Maintain τ_θ ≫ τ_h ≫ τ_u:
- Predictor f_θ: Update every ~100 steps (slowest)
- Haze h: EMA with τ_EMA ≥ 1.0s
- Action u: Update every step (fastest)

### 2:6:2 Role Distribution
```python
# κ values by role
Leader (20%):   κ ~ N(0.3, 0.1²)  # Low sensitivity → aggressive
Follower (60%): κ ~ N(1.0, 0.2²)  # Standard
Reserve (20%):  κ ~ N(1.5, 0.15²) # High sensitivity → cautious
```

## Mathematical Notation Reference

| Symbol | Meaning |
|--------|---------|
| F_EPH | Variational Free Energy |
| G_EPH | Expected Free Energy (action objective) |
| ΔÔ | Predicted occupancy change |
| χ(β) | Susceptibility / response function |
| φ(β) | Order parameter (haze heterogeneity) |
| ξ(β) | Correlation length |

## Validation Targets (V1-V5)

| ID | Test | Success Criterion | Package Dependencies |
|----|------|-------------------|----------------------|
| V1 | β_c parameter dependency | Within ±10% of theory | eph_phase |
| V2 | Phase transition detection | Clear φ discontinuity at β_c | eph_swarm + eph_phase |
| V3 | 2:6:2 optimality | F_swarm(2:6:2) < F_swarm(others) | eph_swarm (roles) |
| V4 | Lyapunov descent | V[k+1] ≤ V[k] monotonically | eph_agent (full dynamics) |
| V5 | Convergence rate | ρ_emp ∈ [0.01, 0.03] | eph_agent + eph_analysis |

## Package-Specific Implementation Notes

### eph_core
- Use Eigen::Tensor for SPM (not Eigen::Matrix) - supports 3D indexing
- AgentState should be POD (Plain Old Data) for easy serialization
- All scalar parameters should be configurable via Config structs

### eph_spm
- **Critical**: Implement boundary conditions correctly
  - θ: `(idx + N_THETA) % N_THETA` for wrap-around
  - r: `std::clamp(idx, 0, N_R-1)` for Neumann (zero-gradient)
- Channel access should return Eigen::Map (no copy)
- Consider cache-friendly memory layout (channel-major: C×θ×r)

### eph_agent
- **Critical**: stop_gradient on haze before computing action gradient
  ```cpp
  auto haze_sg = haze.array();  // No .eval() - just use value
  // DO NOT: backprop through haze computation
  ```
- Time scale separation: Update haze with EMA (slow), action every step (fast)
- κ should be immutable after construction (role determines κ)

### eph_swarm
- Neighbor graph should support dynamic updates (agents move)
- Use spatial hashing for O(N) neighbor queries (not O(N²))
- MB breaking propagation: Update all h_eff simultaneously (not sequentially)
- Role transitions: Implement cooldown to prevent chattering

### eph_phase
- Susceptibility χ requires ensemble averaging over multiple runs
- Critical point detection: Fit χ(β) ~ |β - β_c|^(-γ) with γ ≈ 1
- Use finite-size scaling if N < 50 agents

### eph_bridge (pybind11)
- Return Eigen matrices as NumPy arrays (automatic with pybind11/eigen.h)
- Expose iterators for Swarm to enable `for agent in swarm` in Python
- Use `py::call_guard<py::gil_scoped_release>()` for step() to allow threading

### eph_gui
- Use pyqtgraph for real-time plots (faster than matplotlib)
- SPM polar plot: Use `matplotlib.projections.polar` or custom shader
- Update GUI at ~30 FPS, but simulation can run faster in background thread

### eph_analysis
- Cache validation results to HDF5 for reproducibility
- Use multiprocessing for parallel β sweeps
- Generate publication-quality figures with seaborn + custom RC params

## Common Pitfalls & Debugging

### Haze Collapse
**Symptom**: All haze values → 0 or 1 rapidly
**Cause**: Forgot stop_gradient on M0 channel
**Fix**: Verify `haze.detach()` or equivalent in action gradient computation

### Phase Transition Not Visible
**Symptom**: φ(β) is flat across all β
**Cause**: Insufficient equilibration time or wrong boundary conditions
**Fix**: Run for ≥1000 steps per β value, verify haze propagation

### 2:6:2 Distribution Collapses
**Symptom**: All agents converge to same role
**Cause**: Role update too aggressive or fatigue model not working
**Fix**: Add role transition cooldown, verify fatigue accumulation

### Numerical Instability
**Symptom**: NaN values in SPM or agent states
**Cause**: Division by zero in TTC computation or extreme velocities
**Fix**: Add ε = 1e-6 to denominators, clamp velocities to reasonable range

### Python Binding Crash
**Symptom**: Segfault when calling C++ from Python
**Cause**: Lifetime mismatch (Python object outlives C++ object)
**Fix**: Use `py::keep_alive<1, 2>()` in bindings to manage lifetimes

---

## 詳細ドキュメント体系

このCLAUDE.mdはプロジェクト全体の方針を記載しています。詳細な実装ガイドは `.claude/` ディレクトリを参照してください。

### AI-DLC最適化ドキュメント構造

```
.claude/
├── 00_project/                  プロジェクト要件・方針
│   ├── overview.md              AI最初の読み込みファイル（2ページ要約版）
│   ├── glossary.md              数学記号・略語・型対応表
│   ├── phases.md                [Phase 1以降作成] 開発Phase詳細
│   └── validation_targets.md    [Phase 3以降作成] V1-V5検証目標
│
├── 01_development_docs/         技術設計・実装ガイド
│   ├── architecture/
│   │   ├── package_hierarchy.md [Phase 1以降作成]
│   │   ├── data_flow.md         [Phase 2以降作成]
│   │   └── adr/                 [各Phaseで作成] アーキテクチャ決定記録
│   │
│   ├── cpp_guidelines/
│   │   ├── coding_style.md      命名規則・フォーマット
│   │   ├── eigen_patterns.md    [Phase 1] Eigen使用法
│   │   ├── pybind11_guide.md    [Phase 4] Python連携
│   │   ├── testing_strategy.md  [Phase 1] GoogleTest
│   │   └── numerical_stability.md [Phase 2] 数値安定性
│   │
│   ├── python_guidelines/       [Phase 4以降作成]
│   │
│   ├── package_specs/           パッケージ別実装仕様
│   │   ├── eph_core.md          Layer 0仕様
│   │   ├── eph_spm.md           [Phase 1] Layer 1
│   │   ├── eph_agent.md         [Phase 2] Layer 2
│   │   └── ...                  [各Phase]
│   │
│   └── theory_to_code/          [Phase 2以降作成] 理論→実装の橋渡し
│       ├── haze_dynamics.md     [Phase 2] §4.2→C++
│       ├── efe_gradient.md      [Phase 5] §2.3→action計算
│       └── ...
│
├── 02_design_system/            [Phase 4以降作成] ビジュアル・UX設計
├── 03_library_docs/             [Phase 1以降作成] 外部ライブラリ対策
├── 04_onboarding/               新規参加者向け
│   └── quickstart.md            30分セットアップガイド
└── 05_troubleshooting/          [Phase 1以降作成] 問題解決
```

### クイックリンク

**開発開始前（Phase 0完了）:**
- **環境構築**: `.claude/04_onboarding/quickstart.md` - 30分でセットアップ完了
- **用語集**: `.claude/00_project/glossary.md` - 数学記号（β, κ, χ, φ等）の完全定義
- **プロジェクト概要**: `.claude/00_project/overview.md` - AI最初の読み込みファイル

**実装開始時（Phase 1-6）:**
- **コーディング規約（C++）**: `.claude/01_development_docs/cpp_guidelines/coding_style.md`
- **パッケージ仕様**: `.claude/01_development_docs/package_specs/` - 各Phaseで追加
- **理論→コード変換**: `.claude/01_development_docs/theory_to_code/` - Phase 2以降作成

**トラブル発生時:**
- **トラブルシューティング**: `.claude/05_troubleshooting/` - Phase 1以降に問題解決ガイドを追加

**Phase別必要ドキュメント:**
- **Phase 0（完了）**: `overview.md`, `glossary.md`, `quickstart.md`, `coding_style.md`, `eph_core.md`
- **Phase 1-6**: 各Phaseの開始時に該当パッケージのドキュメントを読み込み

### ドキュメント体系の設計思想

この`.claude/`ディレクトリは、Qiita記事「個人開発を変えるClaude Code活用法：24種類のドキュメントでAI-DLCを最適化する」の手法をEPH v2.1の科学計算・C++/Python統合開発に適用したものです。

**主要な適応ポイント:**
1. **理論→コード変換ドキュメント**: 数学的定式化（`doc/EPH-2.1_main.md`）から実装への明示的な橋渡し
2. **C++/Python統合ガイド**: pybind11, Eigen↔NumPy変換など科学計算特有の課題に対応
3. **数値安定性ガイド**: ゼロ除算、sigmoid飽和、NaN/Inf防止など
4. **段階的作成**: Phase 0で最小限の5ドキュメント、Phase 1-6で段階的に拡充

**既存ドキュメントとの役割分担:**
- `doc/EPH-2.1_main.md` + appendices: 理論・数学的定式化（研究者・査読者向け）
- `CLAUDE.md`: プロジェクト全体方針・パッケージ階層（AI司令塔）
- `.claude/**/*.md`: 実装ガイド（C++/Python開発者・AI-DLC向け）

---

## ドキュメント構造の移行計画（Phase 0 → Phase 1-6）

**Phase 0（完了）**: 既存構造（`00_project/` ベース）でドキュメント作成
**Phase 1以降**: 統合版構造（研究特化版）に段階的移行

### 統合版構造（Phase 1以降で採用）

EPH実践 + ユーザー提案「研究開発版AI-DLC」の統合版:

```
.claude/
├── 00_research_context/           ⭐ Phase 2-3で作成
│   ├── 01_research_concept.md     リサーチクエスチョン・仮説・新規性
│   ├── 02_theoretical_framework.md 数式定義（LaTeX）・変数名対応表
│   ├── 03_literature_review.md    関連研究・SOTA比較
│   └── 04_glossary.md             ← Phase 0から継承（数学記号統一）
│
├── 01_system_architecture/        ⭐ Phase 1-2で作成
│   ├── 01_agent_environment_design.md エージェント・環境・相互作用
│   ├── 02_data_structure_design.md    Tensor形状・ログフォーマット
│   ├── 03_class_interface_design.md   クラス設計・継承関係
│   ├── 04_config_management.md        パラメータ管理（YAML等）
│   ├── 05_package_hierarchy.md        パッケージ依存関係
│   └── adr/                           アーキテクチャ決定記録
│
├── 02_implementation_guides/      ⭐ Phase 1-5で作成
│   ├── theory_to_code/            理論→実装の橋渡し
│   │   ├── haze_dynamics.md       §4.2 → C++実装
│   │   ├── mb_breaking.md         §4.3 → haze伝播
│   │   ├── efe_gradient.md        §2.3 → action計算
│   │   └── phase_transition.md    §5.2 → χ(β)計算
│   │
│   ├── coding_conventions/        言語別ガイドライン
│   │   ├── cpp_style.md           ← Phase 0から継承
│   │   ├── python_style.md        [Phase 4]
│   │   └── naming_rules.md        [Phase 1]
│   │
│   ├── numerical_methods/         数値計算手法
│   │   ├── stability.md           [Phase 2] ゼロ除算・sigmoid飽和
│   │   ├── precision.md           [Phase 5] 浮動小数点精度
│   │   └── boundary_conditions.md [Phase 1] 周期境界・Neumann
│   │
│   └── library_integration/       外部ライブラリ使用法
│       ├── eigen_patterns.md      [Phase 1]
│       ├── pybind11_bridge.md     [Phase 4]
│       └── numpy_scipy.md         [Phase 4]
│
├── 03_experiment_protocol/        ⭐ Phase 3-6で作成
│   ├── 01_experimental_scenarios.md β sweep, 2:6:2分布等
│   ├── 02_evaluation_metrics.md     定量評価・定性評価基準
│   ├── 03_ablation_study_plan.md    アブレーション実験
│   ├── 04_reproducibility_check.md  乱数シード・バージョン管理
│   └── 05_validation_plan.md        V1-V5検証目標
│
├── 04_visualization_ops/          ⭐ Phase 4-6で作成
│   ├── 01_figure_standards.md     論文用図版スタイル（フォント・配色）
│   ├── 02_realtime_monitoring.md  学習曲線・ログ監視
│   └── 03_paper_outline.md        論文構成案・セクション配置
│
├── 05_tech_stack_docs/            ⭐ Phase 1-4で作成
│   ├── 01_library_specifics.md    バージョン依存・既知の問題
│   ├── 02_hpc_cloud_setup.md      計算機環境・Docker
│   └── 03_dependency_management.md CMake, requirements.txt
│
├── 06_onboarding/                 ⭐ Phase 0から継承
│   ├── quickstart.md              30分セットアップ
│   ├── first_experiment.md        [Phase 3] 最初のシミュレーション
│   └── project_tour.md            [Phase 6]
│
└── 07_troubleshooting/            ⭐ Phase 1-6で段階的に作成
    ├── numerical_issues.md        [Phase 2] NaN/Inf対策
    ├── haze_collapse.md           [Phase 2] ヘイズ崩壊
    ├── phase_transition_debug.md  [Phase 3] 相転移検出失敗
    ├── performance.md             [Phase 5] メモリリーク・最適化
    └── debugging_guide.md         [Phase 6]
```

### Phase別作成計画

| Phase | 新規作成ドキュメント | 構造カテゴリ |
|-------|---------------------|-------------|
| **Phase 1** | `01_system_architecture/05_package_hierarchy.md`<br/>`02_implementation_guides/library_integration/eigen_patterns.md`<br/>`02_implementation_guides/numerical_methods/boundary_conditions.md`<br/>`05_tech_stack_docs/03_dependency_management.md` | 01, 02, 05 |
| **Phase 2** | `00_research_context/02_theoretical_framework.md`<br/>`02_implementation_guides/theory_to_code/haze_dynamics.md`<br/>`02_implementation_guides/numerical_methods/stability.md`<br/>`07_troubleshooting/numerical_issues.md`<br/>`07_troubleshooting/haze_collapse.md` | 00, 02, 07 |
| **Phase 3** | `00_research_context/01_research_concept.md`<br/>`03_experiment_protocol/01_experimental_scenarios.md`<br/>`03_experiment_protocol/05_validation_plan.md`<br/>`06_onboarding/first_experiment.md`<br/>`07_troubleshooting/phase_transition_debug.md` | 00, 03, 06, 07 |
| **Phase 4** | `02_implementation_guides/library_integration/pybind11_bridge.md`<br/>`04_visualization_ops/01_figure_standards.md`<br/>`04_visualization_ops/02_realtime_monitoring.md`<br/>`05_tech_stack_docs/02_hpc_cloud_setup.md` | 02, 04, 05 |
| **Phase 5** | `02_implementation_guides/theory_to_code/efe_gradient.md`<br/>`02_implementation_guides/numerical_methods/precision.md`<br/>`03_experiment_protocol/03_ablation_study_plan.md`<br/>`07_troubleshooting/performance.md` | 02, 03, 07 |
| **Phase 6** | `00_research_context/03_literature_review.md`<br/>`04_visualization_ops/03_paper_outline.md`<br/>`06_onboarding/project_tour.md`<br/>`07_troubleshooting/debugging_guide.md` | 00, 04, 06, 07 |

### 移行戦略

1. **Phase 0完了ファイルは保持**: `00_project/overview.md`, `00_project/glossary.md` 等はそのまま維持
2. **Phase 1以降は新構造**: 上記の統合版カテゴリ（00-07）でドキュメント作成
3. **必要に応じて参照**: Phase 0ファイルは新構造から参照可能（例: `04_glossary.md` → `00_project/glossary.md`）
4. **Phase 6完了後**: 全体を整理し、グローバルスキル `research-doc-bootstrap` のテンプレートとして抽出

### 統合版の設計思想

この構造は以下の統合により生まれました:

1. **Qiita「24種類のドキュメント」**: Web開発用AI-DLC最適化
2. **EPH v2.1実践知見**: C++/Python科学計算特有の課題（theory_to_code, numerical_stability等）
3. **ユーザー提案「研究開発版AI-DLC」**: 研究プロセスとの対応（Why→Theory→How→Test→Output）

**主要な差別化ポイント:**
- `00_research_context/`: リサーチクエスチョンと数理モデルの明確化
- `02_implementation_guides/theory_to_code/`: 数式→コードの明示的変換
- `03_experiment_protocol/`: 実験の科学的妥当性（再現性・アブレーション）
- `04_visualization_ops/03_paper_outline.md`: 論文執筆の明示的支援

---

*Phase 0ドキュメント作成完了（2026-02-02）。Phase 1以降は統合版構造で段階的に追加します。*
