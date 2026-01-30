#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ ! -f "$ROOT_DIR/run_app.sh" ]]; then
  echo "Error: run_app.sh not found in: $ROOT_DIR" >&2
  exit 1
fi

exec bash "$ROOT_DIR/run_app.sh"
