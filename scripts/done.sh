#!/bin/bash
set -e
workdir=$(dirname $(dirname $(readlink -f "$0")))

echo "========================= SUCCESS ========================"
echo "[1] $workdir/core/kernel-space/keld.ko"
echo "[2] $workdir/core/user-space/build/keld"
echo "[3] $workdir/apps/cmd/sample/keld-sample"
echo "[4] $workdir/apps/cmd/telegram/keld-telegram"
echo "[5] $workdir/apps/cmd/web/keld-web"
echo "[6] $workdir/apps/cmd/notify/keld-notify"
echo "=========================================================="
