#!/bin/bash
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
for module in "$root"/cmd/*; do
    name=$(basename "$module")
    printf '[build] keld-%s\n' "$name"
    cd "$root"
    go build -buildvcs=false -o "$module/keld-$name" -ldflags="-X github.com/lanthora/keld/apps/pkg/logger.BuildDir=$root/" "./cmd/$name"
done
