# Local branch (Timo's single-player server)

`local` = upstream `master` + changes for this server only (boss-tactic fixes, trial merges of
open pull requests). It is what the server builds. Sharing a fix upstream is optional and goes
through a separate branch off `test-staging`.

- Plan, workflow and experiments queue: Obsidian vault,
  `Arvutimängud/World of Warcraft/solo-bot-worlds.md`, section "Bot tactics: our own branch".
- Rules for any code change here: `AGENTS.md` and `.agents/docs/` (loaded automatically by
  Claude Code through `CLAUDE.md`), and `PULL_REQUEST_TEMPLATE.md` (test steps, pmon numbers,
  AI disclosure).
- Update from upstream: `git fetch --depth 50 origin master && git merge origin/master`.
