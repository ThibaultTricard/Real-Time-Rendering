# Workflow

## 1 Plan Mode Default
- Enter plan mode for ANY non-trivial task (3+ steps or architectural decisions)
- If something goes sideways, STOP and re-plan immediately — don’t keep pushing
- Use plan mode for verification steps, not just building

## 2 Subagent Strategy
- Use subagents liberally to keep the main context window clean
- Offload research, exploration, and parallel analysis to subagents
- For complex problems, throw more compute at it via subagents
- One task per subagent for focused execution

## 3 Self-Improvement Loop
- After ANY correction from the user, update tasks/lessons.md with the pattern
- Write rules for yourself that prevent the same mistake
- Ruthlessly iterate on these lessons until the mistake rate drops
- Review lessons at the session start for the relevant project

## 4 Bug Fixing
- When given a bug report: 
    - If the bug is trivial, just fix it. Don’t ask for hand-holding 
    - If the bug is complex, enter plan mode, formulate hypothesis and propose atomic test to pinpoint the causes.

## 5 Long Term Memory
- At the start of each session, read previously.md and next.md if they exist
- After each significant task, update previously.md (what was done) and next.md (what remains)
- Create these files at the project root if they don't exist yet

# Core Principles
- No Laziness: Find root causes. No temporary fixes. Senior developer standards.
- Minimal Impact: Changes should only touch what’s necessary. Avoid introducing bugs.


# Graphify

This project has a graphify knowledge graph at graphify-out/.

Rules:
- Before answering architecture or codebase questions, read graphify-out/GRAPH_REPORT.md for god nodes and community structure
- If graphify-out/wiki/index.md exists, navigate it instead of reading raw files
- For cross-module "how does X relate to Y" questions, prefer `graphify query "<question>"`, `graphify path "<A>" "<B>"`, or `graphify explain "<concept>"` over grep — these traverse the graph's EXTRACTED + INFERRED edges instead of scanning files
- After modifying code files in this session, run `graphify update .` to keep the graph current (AST-only, no API cost)
