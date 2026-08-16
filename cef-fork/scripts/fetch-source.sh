#!/bin/bash
# Download CEF and Chromium source for branch 7922 (Chromium 151.0.7922.x).
# This step does NOT build. It needs ~30-50 GB disk space and a fast connection.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
WORK_DIR="${WORK_DIR:-$ROOT_DIR/work}"
BRANCH="${CEF_BRANCH:-7922}"
mkdir -p "$WORK_DIR"

# Depot tools
if [ ! -d "$WORK_DIR/depot_tools" ]; then
  git clone --depth=1 https://chromium.googlesource.com/chromium/tools/depot_tools.git "$WORK_DIR/depot_tools"
fi

# CEF automate script
mkdir -p "$WORK_DIR/automate"
if [ ! -f "$WORK_DIR/automate/automate-git.py" ]; then
  curl -L -o "$WORK_DIR/automate/automate-git.py" \
    https://raw.githubusercontent.com/chromiumembedded/cef/master/tools/automate/automate-git.py
  chmod +x "$WORK_DIR/automate/automate-git.py"
fi

export PATH="$WORK_DIR/depot_tools:$PATH"

# Download source only. This runs gclient sync and can take a long time.
python3 "$WORK_DIR/automate/automate-git.py" \
  --download-dir="$WORK_DIR/chromium_git" \
  --depot-tools-dir="$WORK_DIR/depot_tools" \
  --branch="$BRANCH" \
  --no-distrib \
  --no-build

echo "Source downloaded to $WORK_DIR/chromium_git/chromium/src"
