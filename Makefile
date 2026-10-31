UNAME_S := $(shell uname -s 2>/dev/null)
JBROOT_DIR := $(shell find /var/containers/Bundle/Application/ -maxdepth 1 -name '.jbroot-*' -print -quit 2>/dev/null)

ifneq ($(JBROOT_DIR),)
  HOST_KIND := iOS
  SDK ?= $(JBROOT_DIR)/usr/share/SDKs/iPhoneOS.sdk
  CC  := clang
else ifeq ($(UNAME_S),Darwin)
  HOST_KIND := macOS
  SDK ?= $(shell xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
  CC  := $(shell xcrun --sdk iphoneos -f clang 2>/dev/null)
else
  HOST_KIND := Linux
  SDK ?=
  CC  := clang
endif

ARCH     ?= arm64
ifeq ($(ROOTHIDE),1)
    PKG_ARCH := iphoneos-arm64e
else
    PKG_ARCH := iphoneos-arm64
endif

BASE_VER := $(shell cat VERSION 2>/dev/null || echo "1.0.0")
GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "nogit")
VERSION  := $(BASE_VER)-$(GIT_HASH)

TARGET   ?= strmap
SRCS := strmap.c lang.c lang/en.c lang/zh.c
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

CFLAGS += -D_POSIX_C_SOURCE=200809L -O2 -Wall -I. -MMD -MP
CFLAGS += -arch $(ARCH) -target $(ARCH)-apple-ios14.0
LDFLAGS += -arch $(ARCH) -target $(ARCH)-apple-ios14.0

ifneq ($(SDK),)
  ifneq ($(wildcard $(SDK)),)
    CFLAGS += -isysroot $(SDK)
    LDFLAGS += -isysroot $(SDK)
  endif
endif

.PHONY: all check-sdk sign clean info deb FORCE

all: check-sdk version.h $(TARGET) sign

check-sdk:
	@echo "[host] $(HOST_KIND) SDK=$(SDK)"

version.h: FORCE
	@echo "  GEN version.h ($(VERSION))"
	@echo "#define APP_VERSION \"$(VERSION)\"" > version.h

FORCE:

%.o: %.c version.h
	@echo "  CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@echo "  LD  $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS)

sign: $(TARGET)
	@echo "  SIGN $(TARGET) (ldid)"
	@ldid -Sentitlements.plist $(TARGET)

deb: sign
	@echo "  PKG  $(TARGET)_$(PKG_ARCH).deb"
	@rm -rf layout
	@mkdir -p layout/DEBIAN layout/usr/bin
	@sed -e 's/^Version: .*/Version: $(VERSION)/' -e 's/Architecture: .*/Architecture: $(PKG_ARCH)/g' control > layout/DEBIAN/control
	@cp $(TARGET) layout/usr/bin/strmap
	@chmod 755 layout/usr/bin/strmap
	@dpkg-deb --root-owner-group -b layout $(TARGET)_$(PKG_ARCH).deb
	@rm -rf layout

clean:
	@echo "  CLEAN"
	@rm -f $(OBJS) $(DEPS) $(TARGET) version.h
	@find . -type f \( -name "*.o" -o -name "*.d" \) -delete
	@rm -rf layout strmap_*.deb

-include $(DEPS)
