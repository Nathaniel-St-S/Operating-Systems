#!/usr/bin/env python3
"""
Generate bar-chart visualizations (as PNG files) for the CSV metrics in this directory.

Outputs PNG images to ./charts/.
"""

from __future__ import annotations

import csv
from pathlib import Path
from typing import Iterable, List

import matplotlib

matplotlib.use("Agg")  # non-GUI backend
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent
OUTPUT_DIR = ROOT / "charts"

# ----------------------------------------------------------
# NORD THEME SETTINGS
# ----------------------------------------------------------

# If True → fully transparent background (text + grid = white)
# If False → use a Nord dark background color.
TRANSPARENT_BG = True

NORD_BG = "#2E3440"      # Polar Night
NORD_FG = "#ECEFF4"      # Snow Storm (white-ish)
GRID_COLOR = "#4C566A"   # Dim grey-blue

# Nord-inspired bar colors
CATEGORY_COLORS = {
    "time": "#81A1C1",
    "cpu": "#BF616A",
    "throughput": "#A3BE8C",
    "context": "#D08770",
    "cache_hits": "#88C0D0",
    "cache_misses": "#EBCB8B",
}
DEFAULT_BAR_COLOR = "#81A1C1"

COMPARISON_METRICS = [
    ("AvgWaitTime", "Average Waiting Time", "time"),
    ("AvgTurnaroundTime", "Average Turnaround Time", "time"),
    ("AvgResponseTime", "Average Response Time", "time"),
    ("CPUUtilization", "CPU Utilization (%)", "cpu"),
    ("Throughput", "Throughput", "throughput"),
    ("ContextSwitches", "Context Switches", "context"),
    ("L1Hits", "L1 Cache Hits", "cache_hits"),
    ("L1Misses", "L1 Cache Misses", "cache_misses"),
    ("L2Hits", "L2 Cache Hits", "cache_hits"),
    ("L2Misses", "L2 Cache Misses", "cache_misses"),
]


def apply_nord_theme(fig, ax):
    """Apply Nord dark theme + transparency options."""
    if TRANSPARENT_BG:
        fig.patch.set_alpha(0.0)
        ax.set_facecolor("none")
    else:
        fig.patch.set_facecolor(NORD_BG)
        ax.set_facecolor(NORD_BG)

    # White text everywhere
    ax.tick_params(colors=NORD_FG)
    ax.yaxis.label.set_color(NORD_FG)
    ax.xaxis.label.set_color(NORD_FG)
    ax.title.set_color(NORD_FG)

    # Grid lines (soft, nord gray)
    ax.grid(axis="y", linestyle="--", alpha=0.4, color=GRID_COLOR)


def render_bar_chart(
    labels: Iterable[str],
    values: Iterable[float],
    *,
    title: str,
    ylabel: str,
    output_path: Path,
    bar_color: str = DEFAULT_BAR_COLOR,
) -> None:
    """Render a bar chart as a PNG."""
    labels = list(labels)
    values = list(values)

    fig, ax = plt.subplots(figsize=(8, 5))
    bars = ax.bar(labels, values, color=bar_color)

    ax.set_title(title)
    ax.set_ylabel(ylabel)

    # Theme applied here
    apply_nord_theme(fig, ax)

    # Add value labels
    if values:
        value_range = max(values) - min(values)
        padding = 0.02 * value_range if value_range else 0.5
        for bar, value in zip(bars, values):
            y = bar.get_height()
            y_text = y + padding if value >= 0 else y - padding
            va = "bottom" if value >= 0 else "top"
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                y_text,
                f"{value:.2f}",
                ha="center",
                va=va,
                color=NORD_FG,
                fontsize=9,
            )

    plt.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(
        output_path,
        dpi=200,
        transparent=TRANSPARENT_BG,   # key for overlays on your slides
    )
    plt.close()


def plot_comparison_metrics() -> None:
    csv_path = ROOT / "comparison_results.csv"
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    labels = [row["Algorithm"] for row in rows]

    for field, label, category in COMPARISON_METRICS:
        values = [float(row[field]) for row in rows]
        filename = f"comparison_{field}.png"
        render_bar_chart(
            labels,
            values,
            title=f"{label} by Algorithm",
            ylabel=label,
            output_path=OUTPUT_DIR / filename,
            bar_color=CATEGORY_COLORS.get(category, DEFAULT_BAR_COLOR),
        )


def main() -> None:
    plot_comparison_metrics()
    print(f"Charts written to: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()

