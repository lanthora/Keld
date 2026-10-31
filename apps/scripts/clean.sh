#!/bin/bash
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
for module in "$root"/cmd/*; do
    name=$(basename "$module")
    printf '[remove] keld-%s\n' "$name"
    rm -f "$module/keld-$name"
done
rm -rf "$root"/internal/web/webui/*
