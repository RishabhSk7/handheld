FBPCP_ILI9341_VERSION = master
FBPCP_ILI9341_SITE = https://github.com/juj/fbcp-ili9341.git
FBPCP_ILI9341_SITE_METHOD = git

FBPCP_ILI9341_DEPENDENCIES = host-cmake

define FBPCP_ILI9341_CONFIGURE_CMDS
	mkdir -p $(@D)/build
	cd $(@D)/build && cmake ..
endef

define FBPCP_ILI9341_BUILD_CMDS
	$(MAKE) -C $(@D)/build
endef

define FBPCP_ILI9341_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/build/fbcp-ili9341 $(TARGET_DIR)/usr/bin/fbcp-ili9341
endef

$(eval $(generic-package))
