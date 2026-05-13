#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>

/* Official Disney Infinity base USB identifiers observed by Dolphin and other emulators. */
#define DISNEY_INFINITY_USB_VID 0x0E6F
#define DISNEY_INFINITY_USB_PID 0x0129

#define DISNEY_INFINITY_EP_IN  0x81u
#define DISNEY_INFINITY_EP_OUT 0x01u
#define DISNEY_INFINITY_PACKET_SIZE 32u

#if defined(__GNUC__) || defined(__clang__)
#define PACKED __attribute__((packed))
#else
#define PACKED
#pragma pack(push, 1)
#endif

typedef struct PACKED
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t iManufacturer;
  uint8_t iProduct;
  uint8_t iSerialNumber;
  uint8_t bNumConfigurations;
} usb_device_descriptor_t;

typedef struct PACKED
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
} usb_config_descriptor_t;

typedef struct PACKED
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
} usb_interface_descriptor_t;

typedef struct PACKED
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdHID;
  uint8_t bCountryCode;
  uint8_t bNumDescriptors;
  uint8_t bReportDescriptorType;
  uint16_t wReportDescriptorLength;
} usb_hid_descriptor_t;

typedef struct PACKED
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
} usb_endpoint_descriptor_t;

typedef struct PACKED
{
  usb_config_descriptor_t config;
  usb_interface_descriptor_t interface0;
  usb_hid_descriptor_t hid;
  usb_endpoint_descriptor_t ep_in;
  usb_endpoint_descriptor_t ep_out;
} disney_infinity_config_block_t;

const usb_device_descriptor_t* usb_descriptors_get_device(void);
const disney_infinity_config_block_t* usb_descriptors_get_config(void);
const uint8_t* usb_descriptors_get_hid_report(uint16_t* length);
const uint8_t* usb_descriptors_get_string(uint8_t index, uint16_t* length);

#if !defined(__GNUC__) && !defined(__clang__)
#pragma pack(pop)
#endif

#endif
