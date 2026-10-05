/* Arrival-aware scheduler. Input order resolves ties.
   RR: arrivals during a slice join before the running process is requeued. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 20
#define LIMIT 1000

typedef struct { int id, arrival, burst; } Process;
typedef struct { int completion, first, remaining; } State;

static void metrics(const char *name, Process p[], State s[], int n, int end,
                    int busy, FILE *csv) {
    double wait = 0, turn = 0, response = 0;
    printf("\n%s\nID Arrival Burst Finish Waiting Turnaround Response\n", name);
    for (int i = 0; i < n; i++) {
        int t = s[i].completion - p[i].arrival;
        int v = t - p[i].burst;
        int response_time = s[i].first - p[i].arrival;
        printf("P%d %7d %5d %6d %7d %10d %8d\n", p[i].id, p[i].arrival,
               p[i].burst, s[i].completion, v, t, response_time);
        fprintf(csv, "%s,%d,%d,%d,%d,%d,%d,%d\n", name, p[i].id,
                p[i].arrival, p[i].burst, s[i].completion, v, t, response_time);
        wait += v; turn += t; response += response_time;
    }
    printf("Averages: waiting %.2f | turnaround %.2f | response %.2f\n",
           wait/n, turn/n, response/n);
    printf("CPU utilization: %.2f%% | throughput: %.4f jobs/time-unit\n",
           100.0 * busy / end, (double)n / end);
}

static void sequential(const char *name, Process p[], int n, int shortest, FILE *csv) {
    State s[MAX];
    for (int i = 0; i < n; i++) s[i] = (State){0, -1, p[i].burst};
    int time = 0, done = 0, busy = 0;
    printf("\n%s timeline [start,end)\n", name);
    while (done < n) {
        int best = -1, next = LIMIT + 1;
        for (int i = 0; i < n; i++) {
            if (s[i].remaining == 0) continue;
            if (p[i].arrival < next) next = p[i].arrival;
            if (p[i].arrival > time) continue;
            if (best < 0 || (shortest && p[i].burst < p[best].burst) ||
                (!shortest && p[i].arrival < p[best].arrival)) best = i;
        }
        if (best < 0) {
            printf("[%d,%d) IDLE\n", time, next);
            time = next; continue;
        }
        s[best].first = time;
        printf("[%d,%d) P%d\n", time, time + p[best].burst, p[best].id);
        time += p[best].burst; busy += p[best].burst;
        s[best].completion = time; s[best].remaining = 0; done++;
    }
    metrics(name, p, s, n, time, busy, csv);
}

/* Append newly arrived jobs in arrival order, then original input order. */
static void admit(Process p[], int n, int time, int admitted[], int queue[], int *count) {
    while (1) {
        int best = -1;
        for (int i = 0; i < n; i++)
            if (!admitted[i] && p[i].arrival <= time &&
                (best < 0 || p[i].arrival < p[best].arrival)) best = i;
        if (best < 0) return;
        queue[(*count)++] = best;
        admitted[best] = 1;
    }
}

static void rr(Process p[], int n, int quantum, FILE *csv) {
    State s[MAX];
    int queue[MAX], count = 0, admitted[MAX] = {0};
    for (int i = 0; i < n; i++) s[i] = (State){0, -1, p[i].burst};
    int time = 0, done = 0, busy = 0;
    puts("\nRR timeline [start,end)");
    while (done < n) {
        admit(p, n, time, admitted, queue, &count);
        if (count == 0) {
            int next = LIMIT + 1;
            for (int i = 0; i < n; i++)
                if (!admitted[i] && p[i].arrival < next) next = p[i].arrival;
            printf("[%d,%d) IDLE\n", time, next); time = next; continue;
        }
        int index = queue[0];
        for (int i = 1; i < count; i++) queue[i-1] = queue[i];
        count--;
        if (s[index].first < 0) s[index].first = time;
        int slice = s[index].remaining < quantum ? s[index].remaining : quantum;
        printf("[%d,%d) P%d\n", time, time + slice, p[index].id);
        time += slice; busy += slice; s[index].remaining -= slice;
        admit(p, n, time, admitted, queue, &count);
        if (s[index].remaining) queue[count++] = index;
        else { s[index].completion = time; done++; }
    }
    metrics("RR", p, s, n, time, busy, csv);
}

int main(int argc, char **argv) {
    if (argc > 3) { fprintf(stderr,"Usage: %s [workload.txt] [results.csv]\n",argv[0]); return 1; }
    Process p[MAX] = {{1,0,5},{2,1,3},{3,2,8}};
    int n = 3, quantum = 2;
    if (argc >= 2) {
        FILE *f = fopen(argv[1],"r");
        if (!f) { perror("Workload"); return 1; }
        if (fscanf(f,"%d %d",&n,&quantum)!=2 || n<1 || n>MAX || quantum<1 || quantum>LIMIT) {
            fclose(f); fputs("Invalid count or quantum.\n",stderr); return 1;
        }
        for (int i=0;i<n;i++) {
            p[i].id=i+1;
            if (fscanf(f,"%d %d",&p[i].arrival,&p[i].burst)!=2 || p[i].arrival<0 ||
                p[i].arrival>LIMIT || p[i].burst<1 || p[i].burst>LIMIT) {
                fclose(f); fputs("Invalid arrival/burst.\n",stderr); return 1;
            }
        }
        char extra;
        if (fscanf(f," %c",&extra)==1) { fclose(f); fputs("Extra input.\n",stderr); return 1; }
        fclose(f);
    }
    const char *output = argc == 3 ? argv[2] : "results.csv";
    /* Do not let an output file truncate the workload file. */
    if (argc >= 2 && strcmp(argv[1], output) == 0) {
        fputs("Use different input and output paths.\n", stderr); return 1;
    }
    FILE *csv = fopen(output,"w");
    if (!csv) { perror("Results"); return 1; }
    fputs("algorithm,id,arrival,burst,completion,waiting,turnaround,response\n",csv);
    sequential("FCFS",p,n,0,csv); sequential("SJF",p,n,1,csv); rr(p,n,quantum,csv);
    if (fclose(csv) != 0) { perror("Writing results"); return 1; }
    printf("\nSaved %s\n", output);
    return 0;
}
