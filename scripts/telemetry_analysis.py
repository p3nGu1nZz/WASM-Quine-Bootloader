#!/usr/bin/env python3
"""Simple telemetry analysis helper.

Usage:
    python3 scripts/telemetry_analysis.py <data_dir>

Reads all files matching "gen_*.txt" in the given directory and prints a
summary table containing generation number, duration, mutation counts, and
instance count.  If the files are JSON-formatted ("--telemetry-format=json"
was used), it will parse them as JSON instead.
"""

import sys
import os
import re
import json

def parse_text(fn):
    stats = {}
    with open(fn, 'r') as f:
        data = f.read()
    m = re.search(r'Final Generation: (\d+)', data)
    if m:
        stats['gen'] = int(m.group(1))
    m = re.search(r'Gen Duration: ([0-9.]+) ms', data)
    if m:
        stats['duration'] = float(m.group(1))
    m = re.search(r'Mutations Attempted: (\d+)', data)
    if m:
        stats['mut_attempted'] = int(m.group(1))
    m = re.search(r'Mutations Applied: (\d+)', data)
    if m:
        stats['mut_applied'] = int(m.group(1))
    m = re.search(r'INSTANCES: (\d+)', data)
    if m:
        stats['instances'] = int(m.group(1))
    return stats


def parse_json(fn):
    with open(fn, 'r') as f:
        obj = json.load(f)
    stats = {
        'gen': obj.get('Final Generation'),
        'duration': obj.get('Gen Duration'),
        'mut_attempted': obj.get('Mutations Attempted'),
        'mut_applied': obj.get('Mutations Applied'),
        'instances': len(obj.get('Instances', [])) if obj.get('Instances') else 0,
    }
    return stats


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    d = sys.argv[1]
    if not os.path.isdir(d):
        print(f"Directory not found: {d}")
        sys.exit(1)
    files = sorted(f for f in os.listdir(d) if f.startswith('gen_') and f.endswith('.txt'))
    print('gen\tdur(ms)\tattempted\tapplied\tinstances')
    for fn in files:
        path = os.path.join(d, fn)
        try:
            if fn.endswith('.json.txt') or (open(path).read(1) == '{'):
                st = parse_json(path)
            else:
                st = parse_text(path)
        except Exception as e:
            print(f"error parsing {fn}: {e}")
            continue
        print(f"{st.get('gen')}\t{st.get('duration')}\t{st.get('mut_attempted')}\t{st.get('mut_applied')}\t{st.get('instances')}")

if __name__ == '__main__':
    main()
