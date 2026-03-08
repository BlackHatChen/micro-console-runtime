#!/usr/bin/env bash
set -euo pipefail

# Ensure we're at repo root
ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

# 1) Point hooks to the versioned directory
git config core.hooksPath .githooks

# 2) Ensure hooks are executable
chmod +x .githooks/*

echo "[OK] core.hooksPath is set to .githooks"
echo "[OK] hooks are executable"