RETROARCH_VERSION = v1.22.2
RETROARCH_SITE = https://github.com/libretro/RetroArch
RETROARCH_SITE_METHOD = git
RETROARCH_DEPENDENCIES = sdl2 freetype mesa3d libdrm

RETROARCH_CONF_ENV = \
    $(TARGET_MAKE_ENV) \
    HOME=/nonexistent \
    PKG_CONFIG="$(HOST_DIR)/bin/pkg-config" \
    CC="$(TARGET_CC)" \
    CXX="$(TARGET_CXX)" \
    CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
    LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
    PREFIX=/usr

RETROARCH_CONF_OPTS = \
    --host=$(GNU_TARGET_NAME) \
    --disable-x11 \
    --disable-wayland \
    --disable-videocore \
    --enable-kms \
    --enable-egl \
    --enable-opengles \
    --disable-neon

define RETROARCH_CONFIGURE_CMDS
    cd $(@D) && $(RETROARCH_CONF_ENV) ./configure $(RETROARCH_CONF_OPTS)
endef

# This is the magic part: it finds any path that looks like /usr/include
# or /usr/lib in the config and prefixes it with the STAGING_DIR
define RETROARCH_FIXUP_PATHS
    sed -i "s|= /usr/include|= $(STAGING_DIR)/usr/include|g" $(@D)/config.mk
    sed -i "s|= /usr/lib|= $(STAGING_DIR)/usr/lib|g" $(@D)/config.mk
    sed -i "s|-I/usr/include|-I$(STAGING_DIR)/usr/include|g" $(@D)/config.mk
    sed -i "s|-L/usr/lib|-L$(STAGING_DIR)/usr/lib|g" $(@D)/config.mk
    sed -i 's|^HAVE_NEON := 0|HAVE_NEON := 1|g' $(@D)/config.mk
endef
RETROARCH_POST_CONFIGURE_HOOKS += RETROARCH_FIXUP_PATHS

define RETROARCH_BUILD_CMDS
    $(RETROARCH_CONF_ENV) $(MAKE) -C $(@D)
endef

define RETROARCH_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
        DESTDIR=$(TARGET_DIR) \
        PREFIX=/usr \
        BIN_DIR=/usr/bin \
        DATA_DIR=/usr/share/retroarch \
        DOC_DIR=/usr/share/doc/retroarch \
        MAN_DIR=/usr/share/man \
        install
endef

$(eval $(generic-package))
