# Arrival-Aware CPU Scheduling Lab

## Build and run
```
gcc -std=c11 -Wall -Wextra -Werror scheduler.c -o scheduler
./scheduler workload.txt results.csv
python3 report.py results.csv
python3 experiment.py
```
Use Linux/GCC, including your existing class environment. Open comparison.html after report.py. Default invocation ./scheduler uses built-in jobs and writes results.csv. Re-running overwrites output.

## Input
First line: process count and quantum. Each subsequent line: arrival and burst. IDs follow input order. Counts 1-20; arrivals 0-1000; bursts and quantum 1-1000. Example:
```
3 2
0 5
1 3
2 8
```
FCFS picks earliest available arrival; non-preemptive SJF picks shortest available burst; ties follow input order. Round Robin maintains a queue and admits new arrivals in arrival/input order. New arrivals during a slice, including its endpoint, join before the running job is requeued.

## Metrics
Turnaround = completion - arrival. Waiting = turnaround - burst. Response = first execution - arrival. Utilization = busy time / elapsed time from simulation time 0. Throughput = completed processes / elapsed time. Initial and intermediate idle periods count toward elapsed time. CSV stores per-process metrics; terminal output also shows averages and timeline slices.

For the sample, completion P1/P2/P3: FCFS 5/8/16, SJF 5/8/16, RR 12/9/16. SJF and FCFS coincide because only P1 is ready initially and P2 is shorter than P3 when P1 finishes. RR average waiting is 6.00, turnaround 11.33, response 1.00. Do not confuse the new arrival-aware results with the earlier all-at-zero version.

## Implementation
Process stores ID, arrival, and burst. State stores completion, first execution, remaining work. sequential selects a job among ready processes or jumps to the next arrival if idle. admit adds each process only once to the Round Robin queue. rr removes the queue head, executes at most one quantum, admits newly arrived jobs, and requeues unfinished work.

Queue capacity is bounded by the number of processes. It uses an array with shifting after a dequeue; for at most 20 jobs this is readable, though a circular queue would scale better.

experiment.py runs a fixed four-job workload at quantum values 1-8 and writes quantum_experiment.csv. It investigates responsiveness/waiting tradeoffs without claiming a universal best quantum.

## Limits
Single CPU; no I/O blocking, priorities, preemption in SJF, or switching cost. Text input must contain ordinary integers in the documented range. This is a simulation, not operating-system scheduling code. Output paths should be separate from input paths, including aliases/symlinks.

## Your contributions
Generated implementation. Record your own changes and verification here.
