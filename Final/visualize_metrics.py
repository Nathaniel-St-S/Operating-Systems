#!/usr/bin/env python3
"""
Generate bar-chart visualizations (as PNG files) for the CSV metrics in this directory.

Outputs PNG images to ./charts/.
"""

from __future__ import annotations

import csv
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

import matplotlib

matplotlib.use("Agg")  # non-GUI backend
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent
OUTPUT_DIR = ROOT / "charts"


def read_algorithm_metrics(csv_path: Path, value_field: str) -> Tuple[List[str], List[float]]:
    """Read two-column (Algorithm + metric) CSV into label and value lists."""
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        labels: List[str] = []
        values: List[float] = []
        for row in reader:
            labels.append(row["Algorithm"])
            values.append(float(row[value_field]))
    return labels, values


def render_bar_chart(
    labels: Iterable[str],
    values: Iterable[float],
    *,
    title: str,
    ylabel: str,
    output_path: Path,
    bar_color: str = "#4e79a7",
) -> None:
    """Render a bar chart as a PNG."""
    labels = list(labels)
    values = list(values)

    plt.figure(figsize=(8, 5))
    bars = plt.bar(labels, values, color=bar_color)
    plt.title(title)
    plt.ylabel(ylabel)
    plt.grid(axis="y", linestyle="--", alpha=0.35)

    # Add value labels above (or below) each bar.
    if values:
        value_range = max(values) - min(values)
        padding = 0.02 * value_range if value_range else 0.5
        for bar, value in zip(bars, values):
            y = bar.get_height()
            y_text = y + padding if value >= 0 else y - padding
            va = "bottom" if value >= 0 else "top"
            plt.text(bar.get_x() + bar.get_width() / 2, y_text, f"{value:.2f}", ha="center", va=va)

    plt.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output_path, dpi=200)
    plt.close()


def plot_single_metric(csv_filename: str, metric_field: str, ylabel: str) -> None:
    csv_path = ROOT / csv_filename
    labels, values = read_algorithm_metrics(csv_path, metric_field)
    title = ylabel
    output_path = OUTPUT_DIR / f"{csv_path.stem}.png"
    render_bar_chart(labels, values, title=title, ylabel=ylabel, output_path=output_path)


def plot_comparison_metrics() -> None:
    csv_path = ROOT / "comparison_results.csv"
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        rows = [row for row in reader]

    labels = [row["Algorithm"] for row in rows]
    metrics: Dict[str, str] = {
        "AvgWaitTime": "Average Waiting Time",
        "AvgTurnaroundTime": "Average Turnaround Time",
        "AvgResponseTime": "Average Response Time",
        "CPUUtilization": "CPU Utilization (%)",
        "Throughput": "Throughput",
        "ContextSwitches": "Context Switches",
    }

    for field, label in metrics.items():
        values = [float(row[field]) for row in rows]
        filename = f"comparison_{field}.png"
        render_bar_chart(
            labels,
            values,
            title=f"{label} by Algorithm",
            ylabel=label,
            output_path=OUTPUT_DIR / filename,
            bar_color="#e15759" if "Time" in field else "#4e79a7",
        )


def main() -> None:
    plot_single_metric("chart_cpu_utilization.csv", "CPUUtilization", "CPU Utilization (%)")
    plot_single_metric("chart_context_switches.csv", "ContextSwitches", "Context Switches")
    plot_single_metric("chart_waiting_time.csv", "AvgWaitingTime", "Average Waiting Time")
    plot_comparison_metrics()
    print(f"Charts written to: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
