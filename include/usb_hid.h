#ifndef USB_HID_H
#define USB_HID_H

#include <stdbool.h>
#include <stdint.h>

#include "disney_infinity.h"

typedef struct
{
  disney_infinity_t infinity;
  uint8_t tx_frame[DISNEY_INFINITY_FRAME_SIZE];
  bool tx_ready;
} usb_hid_context_t;

void usb_hid_init(usb_hid_context_t* ctx);
void usb_hid_receive_out_report(usb_hid_context_t* ctx, const uint8_t out_report[DISNEY_INFINITY_FRAME_SIZE]);
bool usb_hid_get_in_report(usb_hid_context_t* ctx, uint8_t in_report[DISNEY_INFINITY_FRAME_SIZE]);

#endif
