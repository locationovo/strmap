UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(strip $(UNAME_S)),)
$(error cannot determine host OS)
endif

JBROOT_DIR := $(shell find /var/containers/Bundle/Application/ -maxdepth 1 \
                -name '.jbroot-*' \
                -exec test -d '{}/usr/share/SDKs/iPhoneOS.sdk' \; -print -quit \
                2>/dev/null)

ifneq ($(JBROOT_DIR),)
  HOST_KIND := iOS
  SDK ?= $(JBROOT_DIR)/usr/share/SDKs/iPhoneOS.sdk
  CC  := clang
  LD  := clang
else ifeq ($(UNAME_S),Darwin)
  HOST_KIND := macOS
  SDK ?= $(shell xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
  CC  := $(shell xcrun --sdk iphoneos -f clang 2>/dev/null)
  ifeq ($(CC),)
  CC  := clang
  endif
  LD  := $(CC)
else ifeq ($(UNAME_S),Linux)
  HOST_KIND := Linux
  SDK ?=
  CC  := clang
  LD  := clang
else
  HOST_KIND := unsupported
  SDK ?=
  CC  := clang
  LD  := clang
endif

ARCH     ?= arm64
ifeq ($(ROOTHIDE),1)
    PKG_ARCH := iphoneos-arm64e
else
    PKG_ARCH := iphoneos-arm64
endif

BASE_VER := $(shell cat VERSION 2>/dev/null || echo "1.0.0")
GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "nogit")
GIT_DIRTY := $(shell git diff --quiet 2>/dev/null || echo "-dirty")
VERSION := $(BASE_VER)-$(GIT_HASH)$(GIT_DIRTY)

MIN_IOS  ?= 14.0
TARGET   ?= strmap
LIDID    ?= ldid

SRC_DIR    := .
LANG_DIR   := lang

SRCS := \
    strmap.c \
    lang.c \
    $(LANG_DIR)/en.c \
    $(LANG_DIR)/zh.c

OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

INCLUDES := -I$(SRC_DIR)

CFLAGS += -target $(ARCH)-apple-ios$(MIN_IOS) \
         -isysroot $(SDK) \
         -D_POSIX_C_SOURCE=200809L \
         -fobjc-arc -fPIC \
         -MMD -MP \
         -Wall -Wextra \
         -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function \
         $(INCLUDES)

ifeq ($(DEBUG),1)
CFLAGS += -O0 -g3
else
CFLAGS += -Os -g0
endif

LDFLAGS += -target $(ARCH)-apple-ios$(MIN_IOS) \
           -isysroot $(SDK) \
           -dynamiclib

.PHONY: all check-sdk sign clean info deb FORCE

all: check-sdk $(TARGET)

check-sdk:
	@echo "[host] $(HOST_KIND)  SDK=$(SDK)"
	@case "$(HOST_KIND)" in \
	  iOS|macOS) ;; \
	  Linux) echo "!! Linux cannot build iOS dylib" >&2; exit 1 ;; \
	  *) echo "!! unsupported host: '$(UNAME_S)'" >&2; exit 1 ;; \
	esac
	@test -n "$(SDK)" || { echo "!! empty SDK path" >&2; exit 1; }
	@test -d "$(SDK)"  || { echo "!! SDK not a directory: $(SDK)" >&2; exit 1; }
	@test -d "$(SDK)/System/Library/Frameworks/UIKit.framework" || \
	  { echo "!! not a valid iPhoneOS SDK: $(SDK)" >&2; exit 1; }

version.h: FORCE
	@echo "  GEN version.h ($(VERSION))"
	@echo "#define APP_VERSION \"$(VERSION)\"" > version.h

FORCE:

%.o: %.c version.h
	@mkdir -p $(dir $@)
	@echo "  CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): check-sdk $(OBJS)
	@echo "  LD  $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

deb: $(TARGET)
	@echo "  PKG  $(TARGET)_$(PKG_ARCH).deb"
	@rm -rf layout
	@mkdir -p layout/DEBIAN layout/usr/bin
	@sed -e 's/^Version: .*/Version: $(VERSION)/' -e 's/Architecture: .*/Architecture: $(PKG_ARCH)/g' control > layout/DEBIAN/control
	@cp $(TARGET) layout/usr/bin/strmap
	@chmod 755 layout/usr/bin/strmap
	@dpkg-deb -b layout $(TARGET)_$(PKG_ARCH).deb
	@rm -rf layout
	@echo "  [OK] Package created."

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET) strmap_out version.h
	@find . -type f \( -name "*.o" -o -name "*.d" \) -delete
	@rm -rf layout strmap_*.deb

info:
	@echo "host    = $(HOST_KIND)"
	@echo "sdk     = $(SDK)"
	@echo "cc      = $(CC)"
	@echo "arch    = $(ARCH)  min_ios = $(MIN_IOS)"
	@echo "version = $(VERSION)"
	@echo "srcs    ="
	@for f in $(SRCS); do echo "  $$f"; done

-include $(DEPS)
