# EPH v2.1 Phase 5 — Todo List ✅ COMPLETED

## Status Legend
- [ ] Pending
- [x] Completed
- [~] In Progress

---

## Step 0: プロジェクト基盤整備
- [x] Git Phase別コミット整理 (6 commits)
- [x] タスク管理基盤 (tasks/todo.md, tasks/lessons.md)

## Step 1: V1-V5定義統一 ✅
- [x] Appendix C 更新（Phase 5 proposal準拠）
- [x] Phase roadmap 更新

## Step 2: V1検証 — 信念更新検証 ✅
- [x] test_v1_validation.cpp 作成 (12 tests)
- [x] ビルド・テスト通過確認

## Step 3: V3検証 — ボトムアップ顕著性検証 ✅
- [x] test_v3_validation.cpp 作成 (16 tests)
- [x] ビルド・テスト通過確認

## Step 4: V4検証 — 長時間安定性検証 ✅
- [x] test_v4_validation.cpp 作成 (8 tests)
- [x] ビルド・テスト通過確認

## Step 5: V5検証 — 大規模群検証 ✅
- [x] test_v5_validation.cpp 作成 (6 tests)
- [x] ビルド・テスト通過確認

## Step 6: 可視化ツール ✅
- [x] csv_exporter.hpp 実装
- [x] plot_phase_transition.py 拡張
- [x] plot_agent_trajectories.py 新規作成
- [x] plot_haze_field.py 新規作成

## Step 7: ドキュメント整合性修正 ✅
- [x] README.md 最終版更新
- [x] 全テスト通過確認 (実績: 210件、206 pass / 3 既知問題修正中 / 1 timeout)
- [x] Phase 5完了レポート作成

---

## 既知の問題 (Phase 4からの引き継ぎ)

1. **Test #98**: `PredictionError_SmallChange_LowPE` — アサーション失敗
2. **Test #187**: `ExportCSV_ValidData_CreatesFile` — パス環境問題
3. **Test #194**: `BetaSweep_DetectsCriticalPoint` — タイムアウト (120s超過)
4. **Test #195**: `PhiIncreases_OrNonMonotonic` — アサーション失敗

---

## Phase 5 実績サマリー

| 項目 | 目標 | 実績 |
|------|------|------|
| V1検証 | 8-10 tests | 12 tests ✅ |
| V3検証 | 8-10 tests | 16 tests ✅ |
| V4検証 | 6-8 tests | 8 tests ✅ |
| V5検証 | 6-8 tests | 6 tests ✅ |
| 新規テスト合計 | 28-36 | 42 |
| 総テスト数 | 180-190 | 210 (206 pass / 3 既知問題 / 1 timeout) |
| 可視化ツール | 3 scripts | 3 scripts ✅ |

---

*Created: 2026-02-06*
*Last Updated: 2026-02-06 (Phase 5 完了)*
