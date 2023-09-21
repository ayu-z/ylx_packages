include $(TOPDIR)/rules.mk

PKG_NAME:= ylx_hotplug
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/ylx_hotplug
  CATEGORY:=LINK4ALL
  TITLE:=ylx_hotplug
#   DEPENDS:=+libudev
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		CC="$(TARGET_CC)"
endef

define Package/ylx_hotplug/install
	$(INSTALL_DIR) $(1)/usr/bin 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ylx_hotplug $(1)/usr/bin
endef

$(eval $(call BuildPackage,ylx_hotplug))