#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

python3 -c "
import json, pathlib, sys
content = pathlib.Path(sys.argv[1]).read_text()
print(json.dumps({'additionalContext': content}))
" "$SCRIPT_DIR/pre-message.md"
