#include "usb_hid.h"

#include <string.h>

void usb_hid_init(usb_hid_context_t* ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  disney_infinity_init(&ctx->infinity);
}

void usb_hid_receive_out_report(usb_hid_context_t* ctx, const uint8_t out_report[DISNEY_INFINITY_FRAME_SIZE])
{
  bool valid = false;
  disney_infinity_handle_packet(&ctx->infinity, out_report, &valid, ctx->tx_frame);
  ctx->tx_ready = valid;
}

bool usb_hid_get_in_report(usb_hid_context_t* ctx, uint8_t in_report[DISNEY_INFINITY_FRAME_SIZE])
{
  if (!ctx->tx_ready)
    return false;
  memcpy(in_report, ctx->tx_frame, DISNEY_INFINITY_FRAME_SIZE);
  memset(ctx->tx_frame, 0, DISNEY_INFINITY_FRAME_SIZE);
  ctx->tx_ready = false;
  return true;
}
