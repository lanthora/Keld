# Copyright 2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=8

DESCRIPTION="Host Intrusion Detection and Prevention System"
HOMEPAGE="https://github.com/lanthora/Keld"
SRC_URI="https://github.com/lanthora/Keld/archive/refs/tags/v${PV}.tar.gz"

S="${WORKDIR}/Keld-${PV}"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~amd64 ~arm64 ~arm"

DEPEND="
	dev-libs/libnl
	sys-kernel/dkms
"
RDEPEND="${DEPEND}"
BDEPEND="
	dev-util/cmake
	sys-devel/make
	dev-lang/go
	dev-cpp/nlohmann_json
	net-libs/nodejs[npm]
"

src_compile(){
	make gentoo-build
}

src_install(){
	DRIVER_VERSION=$(grep -Po '(?<=^PACKAGE_VERSION=")(.*)(?="$)' core/kernel-space/dkms.conf)
	insinto /usr/src/keld-${DRIVER_VERSION}
	doins -r core/kernel-space/*
	dobin core/user-space/build/keld
	dobin apps/cmd/sample/keld-sample
	dobin apps/cmd/telegram/keld-telegram
	dobin apps/cmd/web/keld-web
	dobin apps/cmd/notify/keld-notify
	insinto /etc/keld
	doins apps/configs/telegram.yaml
	doins apps/configs/web.yaml
	doins apps/configs/notify.yaml
	insinto /etc/modules-load.d
	doins core/configs/modules-load/keld.conf
	insinto /usr/lib/systemd/system
	doins core/configs/systemd/keld.service
	doins apps/init/keld-telegram.service
	doins apps/init/keld-web.service
	insinto /usr/lib/systemd/user
	doins apps/init/keld-notify.service
}
