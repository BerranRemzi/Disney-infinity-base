#include "usb_descriptors.h"

#include <stddef.h>

static const usb_device_descriptor_t g_device_descriptor = {
    .bLength = sizeof(usb_device_descriptor_t),
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 0x20,
    .idVendor = DISNEY_INFINITY_USB_VID,
    .idProduct = DISNEY_INFINITY_USB_PID,
    .bcdDevice = 0x0200,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

static const disney_infinity_config_block_t g_config_descriptor = {
    .config =
        {
            .bLength = sizeof(usb_config_descriptor_t),
            .bDescriptorType = 0x02,
            .wTotalLength = sizeof(disney_infinity_config_block_t),
            .bNumInterfaces = 0x01,
            .bConfigurationValue = 0x01,
            .iConfiguration = 0x00,
            .bmAttributes = 0x80,
            .bMaxPower = 0xFA,
        },
    .interface0 =
        {
            .bLength = sizeof(usb_interface_descriptor_t),
            .bDescriptorType = 0x04,
            .bInterfaceNumber = 0x00,
            .bAlternateSetting = 0x00,
            .bNumEndpoints = 0x02,
            .bInterfaceClass = 0x03,
            .bInterfaceSubClass = 0x00,
            .bInterfaceProtocol = 0x00,
            .iInterface = 0x00,
        },
    .hid =
        {
            .bLength = sizeof(usb_hid_descriptor_t),
            .bDescriptorType = 0x21,
            .bcdHID = 0x0111,
            .bCountryCode = 0x00,
            .bNumDescriptors = 0x01,
            .bReportDescriptorType = 0x22,
            .wReportDescriptorLength = 0x001D,
        },
    .ep_in =
        {
            .bLength = sizeof(usb_endpoint_descriptor_t),
            .bDescriptorType = 0x05,
            .bEndpointAddress = DISNEY_INFINITY_EP_IN,
            .bmAttributes = 0x03,
            .wMaxPacketSize = DISNEY_INFINITY_PACKET_SIZE,
            .bInterval = 0x01,
        },
    .ep_out =
        {
            .bLength = sizeof(usb_endpoint_descriptor_t),
            .bDescriptorType = 0x05,
            .bEndpointAddress = DISNEY_INFINITY_EP_OUT,
            .bmAttributes = 0x03,
            .wMaxPacketSize = DISNEY_INFINITY_PACKET_SIZE,
            .bInterval = 0x01,
        },
};

static const uint8_t g_hid_report_descriptor[] = {
    0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01, 0x85, 0x00, 0x09,
    0x02, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x20,
    0x81, 0x02, 0x09, 0x03, 0x95, 0x20, 0x91, 0x02, 0xC0,
};

static const uint8_t g_string_lang[] = {0x04, 0x03, 0x09, 0x04};
static const uint8_t g_string_manufacturer[] = {
    18, 0x03, 'P', 0, 'D', 0, 'P', 0, ' ', 0, 'A', 0, 'U', 0, 'D', 0, 'I', 0};
static const uint8_t g_string_product[] = {
    42, 0x03, 'D', 0, 'i', 0, 's', 0, 'n', 0, 'e', 0, 'y', 0, ' ', 0, 'I', 0,
    'n', 0, 'f', 0, 'i', 0, 'n', 0, 'i', 0, 't', 0, 'y', 0, ' ', 0, 'B', 0, 'a', 0, 's', 0, 'e', 0};
static const uint8_t g_string_serial[] = {
    18, 0x03, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0, 'A', 0};

const usb_device_descriptor_t* usb_descriptors_get_device(void)
{
  return &g_device_descriptor;
}

const disney_infinity_config_block_t* usb_descriptors_get_config(void)
{
  return &g_config_descriptor;
}

const uint8_t* usb_descriptors_get_hid_report(uint16_t* length)
{
  if (length != NULL)
    *length = (uint16_t)sizeof(g_hid_report_descriptor);
  return g_hid_report_descriptor;
}

const uint8_t* usb_descriptors_get_string(uint8_t index, uint16_t* length)
{
  const uint8_t* descriptor = NULL;
  switch (index)
  {
  case 0:
    descriptor = g_string_lang;
    break;
  case 1:
    descriptor = g_string_manufacturer;
    break;
  case 2:
    descriptor = g_string_product;
    break;
  case 3:
    descriptor = g_string_serial;
    break;
  default:
    if (length != NULL)
      *length = 0;
    return NULL;
  }

  if (length != NULL)
    *length = descriptor[0];
  return descriptor;
}
