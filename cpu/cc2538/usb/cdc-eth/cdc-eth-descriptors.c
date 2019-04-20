#include "descriptors.h"
#include <cdc-acm/cdc.h>
#include <contiki-conf.h>
#include <usb-arch.h>

#define VENDOR_ID             0x0451 /* Vendor: TI */

#define PRODUCT_ID            0x1000

const struct usb_st_device_descriptor device_descriptor =
  {
    sizeof(struct usb_st_device_descriptor),
    USB_DT_DEVICE,
    0x0210,
    CDC,
    0,
    0,
    CTRL_EP_SIZE,
    VENDOR_ID,
    PRODUCT_ID,
    0x0010, /*bcdDevice*/
    2,/*iManufacturer*/
    1,/*iProduct*/
    3,/*iSerialNumber*/
    1 /*bNumConfigurations*/
  };

const struct configuration_st {
  struct usb_st_configuration_descriptor configuration;
  struct usb_st_interface_descriptor comm;
  struct usb_cdc_header_func_descriptor header;
  struct usb_cdc_union_func_descriptor union_descr;
  struct usb_cdc_ethernet_func_descriptor ethernet;
  struct usb_st_endpoint_descriptor ep_notification;
  struct usb_st_interface_descriptor data;
  struct usb_st_endpoint_descriptor ep_in;
  struct usb_st_endpoint_descriptor ep_out;

  struct usb_st_bos_descriptor bos;
  struct usb_st_ext_cap_descriptor dev_ext_cap;
  struct usb_st_ss_cap_descriptor dev_ss_cap;
  

} BYTE_ALIGNED configuration_block =
  {
    /* Configuration */
    {
      sizeof(configuration_block.configuration),
      USB_DT_CONFIGURATION,
      sizeof(configuration_block),
      2,
      1,
      0,
      0x80,
      50
    },
    {
      sizeof(configuration_block.comm), /* 0 bLength */
      USB_DT_INTERFACE,                 /* 1 bDescriptorType - Interface */
      0,                                /* 2 bInterfaceNumber - Interface 0 */
      0,                                /* 3 bAlternateSetting */
      2,                                /* 4 bNumEndpoints */
      CDC,                              /* 5 bInterfaceClass */
      ETHERNET_NETWORKING_CONTROL_MODEL,/* 6 bInterfaceSubClass - Ethernet Control Model */
      0,                                /* 7 bInterfaceProtocol - No specific protocol */
      0                                 /* 8 iInterface - No string descriptor */
    },
    {
      sizeof(configuration_block.header),
      CS_INTERFACE,
      CDC_FUNC_DESCR_HEADER,
      0x0110
    },
    {
      sizeof(configuration_block.union_descr),
      CS_INTERFACE,
      CDC_FUNC_DESCR_UNION,
      0, /* Master */
      {1}  /* Slave */
    },
    /*ethernet*/
    {
      sizeof(configuration_block.ethernet),
      CS_INTERFACE,
      CDC_FUNC_DESCR_ETHERNET,
      4, /*iMACAddress*/
      0, /* No statistics */
      UIP_CONF_BUFFER_SIZE - UIP_CONF_LLH_LEN + 14,
      0, /* No multicast filters */
      0 /* No wake-up filters */
    },
    {
      sizeof(configuration_block.ep_notification),/*bLength*/
      USB_DT_ENDPOINT,                            /*bDescriptorType*/
      INTERRUPT_IN,                               /*bEndpointAddress*/
      0x03,                                       /*bmAttributes*/
      INT_IN_PKT_SIZE_MAX,                        /*wMaxPacketSize*/
      100                                         /*bInterval*/
    },
    {
      sizeof(configuration_block.data),
      USB_DT_INTERFACE,
      1,
      0,
      2,
      CDC_DATA,
      0,
      TRANSPARENT_PROTOCOL,
      0
    },
    {
      sizeof(configuration_block.ep_in),  /*bLength*/
      USB_DT_ENDPOINT,                    /*bDescriptorType*/
      DATA_IN,                            /*bEndpointAddress*/
      0x02,                               /*bmAttributes*/
      DATA_IN_PKT_SIZE_MAX,               /*wMaxPacketSize*/
      0                                   /*bInterval*/
    },
    {
      sizeof(configuration_block.ep_out), /*bLength*/
      USB_DT_ENDPOINT,                    /*bDescriptorType*/
      DATA_OUT,                           /*bEndpointAddress*/
      0x02,                               /*bmAttributes*/
      DATA_OUT_PKT_SIZE_MAX,              /*wMaxPacketSize*/
      0                                   /*bInterval*/
    },
    {
      sizeof(configuration_block.bos),
      USB_DT_BOS,
      (sizeof(configuration_block.bos) + sizeof(configuration_block.dev_ext_cap) \
      + sizeof(configuration_block.dev_ss_cap)),
      2,
    },
    {
      sizeof(configuration_block.dev_ext_cap),
      USB_DT_DEVICE_CAPABILITY,
      USB_CAP_TYPE_EXT,
      USB_LPM_SUPPORT,
    },
    {
      sizeof(configuration_block.dev_ss_cap),
      USB_DT_DEVICE_CAPABILITY,
      USB_SS_CAP_TYPE,
      0,
      0x000e,
      USB_LOW_SPEED_OPERATION,
      1,
      0x0065
    }
  };

const struct usb_st_configuration_descriptor  *configuration_head =
(struct usb_st_configuration_descriptor const*)&configuration_block;

const struct usb_st_bos_descriptor  *bos_head =
(struct usb_st_bos_descriptor const*)&configuration_block.bos;


