#!/usr/bin/env bash
set -euo pipefail

say() { printf '%s\n' "$*"; }
ok()  { say "[OK]  $*"; }
warn(){ say "[WARN] $*"; }
err() { say "[ERROR] $*"; exit 1; }

# 0) Must be inside a git repo
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || err "Not inside a Git repository."
ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT" || err "Cannot cd to repo root: $ROOT"

# 1) hooks path
[[ -d .githooks ]] || err ".githooks/ directory not found at repo root: $ROOT"
PREV_HOOKS_PATH="$(git config --local --get core.hooksPath || true)"
git config --local core.hooksPath .githooks
ok "core.hooksPath was: ${PREV_HOOKS_PATH:-<unset>}"
ok "core.hooksPath is set to .githooks"

# 2) make hooks executable (files only)
shopt -s nullglob
hooks=(.githooks/*)
if ((${#hooks[@]})); then
  chmod +x "${hooks[@]}"
  ok "hooks are executable (${#hooks[@]} files)"
else
  warn ".githooks/ contains no hook files"
fi
shopt -u nullglob

# 3) commit message template (optional)
if [[ -f .gitmessage ]]; then
  git config --local commit.template .gitmessage
  ok "commit.template set to .gitmessage"
else
  warn "no .gitmessage found (optional)"
fi

# 4) quick sanity: print key configs
say "---- git local config ----"
git config --local --get core.hooksPath || true
git config --local --get commit.template || true
say "--------------------------"