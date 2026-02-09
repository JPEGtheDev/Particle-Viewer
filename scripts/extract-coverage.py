#!/usr/bin/env python3
"""
Extract coverage metrics from gcovr text output.

Reads coverage.txt and outputs line coverage percentage.
Format: line_coverage branch_coverage (branch always 0 in text format)
"""

import re
import sys


def main():
    """Extract and print coverage metrics from coverage.txt."""
    coverage_file = "coverage.txt"
    line_coverage = 0
    
    try:
        with open(coverage_file, 'r') as f:
            for line in f:
                # Look for TOTAL line: "TOTAL     893     442    49%"
                if line.startswith('TOTAL'):
                    # Extract percentage using regex
                    match = re.search(r'(\d+)%', line)
                    if match:
                        line_coverage = int(match.group(1))
                        break
        
        print(f"{line_coverage} 0")
        
    except FileNotFoundError:
        print(f"ERROR: {coverage_file} not found", file=sys.stderr)
        print("0 0")
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        print("0 0")


if __name__ == "__main__":
    main()
