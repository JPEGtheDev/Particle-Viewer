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
    
    # Default values if file doesn't exist or parsing fails
    line_coverage = 0.0
    branch_coverage = 0.0
    
    try:
        if os.path.exists(coverage_file):
            with open(coverage_file, 'r') as f:
                data = json.load(f)
                line_coverage = round(data.get('line_percent', 0.0), 2)
                branch_coverage = round(data.get('branch_percent', 0.0), 2)
    except (FileNotFoundError, json.JSONDecodeError, KeyError) as e:
        print(f"Warning: Could not read coverage data: {e}", file=sys.stderr)
        # Use default values (0.0)
    
    # Output in format: line_coverage branch_coverage
    print(f"{line_coverage} {branch_coverage}")


if __name__ == "__main__":
    main()
