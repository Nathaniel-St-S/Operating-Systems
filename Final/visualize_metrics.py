#!/usr/bin/env python3
"""
Generate bar-chart visualizations (as PNG files) for the CSV metrics in this directory.

Outputs PNG images to ./charts/, using a Nord-inspired dark theme.
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

# Color palette taken from ColorBrewer Set2 shades to keep a consistent theme
# across all generated charts.
CATEGORY_COLORS = {
    "time": "#4e79a7",
    "cpu": "#f28e2b",
    "throughput": "#59a14f",
    "context": "#e15759",
    "cache_hits": "#76b7b2",
    "cache_misses": "#edc948",
}
DEFAULT_BAR_COLOR = "#4e79a7"


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


def render_bar_chart(
    labels: Iterable[str],
    values: Iterable[float],
    *,
    title: str,
    ylabel: str,
    output_path: Path,
    bar_color: str = NORD["nord9"],
) -> None:
    """Render a bar chart as a PNG (Nord-themed)."""
    labels = list(labels)
    values = list(values)

    # Create figure and axes
    fig, ax = plt.subplots(figsize=(8, 5))

    # Apply Nord background colors
    fig.patch.set_facecolor(NORD["nord0"])
    ax.set_facecolor(NORD["nord1"])

    # Draw bars
    bars = ax.bar(labels, values, color=bar_color)

    # Titles and labels with Nord text color
    ax.set_title(title, color=NORD["nord6"])
    ax.set_ylabel(ylabel, color=NORD["nord6"])

    # Grid styling (subtle Nord grid)
    ax.grid(
        axis="y",
        linestyle="--",
        alpha=0.35,
        color=NORD["nord3"],
    )

    # Tick styling
    ax.tick_params(axis="x", colors=NORD["nord5"])
    ax.tick_params(axis="y", colors=NORD["nord5"])
    for label in ax.get_xticklabels():
        label.set_color(NORD["nord5"])

    # Axis spine styling
    for spine in ax.spines.values():
        spine.set_color(NORD["nord3"])

    # Add value labels above (or below) each bar.
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
                color=NORD["nord6"],
            )

    fig.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=200, facecolor=fig.get_facecolor())
    plt.close(fig)


def plot_comparison_metrics() -> None:
    csv_path = ROOT / "comparison_results.csv"
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        rows = [row for row in reader]

    labels = [row["Algorithm"] for row in rows]
    for field, label, category in COMPARISON_METRICS:
        values = [float(row[field]) for row in rows]
        filename = f"comparison_{field}.png"

        # Use a warm Nord red/orange for time-based metrics, cool blue for others
        if "Time" in field:
            color = NORD["nord11"]  # red
        else:
            color = NORD["nord9"]   # blue

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

