#!/usr/bin/env python3
"""
Phase Transition Visualization Tool (Publication Quality)

CSVデータから相転移データ（φ(β), χ(β)）を可視化します。
臨界点β_cを自動検出してマーキングします。

Usage:
    python plot_phase_transition.py <csv_file> [--output_dir <dir>]
    python plot_phase_transition.py --demo [--output_dir <dir>]

Example:
    python plot_phase_transition.py phase_data.csv
    python plot_phase_transition.py phase_data.csv --output_dir results/
    python plot_phase_transition.py --demo --output_dir demo_output/
"""

import argparse
import sys
import os
import numpy as np

# Use non-interactive backend when saving to file
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
from pathlib import Path


# ─── Publication-quality style settings ──────────────────────────────────────
STYLE = {
    "font.family": "serif",
    "font.size": 11,
    "axes.labelsize": 13,
    "axes.titlesize": 14,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
    "legend.fontsize": 10,
    "figure.dpi": 150,
    "savefig.dpi": 300,
    "axes.linewidth": 1.0,
    "xtick.major.width": 0.8,
    "ytick.major.width": 0.8,
    "xtick.minor.width": 0.5,
    "ytick.minor.width": 0.5,
    "lines.linewidth": 1.8,
    "lines.markersize": 5,
}
plt.rcParams.update(STYLE)

# Color palette (colorblind-friendly)
COLOR_PHI = "#2E86AB"
COLOR_CHI = "#A23B72"
COLOR_BC_EMPIRICAL = "#E63946"
COLOR_BC_THEORY = "#F4A261"

