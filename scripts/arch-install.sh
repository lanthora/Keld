#!/bin/bash
set -e
workdir=$(dirname $(dirname $(readlink -f "$0")))

cd $workdir/core/kernel-space/
make clean
DRIVER_VERSION=$(grep -Po '(?<=^PACKAGE_VERSION=")(.*)(?="$)' dkms.conf)
mkdir -p $DESTDIR/usr/src/keld-$DRIVER_VERSION
cp -r $workdir/core/kernel-space/* $DESTDIR/usr/src/keld-$DRIVER_VERSION

mkdir -p $DESTDIR/usr/bin/
cp $workdir/core/user-space/build/keld $DESTDIR/usr/bin/keld
cp $workdir/apps/cmd/sample/keld-sample $DESTDIR/usr/bin/keld-sample
cp $workdir/apps/cmd/telegram/keld-telegram $DESTDIR/usr/bin/keld-telegram
cp $workdir/apps/cmd/web/keld-web $DESTDIR/usr/bin/keld-web
cp $workdir/apps/cmd/notify/keld-notify $DESTDIR/usr/bin/keld-notify

mkdir -p $DESTDIR/etc/keld/
cp $workdir/apps/configs/telegram.yaml $DESTDIR/etc/keld/telegram.yaml
cp $workdir/apps/configs/web.yaml $DESTDIR/etc/keld/web.yaml
cp $workdir/apps/configs/notify.yaml $DESTDIR/etc/keld/notify.yaml

mkdir -p $DESTDIR/etc/modules-load.d/
mkdir -p $DESTDIR/usr/lib/systemd/system/
mkdir -p $DESTDIR/usr/lib/systemd/user/
cp $workdir/core/configs/modules-load/keld.conf $DESTDIR/etc/modules-load.d/keld.conf
cp $workdir/core/configs/systemd/keld.service $DESTDIR/usr/lib/systemd/system/keld.service
cp $workdir/apps/init/keld-telegram.service $DESTDIR/usr/lib/systemd/system/keld-telegram.service
cp $workdir/apps/init/keld-web.service $DESTDIR/usr/lib/systemd/system/keld-web.service
cp $workdir/apps/init/keld-notify.service $DESTDIR/usr/lib/systemd/user/keld-notify.service
