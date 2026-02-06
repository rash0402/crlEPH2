#!/usr/bin/env python3
"""
Phase Transition Visualization Tool

CSVデータから相転移データ（φ(β), χ(β)）を可視化します。
臨界点β_cを自動検出してマーキングします。

Usage:
    python plot_phase_transition.py <csv_file> [--output <output_file>]

Example:
    python plot_phase_transition.py phase_data.csv
    python plot_phase_transition.py phase_data.csv --output phase_plot.png
"""

import argparse
import sys
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

def find_beta_c(betas, phis):
    """
    臨界点β_cを検出（dφ/dβの最大値）

    Args:
        betas: β値の配列
        phis: φ値の配列

    Returns:
        β_c: 臨界点
    """
    if len(betas) < 3:
        return betas[len(betas) // 2]  # デフォルト: 中央値

    # 中心差分で微分計算
    derivatives = []
    for i in range(1, len(betas) - 1):
        dphi = phis[i + 1] - phis[i - 1]
        dbeta = betas[i + 1] - betas[i - 1]
        if dbeta > 1e-12:
            derivatives.append(dphi / dbeta)
        else:
            derivatives.append(0.0)

    # 最大勾配点を探索
    if derivatives:
        max_idx = np.argmax(derivatives)
        return betas[max_idx + 1]  # index調整
    else:
        return betas[len(betas) // 2]

def plot_phase_transition(csv_file, output_file=None):
    """
    相転移データを可視化

    Args:
        csv_file: CSVファイルパス
        output_file: 出力画像ファイル（Noneの場合は画面表示）
    """
    # CSVデータ読み込み
    try:
        data = np.genfromtxt(csv_file, delimiter=',', names=True)
    except Exception as e:
        print(f"Error: Could not read CSV file '{csv_file}': {e}", file=sys.stderr)
        sys.exit(1)

    betas = data['beta']
    phis = data['phi']
    chis = data['chi']

    # β_c検出
    beta_c = find_beta_c(betas, phis)

    # 理論値（EPH v2.1）
    beta_c_theory = 0.098

    # プロット設定
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
    fig.suptitle('EPH Phase Transition Analysis', fontsize=16, fontweight='bold')

    # === 上段: φ(β)プロット ===
    ax1.plot(betas, phis, 'o-', linewidth=2, markersize=6, color='#2E86AB', label='φ(β)')
    ax1.axvline(beta_c, color='red', linestyle='--', linewidth=2, label=f'β_c (empirical) = {beta_c:.3f}')
    ax1.axvline(beta_c_theory, color='orange', linestyle=':', linewidth=2, label=f'β_c (theory) = {beta_c_theory:.3f}')
    ax1.set_ylabel('Order Parameter φ', fontsize=12, fontweight='bold')
    ax1.set_title('Order Parameter vs MB Breaking Strength', fontsize=14)
    ax1.legend(loc='upper left', fontsize=10)
    ax1.grid(True, alpha=0.3)
    ax1.set_ylim([0, max(phis) * 1.1])

    # === 下段: χ(β)プロット ===
    ax2.plot(betas, chis, 's-', linewidth=2, markersize=6, color='#A23B72', label='χ(β)')
    ax2.axvline(beta_c, color='red', linestyle='--', linewidth=2, label=f'β_c (empirical) = {beta_c:.3f}')
    ax2.axvline(beta_c_theory, color='orange', linestyle=':', linewidth=2, label=f'β_c (theory) = {beta_c_theory:.3f}')
    ax2.set_xlabel('MB Breaking Strength β', fontsize=12, fontweight='bold')
    ax2.set_ylabel('Susceptibility χ', fontsize=12, fontweight='bold')
    ax2.set_title('Susceptibility vs MB Breaking Strength', fontsize=14)
    ax2.legend(loc='upper right', fontsize=10)
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim([0, max(chis) * 1.1])

    # χの最大値をマーク
    max_chi_idx = np.argmax(chis)
    beta_at_max_chi = betas[max_chi_idx]
    chi_max = chis[max_chi_idx]
    ax2.plot(beta_at_max_chi, chi_max, 'r*', markersize=20, label=f'χ_max at β={beta_at_max_chi:.3f}')
    ax2.legend(loc='upper right', fontsize=10)

    plt.tight_layout()

    # 保存または表示
    if output_file:
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Plot saved: {output_file}")
    else:
        plt.show()

    # 統計情報出力
    print("\n=== Phase Transition Analysis ===")
    print(f"β_c (theory):    {beta_c_theory:.3f}")
    print(f"β_c (empirical): {beta_c:.3f}")
    print(f"Deviation:       {abs(beta_c - beta_c_theory):.3f} ({abs(beta_c - beta_c_theory) / beta_c_theory * 100:.1f}%)")
    print(f"χ_max:           {chi_max:.3f} at β={beta_at_max_chi:.3f}")

    # V2検証成功判定
    tolerance = beta_c_theory * 0.1  # ±10%
    if abs(beta_c - beta_c_theory) <= tolerance:
        print(f"\n✅ V2 Validation: SUCCESS (within ±10% tolerance)")
    else:
        print(f"\n❌ V2 Validation: FAILED (exceeds ±10% tolerance)")

    print(f"\nData points: {len(betas)}")
    print(f"β range: [{min(betas):.3f}, {max(betas):.3f}]")
    print(f"φ range: [{min(phis):.3f}, {max(phis):.3f}]")
    print(f"χ range: [{min(chis):.3f}, {max(chis):.3f}]")

def main():
    parser = argparse.ArgumentParser(
        description='Visualize EPH phase transition data from CSV',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('csv_file', type=str, help='Input CSV file (beta,phi,chi format)')
    parser.add_argument('--output', '-o', type=str, default=None,
                        help='Output image file (PNG/PDF). If not specified, displays interactively.')

    args = parser.parse_args()

    # ファイル存在確認
    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: CSV file not found: {args.csv_file}", file=sys.stderr)
        sys.exit(1)

    # プロット実行
    plot_phase_transition(args.csv_file, args.output)

if __name__ == '__main__':
    main()
