ifneq ($(KERNELRELEASE),)

obj-m := keld.o
keld-objs += base/main.o base/netlink.o base/syscall.o base/ftrace.o base/watchdog.o
keld-objs += watchdog/watchdog.o
keld-objs += handshake/core.o handshake/netlink.o
keld-objs += process/core.o process/netlink.o process/utils.o
keld-objs += file/core.o file/netlink.o file/utils.o
keld-objs += net/core.o net/netlink.o

ccflags-y += $(KELD_MODULE_CFLAGS)
ccflags-y += -I$(src)
ccflags-y += $(call cc-option,-fmacro-prefix-map=$(PWD)/=)
# Original calls must keep a return address inside this module for the
# ftrace recursion check, including when building the legacy backend.
ccflags-y += -fno-optimize-sibling-calls

else

KERNELRELEASE ?= $(shell uname -r)
KDIR ?= /lib/modules/$(KERNELRELEASE)/build
DRIVER_VERSION ?= $(shell grep -Po '(?<=^PACKAGE_VERSION=")(.*)(?="$$)' dkms.conf)

release:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	strip keld.ko --strip-unneeded

debug:
	$(MAKE) -C $(KDIR) M=$(PWD) KELD_MODULE_CFLAGS="-DDEBUG" modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install: clean
	mkdir -p /usr/src/keld-$(DRIVER_VERSION)
	cp -r * /usr/src/keld-$(DRIVER_VERSION)
	dkms add -m keld -v $(DRIVER_VERSION)
	dkms build -m keld -v $(DRIVER_VERSION)
	dkms install -m keld -v $(DRIVER_VERSION)
	dkms status

remove:
	dkms remove -m keld -v $(DRIVER_VERSION) --all
	rm -rf /usr/src/keld-$(DRIVER_VERSION)

endif
