# Review Instructions

How to conduct a skill review. The criteria for each checklist item are in the reference
sections injected above this document. Apply them exactly.

---

## Step 0 -- Pre-Checks

Run before any element checks:

```bash
wc -l "{{SKILL_PATH}}"
wc -c "{{SKILL_PATH}}"
head -6 "{{SKILL_PATH}}"
```

Record the output. Apply size limits from SIZE_AND_COMPRESSION. Apply frontmatter rules
from SKILL_ANATOMY_ELEMENTS Element 1.

---

## Checklist

Run every item. Mark ✅ (pass) or ❌ (fail) with file:line evidence. Criteria for each
item are in the reference sections above.

- [ ] **Frontmatter** -- `name:` and `description: Use when...` present, valid, under 200 characters?
- [ ] **Size** -- within limits defined in SIZE_AND_COMPRESSION?
- [ ] **Iron Law block** -- backtick-wrapped; ALL CAPS rule; `YOU MUST`; `No exceptions.`; all INSIDE the backtick block?
- [ ] **Iron Law letter/spirit line** -- "Violating the letter of this rule is violating the spirit of this rule." present?
- [ ] **Announcement** -- explicit exact wording present, not implied?
- [ ] **Gate Function** -- `## BEFORE PROCEEDING` with numbered conditions and ✓/✗ branches?
- [ ] **Rationalization Prevention** -- table with ≥5 rows, specific excuses and specific counters?
- [ ] **Red Flags→STOP** -- section with ≥5 trigger thoughts, each with a concrete action?
- [ ] **No weak language** -- run: `grep -n "should\|prefer\|consider\|try to\|might be worth\|could potentially" "{{SKILL_PATH}}"` -- any hit in a rule body is a FAIL?
- [ ] **Acronym Rule** -- all terms spelled out on first use per VOICE_AUTHORITY_RULES?
- [ ] **No absolute paths** -- no literal `/home/`, `/usr/`, `/root/` or machine-specific prefixes?
- [ ] **No cross-skill file path refs** -- other skills referenced by name in prose only, not by `../other-skill/references/FILE.md` paths?
- [ ] **Skill-specific content** -- no generic placeholder-only examples?
- [ ] **Related skill cross-reference** -- at least one related skill named?
- [ ] **Domain language** -- matches skill scope; no wrong-platform commands or terminology?

✓ All pass → verdict: PASS
✗ Any unmet → verdict: NEEDS WORK -- list every failing item

---

## Qualitative Review

After the checklist, read the skill again and answer:

**1. Gating effectiveness**
- Does the Iron Law stop the most common failure mode for this domain?
- Does the BEFORE PROCEEDING gate ask the right questions or rubber-stamp most cases?
- Would an agent under pressure to move fast rationalize past this skill?

**2. Alexandrian form gaps**
- Does each major rule state when it applies and when it does NOT?
- Does each rule explain the tension it resolves?
- Identify up to 3 bare imperatives without context/forces. Suggest how to expand them.

**3. Rationalization table quality**
- Are the excuses realistic things agents actually say?
- Are the counters specific, not just "don't do that"?
- Flag any row where the counter could itself be rationalized around.

**4. Improvement suggestions**
List 1-5 concrete, actionable suggestions. Each must name the specific section or line,
state the problem, and provide example replacement text or a clear direction.
Do not suggest padding or structural bloat.

---

## Red Flags -- Stop Before Filing

- "It has most of the required elements" -- count every element. Partial is FAIL.
- "The Iron Law is present but mild" -- `YOU MUST` and `No exceptions.` must be INSIDE the backtick block.
- "4 rows is close enough" -- 4 is a FAIL. ≥5 is the rule.
- "The announcement is implied" -- implied is a FAIL. Must be an explicit line.
- "I'll note issues but still give PASS" -- no PASS with open issues.
- "The path is just an example" -- any machine-specific absolute path is a FAIL.
- "The gate function exists under a different heading" -- must be `## BEFORE PROCEEDING`.
- "`YOU MUST` and `No exceptions.` are in prose, not the block" -- prose-only is a FAIL.
- "The skill is long but comprehensive" -- SIZE ALERT regardless of content quality.
- "The description explains what the skill does" -- must start "Use when...".
- "`should` is just writing style" -- soft language in rule bodies is a FAIL.
- "The acronym is obvious" -- spell it out. No exceptions.

---

## Return Format

Return findings in EXACTLY this structure:

```markdown
## Skill Review: [skill-name]

### Size and Frontmatter
- Lines: [N] | Bytes: [N] | Size status: OK / SIZE ALERT
- Frontmatter: ✅/❌ -- [note any missing or malformed fields]

### Gate Elements
| Element | Present | Quality | Notes |
|---------|---------|---------|-------|
| Iron Law | ✅/❌ | Strong/Weak | file:line -- [quote key phrase] |
| Iron Law hard language in block | ✅/❌ | In block / Prose only | file:line |
| Iron Law letter/spirit line | ✅/❌ | Present/Absent | file:line |
| Announcement | ✅/❌ | Clear/Vague | file:line |
| Gate Function | ✅/❌ | Present/Absent | file:line |
| Rationalization Table | ✅/❌ | N rows | file:line |
| Red Flags→STOP | ✅/❌ | N items | file:line |

### Checklist
| Item | Result | Evidence |
|------|--------|----------|
| Frontmatter name + description | ✅/❌ | file:line |
| Size within limits | ✅/❌ | [N lines, N bytes] |
| Iron Law hard language in block | ✅/❌ | file:line |
| Iron Law letter/spirit line | ✅/❌ | file:line |
| Announcement explicit | ✅/❌ | file:line |
| Gate Function BEFORE PROCEEDING | ✅/❌ | file:line |
| Rationalization ≥5 rows | ✅/❌ | file:line |
| Red Flags ≥5 items | ✅/❌ | file:line |
| No weak language | ✅/❌ | grep output or "no instances found" |
| Acronym Rule | ✅/❌ | file:line or "no issues found" |
| No absolute paths | ✅/❌ | grep output or "no issues found" |
| No cross-skill path refs | ✅/❌ | grep output or file:line |
| Skill-specific content | ✅/❌ | file:line |
| Related skill cross-reference | ✅/❌ | file:line |
| Domain language match | ✅/❌ | file:line or "no issues found" |

### Issues Found
[Each issue: file:line -- description]

### Qualitative Assessment

**Gating effectiveness:** [1-2 sentences]

**Alexandrian form gaps:** [up to 3 rules; file:line and what context/forces are missing]

**Rationalization table quality:** [note weak counters or unrealistic excuses]

**Improvement suggestions:**
1. [file:line -- problem -- example fix]
2. ...

### Verdict: PASS / NEEDS WORK
```

NEEDS WORK means the skill MUST be updated before production dispatch.
