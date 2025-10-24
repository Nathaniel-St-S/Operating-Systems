import math
from collections import deque
import matplotlib.pyplot as plt
import pandas as pd
from typing import List, Dict, Tuple
from datetime import datetime
import os


def fcfs_schedule(processes: Dict[str, Tuple[int, int]]):
    """
    Non-preemptive First-Come, First-Served scheduling.
    processes: dict of pid -> (arrival, burst)
    Returns: (timeline, metrics)
      - timeline: list of (start, end, pid) sorted by start
      - metrics: dict pid -> metrics dict
    """
    # Sort by arrival, then by name for stable order
    items = sorted(processes.items(), key=lambda kv: (kv[1][0], kv[0]))
    t = 0
    timeline = []
    first_start = {}
    completion = {}
    
    for pid, (arrival, burst) in items:
        if t < arrival:
            t = arrival
        start = t
        end = t + burst
        timeline.append((start, end, pid))
        t = end
        completion[pid] = end
        first_start.setdefault(pid, start)
    
    metrics = {}
    for pid, (arrival, burst) in processes.items():
        comp = completion[pid]
        turnaround = comp - arrival
        waiting = turnaround - burst
        response = first_start[pid] - arrival
        metrics[pid] = {
            "Arrival": arrival,
            "Burst": burst,
            "Completion": comp,
            "Turnaround": turnaround,
            "Waiting": waiting,
            "Response": response,
        }
    return sorted(timeline, key=lambda x: x[0]), metrics


def rr_schedule(processes: Dict[str, Tuple[int, int]], quantum: int):
    """
    Preemptive Round-Robin scheduling with fixed time quantum.
    processes: dict of pid -> (arrival, burst)
    Returns: (timeline, metrics)
      - timeline: list of (start, end, pid)
      - metrics: dict pid -> metrics dict
    """
    # Prepare arrival ordering
    arrivals = sorted([(a, pid, b) for pid, (a, b) in processes.items()], key=lambda x: (x[0], x[1]))
    remaining = {pid: b for pid, (a, b) in processes.items()}
    first_start = {}
    completion = {}
    
    time = 0
    idx = 0
    ready = deque()
    timeline = []
    
    def enqueue_arrivals(up_to_time):
        nonlocal idx
        while idx < len(arrivals) and arrivals[idx][0] <= up_to_time:
            a, pid, b = arrivals[idx]
            ready.append(pid)
            idx += 1
    
    # Start by enqueuing arrivals at time 0 (or earliest arrival)
    if arrivals:
        time = min(0, arrivals[0][0])
        enqueue_arrivals(time)
        if not ready and idx < len(arrivals):
            time = arrivals[0][0]
            enqueue_arrivals(time)
    
    while ready or idx < len(arrivals):
        if not ready:
            # Jump to next arrival if CPU idle
            time = arrivals[idx][0]
            enqueue_arrivals(time)
        
        pid = ready.popleft()
        # Record first response time if not seen
        if pid not in first_start:
            first_start[pid] = time
        
        # Slice
        run = min(quantum, remaining[pid])
        start = time
        end = time + run
        timeline.append((start, end, pid))
        
        # Advance time, enqueue new arrivals that appear during [start, end]
        # For correctness with multiple arrivals inside the slice, enqueue as time progresses
        # but we can batch by stepping to 'end' and adding any with arrival <= end.
        time = end
        enqueue_arrivals(time)
        
        remaining[pid] -= run
        if remaining[pid] > 0:
            # Put it back at tail
            ready.append(pid)
        else:
            completion[pid] = time
    
    metrics = {}
    for pid, (arrival, burst) in processes.items():
        comp = completion[pid]
        turnaround = comp - arrival
        waiting = turnaround - burst
        response = first_start[pid] - arrival
        metrics[pid] = {
            "Arrival": arrival,
            "Burst": burst,
            "Completion": comp,
            "Turnaround": turnaround,
            "Waiting": waiting,
            "Response": response,
        }
    return sorted(timeline, key=lambda x: x[0]), metrics


def plot_gantt(timeline: List[Tuple[int, int, str]], title: str, outfile: str):
    """
    Draws a simple horizontal Gantt chart for a given timeline of (start, end, pid).
    Saves to 'outfile' (PNG).
    """
    # Collect unique pids in order of first appearance for y positions
    order = []
    for s, e, pid in timeline:
        if pid not in order:
            order.append(pid)
    y_positions = {pid: i for i, pid in enumerate(order)}
    
    fig, ax = plt.subplots(figsize=(12, max(2.5, 0.6 * len(order) + 1)))
    
    # Draw each block as a barh segment
    for s, e, pid in timeline:
        y = y_positions[pid]
        ax.barh(y, width=(e - s), left=s)
        ax.text((s + e) / 2, y, pid, va='center', ha='center')
    
    ax.set_xlabel("Time")
    ax.set_ylabel("Process")
    ax.set_yticks(list(y_positions.values()))
    ax.set_yticklabels(order)
    ax.set_title(title)
    ax.set_xlim(left=min(s for s, _, _ in timeline), right=max(e for _, e, _ in timeline))
    ax.grid(True, axis='x', linestyle='--', alpha=0.5)
    
    fig.tight_layout()
    fig.savefig(outfile, dpi=150, bbox_inches='tight')
    plt.close(fig)


def metrics_dataframe(metrics: Dict[str, Dict]):
    df = pd.DataFrame.from_dict(metrics, orient='index')
    df = df[["Arrival", "Burst", "Completion", "Turnaround", "Waiting", "Response"]].sort_index()
    df.loc["AVERAGE"] = {
        "Arrival": "",
        "Burst": "",
        "Completion": "",
        "Turnaround": round(df["Turnaround"].astype(float).mean(), 2),
        "Waiting": round(df["Waiting"].astype(float).mean(), 2),
        "Response": round(df["Response"].astype(float).mean(), 2),
    }
    return df


# ---------- Sample data from your assignment ----------

# Q1 (FCFS)
processes_q1 = {
    "P3": (0, 15),
    "P1": (2, 20),
    "P6": (3, 16),
    "P2": (5, 14),
    "P5": (6, 24),
    "P4": (8, 11),
}

# Q2 (RR, q=3)
processes_q2 = {
    "P4": (0, 24),
    "P2": (1, 18),
    "P1": (2, 11),
    "P3": (3, 7),
    "P6": (4, 15),
    "P5": (6, 5),
}
quantum_q2 = 3

# ---------- Generate charts & tables ----------

# FCFS for Q1
tl1, m1 = fcfs_schedule(processes_q1)
png1 = "/mnt/data/gantt_fcfs_q1.png"
plot_gantt(tl1, "FCFS Gantt (Q1)", png1)
df1 = metrics_dataframe(m1)

# RR for Q2
tl2, m2 = rr_schedule(processes_q2, quantum=quantum_q2)
png2 = "/mnt/data/gantt_rr_q2.png"
plot_gantt(tl2, f"Round Robin (q={quantum_q2}) Gantt (Q2)", png2)
df2 = metrics_dataframe(m2)

# Display timelines and dataframes to the user
from caas_jupyter_tools import display_dataframe_to_user
display_dataframe_to_user("Q1 — FCFS metrics", df1)
display_dataframe_to_user("Q2 — RR (q=3) metrics", df2)

# Print quick list of timeline segments for copy/paste
print("Q1 FCFS timeline segments (start, end, pid):")
print(tl1)
print()
print("Q2 RR timeline segments (start, end, pid):")
print(tl2)

# Confirm output files
print("\nSaved charts:")
print(png1)
print(png2)

