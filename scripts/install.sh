#!/bin/bash
set -eu
workdir=$(dirname $(dirname $(readlink -f "$0")))

cp $workdir/core/kernel-space/keld.ko /lib/modules/$(uname -r)/keld.ko
depmod

mkdir -p /usr/bin/
cp $workdir/core/user-space/build/keld /usr/bin/keld
cp $workdir/apps/cmd/sample/keld-sample /usr/bin/keld-sample
cp $workdir/apps/cmd/telegram/keld-telegram /usr/bin/keld-telegram
cp $workdir/apps/cmd/web/keld-web /usr/bin/keld-web
cp $workdir/apps/cmd/notify/keld-notify /usr/bin/keld-notify

mkdir -p /etc/keld/
cp $workdir/apps/configs/telegram.yaml /etc/keld/telegram.yaml
cp $workdir/apps/configs/web.yaml /etc/keld/web.yaml
cp $workdir/apps/configs/notify.yaml /etc/keld/notify.yaml

mkdir -p /etc/modules-load.d/
mkdir -p /usr/lib/systemd/system/
mkdir -p /usr/lib/systemd/user/
cp $workdir/core/configs/modules-load/keld.conf /etc/modules-load.d/keld.conf
cp $workdir/core/configs/systemd/keld.service /usr/lib/systemd/system/keld.service
cp $workdir/apps/init/keld-telegram.service /usr/lib/systemd/system/keld-telegram.service
cp $workdir/apps/init/keld-web.service /usr/lib/systemd/system/keld-web.service
cp $workdir/apps/init/keld-notify.service /usr/lib/systemd/user/keld-notify.service

systemctl daemon-reload
