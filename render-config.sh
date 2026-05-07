#!/usr/bin/env bash
# Render code/Gateway/ESP1/config.h from config.h.tpl using 1Password CLI.
# Run this before compiling/flashing the sketch from Arduino IDE.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TPL="$REPO_ROOT/code/Gateway/ESP1/config.h.tpl"
OUT="$REPO_ROOT/code/Gateway/ESP1/config.h"

if ! command -v op >/dev/null 2>&1; then
  echo "error: 1Password CLI ('op') not found. install with: brew install 1password-cli" >&2
  exit 1
fi

if ! op whoami >/dev/null 2>&1; then
  echo "error: not signed in to 1Password. run: eval \$(op signin)" >&2
  exit 1
fi

if [[ ! -f "$TPL" ]]; then
  echo "error: template not found at $TPL" >&2
  exit 1
fi

echo "rendering $OUT from $(basename "$TPL")..."
op inject -i "$TPL" -o "$OUT" --force
echo "done. config.h is gitignored; do not commit."
