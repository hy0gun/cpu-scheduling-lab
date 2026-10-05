"""Measure Round Robin response/wait tradeoffs for quantum values 1-8."""
import csv
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent

def run():
    executable=HERE/"scheduler"
    if not executable.exists():
        raise SystemExit("Compile scheduler first (see README).")
    rows=[]
    with tempfile.TemporaryDirectory() as folder:
        workload=Path(folder)/"input.txt"; results=Path(folder)/"result.csv"
        for quantum in range(1,9):
            workload.write_text(f"4 {quantum}\n0 9\n1 3\n2 5\n6 2\n")
            subprocess.run([str(executable),str(workload),str(results)],check=True,capture_output=True)
            with results.open() as f:
                rr=[row for row in csv.DictReader(f) if row["algorithm"]=="RR"]
            rows.append([quantum]+[sum(int(row[key]) for row in rr)/len(rr) for key in ("waiting","turnaround","response")])
    with (HERE/"quantum_experiment.csv").open("w",newline="") as f:
        writer=csv.writer(f); writer.writerow(["quantum","avg_waiting","avg_turnaround","avg_response"]); writer.writerows(rows)
    for row in rows: print(row)
    print("Saved quantum_experiment.csv. Results describe this workload only.")

if __name__=="__main__": run()
