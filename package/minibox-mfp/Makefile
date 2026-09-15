include $(TOPDIR)/rules.mk

PKG_NAME:=minibox-mfp
PKG_VERSION:=0.3.0
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/minibox-mfp
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=MiniBox MFP Wi-Fi print and scan bridge
  DEPENDS:=+libusb-1.0
endef

define Package/minibox-mfp/description
 Minimal IPP/eSCL/mDNS runtime and libusb backend for HP LaserJet M1522n.
 No CUPS and no usblp dependency.
endef

define Build/Prepare
	$(RM) -r $(PKG_BUILD_DIR)
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) $(CURDIR)/../../src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -std=c99 -Wall -Wextra

MB_CC=$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(TARGET_LDFLAGS)

define Build/Compile
	$(MB_CC) -o $(PKG_BUILD_DIR)/minibox-printerd \
		$(PKG_BUILD_DIR)/minibox-printerd/main.c \
		$(PKG_BUILD_DIR)/minibox-ipp/ipp.c \
		$(PKG_BUILD_DIR)/minibox-ipp/print_job.c \
		$(PKG_BUILD_DIR)/minibox-usb/stream.c \
		$(PKG_BUILD_DIR)/minibox-usb/libusb_m1522.c \
		$(PKG_BUILD_DIR)/minibox-usb/print_m1522.c -lusb-1.0
	$(MB_CC) -o $(PKG_BUILD_DIR)/minibox-scand \
		$(PKG_BUILD_DIR)/minibox-scand/main.c \
		$(PKG_BUILD_DIR)/minibox-scand/scan_session.c \
		$(PKG_BUILD_DIR)/minibox-escl/escl.c
	$(MB_CC) -o $(PKG_BUILD_DIR)/minibox-discoveryd \
		$(PKG_BUILD_DIR)/minibox-discoveryd/main.c \
		$(PKG_BUILD_DIR)/minibox-discoveryd/service.c \
		$(PKG_BUILD_DIR)/minibox-discoveryd/mdns.c
	$(MB_CC) -o $(PKG_BUILD_DIR)/minibox-scan-diag \
		$(PKG_BUILD_DIR)/minibox-usb/scan_diag.c \
		$(PKG_BUILD_DIR)/minibox-usb/scan_m1522.c -lusb-1.0
endef

define Package/minibox-mfp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minibox-printerd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minibox-scand $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minibox-discoveryd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minibox-scan-diag $(1)/usr/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,minibox-mfp))
