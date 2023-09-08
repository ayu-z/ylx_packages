include $(TOPDIR)/rules.mk

PKG_NAME:= quectel-CM
PKG_VERSION:=1.6.4
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/quectel-CM
  SECTION:=utils
  CATEGORY:=iyunlink
  TITLE:=quectel-CM
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

define Package/quectel-CM/install
	$(INSTALL_DIR) $(1)/usr/bin 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/bin
endef

$(eval $(call BuildPackage,quectel-CM))