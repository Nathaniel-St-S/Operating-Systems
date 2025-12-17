import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV
df = pd.read_csv("results.csv")

# Metrics to plot
metrics = ["L1Hits", "L1Misses", "L2Hits", "L2Misses"]
labelNames = ["L1 Cache Hits", "L1 Cache Misses", "L2 Cache Hits", "L2 Cache Misses"]
# X-axis labels from the CSV
algorithms = df["Algorithm"]
i=0
# Generate one chart per metric
for metric in metrics:
    
    plt.figure()
    bars = plt.bar(algorithms, df[metric])
    plt.title(f"{labelNames[i]} by Algorithm")
    plt.grid(axis="y", linestyle="--", alpha=0.35)
    #plt.xlabel("Scheduling Algorithm")
    plt.ylabel(labelNames[i])
    for bar in bars:
        height = bar.get_height()
        plt.text(
            bar.get_x() + bar.get_width() / 2,
            height,
            f"{int(height)}",
            ha="center",
            va="bottom"
        )
    plt.tight_layout()
    plt.show()
    i+=1