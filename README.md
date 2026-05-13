# Disney Infinity Base (STM32F103 + libopencm3)

This repository contains a modular embedded C implementation of Disney Infinity USB base behavior, now structured around a **portable core** and a **libopencm3 STM32F103 platform layer**.

## Protocol behavior mirrored from Dolphin

- **VID/PID:** `0x0E6F:0x0129`
- **USB class:** HID
- **Endpoints:** Interrupt IN `0x81`, Interrupt OUT `0x01`
- **Packet size:** 32 bytes
- **Frames:**
  - Host command prefix: `0xFF`
  - Device response prefix: `0xAA`
  - Figure add/remove event prefix: `0xAB`
- **Checksum:** additive (`sum(payload) & 0xFF`)
- **Figure layout:** `20 * 16 = 320` bytes
- **Block mapping:** `0 -> 1`, otherwise `n -> n * 4`

## New project structure

- `include/`
  - `disney_infinity.h`
  - `figure_storage.h`
  - `usb_hid.h`
  - `usb_descriptors.h`
  - `platform/stm32f103/usb_base_platform.h`
- `src/core/`
  - Protocol/queue/state machine and figure storage modules
- `src/platform/stm32f103/`
  - `usb_base_platform.c`: libopencm3 USB FS integration for STM32F103
  - `main.c`: firmware entrypoint loop
- `Makefile`
  - libopencm3-oriented firmware build and host-side core compile check

## Build requirements

- `arm-none-eabi-gcc`
- `make`
- `libopencm3` built and available at `OPENCM3_DIR` (default: `/opt/libopencm3`)

## Build firmware (STM32F103)

```bash
make OPENCM3_DIR=/path/to/libopencm3
```

## Validate portable core modules on host

```bash
make core-check
```

## Notes

- The STM32 integration now uses libopencm3 USB device APIs and a dedicated STM32F103 platform module.
- Core protocol logic remains hardware-agnostic and can be reused on other targets by replacing `src/platform/*`.
