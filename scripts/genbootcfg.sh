#!/usr/bin/env bash
set -euo pipefail

OUT_PATH=${1:?Usage: $0 out_path boot_guid boot_path}
BOOT_DISK_GUID=${2:?Usage: $0 out_path boot_guid boot_path}
BOOT_PATH=${3:?Usage: $0 out_path boot_guid boot_path}

echo $OUT_PATH

TMP_FILE=$(mktemp)

cat >"$TMP_FILE" <<EOF
BOOT_DISK = GUID{$BOOT_DISK_GUID}
BOOT_PATH = UTF16{$BOOT_PATH}
EOF

nyxtools-mkcfg -i "$TMP_FILE" -o "$OUT_PATH"

rm -f "$TMP_FILE"