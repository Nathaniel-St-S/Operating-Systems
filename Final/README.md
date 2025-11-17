# Advanced Operating System Simulator
### CSAS3111 – Operating Systems  
### Professor: Ashlin Johnsy  
### Due: December 8, 2025  
### Presentation: December 11 (8:00 AM – 10:00 AM)

---

# Full Project Checklist

---

<details>
<summary><h2>1. Documentation & Title Page</h2></summary>

### Title Page
- [ ] Project title  
- [ ] Member names
- [ ] Date  
- [ ] Course information  

### Documentation Sections
- [ ] Introduction (Projects 1–3 overview)  
- [ ] Key Concepts & Features  
- [ ] Module Breakdown  
- [ ] Running the Simulator  
- [ ] Testing & Debugging  
- [ ] Conclusion  
- [ ] Screenshots  
- [ ] Final Report

</details>

---

<details>
<summary><h2>2. Module 1 — Process Simulation</h2></summary>

### CPU Registers
- [ ] Program Counter (PC)  
- [ ] Accumulator (ACC)  
- [ ] Instruction Register (IR)  
- [ ] Status Register (Zero, Carry, Overflow flags)  

### Process Control Block (PCB)
- [ ] PID  
- [ ] PC  
- [ ] ACC  
- [ ] State (READY, RUNNING, BLOCKED)  
- [ ] Priority  
- [ ] Time Remaining (SRT)  
- [ ] Response Ratio (HRRN)

### Fetch–Decode–Execute Cycle
- [ ] Fetch instruction  
- [ ] Decode instruction  
- [ ] Execute instruction  
- [ ] Update process state  

</details>

---

<details>
<summary><h2>3. Module 2 — Advanced Memory Management</h2></summary>

### Hierarchical Memory
- [ ] L1 Cache  
- [ ] L2 Cache  
- [ ] RAM  
- [ ] Cache hit/miss detection  
- [ ] read() with L1→L2→RAM lookup  
- [ ] write-through policy  
- [ ] write-back policy + dirty bits  

### Memory Table
Tracks:
- [ ] Process ID  
- [ ] Memory Start Address  
- [ ] Memory End Address  
- [ ] isFree flag  

### Dynamic Memory Allocation
- [ ] First-Fit  
- [ ] Best-Fit  
- [ ] allocateMemory()  
- [ ] deallocateMemory()

</details>

---

<details>
<summary><h2>4. Module 3 — Scheduling & Context Switching</h2></summary>

### PCB Enhancements
- [ ] Priority  
- [ ] Time Remaining  
- [ ] Response Ratio  
- [ ] State transitions implemented  

### Scheduling Algorithms
- [ ] FCFS  
- [ ] Round Robin (RR)  
- [ ] Priority Scheduling  
- [ ] Shortest Remaining Time (SRT)  
- [ ] Highest Response Ratio Next (HRRN)  
- [ ] Shortest Process Next (SPN)  
- [ ] Multi-Level Feedback Queue  

### Context Switching
- [ ] saveState()  
- [ ] loadState()  
- [ ] contextSwitch()  
- [ ] READY → RUNNING  
- [ ] RUNNING → BLOCKED  
- [ ] RUNNING → READY  

### Integration With F-D-E Cycle
- [ ] Check time slice for RR  
- [ ] Preempt on higher priority  
- [ ] Automatic state updates  

</details>

---

<details>
<summary><h2>5. Module 4 — Interrupt Handling</h2></summary>

### Interrupt Types
- [ ] Timer interrupt  
- [ ] I/O interrupt  
- [ ] System call interrupt  
- [ ] Trap interrupt  

### Interrupt Vector Table (IVT)
- [ ] IVT[0] → Timer handler  
- [ ] IVT[1] → I/O handler  
- [ ] IVT[2] → System call handler  
- [ ] IVT[3] → Trap handler  

### Interrupt Handlers
- [ ] Save CPU state  
- [ ] Execute ISR  
- [ ] Prepare dispatcher  

### Dispatcher
- [ ] Select next process  
- [ ] Restore state  
- [ ] Perform context switch  

</details>

---

<details>
<summary><h2>6. Module 5 — Efficiency Analysis</h2></summary>

### Performance Metrics
- [ ] Execution time  
- [ ] CPU utilization  
- [ ] Memory utilization (cache hits/misses)  
- [ ] Waiting time  
- [ ] Turnaround time  
- [ ] Context switch count  

### Time Tracking
- [ ] Track CPU operation time  
- [ ] Track memory operation time  
- [ ] Track each scheduling algorithm  

### Data Collection
- [ ] Store metrics in structs/arrays  
- [ ] CPU-bound workload data  
- [ ] I/O-bound workload data  
- [ ] Mixed workload data  

### Visualization
- [ ] Execution Time chart  
- [ ] CPU Utilization chart  
- [ ] Memory Usage (cache hits/misses) chart  
- [ ] Algorithm comparison table  

</details>

---

<details>
<summary><h2>7. Output Requirements</h2></summary>

- [ ] Show ready queue  
- [ ] Execution trace per algorithm  
- [ ] Waiting time per process  
- [ ] Turnaround time  
- [ ] CPU usage  
- [ ] Consolidated comparison table  

### Required Example Algorithms
- [ ] FCFS  
- [ ] Round Robin  
- [ ] SJF  
- [ ] HRRN  
- [ ] Additional implemented algorithms  

</details>

---

<details>
<summary><h2>8. Submission Requirements</h2></summary>

- [ ] Full source code  
- [ ] Output screenshots  
- [ ] Combined Project 1–3 report
- [ ] PowerPoint presentation  
- [ ] Prepare 15-minute live demo  
- [ ] All group members present  

</details>

---

<details>
<summary><h2>9. Grading Breakdown</h2></summary>

| Category | Points |
|---------|--------|
| Documentation | 5 |
| Code Implementation | 50 |
| Testing & Debugging | 15 |
| Performance Analysis | 10 |
| Presentation & Submission | 20 |
| **Total** | **100** |

</details>
