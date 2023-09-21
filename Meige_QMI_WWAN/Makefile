#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qmi_wwan_m
PKG_RELEASE:=1.0.2
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/qmi_wwan_m
  SUBMENU:=WWAN Support
  CATEGORY:=LINK4ALL
  TITLE:=Meige Linux USB QMI WWAN Driver
  DEPENDS:=+kmod-usb-net +kmod-usb-wdm
  FILES:=$(PKG_BUILD_DIR)/qmi_wwan_m.ko
  AUTOLOAD:=$(call AutoLoad,81,qmi_wwan_m,1)
  KCONFIG:=
endef

define KernelPackage/qmi_wwan_m/description
 qmi_wwan_m driver
endef


define Build/Prepare
	$(CP) src/* $(PKG_BUILD_DIR)
	$(call Build/Prepare/Default)
endef

# define Build/Compile
# 	$(MAKE) -C "$(LINUX_DIR)" \
# 		CROSS_COMPILE="$(TARGET_CROSS)" \
# 		ARCH="$(LINUX_KARCH)" \
# 		SUBDIRS="$(PKG_BUILD_DIR)" \
# 		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
# 		modules
# endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,qmi_wwan_m))
