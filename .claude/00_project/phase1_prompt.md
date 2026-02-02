# Phase 1開始時プロンプト

**Phase 1目標**: eph_core + eph_spm実装（Week 1-2）

---

## AIへの指示（Phase 1開始時）

```markdown
Phase 1（eph_core + eph_spm実装）を開始します。

以下のドキュメントを統合版構造（`.claude/` 参照）で作成してください:

### 1. システムアーキテクチャ（01_system_architecture/）

**01_system_architecture/05_package_hierarchy.md**
- 8パッケージの依存関係図（mermaid）
- eph_core → eph_spm の詳細説明
- CMakeのtarget依存関係

### 2. 実装ガイド（02_implementation_guides/）

**02_implementation_guides/library_integration/eigen_patterns.md**
- Eigenの基本使用法（Vector, Matrix, Tensor）
- メモリレイアウト（行優先 vs 列優先）
- SPM用Tensor3の使い方
- Eigen::Mapの活用（ゼロコピー）
- 参考: `doc/appendix/EPH-2.1_appendix-B_spm.md` の実装要件

**02_implementation_guides/numerical_methods/boundary_conditions.md**
- 周期境界（θ方向）の実装方法
- Neumann境界（r方向）の実装方法
- 境界条件のテスト方法
- 参考: CLAUDE.md §Critical Implementation Notes

### 3. 技術スタック（05_tech_stack_docs/）

**05_tech_stack_docs/03_dependency_management.md**
- CMakeLists.txt 構造（トップレベル + パッケージ別）
- find_package(Eigen3) の設定
- find_package(pybind11) の設定（Phase 4で使用）
- GoogleTest統合

---

## 実装タスク

ドキュメント作成後、以下を実装:

1. **eph_core**: `.claude/01_development_docs/package_specs/eph_core.md` を参照
2. **eph_spm**: 新規作成する `01_system_architecture/02_data_structure_design.md` の仕様に従う
3. **単体テスト**: GoogleTestで境界条件を検証

---

## 検証基準（Phase 1完了判定）

```bash
# ビルド成功
cmake -B build -S . && cmake --build build

# テスト全通過
cd build && ctest --output-on-failure
# 期待: eph_core_types_test, eph_spm_boundary_test がPASS

# 境界条件の確認
./packages/eph_spm/tests/test_gradients
# 期待: θ方向は周期的、r方向はNeumann
```

---

## 参照ドキュメント

- **理論**: `doc/appendix/EPH-2.1_appendix-B_spm.md`（SPM完全仕様）
- **全体方針**: `CLAUDE.md`
- **用語集**: `.claude/00_project/glossary.md`
- **コーディング規約**: `.claude/01_development_docs/cpp_guidelines/coding_style.md`
```

---

このプロンプトをPhase 1開始時にClaude Codeに与えることで、統合版構造でドキュメントと実装が自動生成されます。
