#!/bin/bash
set -eu
systemctl disable --now keld-web.service keld-telegram.service keld.service || true
if lsmod | awk '{print $1}' | grep -qx keld; then
    modprobe -r keld
fi
rm -f /lib/modules/$(uname -r)/keld.ko
depmod -a
rm -f /usr/bin/keld /usr/bin/keld-{sample,telegram,web,notify}
rm -f /etc/keld/{telegram,web,notify}.yaml
rmdir /etc/keld 2>/dev/null || true
rm -f /etc/modules-load.d/keld.conf
rm -f /usr/lib/systemd/system/keld{,-telegram,-web}.service
rm -f /usr/lib/systemd/user/keld-notify.service
systemctl daemon-reload