# Theoretical critical point (EPH v2.1)
BETA_C_THEORY = 0.098


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
        return betas[len(betas) // 2]

    # 中心差分で微分計算
    derivatives = []
    for i in range(1, len(betas) - 1):
        dphi = phis[i + 1] - phis[i - 1]
        dbeta = betas[i + 1] - betas[i - 1]
        if dbeta > 1e-12:
            derivatives.append(dphi / dbeta)
        else:
            derivatives.append(0.0)

    if derivatives:
        max_idx = np.argmax(derivatives)
        return betas[max_idx + 1]
    else:
        return betas[len(betas) // 2]


def generate_demo_data():
    """
    デモ用の相転移データを生成

    Returns:
        betas, phis, chis: numpy arrays
    """
    np.random.seed(42)
    betas = np.linspace(0.0, 0.3, 60)

    # Sigmoid-like order parameter with noise
    phis = 1.0 / (1.0 + np.exp(-80.0 * (betas - BETA_C_THEORY)))
    phis += np.random.normal(0, 0.015, len(betas))
    phis = np.clip(phis, 0.0, 1.0)

    # Susceptibility: peaked at β_c
    chis = 4.0 * np.exp(-((betas - BETA_C_THEORY) ** 2) / (2.0 * 0.02**2))
    chis += np.random.normal(0, 0.08, len(betas))
    chis = np.clip(chis, 0.0, None)

    return betas, phis, chis


def load_csv(csv_file):
    """
    CSVファイルからデータを読み込む

    Args:
        csv_file: CSVファイルパス

    Returns:
        betas, phis, chis: numpy arrays
    """
    try:
        data = np.genfromtxt(csv_file, delimiter=",", names=True)
    except Exception as e:
        print(f"Error: Could not read CSV file '{csv_file}': {e}", file=sys.stderr)
        sys.exit(1)

    return data["beta"], data["phi"], data["chi"]


def plot_phase_transition(betas, phis, chis, output_dir=None, show=False):
    """
    相転移データをpublication-quality plotで可視化

    Args:
        betas: β値の配列
        phis: φ値の配列
        chis: χ値の配列
        output_dir: 出力ディレクトリ（Noneの場合は表示のみ）
        show: 画面に表示するか
    """
    beta_c = find_beta_c(betas, phis)

    # ─── Figure: Subplot layout ──────────────────────────────────────────
    fig, (ax1, ax2) = plt.subplots(
        2, 1, figsize=(7.0, 8.0), sharex=True,
        gridspec_kw={"hspace": 0.08, "height_ratios": [1, 1]}
    )

    # ─── Upper panel: Order parameter φ(β) ──────────────────────────────
    ax1.plot(
        betas, phis, "o-",
        color=COLOR_PHI, markeredgecolor="white", markeredgewidth=0.5,
        label=r"$\varphi(\beta)$", zorder=3,
    )

    # Critical point lines
    ax1.axvline(
        beta_c, color=COLOR_BC_EMPIRICAL, linestyle="--", linewidth=1.5,
        label=rf"$\beta_c$ (empirical) = {beta_c:.4f}", zorder=2,
    )
    ax1.axvline(
        BETA_C_THEORY, color=COLOR_BC_THEORY, linestyle=":", linewidth=1.5,
        label=rf"$\beta_c$ (theory) = {BETA_C_THEORY:.3f}", zorder=2,
    )

    # Shaded region around β_c
    ax1.axvspan(
        BETA_C_THEORY * 0.9, BETA_C_THEORY * 1.1,
        alpha=0.08, color=COLOR_BC_THEORY, zorder=1,
    )

    ax1.set_ylabel(r"Order Parameter $\varphi$")
    ax1.set_title("EPH Phase Transition Analysis", fontweight="bold", fontsize=14)
    ax1.legend(loc="upper left", framealpha=0.9, edgecolor="gray")
    ax1.grid(True, alpha=0.25, linestyle="-", linewidth=0.5)
    ax1.xaxis.set_minor_locator(AutoMinorLocator())
    ax1.yaxis.set_minor_locator(AutoMinorLocator())
    ax1.set_ylim(bottom=-0.02, top=max(phis) * 1.12)
    ax1.text(
        0.97, 0.05,
        "(a)",
        transform=ax1.transAxes, fontsize=13, fontweight="bold",
        ha="right", va="bottom",
    )

    # ─── Lower panel: Susceptibility χ(β) ───────────────────────────────
    ax2.plot(
        betas, chis, "s-",
        color=COLOR_CHI, markeredgecolor="white", markeredgewidth=0.5,
        label=r"$\chi(\beta)$", zorder=3,
    )

    ax2.axvline(
        beta_c, color=COLOR_BC_EMPIRICAL, linestyle="--", linewidth=1.5,
        label=rf"$\beta_c$ (empirical) = {beta_c:.4f}", zorder=2,
    )
    ax2.axvline(
        BETA_C_THEORY, color=COLOR_BC_THEORY, linestyle=":", linewidth=1.5,
        label=rf"$\beta_c$ (theory) = {BETA_C_THEORY:.3f}", zorder=2,
    )

    ax2.axvspan(
        BETA_C_THEORY * 0.9, BETA_C_THEORY * 1.1,
        alpha=0.08, color=COLOR_BC_THEORY, zorder=1,
    )

    # Mark χ_max
    max_chi_idx = np.argmax(chis)
    beta_at_max_chi = betas[max_chi_idx]
    chi_max = chis[max_chi_idx]
    ax2.plot(
        beta_at_max_chi, chi_max, "*",
        color=COLOR_BC_EMPIRICAL, markersize=14, markeredgecolor="black",
        markeredgewidth=0.5, zorder=4,
        label=rf"$\chi_{{max}}$ = {chi_max:.3f} at $\beta$ = {beta_at_max_chi:.4f}",
    )

    ax2.set_xlabel(r"MB Breaking Strength $\beta$")
    ax2.set_ylabel(r"Susceptibility $\chi$")
    ax2.legend(loc="upper right", framealpha=0.9, edgecolor="gray")
    ax2.grid(True, alpha=0.25, linestyle="-", linewidth=0.5)
    ax2.xaxis.set_minor_locator(AutoMinorLocator())
    ax2.yaxis.set_minor_locator(AutoMinorLocator())
    ax2.set_ylim(bottom=-0.05, top=chi_max * 1.2)
    ax2.text(
        0.97, 0.05,
        "(b)",
        transform=ax2.transAxes, fontsize=13, fontweight="bold",
        ha="right", va="bottom",
    )

    plt.tight_layout()

    # ─── Save ────────────────────────────────────────────────────────────
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        for ext in ["png", "pdf"]:
            outpath = os.path.join(output_dir, f"phase_transition.{ext}")
            fig.savefig(outpath, dpi=300, bbox_inches="tight")
            print(f"Saved: {outpath}")

    if show:
        plt.show()

    plt.close(fig)

    # ─── Summary statistics ──────────────────────────────────────────────
    print("\n=== Phase Transition Analysis ===")
    print(f"  beta_c (theory):    {BETA_C_THEORY:.4f}")
    print(f"  beta_c (empirical): {beta_c:.4f}")
    deviation = abs(beta_c - BETA_C_THEORY)
    print(f"  Deviation:          {deviation:.4f} ({deviation / BETA_C_THEORY * 100:.1f}%)")
    print(f"  chi_max:            {chi_max:.4f} at beta = {beta_at_max_chi:.4f}")

    tolerance = BETA_C_THEORY * 0.1
    if deviation <= tolerance:
        print(f"\n  V2 Validation: SUCCESS (within +/-10% tolerance)")
    else:
        print(f"\n  V2 Validation: FAILED (exceeds +/-10% tolerance)")

    print(f"\n  Data points: {len(betas)}")
    print(f"  beta range:  [{min(betas):.4f}, {max(betas):.4f}]")
    print(f"  phi range:   [{min(phis):.4f}, {max(phis):.4f}]")
    print(f"  chi range:   [{min(chis):.4f}, {max(chis):.4f}]")


def main():
    parser = argparse.ArgumentParser(
        description="Visualize EPH phase transition data (publication quality)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "csv_file", type=str, nargs="?", default=None,
        help="Input CSV file (beta,phi,chi format)",
    )
    parser.add_argument(
        "--output_dir", "-o", type=str, default=None,
        help="Output directory for PNG/PDF. If not specified, prints to stdout info only.",
    )
    parser.add_argument(
        "--demo", action="store_true",
        help="Generate demo data and plot (no CSV file required).",
    )

    args = parser.parse_args()

    if args.demo:
        print("Generating demo phase transition data...")
        betas, phis, chis = generate_demo_data()
        out = args.output_dir or "demo_output"
        plot_phase_transition(betas, phis, chis, output_dir=out)
        return

    if args.csv_file is None:
        parser.error("csv_file is required unless --demo is specified")

    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: CSV file not found: {args.csv_file}", file=sys.stderr)
        sys.exit(1)

    betas, phis, chis = load_csv(args.csv_file)
    plot_phase_transition(betas, phis, chis, output_dir=args.output_dir)


if __name__ == "__main__":
    main()
