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
      sizeof(configuration_block.comm),
      USB_DT_INTERFACE,
      0,
      0,
      1,
      CDC,
       ETHERNET_NETWORKING_CONTROL_MODEL,
      0,
      0
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
      sizeof(configuration_block.ep_notification),
      USB_DT_ENDPOINT,
      INTERRUPT_IN,
      0x03,
      USB_EP3_SIZE,
      100
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
      sizeof(configuration_block.ep_in),
      USB_DT_ENDPOINT,
      DATA_IN,
      0x02,
      USB_EP1_SIZE,
      0
    },
    {
      sizeof(configuration_block.ep_out),
      USB_DT_ENDPOINT,
      DATA_OUT,
      0x02,
      USB_EP2_SIZE,
      0
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


