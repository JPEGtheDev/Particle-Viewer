#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Read event source from stdin; skip on resume (context already present)
source=$(python3 -c "import sys,json; print(json.load(sys.stdin).get('source','startup'))" 2>/dev/null || echo "startup")
[[ "$source" == "resume" ]] && exit 0

python3 -c "
import json, pathlib, sys
content = pathlib.Path(sys.argv[1]).read_text()
print(json.dumps({'additionalContext': content}))
" "$SCRIPT_DIR/session-start.md"
