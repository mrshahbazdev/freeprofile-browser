#!/bin/bash
# Apply FreeProfile anti-detect patches to the downloaded Chromium source and
# build a Release CEF binary.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
WORK_DIR="${WORK_DIR:-$ROOT_DIR/work}"
BRANCH="${CEF_BRANCH:-7922}"
CHROMIUM_SRC="$WORK_DIR/chromium_git/chromium/src"

if [ ! -d "$CHROMIUM_SRC" ]; then
  echo "Chromium source not found. Run fetch-source.sh first."
  exit 1
fi

export PATH="$WORK_DIR/depot_tools:$PATH"

# Apply FreeProfile engine-level fingerprint patches.
python3 "$ROOT_DIR/scripts/apply-patches.py" "$CHROMIUM_SRC"

# Build CEF.  --no-update means do not re-run gclient sync (which would revert our patches).
# --no-debug-build builds only Release.  --build-target=cefsimple keeps the build small.
python3 "$WORK_DIR/automate/automate-git.py" \
  --download-dir="$WORK_DIR/chromium_git" \
  --depot-tools-dir="$WORK_DIR/depot_tools" \
  --branch="$BRANCH" \
  --no-update \
  --no-distrib \
  --no-debug-build \
  --force-build \
  --build-target=cefsimple

echo "Build complete. Output is under $CHROMIUM_SRC/out/Release_GN_x64/"
