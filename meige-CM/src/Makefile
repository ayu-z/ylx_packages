ifneq ($(CROSS_COMPILE),)
CROSS-COMPILE:=$(CROSS_COMPILE)
endif
#CROSS-COMPILE:=/workspace/buildroot/buildroot-qemu_mips_malta_defconfig/output/host/usr/bin/mips-buildroot-linux-uclibc-
#CROSS-COMPILE:=/workspace/buildroot/buildroot-qemu_arm_vexpress_defconfig/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-
#CROSS-COMPILE:=/workspace/buildroot-git/qemu_mips64_malta/output/host/usr/bin/mips-gnu-linux-
ifeq ($(CC),cc)
CC:=$(CROSS-COMPILE)gcc
endif
LD:=$(CROSS-COMPILE)ld

QL_CM_SRC=QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c qmap_bridge_mode.c mbim-cm.c device.c
ifeq (1,1)
QL_CM_DHCP=udhcpc.c
else
LIBMNL=libmnl/ifutils.c libmnl/attr.c libmnl/callback.c libmnl/nlmsg.c libmnl/socket.c
DHCP=libmnl/dhcp/dhcpclient.c libmnl/dhcp/dhcpmsg.c libmnl/dhcp/packet.c
QL_CM_DHCP=udhcpc_netlink.c
QL_CM_DHCP+=${LIBMNL}
endif

CFLAGS+=-Wall -Wextra -Wfatal-errors -Wunused -Werror #-Wformat-truncation=2 -Wformat-overflow=2

release: clean qmi-proxy mbim-proxy
	$(CC) ${CFLAGS} -s ${QL_CM_SRC} ${QL_CM_DHCP} -o meig-cm -lpthread -ldl -lrt

debug: clean
	$(CC) ${CFLAGS} -g -DCM_DEBUG ${QL_CM_SRC} ${QL_CM_DHCP} -o meig-cm -lpthread -ldl -lrt

qmi-proxy:
	$(CC) ${CFLAGS} -s meig-qmi-proxy.c  -o meig-qmi-proxy -lpthread -ldl -lrt

mbim-proxy:
	$(CC) ${CFLAGS} -s meig-mbim-proxy.c  -o meig-mbim-proxy -lpthread -ldl -lrt

clean:
	rm -rf *.o libmnl/*.o meig-cm Meig-qmi-proxy Meig-mbim-proxy
