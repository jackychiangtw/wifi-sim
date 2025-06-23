#!/usr/bin/env python3
import sys
from collections import defaultdict
import matplotlib.pyplot as plt
import pathlib

# ------------------------------------------------------------------ helpers
def parse_file(path: str):
    """return dict{ac: list[values]}, list[offered]"""
    ac_data   = defaultdict(list)
    offered   = []
    current   = None

    with open(path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue                      # skip blank line
            tok = line.split()
            if tok[0] == 'OfferedTraffic':
                current = float(tok[1])
                offered.append(current)
            else:                             # AC line
                ac = tok[0]          # e.g. AC_VI
                val = float(tok[1])
                ac_data[ac].append(val)
    return offered, ac_data

# ------------------------------------------------------------------ main
def main():
    if len(sys.argv) != 2:
        print("usage: python3 plot_qbss.py <output_file.txt>")
        sys.exit(1)

    infile = pathlib.Path(sys.argv[1])
    if not infile.exists():
        sys.exit(f"file not found: {infile}")

    offered, ac = parse_file(infile)

    plt.figure(figsize=(9, 5))
    markers = {'AC_VI': 'o', 'AC_VO': 'o', 'AC_BE': 'o', 'AC_BK': 'o'}
    for ac_name, y in ac.items():
        plt.plot(offered, y,
                 marker=markers.get(ac_name, 'o'),
                 label=ac_name)

    plt.title("Throughput vs Offered Traffic")
    plt.xlabel("Offered Traffic (Mbps)")
    plt.ylabel("Throughput (Mbps)")
    plt.grid(True, which='both', linestyle='--', alpha=.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig("qbss.png", dpi=120)
    #plt.show()

if __name__ == "__main__":
    main()

