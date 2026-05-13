.RECIPEPREFIX := >
.DEFAULT_GOAL := all

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

.PHONY: core-check
core-check:
>gcc -std=c99 -Wall -Wextra -Wpedantic -Iinclude -c src/core/usb_descriptors.c src/core/usb_hid.c src/core/disney_infinity.c src/core/figure_storage.c
>rm -f usb_descriptors.o usb_hid.o disney_infinity.o figure_storage.o

ifeq ($(wildcard $(OPENCM3_DIR)/mk/libopencm3-targets.mk),)
.PHONY: all
all:
>@echo "libopencm3 not found at $(OPENCM3_DIR)"
>@echo "Set OPENCM3_DIR to your libopencm3 checkout path."
>@false
else
include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/libopencm3-targets.mk
endif
