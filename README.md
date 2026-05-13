# Disney Infinity Base (STM32-oriented)

This repository now contains a lightweight embedded C port of the Disney Infinity USB base emulation flow, based on Dolphin Emulator's `Infinity.cpp` behavior.

## What was extracted from Dolphin

- **VID/PID:** `0x0E6F:0x0129`
- **USB class:** HID
- **Endpoints:** Interrupt IN `0x81`, Interrupt OUT `0x01`
- **Packet size:** 32 bytes
- **Core framing:**
  - Host command frame starts with `0xFF`
  - Device response frame starts with `0xAA` (normal) or `0xAB` (figure add/remove event)
  - Last byte of protocol payload uses additive checksum (`sum(payload bytes) & 0xFF`)
- **Key commands mirrored from Dolphin:**
  - `0x80` activate base
  - `0x81` seed/auth challenge setup (descramble + RNG seed)
  - `0x83` auth challenge response (RNG next + scramble)
  - `0x90/0x92/0x93/0x95/0x96` color commands (ack)
  - `0xA1` present figures
  - `0xA2` read figure block
  - `0xA3` write figure block
  - `0xB4` read figure identifier
  - `0xB5` status ack
- **Figure memory layout:** 20 blocks × 16 bytes = 320 bytes per figure.
- **Block mapping (protocol → storage):** `0 -> 1`, otherwise `n -> n*4` (same as Dolphin).
- **Authentication logic:** same scramble/descramble mask and RNG state update pattern used by Dolphin.

## Added embedded-oriented modules

- `usb_descriptors.c`
- `usb_descriptors.h`
- `usb_hid.c`
- `usb_hid.h`
- `disney_infinity.c`
- `disney_infinity.h`
- `figure_storage.c`
- `figure_storage.h`

## Architecture mapping (Dolphin C++ → embedded C)

- `InfinityUSB` transport behavior → `usb_hid.*`
- `InfinityBase` command logic → `disney_infinity.*`
- Figure slot and order tracking (`m_figures`, `order_added`) → `figure_storage.*`
- USB descriptor values from emulator implementations (Dolphin/Cemu) → `usb_descriptors.*`

## Bare-metal integration strategy (STM32F103 USB FS)

1. Use your USB device layer to return descriptors from `usb_descriptors.*`.
2. Configure HID interrupt endpoints (IN/OUT, 32-byte reports).
3. On OUT report reception call:
   - `usb_hid_receive_out_report(&ctx, out_report)`
4. On IN token / periodic poll call:
   - `usb_hid_get_in_report(&ctx, in_report)` and send only if `true`.
5. Mount/unmount figures from flash/SD dumps using:
   - `disney_infinity_mount_figure(...)`
   - `disney_infinity_unmount_figure(...)`

No dynamic allocation, no STL, fixed-size queues/arrays only.

## Timing-sensitive notes and limitations

- Dolphin schedules responses roughly at sub-millisecond to 1ms granularity; this port uses immediate queueing and relies on host IN polling cadence.
- If stricter timing is needed for specific consoles, add a timestamp gate before releasing queued responses.
- Figure data is treated as pre-existing encrypted NFC payload blocks (same practical assumption as Dolphin runtime read/write path).
- HID report descriptor here is a compact vendor-defined 32-byte IN/OUT descriptor; if your target console is strict, capture and mirror exact report descriptor bytes from a real base.
