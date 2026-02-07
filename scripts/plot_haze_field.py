#!/usr/bin/env python3
"""
Haze Field Visualization Tool (Publication Quality)

Hazeフィールドデータ（12x12グリッド）をCSVから読み込み、
ヒートマップとして可視化します。複数β値/タイムステップのパネル表示に対応。

Usage:
    python plot_haze_field.py <csv_file> [--output_dir <dir>]
    python plot_haze_field.py --demo [--output_dir <dir>]

CSV format:
    beta,theta_idx,r_idx,haze_value
    0.00,0,0,0.123
    0.00,0,1,0.234
    ...
"""

import argparse
import sys
import os
import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, FixedLocator, FixedFormatter
import matplotlib.cm as cm
from pathlib import Path


# ─── Publication-quality style settings ──────────────────────────────────────
STYLE = {
    "font.family": "serif",
    "font.size": 11,
    "axes.labelsize": 13,
    "axes.titlesize": 13,
    "xtick.labelsize": 9,
    "ytick.labelsize": 9,
    "legend.fontsize": 9,
    "figure.dpi": 150,
    "savefig.dpi": 300,
    "axes.linewidth": 1.0,
}
plt.rcParams.update(STYLE)

# Grid dimensions (EPH default)
N_THETA = 12
N_R = 12

# Haze colormap
HAZE_CMAP = "inferno"


def generate_demo_data():
    """
    デモ用のHazeフィールドデータを生成

    Returns:
        data: dict with keys beta, theta_idx, r_idx, haze_value
    """
    np.random.seed(77)

    beta_values = [0.00, 0.05, 0.098, 0.15, 0.20, 0.30]

    rows_beta = []
    rows_theta = []
    rows_r = []
    rows_haze = []

    for beta in beta_values:
        for ti in range(N_THETA):
            for ri in range(N_R):
                # Haze pattern: depends on beta and position
                # Below critical: uniform low haze
                # Near critical: structured pattern emerges
                # Above critical: strong structured field
                theta_norm = ti / N_THETA  # [0, 1)
                r_norm = ri / N_R          # [0, 1)

                base = 0.1 + 0.3 * np.tanh(5.0 * (beta - 0.098))
                # Radial gradient
                radial = np.exp(-2.0 * (r_norm - 0.5) ** 2)
                # Angular modulation (2-fold symmetry)
                angular = 0.5 + 0.5 * np.cos(2 * np.pi * 2 * theta_norm)
                # Combine
                haze = base + 0.4 * radial * angular * min(beta / 0.098, 1.5)
                haze += np.random.normal(0, 0.03)
                haze = np.clip(haze, 0.0, 1.0)

                rows_beta.append(beta)
                rows_theta.append(ti)
                rows_r.append(ri)
                rows_haze.append(haze)

    data = {
        "beta": np.array(rows_beta),
        "theta_idx": np.array(rows_theta, dtype=int),
        "r_idx": np.array(rows_r, dtype=int),
        "haze_value": np.array(rows_haze),
    }
    return data


def load_csv(csv_file):
    """
    CSVファイルからHazeフィールドデータを読み込む

    Args:
        csv_file: CSVファイルパス

    Returns:
        data: dict with column arrays
    """
    try:
        raw = np.genfromtxt(csv_file, delimiter=",", names=True)
    except Exception as e:
        print(f"Error: Could not read CSV file '{csv_file}': {e}", file=sys.stderr)
        sys.exit(1)

    required_cols = ["beta", "theta_idx", "r_idx", "haze_value"]
    for col in required_cols:
        if col not in raw.dtype.names:
            print(f"Error: Missing column '{col}' in CSV file.", file=sys.stderr)
            sys.exit(1)

    data = {}
    for col in required_cols:
        data[col] = raw[col]
    return data


