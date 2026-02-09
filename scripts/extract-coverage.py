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
                print(f"DEBUG: JSON top-level keys: {list(data.keys())}", file=sys.stderr)
                
                # gcovr JSON format has different structures depending on version
                # Try multiple possible key locations
                
                # Option 1: Direct keys (older gcovr)
                if 'line_percent' in data:
                    line_coverage = round(data['line_percent'], 2)
                    branch_coverage = round(data.get('branch_percent', 0.0), 2)
                    print(f"DEBUG: Found direct keys - line: {line_coverage}, branch: {branch_coverage}", file=sys.stderr)
                
                # Option 2: Nested in 'root' (common format)
                elif 'root' in data and isinstance(data['root'], dict):
                    root = data['root']
                    print(f"DEBUG: Root keys: {list(root.keys())}", file=sys.stderr)
                    
                    # Try line_covered/line_total format
                    if 'line_covered' in root and 'line_total' in root:
                        line_total = root['line_total']
                        line_covered = root['line_covered']
                        if line_total > 0:
                            line_coverage = round((line_covered / line_total) * 100, 2)
                        print(f"DEBUG: Calculated line coverage: {line_covered}/{line_total} = {line_coverage}%", file=sys.stderr)
                    
                    # Try branch_covered/branch_total format
                    if 'branch_covered' in root and 'branch_total' in root:
                        branch_total = root['branch_total']
                        branch_covered = root['branch_covered']
                        if branch_total > 0:
                            branch_coverage = round((branch_covered / branch_total) * 100, 2)
                        print(f"DEBUG: Calculated branch coverage: {branch_covered}/{branch_total} = {branch_coverage}%", file=sys.stderr)
                
                # Option 3: In summary field
                elif 'summary' in data and isinstance(data['summary'], dict):
                    summary = data['summary']
                    print(f"DEBUG: Summary keys: {list(summary.keys())}", file=sys.stderr)
                    line_coverage = round(summary.get('line_percent', 0.0), 2)
                    branch_coverage = round(summary.get('branch_percent', 0.0), 2)
                
                else:
                    print(f"ERROR: Unknown JSON structure. Dumping first 1000 chars:", file=sys.stderr)
                    print(json.dumps(data, indent=2)[:1000], file=sys.stderr)
                
                print(f"DEBUG: Final values - line: {line_coverage}%, branch: {branch_coverage}%", file=sys.stderr)
        else:
            print(f"ERROR: coverage.json not found in {os.getcwd()}", file=sys.stderr)
            print(f"DEBUG: Files in current directory:", file=sys.stderr)
            for f in os.listdir('.'):
                print(f"  - {f}", file=sys.stderr)
    except (FileNotFoundError, json.JSONDecodeError, KeyError) as e:
        print(f"ERROR: Could not read coverage data: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc(file=sys.stderr)
        # Use default values (0.0)
    
    # Output in format: line_coverage branch_coverage
    print(f"{line_coverage} {branch_coverage}")


if __name__ == "__main__":
    main()
