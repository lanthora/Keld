#!/bin/sh
set -eu
workdir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$workdir/webui"
npm ci
npm run build:embed
cd "$workdir/apps"
go build -buildvcs=false -o cmd/web/keld-web ./cmd/web
