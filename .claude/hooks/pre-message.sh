#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel 2>/dev/null)"
HONESTY_SKILL="$REPO_ROOT/.github/skills/honesty/SKILL.md"

if ! command -v python3 &>/dev/null; then
  printf '{"hookSpecificOutput":{"hookEventName":"UserPromptSubmit","additionalContext":"[ERROR: python3 not found; hook content unavailable]"}}\n'
  exit 0
fi

python3 - "$HONESTY_SKILL" << 'PYEOF'
import json, pathlib, re, sys

skill_path = pathlib.Path(sys.argv[1])

if skill_path.exists():
    raw = skill_path.read_text()
    body = re.sub(r'^---\n.*?---\n', '', raw, count=1, flags=re.DOTALL)
else:
    body = f"[ERROR: honesty/SKILL.md not found at {sys.argv[1]}]"

HEADER = """\
<IMPORTANT>
## Bootstrap Check

If you have no memory of bootstrapping this session, load `session-bootstrap` now before responding.

---

## Honesty Skill — Active on Every Turn

"""

FOOTER = """
---

## Iron Laws — Active on Every Response

| # | Law |
|---|-----|
| 1 | NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST. |
| 2 | NO COMPLETION CLAIMS WITHOUT FRESH VERIFICATION. Run `cmake --build build && ./build/tests/ParticleViewerTests` in THIS session. Show output inline. |
| 3 | NO FIXES WITHOUT ROOT CAUSE INVESTIGATION. |
| 4 | EVERY COMMIT USES CONVENTIONAL FORMAT. `<type>[scope]: <description>` |
| 5 | FORMAT BEFORE EVERY COMMIT. `find src tests -name "*.cpp" -o -name "*.hpp" \\| xargs clang-format -i` |
| 6 | FAILURE IS RECOVERABLE. FALSE CONFIDENCE IS NOT. "Should work" is banned. |
| 7 | CLARIFY FIRST. PLAN BEFORE CODE. NO PLACEHOLDERS. |
| 8 | NO CODE UNTIL THE DESIGN GATE IS PASSED. |
| 9 | DISPATCH BEFORE GUESSING. |
| 10 | DISPATCH REVIEWERS AFTER EVERY TODO. |
| 11 | THE BROWN M&M LAW. Every skill with a `## Canary` section: produce that canary output when applying the skill. A missing canary is a trust violation. |

If you are tempted to rationalize past any of these: that thought is the rationalization. Stop. Follow the rule.

---

## Skill Reload Triggers — STOP and Reload When:

1. Picking up a new todo -- reload the skill(s) for that todo's domain
2. After 3 user prompts without a skill reload -- reload the skill for whatever you are doing
3. After a user correction or redirect -- reload the misapplied skill immediately
4. After context compaction -- any compaction requires full skill reload for the active domain

Do NOT say "I remember the skill content." Load fresh. Skill routing table is in `session-bootstrap`.
</IMPORTANT>"""

content = HEADER + body + FOOTER
print(json.dumps({
    "hookSpecificOutput": {
        "hookEventName": "UserPromptSubmit",
        "additionalContext": content
    }
}))
PYEOF
