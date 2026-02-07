#!/usr/bin/env python3
"""
Agent Trajectory Visualization Tool (Publication Quality)

エージェントの軌跡データをCSVから読み込み、2Dスキャッタープロットを生成します。
Haze値によるカラーコーディング、複数タイムステップのスナップショット表示に対応。

Usage:
    python plot_agent_trajectories.py <csv_file> [--output_dir <dir>]
    python plot_agent_trajectories.py --demo [--output_dir <dir>]

CSV format:
    timestep,agent_id,x,y,vx,vy,haze_mean,fatigue
    0,0,1.2,3.4,0.5,0.3,0.45,0.1
    ...
"""

import argparse
import sys
import os
import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
import matplotlib.cm as cm
from pathlib import Path


# ─── Publication-quality style settings ──────────────────────────────────────
STYLE = {
    "font.family": "serif",
    "font.size": 11,
    "axes.labelsize": 13,
    "axes.titlesize": 14,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
    "legend.fontsize": 9,
    "figure.dpi": 150,
    "savefig.dpi": 300,
    "axes.linewidth": 1.0,
    "xtick.major.width": 0.8,
    "ytick.major.width": 0.8,
    "xtick.minor.width": 0.5,
    "ytick.minor.width": 0.5,
}
plt.rcParams.update(STYLE)

# Colormap for Haze
HAZE_CMAP = "viridis"


def generate_demo_data():
    """
    デモ用のエージェント軌跡データを生成

    Returns:
        data: dict with keys timestep, agent_id, x, y, vx, vy, haze_mean, fatigue
    """
    np.random.seed(123)
    n_agents = 20
    n_timesteps = 5
    timesteps_list = [0, 50, 100, 200, 500]

    rows = []
    # Initial positions: random in [0, 10] x [0, 10]
    positions = np.random.uniform(0, 10, (n_agents, 2))
    velocities = np.random.normal(0, 0.3, (n_agents, 2))
    haze_values = np.random.uniform(0.1, 0.9, n_agents)
    fatigue_values = np.random.uniform(0.0, 0.3, n_agents)

    for t_idx, t in enumerate(timesteps_list):
        for a in range(n_agents):
            rows.append({
                "timestep": t,
                "agent_id": a,
                "x": positions[a, 0],
                "y": positions[a, 1],
                "vx": velocities[a, 0],
                "vy": velocities[a, 1],
                "haze_mean": haze_values[a],
                "fatigue": fatigue_values[a],
            })

        # Evolve: agents cluster over time (simulate cooperation emergence)
        center = np.mean(positions, axis=0)
        attraction = 0.05 * (center - positions)
        noise = np.random.normal(0, 0.15, (n_agents, 2))
        positions = positions + velocities * 0.5 + attraction + noise
        velocities = velocities * 0.95 + attraction * 0.3 + np.random.normal(0, 0.05, (n_agents, 2))

        # Haze increases over time, fatigue accumulates
        haze_values = np.clip(haze_values + np.random.normal(0.02, 0.03, n_agents), 0.0, 1.0)
        fatigue_values = np.clip(fatigue_values + np.random.uniform(0.01, 0.05, n_agents), 0.0, 1.0)

    # Build structured arrays
    n_rows = len(rows)
    data = {
        "timestep": np.array([r["timestep"] for r in rows], dtype=int),
        "agent_id": np.array([r["agent_id"] for r in rows], dtype=int),
        "x": np.array([r["x"] for r in rows]),
        "y": np.array([r["y"] for r in rows]),
        "vx": np.array([r["vx"] for r in rows]),
        "vy": np.array([r["vy"] for r in rows]),
        "haze_mean": np.array([r["haze_mean"] for r in rows]),
        "fatigue": np.array([r["fatigue"] for r in rows]),
    }
    return data