def plot_haze_field(data, output_dir=None, show=False):
    """
    Hazeフィールドをヒートマップパネルで可視化

    Args:
        data: dict with column arrays
        output_dir: 出力ディレクトリ
        show: 画面に表示するか
    """
    # Identify unique beta values
    betas_unique = np.unique(data["beta"])
    # Round to avoid floating point issues
    betas_unique = np.round(betas_unique, 6)
    n_panels = len(betas_unique)

    # Determine grid size from data
    n_theta = int(np.max(data["theta_idx"])) + 1
    n_r = int(np.max(data["r_idx"])) + 1

    # Layout
    if n_panels <= 3:
        n_cols = n_panels
        n_rows_layout = 1
    elif n_panels <= 6:
        n_cols = 3
        n_rows_layout = 2
    elif n_panels <= 9:
        n_cols = 3
        n_rows_layout = 3
    else:
        n_cols = 4
        n_rows_layout = int(np.ceil(n_panels / 4))

    fig_width = 3.8 * n_cols + 1.0  # extra for colorbar
    fig_height = 3.5 * n_rows_layout + 0.8

    fig, axes = plt.subplots(
        n_rows_layout, n_cols, figsize=(fig_width, fig_height),
        squeeze=False,
    )

    # Theta labels (angles in degrees)
    theta_labels = [f"{int(i * 360 / n_theta)}" for i in range(n_theta)]
    r_labels = [f"{i}" for i in range(n_r)]

    norm = plt.Normalize(vmin=0.0, vmax=1.0)
    cmap = matplotlib.colormaps[HAZE_CMAP]

    im = None  # will hold the last AxesImage for colorbar

    for idx, beta_val in enumerate(betas_unique):
        row_idx = idx // n_cols
        col_idx = idx % n_cols
        ax = axes[row_idx][col_idx]

        # Extract data for this beta
        beta_rounded = np.round(data["beta"], 6)
        mask = beta_rounded == beta_val
        theta_idx = data["theta_idx"][mask].astype(int)
        r_idx = data["r_idx"][mask].astype(int)
        haze = data["haze_value"][mask]

        # Build 2D grid (theta x r)
        grid = np.full((n_r, n_theta), np.nan)
        for ti, ri, h in zip(theta_idx, r_idx, haze):
            if ti < n_theta and ri < n_r:
                grid[ri, ti] = h

        # Plot heatmap
        im = ax.imshow(
            grid, cmap=HAZE_CMAP, norm=norm,
            aspect="auto", origin="lower",
            interpolation="nearest",
        )

        # Axis labels
        # X-axis: theta (angle in degrees)
        theta_tick_positions = np.arange(0, n_theta, max(1, n_theta // 6))
        theta_tick_labels_subset = [f"{int(i * 360 / n_theta)}" for i in theta_tick_positions]
        ax.set_xticks(theta_tick_positions)
        ax.set_xticklabels(theta_tick_labels_subset)

        # Y-axis: r (radial index)
        r_tick_positions = np.arange(0, n_r, max(1, n_r // 6))
        ax.set_yticks(r_tick_positions)
        ax.set_yticklabels([str(int(i)) for i in r_tick_positions])

        # Highlight critical beta
        is_critical = abs(beta_val - 0.098) < 0.005
        title_color = "#E63946" if is_critical else "black"
        title_suffix = " *" if is_critical else ""
        ax.set_title(
            rf"$\beta$ = {beta_val:.3f}{title_suffix}",
            fontweight="bold" if is_critical else "normal",
            color=title_color,
        )

        if col_idx == 0:
            ax.set_ylabel(r"Radial index $r$")
        if row_idx == n_rows_layout - 1:
            ax.set_xlabel(r"Angle $\theta$ (deg)")

    # Hide unused axes
    for idx in range(n_panels, n_rows_layout * n_cols):
        row_idx = idx // n_cols
        col_idx = idx % n_cols
        axes[row_idx][col_idx].set_visible(False)

    # Colorbar
    cbar = fig.colorbar(
        cm.ScalarMappable(norm=norm, cmap=cmap),
        ax=axes.ravel().tolist(),
        orientation="vertical",
        fraction=0.02, pad=0.03,
        label="Haze Intensity",
    )

    fig.suptitle(
        r"EPH Haze Field $H(\theta, r)$ across $\beta$ values",
        fontsize=14, fontweight="bold", y=1.01,
    )

    plt.tight_layout()

    # ─── Save ────────────────────────────────────────────────────────────
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        for ext in ["png", "pdf"]:
            outpath = os.path.join(output_dir, f"haze_field.{ext}")
            fig.savefig(outpath, dpi=300, bbox_inches="tight")
            print(f"Saved: {outpath}")

    if show:
        plt.show()

    plt.close(fig)

    # ─── Summary ─────────────────────────────────────────────────────────
    print(f"\n=== Haze Field Summary ===")
    print(f"  Grid size:    {n_theta} (theta) x {n_r} (r)")
    print(f"  Beta values:  {len(betas_unique)} ({[f'{b:.3f}' for b in betas_unique]})")
    print(f"  Total cells:  {len(data['haze_value'])}")
    print(f"  Haze range:   [{np.min(data['haze_value']):.4f}, {np.max(data['haze_value']):.4f}]")
    print(f"  Haze mean:    {np.mean(data['haze_value']):.4f}")

    # Per-beta statistics
    for beta_val in betas_unique:
        mask = np.round(data["beta"], 6) == beta_val
        h = data["haze_value"][mask]
        marker = " <-- beta_c" if abs(beta_val - 0.098) < 0.005 else ""
        print(f"    beta={beta_val:.3f}: mean={np.mean(h):.4f}, std={np.std(h):.4f}{marker}")


def main():
    parser = argparse.ArgumentParser(
        description="Visualize EPH Haze field data (publication quality)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "csv_file", type=str, nargs="?", default=None,
        help="Input CSV file (beta,theta_idx,r_idx,haze_value format)",
    )
    parser.add_argument(
        "--output_dir", "-o", type=str, default=None,
        help="Output directory for PNG/PDF.",
    )
    parser.add_argument(
        "--demo", action="store_true",
        help="Generate demo data and plot (no CSV file required).",
    )

    args = parser.parse_args()

    if args.demo:
        print("Generating demo Haze field data...")
        data = generate_demo_data()
        out = args.output_dir or "demo_output"
        plot_haze_field(data, output_dir=out)
        return

    if args.csv_file is None:
        parser.error("csv_file is required unless --demo is specified")

    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: CSV file not found: {args.csv_file}", file=sys.stderr)
        sys.exit(1)

    data = load_csv(args.csv_file)
    plot_haze_field(data, output_dir=args.output_dir)


if __name__ == "__main__":
    main()
