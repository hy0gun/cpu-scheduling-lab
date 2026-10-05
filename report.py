"""Create an offline HTML comparison from scheduler CSV."""
import csv
from html import escape
from pathlib import Path
import sys

def render(source, destination):
    with source.open(newline="", encoding="utf-8") as file:
        rows = list(csv.DictReader(file))
    algorithms = {}
    for row in rows:
        algorithms.setdefault(row["algorithm"], []).append(row)
    body = "<h1>CPU Scheduling Lab</h1><p>One CPU; no I/O or context-switch cost. Lower averages depend on the workload.</p>"
    for algorithm, records in algorithms.items():
        body += f"<h2>{escape(algorithm)}</h2>"
        for metric in ("waiting", "turnaround", "response"):
            average = sum(int(row[metric]) for row in records) / len(records)
            body += f"<p>Average {metric}: <strong>{average:.2f}</strong></p>"
        body += "<table><tr>" + "".join(f"<th>{escape(k)}</th>" for k in records[0]) + "</tr>"
        for row in records:
            body += "<tr>" + "".join(f"<td>{escape(value)}</td>" for value in row.values()) + "</tr>"
        body += "</table>"
    destination.write_text('<!doctype html><meta charset="utf-8"><title>Scheduling comparison</title><style>body{font:17px system-ui;max-width:1000px;margin:40px auto;padding:20px;background:#f2f5fb;color:#17243b}table{border-collapse:collapse;background:white}td,th{padding:12px;border:1px solid #ccc}h2{color:#3158a7}</style>'+body, encoding="utf-8")

if __name__ == "__main__":
    render(Path(sys.argv[1]) if len(sys.argv)>1 else Path("results.csv"), Path("comparison.html"))
    print("Open comparison.html in your browser.")
