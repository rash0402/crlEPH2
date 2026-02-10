# EPH v2.1 PyQt6 GUI - 完全設計書

**作成日**: 2026-02-10
**バージョン**: 1.0
**ステータス**: 承認済み
**設計チーム**: UI/UXスペシャリスト (Sarah Chen) + シニアPythonエンジニア (Marcus Rodriguez)

---

## 目次

1. [システム概要](#1-システム概要)
2. [アーキテクチャ](#2-アーキテクチャ)
3. [エラーハンドリング & 通信](#3-エラーハンドリング--通信)
4. [初期UX & レイアウト](#4-初期ux--レイアウト)
5. [パネル詳細設計](#5-パネル詳細設計)
6. [コントロール & メニュー](#6-コントロール--メニュー)
7. [実装計画](#7-実装計画)

---

## 1. システム概要

### 1.1 目的

EPH v2.1 PyQt6 GUIは、C++で実装されたマルチエージェントシミュレーションエンジンのリアルタイム可視化と制御を提供します。研究者が以下を行えるようにします：

- **パラメータ探索**: β、η等をリアルタイムに調整して挙動を観察
- **詳細分析**: 個別エージェントのSPMマップと内部状態を可視化
- **デモンストレーション**: 学会発表やプレゼンテーション用の高品質な可視化

### 1.2 主要機能

- ✅ **リアルタイム可視化**: 50-100エージェントの位置、速度、Haze、疲労度
- ✅ **インタラクティブ制御**: パラメータ調整、再生/一時停止、ステップ実行
- ✅ **多次元データ表示**: φ(t)、χ(t)、EFE等の時系列プロット
- ✅ **個別エージェント分析**: 12×12極座標SPMマップ（270度視野）
- ✅ **録画機能**: スナップショット、動画出力、CSVエクスポート

### 1.3 技術要件

**Python側:**
- PyQt6 6.4+
- matplotlib 3.8+
- numpy 1.24+
- imageio 2.31+

**C++側:**
- 既存のEPH v2.1エンジン
- UDPソケットサーバー（新規実装）
- バイナリプロトコル（新規実装）

---

## 2. アーキテクチャ

### 2.1 3層アーキテクチャ

```
┌─────────────────────────────────────────┐
│ Layer 3: PyQt6 GUI                      │
│ - MainWindow (ドッキングパネル)         │
│ - 4パネル: Param/View/Metrics/Detail    │
│ - ツールバー、メニュー、ステータスバー   │
└───────────────┬─────────────────────────┘
                │ Signal/Slot
┌───────────────▼─────────────────────────┐
│ Layer 2: Python Bridge                  │
│ - UDPクライアント (非ブロッキング)      │
│ - データデシリアライゼーション           │
│ - リングバッファ（最新100フレーム）     │
│ - Watchdog（接続監視）                  │
└───────────────┬─────────────────────────┘
                │ UDP Socket
┌───────────────▼─────────────────────────┐
│ Layer 1: C++ Simulation Engine          │
│ - SwarmManager, PhaseAnalyzer           │
│ - UDPサーバー (2ポート)                 │
│   - Port 5555: 状態データ送信 (C++→Py) │
│   - Port 5556: 制御コマンド受信 (Py→C++)│
└─────────────────────────────────────────┘
```

### 2.2 データフロー

**状態データ送信（高頻度）:**
```
C++ Simulation
  └─> swarm.update()
      └─> if (step % SEND_INTERVAL == 0):
          └─> serialize_state_binary()
              └─> UDP 5555 → Python Bridge
                  └─> deserialize()
                      └─> Signal: data_received(dict)
                          └─> MainWindow.update_display()
```

**制御コマンド受信（低頻度）:**
```
GUI Parameter Change
  └─> user adjusts β slider
      └─> validate_parameter()
          └─> build_json_command()
              └─> UDP 5556 → C++ Server
                  └─> parse_json()
                      └─> swarm.set_beta(new_value)
```

### 2.3 通信プロトコル

#### ハイブリッド方式

**制御メッセージ（Python → C++）: JSON**
```json
{
  "type": "set_parameter",
  "parameter": "beta",
  "value": 0.105,
  "timestamp": 1234567890
}
```

**状態データ（C++ → Python）: バイナリ**
```
Header (24 bytes):
  [0-3]   magic_number   0xEFE20210
  [4-7]   sequence_num   uint32
  [8-11]  timestep       uint32
  [12-15] num_agents     uint32
  [16-19] data_length    uint32
  [20-23] checksum       uint32 (CRC32)

Payload (per agent, 32 bytes):
  [0-1]   agent_id       uint16
  [2-5]   x              float32
  [6-9]   y              float32
  [10-13] vx             float32
  [14-17] vy             float32
  [18-21] haze_mean      float32
  [22-25] fatigue        float32
  [26-29] efe            float32
  [30-31] padding        uint16

Metrics (48 bytes):
  [0-7]   phi            float64
  [8-15]  chi            float64
  [16-23] beta_current   float64
  [24-31] avg_haze       float64
  [32-39] avg_speed      float64
  [40-47] avg_fatigue    float64
```

---

## 3. エラーハンドリング & 通信

### 3.1 スレッドモデル

```python
MainWindow (QMainWindow) - メインスレッド
├── DataReceiverThread (QThread) - UDP受信専用
│   └── Signal: data_received(dict)
├── CommandSenderThread (QThread) - UDP送信専用
│   └── Slot: send_command(dict)
└── WatchdogTimer (QTimer) - 接続監視
    └── 3秒ごとにハートビートチェック
```

**スレッド安全性の保証:**
- QThreadからGUI更新は禁止 → Signal/Slotのみ
- リングバッファは`threading.Lock`で保護
- すべてのGUI更新はメインスレッドで実行

### 3.2 パケットロス検出

```python
class PacketValidator:
    def __init__(self):
        self.last_seq = -1
        self.lost_packets = 0

    def validate(self, packet):
        seq = packet['sequence_num']

        # シーケンス番号チェック
        if self.last_seq >= 0:
            expected = self.last_seq + 1
            if seq != expected:
                gap = seq - expected
                self.lost_packets += gap
                logging.warning(f"Packet loss: {gap} packets")

        self.last_seq = seq

        # チェックサム検証
        if not verify_checksum(packet):
            logging.error("Checksum mismatch")
            return False

        return True
```

### 3.3 エラーリカバリー戦略

**レベル1（軽微）: パケットロス 1-5%**
- 動作: ステータスバーに警告表示
- リカバリー: 不要（次フレームで更新）

**レベル2（中程度）: 接続タイムアウト 3秒**
- 動作: インフォバー「接続が不安定です」
- リカバリー: 自動再接続試行（3回まで）

**レベル3（重大）: C++プロセスクラッシュ**
- 動作: モーダルダイアログ
  ```
  ⚠️  シミュレーション接続エラー
  C++エンジンとの通信が切断されました。

  [再接続] [ログ表示] [終了]
  ```
- リカバリー:
  - 再接続試行
  - C++のstderr/stdout表示
  - オプション: プロセス再起動（要確認）

---

## 4. 初期UX & レイアウト

### 4.1 ウェルカムダイアログ

起動時に1回のみ表示（「次回から表示しない」チェック可能）:

```
┌─────────────────────────────────────────┐
│  EPH v2.1 - Emergent Phase Haze        │
│  ────────────────────────────────────   │
│                                         │
│  [🚀 クイックスタート]                   │
│   デフォルトパラメータで即座に開始       │
│   (N=50, β=0.098, 270° FOV)           │
│                                         │
│  [⚙️  カスタム設定]                      │
│   パラメータを調整してから開始           │
│                                         │
│  [📂 リプレイを読み込む]                 │
│   保存済みシミュレーションを再生         │
│                                         │
│  [✓] 次回から表示しない                 │
└─────────────────────────────────────────┘
```

### 4.2 レイアウトプリセット

**1. Research Mode（デフォルト）**
- 左: パラメータパネル（中サイズ）
- 中央: グローバルビュー（大）
- 右: メトリクスパネル（中サイズ）
- 下: エージェント詳細（折りたたみ）

**2. Presentation Mode**
- グローバルビュー最大化
- 他パネル非表示
- ステータスバーに主要メトリクス表示

**3. Analysis Mode**
- 下パネル展開（画面の50%）
- SPM詳細表示
- 速度x0.5に自動設定

**切り替え:**
- メニュー: View → Layout
- ショートカット: Ctrl+1/2/3

### 4.3 接続状態インジケーター

グローバルビュー左上:
```
● Connected  60 FPS    (緑: 正常)
● Reconnecting...      (黄: 再接続中)
● Disconnected         (赤: 切断)
```

---

## 5. パネル詳細設計

### 5.1 パラメータパネル（左ドック）

**折りたたみ式グループ:**

```
▼ Phase Transition
  β (MB Breaking)
  ├─────●────┤ 0.098
  Range: [0.00 - 0.30]
  Critical: ~0.098

  z (Avg Neighbors)
  ├──●──────┤ 6
  Range: [3 - 12]

▶ Active Inference
  (折りたたみ中)

▼ Agent Properties
  κ Sensitivity
  Leader  [───●──] 0.3
  Reserve [─────●] 1.5
  ☑ Enable Role Differentiation

▶ Simulation Settings
▼ Visualization
  FPS Target: 30
  Trail Length: 20 frames
  ☑ Show Neighbor Links
  ☑ Show Velocity Arrows

[Apply Changes]
[Reset to Default]
```

**パラメータ変更分類:**

1. **即座反映**: FPS、Trail、表示オプション → GUIのみ
2. **実行中変更可能**: β、η、τ、κ → UDP送信
3. **再起動必須**: N、視野角度 → 警告ダイアログ

### 5.2 グローバルビュー（中央）

**matplotlib FigureCanvas:**

- **エージェント表示**: 円 + 速度ベクトル矢印
  - 色: Haze値（青=低 → 黄=高）
  - サイズ: 疲労度（小=元気 → 大=疲労）
  - 矢印: 速度の向きと大きさ

- **選択エージェント**: 黄色太枠 + 近傍6エージェントに線

- **インタラクション**:
  - クリック: エージェント選択
  - ドラッグ: パン
  - ホイール: ズーム
  - 右クリック: コンテキストメニュー

### 5.3 メトリクスパネル（右ドック）

**タブ切り替え式:**

**Tab 1: Phase**
- φ(t) - 秩序パラメータ
- χ(t) - 応答関数
- β履歴

**Tab 2: Agent**
- 平均Haze
- 平均速度
- 平均疲労度

**Tab 3: AI**
- 平均EFE
- 予測誤差
- 統計情報

**機能:**
- 時間窓: 500-1000ステップ（設定可能）
- スライドウィンドウ
- ズーム/パン
- 右クリック → Export PNG/CSV

### 5.4 エージェント詳細パネル（下ドック）

**3カラムレイアウト:**

```
┌───────────────┬───────────────┬─────────────┐
│ SPM Heatmap   │ SPM Polar     │ Statistics  │
│ (12x12)       │ (円形)        │             │
│               │      0°       │ Position:   │
│   θ (deg)     │      ↑        │  x: 2.50    │
│ r ┌─────────┐ │   ╱─●─╲       │  y: 3.10    │
│12 │■□□□□■■■│ │  │  ●  │      │             │
│10 │■■■□□□■■│ │   ╲─●─╱       │ Velocity:   │
│ 8 │□■■■□□□■│ │     ↓         │  vx: 0.50   │
│   └─────────┘ │    180°       │  |v|: 0.58  │
│               │               │             │
│ Color: Low→High│ FOV: 270°    │ Haze: 0.450 │
│               │ (前方中心)    │ EFE:  1.234 │
│               │               │             │
│ ▶ Gradient    │ ▶ Show Mask   │ Neighbors:6 │
│   Max: θ=45°  │   (背後90°)   │ #12,#15,... │
└───────────────┴───────────────┴─────────────┘
```

**重要な実装:**
- **進行方向を上に固定**: エージェントの速度ベクトルから角度計算し、SPMを回転表示
- **270度視野マスク**: 極座標表示で後方90度（315°-45°）をグレーアウト

---

## 6. コントロール & メニュー

### 6.1 再生コントロールツールバー

```
[▶] [⏸] [⏹] [⏭] │ Speed: [x1 ▼] │ [📷] [🎬] │ Settings
```

| ボタン | ショートカット | 動作 |
|--------|---------------|------|
| ▶ Play | Space | 開始 |
| ⏸ Pause | Space | 一時停止 |
| ⏹ Stop | Ctrl+S | 停止＆リセット |
| ⏭ Step | → | 1ステップ実行 |
| 📷 Snapshot | S | PNG保存 |
| 🎬 Record | Ctrl+R | 録画開始/停止 |

### 6.2 メニューバー

```
File
├── New Simulation...
├── Load Replay...
├── Save Replay...
├── Export
│   ├── Snapshot (PNG)
│   ├── All Metrics (CSV)
│   └── Current State (JSON)
└── Quit

Edit
├── Copy Parameters
├── Paste Parameters
└── Reset All to Default

View
├── Layout (Research/Presentation/Analysis)
├── Show/Hide Panels
└── Full Screen

Simulation
├── Start/Pause/Stop
├── Step Forward
├── Speed
└── Restart

Tools
├── Phase Transition Scanner
├── Parameter Optimizer
└── View Logs

Help
├── Documentation
├── Keyboard Shortcuts
└── About EPH v2.1
```

### 6.3 ステータスバー

```
● Connected │ Step: 1250 │ φ: 0.034 │ β: 0.098 │ 30 FPS
```

### 6.4 ホットキー一覧

| キー | 動作 |
|------|------|
| Space | 再生/一時停止 |
| → | 1ステップ進む |
| S | スナップショット |
| Ctrl+R | 録画開始/停止 |
| Ctrl+1/2/3 | レイアウト切替 |
| F11 | フルスクリーン |
| Esc | 選択解除 |

---

## 7. 実装計画

### 7.1 ディレクトリ構造

```
crlEPH2/
├── gui/                         # 新規: PyQt6 GUI
│   ├── main.py
│   ├── requirements.txt
│   ├── bridge/                  # UDP通信層
│   │   ├── udp_client.py
│   │   ├── protocol.py
│   │   ├── data_buffer.py
│   │   └── watchdog.py
│   ├── widgets/                 # UIウィジェット
│   │   ├── main_window.py
│   │   ├── parameter_panel.py
│   │   ├── global_view.py
│   │   ├── metrics_panel.py
│   │   └── agent_detail_panel.py
│   ├── dialogs/
│   ├── utils/
│   └── resources/
│
└── cpp_server/                  # 新規: C++ UDP実装
    ├── CMakeLists.txt
    ├── main.cpp
    ├── udp_server.hpp/cpp
    └── protocol.hpp/cpp
```

### 7.2 実装フェーズ

**Phase 1: 基盤実装（1-2週間）**
- C++: UDPサーバー、バイナリプロトコル、270度視野対応
- Python: UDP通信層、基本MainWindow

**Phase 2: コアUI実装（2-3週間）**
- グローバルビュー、パラメータパネル
- 再生コントロール、ステータスバー

**Phase 3: 高度な機能（2-3週間）**
- メトリクスパネル、エージェント詳細パネル
- エラーハンドリング、レイアウトシステム

**Phase 4: 仕上げ（1-2週間）**
- 録画機能、ホットキー、ドキュメント
- テスト & デバッグ

### 7.3 技術スタック

**Python:**
- PyQt6 6.4+
- matplotlib 3.8+
- numpy 1.24+
- imageio 2.31+

**C++:**
- 既存のEPH v2.1エンジン
- 標準ライブラリソケットAPI

### 7.4 開発環境セットアップ

```bash
# Python
cd crlEPH2/gui
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

# C++
cd crlEPH2
cmake -B build -S . -DBUILD_GUI_SERVER=ON
cmake --build build --target eph_udp_server
```

### 7.5 起動方法

```bash
# ターミナル1: C++サーバー
./build/cpp_server/eph_udp_server

# ターミナル2: GUI
cd gui && python main.py

# または統合スクリプト
./scripts/launch_gui.sh
```

---

## 付録A: C++側の実装変更点

### A.1 視野角度パラメータ化

**constants.hpp:**
```cpp
// 新規追加
constexpr Scalar FIELD_OF_VIEW_DEGREES = 270.0;  // デフォルト270度
constexpr int N_THETA = 12;  // 既存

// θインデックスの解釈変更
// 従来: θ ∈ [0°, 360°)
// 新規: θ ∈ [-135°, +135°] (FOV=270の場合)
```

**saliency_polar_map.hpp:**
```cpp
// θインデックス→角度変換を修正
auto theta_to_angle(int theta_idx) const -> Scalar {
    const Scalar fov_rad = FIELD_OF_VIEW_DEGREES * M_PI / 180.0;
    const Scalar half_fov = fov_rad / 2.0;
    return -half_fov + (theta_idx * fov_rad / N_THETA);
}
```

### A.2 UDPサーバー実装例

**udp_server.hpp:**
```cpp
class UDPServer {
public:
    UDPServer(uint16_t send_port, uint16_t recv_port);

    // 状態データ送信
    void send_state(const SwarmManager& swarm,
                   const PhaseAnalyzer& analyzer);

    // 制御コマンド受信（非ブロッキング）
    auto receive_command() -> std::optional<nlohmann::json>;

private:
    int send_socket_;
    int recv_socket_;
    std::vector<uint8_t> send_buffer_;
};
```

---

## 付録B: Agent Team レビューコメント

### UI/UXスペシャリスト (Sarah Chen)

**総合評価: ⭐⭐⭐⭐⭐ (5/5) - 改善後**

✅ 初回UXの改善が完璧
✅ レイアウトプリセットにより柔軟性と使いやすさを両立
✅ 視覚的フィードバックが充実

### シニアPythonエンジニア (Marcus Rodriguez)

**総合評価: ⭐⭐⭐⭐⭐ (5/5) - 改善後**

✅ エラーハンドリングが堅牢
✅ スレッドモデルが明確
✅ パケットロス対策が適切
✅ 拡張性が高い設計

---

## 変更履歴

| 日付 | バージョン | 変更内容 |
|------|-----------|---------|
| 2026-02-10 | 1.0 | 初版作成（設計承認） |

---

**次のステップ**: 実装フェーズに移行
**関連ドキュメント**: README.md, phase_roadmap.md
