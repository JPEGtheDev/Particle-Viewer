#!/usr/bin/env python3
"""
Extract coverage metrics from gcovr JSON output.

Reads coverage.json and outputs line and branch coverage percentages
in a format suitable for GitHub Actions output variables.
"""

import json
import os
import sys


def main():
    """Extract and print coverage metrics."""
    coverage_file = "coverage.json"
    
    # Debug: print current directory and check for file
    print(f"DEBUG: Current working directory: {os.getcwd()}", file=sys.stderr)
    print(f"DEBUG: Looking for: {coverage_file}", file=sys.stderr)
    print(f"DEBUG: File exists: {os.path.exists(coverage_file)}", file=sys.stderr)
    if os.path.exists(coverage_file):
        print(f"DEBUG: File size: {os.path.getsize(coverage_file)} bytes", file=sys.stderr)
    
    # Default values if file doesn't exist or parsing fails
    line_coverage = 0.0
    branch_coverage = 0.0
    
    try:
        if os.path.exists(coverage_file):
            with open(coverage_file, 'r') as f:
                data = json.load(f)
                print(f"DEBUG: JSON keys: {list(data.keys())}", file=sys.stderr)
                line_coverage = round(data.get('line_percent', 0.0), 2)
                branch_coverage = round(data.get('branch_percent', 0.0), 2)
                print(f"DEBUG: Raw line_percent: {data.get('line_percent')}", file=sys.stderr)
                print(f"DEBUG: Raw branch_percent: {data.get('branch_percent')}", file=sys.stderr)
        else:
            print(f"ERROR: coverage.json not found in {os.getcwd()}", file=sys.stderr)
            print(f"DEBUG: Files in current directory:", file=sys.stderr)
            for f in os.listdir('.'):
                print(f"  - {f}", file=sys.stderr)
    except (FileNotFoundError, json.JSONDecodeError, KeyError) as e:
        print(f"ERROR: Could not read coverage data: {e}", file=sys.stderr)
        # Use default values (0.0)
    
    # Output in format: line_coverage branch_coverage
    print(f"{line_coverage} {branch_coverage}")


if __name__ == "__main__":
    main()