def load_csv(csv_file):
    """
    CSVファイルからエージェント軌跡データを読み込む

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

    required_cols = ["timestep", "agent_id", "x", "y", "vx", "vy", "haze_mean", "fatigue"]
    for col in required_cols:
        if col not in raw.dtype.names:
            print(f"Error: Missing column '{col}' in CSV file.", file=sys.stderr)
            sys.exit(1)

    data = {}
    for col in required_cols:
        data[col] = raw[col]
    return data


def plot_agent_trajectories(data, output_dir=None, show=False):
    """
    エージェント軌跡をスナップショットパネルで可視化

    Args:
        data: dict with column arrays
        output_dir: 出力ディレクトリ
        show: 画面に表示するか
    """
    timesteps = np.unique(data["timestep"].astype(int))
    n_panels = len(timesteps)

    # Choose layout: up to 6 panels in 2 rows, beyond that 3 columns
    if n_panels <= 3:
        n_cols = n_panels
        n_rows = 1
    elif n_panels <= 6:
        n_cols = 3
        n_rows = 2
    else:
        n_cols = 3
        n_rows = int(np.ceil(n_panels / 3))

    fig_width = 4.5 * n_cols
    fig_height = 4.0 * n_rows + 0.8  # extra space for colorbar + title

    fig, axes = plt.subplots(
        n_rows, n_cols, figsize=(fig_width, fig_height),
        squeeze=False,
    )

    # Global x/y limits
    x_all = data["x"]
    y_all = data["y"]
    pad = 0.5
    x_min, x_max = np.min(x_all) - pad, np.max(x_all) + pad
    y_min, y_max = np.min(y_all) - pad, np.max(y_all) + pad

    # Normalize colormap
    norm = plt.Normalize(vmin=0.0, vmax=1.0)
    cmap = matplotlib.colormaps[HAZE_CMAP]

    for idx, t in enumerate(timesteps):
        row_idx = idx // n_cols
        col_idx = idx % n_cols
        ax = axes[row_idx][col_idx]

        mask = data["timestep"].astype(int) == t
        x = data["x"][mask]
        y = data["y"][mask]
        vx = data["vx"][mask]
        vy = data["vy"][mask]
        haze = data["haze_mean"][mask]
        fatigue = data["fatigue"][mask]

        # Scatter: position colored by Haze
        sc = ax.scatter(
            x, y,
            c=haze, cmap=HAZE_CMAP, norm=norm,
            s=50 + fatigue * 80,  # size reflects fatigue
            edgecolors="black", linewidths=0.4,
            alpha=0.85, zorder=3,
        )

        # Velocity arrows
        ax.quiver(
            x, y, vx, vy,
            angles="xy", scale_units="xy", scale=2.0,
            color="gray", alpha=0.5, width=0.005, zorder=2,
        )

        ax.set_xlim(x_min, x_max)
        ax.set_ylim(y_min, y_max)
        ax.set_aspect("equal", adjustable="box")
        ax.set_title(f"t = {t}", fontsize=12, fontweight="bold")
        ax.grid(True, alpha=0.2, linewidth=0.5)
        ax.xaxis.set_minor_locator(AutoMinorLocator())
        ax.yaxis.set_minor_locator(AutoMinorLocator())

        if col_idx == 0:
            ax.set_ylabel("y")
        if row_idx == n_rows - 1:
            ax.set_xlabel("x")

    # Hide unused axes
    for idx in range(n_panels, n_rows * n_cols):
        row_idx = idx // n_cols
        col_idx = idx % n_cols
        axes[row_idx][col_idx].set_visible(False)

    # Colorbar
    cbar = fig.colorbar(
        cm.ScalarMappable(norm=norm, cmap=cmap),
        ax=axes.ravel().tolist(),
        orientation="vertical",
        fraction=0.02, pad=0.02,
        label="Haze Mean",
    )

    fig.suptitle(
        "EPH Agent Trajectories (color = Haze, size = fatigue)",
        fontsize=14, fontweight="bold", y=1.01,
    )

    plt.tight_layout()

    # ─── Save ────────────────────────────────────────────────────────────
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        for ext in ["png", "pdf"]:
            outpath = os.path.join(output_dir, f"agent_trajectories.{ext}")
            fig.savefig(outpath, dpi=300, bbox_inches="tight")
            print(f"Saved: {outpath}")

    if show:
        plt.show()

    plt.close(fig)

    # ─── Summary ─────────────────────────────────────────────────────────
    n_agents = len(np.unique(data["agent_id"]))
    print(f"\n=== Agent Trajectory Summary ===")
    print(f"  Timesteps:  {len(timesteps)} ({timesteps.tolist()})")
    print(f"  Agents:     {n_agents}")
    print(f"  Total rows: {len(data['x'])}")
    print(f"  x range:    [{np.min(x_all):.3f}, {np.max(x_all):.3f}]")
    print(f"  y range:    [{np.min(y_all):.3f}, {np.max(y_all):.3f}]")
    print(f"  Haze range: [{np.min(data['haze_mean']):.3f}, {np.max(data['haze_mean']):.3f}]")


def main():
    parser = argparse.ArgumentParser(
        description="Visualize EPH agent trajectory data (publication quality)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "csv_file", type=str, nargs="?", default=None,
        help="Input CSV file (timestep,agent_id,x,y,vx,vy,haze_mean,fatigue format)",
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
        print("Generating demo agent trajectory data...")
        data = generate_demo_data()
        out = args.output_dir or "demo_output"
        plot_agent_trajectories(data, output_dir=out)
        return

    if args.csv_file is None:
        parser.error("csv_file is required unless --demo is specified")

    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: CSV file not found: {args.csv_file}", file=sys.stderr)
        sys.exit(1)

    data = load_csv(args.csv_file)
    plot_agent_trajectories(data, output_dir=args.output_dir)


if __name__ == "__main__":
    main()
