#include "platform/stm32f103/usb_base_platform.h"

#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>

#include "usb_hid.h"

#define CTRL_BUFFER_SIZE 128u

static usb_hid_context_t g_hid_ctx;
static usbd_device* g_usbd_dev;
static uint8_t g_ctrl_buffer[CTRL_BUFFER_SIZE];

static const struct usb_device_descriptor g_device_descriptor = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 0x20,
    .idVendor = 0x0E6F,
    .idProduct = 0x0129,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const uint8_t g_hid_report_descriptor[] = {
    0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01, 0x85, 0x00, 0x09,
    0x02, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x20,
    0x81, 0x02, 0x09, 0x03, 0x95, 0x20, 0x91, 0x02, 0xC0,
};

static const struct
{
  struct usb_hid_descriptor hid;
  struct
  {
    uint8_t bReportDescriptorType;
    uint16_t wDescriptorLength;
  } __attribute__((packed)) report;
} __attribute__((packed)) g_hid_function = {
    .hid =
        {
            .bLength = sizeof(struct usb_hid_descriptor),
            .bDescriptorType = USB_DT_HID,
            .bcdHID = 0x0111,
            .bCountryCode = 0,
            .bNumDescriptors = 1,
        },
    .report =
        {
            .bReportDescriptorType = USB_DT_REPORT,
            .wDescriptorLength = sizeof(g_hid_report_descriptor),
        },
};

static const struct usb_endpoint_descriptor g_endpoints[] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x81,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = 32,
        .bInterval = 1,
    },
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x01,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = 32,
        .bInterval = 1,
    },
};

static const struct usb_interface_descriptor g_interface = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_HID,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
    .endpoint = g_endpoints,
    .extra = &g_hid_function,
    .extralen = sizeof(g_hid_function),
};

static const struct usb_interface g_interfaces[] = {
    {
        .num_altsetting = 1,
        .altsetting = &g_interface,
    },
};

static const struct usb_config_descriptor g_config_descriptor = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0x80,
    .bMaxPower = 0xFA,
    .interface = g_interfaces,
};

static const char* g_usb_strings[] = {
    "PDP AUDIO",
    "Disney Infinity Base",
    "00000001A",
};

static enum usbd_request_return_codes hid_control_request(usbd_device* usbd_dev,
                                                          struct usb_setup_data* req,
                                                          uint8_t** buf,
                                                          uint16_t* len,
                                                          usbd_control_complete_callback* complete)
{
  (void)usbd_dev;
  (void)complete;

  if ((req->bmRequestType != 0x81) || (req->bRequest != USB_REQ_GET_DESCRIPTOR))
    return USBD_REQ_NEXT_CALLBACK;

  if ((req->wValue >> 8) != USB_DT_REPORT)
    return USBD_REQ_NEXT_CALLBACK;

  *buf = (uint8_t*)g_hid_report_descriptor;
  *len = sizeof(g_hid_report_descriptor);
  return USBD_REQ_HANDLED;
}

static void usb_out_callback(usbd_device* usbd_dev, uint8_t ep)
{
  uint8_t out_report[DISNEY_INFINITY_FRAME_SIZE] = {0};
  int len = usbd_ep_read_packet(usbd_dev, ep, out_report, sizeof(out_report));

  if (len == (int)DISNEY_INFINITY_FRAME_SIZE)
    usb_hid_receive_out_report(&g_hid_ctx, out_report);
}

static void usb_set_config(usbd_device* usbd_dev, uint16_t wValue)
{
  (void)wValue;

  usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, DISNEY_INFINITY_FRAME_SIZE, NULL);
  usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_INTERRUPT, DISNEY_INFINITY_FRAME_SIZE, usb_out_callback);

  usbd_register_control_callback(usbd_dev,
                                 USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
                                 USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                                 hid_control_request);
}

void usb_base_platform_init(void)
{
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USB);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
  gpio_clear(GPIOA, GPIO12);
  for (volatile unsigned i = 0; i < 800000u; ++i)
    __asm__("nop");

  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO12);

  usb_hid_init(&g_hid_ctx);

  g_usbd_dev = usbd_init(&st_usbfs_v1_usb_driver,
                         &g_device_descriptor,
                         &g_config_descriptor,
                         g_usb_strings,
                         3,
                         g_ctrl_buffer,
                         sizeof(g_ctrl_buffer));

  usbd_register_set_config_callback(g_usbd_dev, usb_set_config);
  nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
}

void usb_base_platform_poll(void)
{
  uint8_t in_report[DISNEY_INFINITY_FRAME_SIZE];
  usbd_poll(g_usbd_dev);

  if (usb_hid_get_in_report(&g_hid_ctx, in_report))
    usbd_ep_write_packet(g_usbd_dev, 0x81, in_report, DISNEY_INFINITY_FRAME_SIZE);
}

void usb_lp_can_rx0_isr(void)
{
  usbd_poll(g_usbd_dev);
}
