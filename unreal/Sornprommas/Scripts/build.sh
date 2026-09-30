#!/usr/bin/env bash
# Build the editor target and run the one-time asset setup.
# Usage: Scripts/build.sh [/path/to/UE_5.x]
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
UPROJECT="$PROJECT_DIR/Sornprommas.uproject"
ENGINE="${1:-$(ls -d "/Users/Shared/Epic Games"/UE_5.* 2>/dev/null | sort -V | tail -1)}"
if [ -z "$ENGINE" ] || [ ! -d "$ENGINE" ]; then
  echo "Unreal Engine not found. Pass the engine folder, e.g. Scripts/build.sh '/Users/Shared/Epic Games/UE_5.6'" >&2
  exit 1
fi
echo "Engine: $ENGINE"
"$ENGINE/Engine/Build/BatchFiles/Mac/Build.sh" SornprommasEditor Mac Development -Project="$UPROJECT" -WaitMutex
if [ ! -f "$PROJECT_DIR/Content/Maps/Battle.umap" ]; then
  "$ENGINE/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" "$UPROJECT" \
    -run=pythonscript -script="$PROJECT_DIR/Scripts/setup_project.py" -unattended -nosplash
fi
echo "Done. Open with: open '$UPROJECT'"
