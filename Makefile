PROJECT := disney_infinity_base
DEVICE ?= stm32f103c8t6
OPENCM3_DIR ?= /opt/libopencm3

CPPFLAGS += -Iinclude -MMD -MP
CFLAGS += -std=c99 -Os -g3 -ffunction-sections -fdata-sections -Wall -Wextra -Wpedantic
LDFLAGS += -Wl,--gc-sections

SRC := \
  src/core/disney_infinity.c \
  src/core/figure_storage.c \
  src/core/usb_hid.c \
  src/core/usb_descriptors.c \
  src/platform/stm32f103/main.c \
  src/platform/stm32f103/usb_base_platform.c

BINARY = $(PROJECT)

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/libopencm3-targets.mk

.PHONY: core-check
core-check:
gcc -std=c99 -Wall -Wextra -Wpedantic -Iinclude -c src/core/usb_descriptors.c src/core/usb_hid.c src/core/disney_infinity.c src/core/figure_storage.c
rm -f usb_descriptors.o usb_hid.o disney_infinity.o figure_storage.o
