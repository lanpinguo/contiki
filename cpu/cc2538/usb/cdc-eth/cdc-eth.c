#include <cdc-eth.h>
#include "cdc.h"
#include <usb-api.h>
#include "usb-core.h"

//#include <uip_arp.h>
#include <stdio.h>
#include <string.h>
//#include <net/ipv4/uip-fw.h>

#define DEBUG 1
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/**
 * The Ethernet header.
 */
struct uip_eth_hdr {
  struct uip_eth_addr dest;
  struct uip_eth_addr src;
  uint16_t type;
};

#define UIP_ETHTYPE_ARP  0x0806
#define UIP_ETHTYPE_IP   0x0800
#define UIP_ETHTYPE_IPV6 0x86dd

/*---------------------------------------------------------------------------*/
struct lang_id {
  uint8_t size;
  uint8_t type;
  uint16_t langid;
};
/* Language ID string (US English) */
static const struct lang_id lang_id = { sizeof(lang_id), 3, 0x0409 }; 
/*---------------------------------------------------------------------------*/
struct serial_nr {
  uint8_t size;
  uint8_t type;
  uint16_t string[16];
};
static struct serial_nr serial_nr = {
  sizeof(serial_nr),
  3, {
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'
  }
};
/*---------------------------------------------------------------------------*/
struct manufacturer {
  uint8_t size;
  uint8_t type;
  uint16_t string[17];
};
static const struct manufacturer manufacturer = {
  sizeof(manufacturer),
  3,
  {
    'T', 'e', 'x', 'a', 's', ' ',
    'I', 'n', 's', 't', 'r', 'u', 'm', 'e', 'n', 't', 's'
  }
};
/*---------------------------------------------------------------------------*/
struct product {
  uint8_t size;
  uint8_t type;
  uint16_t string[21];
};
static const struct product product = {
  sizeof(product),
  3,
  {
    'c', 'c', '2', '5', '3', '8', ' ',
    'S', 'y', 's', 't', 'e', 'm', '-', 'o', 'n', '-', 'C', 'h', 'i', 'p'
  }
};

/*---------------------------------------------------------------------------*/
struct eth_mac_adderss{
  uint8_t size;
  uint8_t type;
  uint16_t string[12];
};
static const struct eth_mac_adderss ethaddr = {
  sizeof(ethaddr),
  3,
  {
    '0','2','0','0','0','0','0','0','0','0','1','2'
  }
};



struct uip_eth_addr default_uip_ethaddr = {{0x02,0x00,0x00,0x00,0x00,0x12}};

/* For Diagnosis*/
uint32_t loopback = 1;

/*---------------------------------------------------------------------------*/
uint8_t *
usb_class_get_string_descriptor(uint16_t lang, uint8_t string)
{
  PRINTF("get_string_descriptor: Index %d \n", string);  
  switch (string) {
  case 0:
    return (uint8_t *)&lang_id;
  case 1:
    return (uint8_t *)&manufacturer;
  case 2:
    return (uint8_t *)&product;
  case 3:
    return (uint8_t *)&serial_nr;
  case 4:
    return (uint8_t *)&ethaddr;
  default:
    return NULL;
  }
}

 
static unsigned int
handle_cdc_eth_requests()
{
  PRINTF("CDC request %02x %02x\n", usb_setup_buffer.bmRequestType,
         usb_setup_buffer.bRequest);
  return 0;
}

static const struct USBRequestHandler cdc_eth_request_handler =
  {
    0x21, 0x7f,
    0x00, 0x00,
    handle_cdc_eth_requests
  };

static struct USBRequestHandlerHook cdc_eth_request_hook =
  {
    NULL,
    &cdc_eth_request_handler
  };

static USBBuffer recv_buffer;
static uint8_t recv_data[UIP_BUFSIZE];

static USBBuffer xmit_buffer[3];
static uint8_t xmit_data[UIP_BUFSIZE];
static uint8_t xmit_idle = 1;


static void
init_recv_buffer()
{
  recv_buffer.next = NULL;
  recv_buffer.data = recv_data;
  recv_buffer.left = UIP_BUFSIZE;
  recv_buffer.flags = USB_BUFFER_SHORT_END | USB_BUFFER_NOTIFY;
}

uint8_t
usbeth_send(void)
{
  //if ((xmit_buffer[0].flags & USB_BUFFER_SUBMITTED)) return UIP_FW_DROPPED;
  //uip_arp_out();
  memcpy(xmit_data, uip_buf, uip_len);
  xmit_buffer[0].next = NULL;
  xmit_buffer[0].left = uip_len;
  xmit_buffer[0].flags = USB_BUFFER_NOTIFY | USB_BUFFER_SHORT_END;
  xmit_buffer[0].data = xmit_data;
  
  /* printf("usbeth_send: %d\n", uip_len);  */
  usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
  return 0;
}

/*static struct uip_fw_netif usbethif =
  {UIP_FW_NETIF(172,16,0,1, 255,255,255,255, usbeth_send)};*/

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

PROCESS(usb_eth_process, "USB ethernet");

PROCESS_THREAD(usb_eth_process, ev , data)
{
  PROCESS_BEGIN();
  usb_register_request_handler(&cdc_eth_request_hook);
  usb_setup();
  usb_set_ep_event_process(DATA_OUT, process_current);
  usb_set_global_event_process(process_current);
  //uip_fw_default(&usbethif);
  //uip_setethaddr(default_uip_ethaddr);
  //uip_arp_init();
  
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_EXIT) break;
    if (ev == PROCESS_EVENT_POLL)
    {
      unsigned int events = usb_get_global_events();
      if (events) 
      {
        if (events & USB_EVENT_CONFIG) 
        {
          if (usb_get_current_configuration() != 0)
          {
            printf("Configured\n");
            usb_setup_bulk_endpoint(DATA_IN);
            usb_setup_bulk_endpoint(DATA_OUT);
            usb_setup_interrupt_endpoint(INTERRUPT_IN);
            init_recv_buffer();
            usb_submit_recv_buffer(DATA_OUT, &recv_buffer);
#if 0
            {
            static const uint8_t foo[4] = {0x12,0x34,0x56,0x78};
            xmit_buffer[0].next = NULL;
            xmit_buffer[0].left = sizeof(foo);
            xmit_buffer[0].flags = USB_BUFFER_SHORT_END;
            xmit_buffer[0].data = &foo;

            usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
            }
#endif
          }
          else 
          {
            usb_disable_endpoint(DATA_IN);
            usb_disable_endpoint(DATA_OUT);
            usb_disable_endpoint(INTERRUPT_IN);
          }
        }
      }
      
      events = usb_get_ep_events(DATA_IN);
      if (events & USB_EP_EVENT_NOTIFICATION)
      {
        xmit_idle = 1;
      }

      
      events = usb_get_ep_events(DATA_OUT);
      if (events & USB_EP_EVENT_NOTIFICATION)
      {
        uip_len = sizeof(recv_data) - recv_buffer.left;
        printf("\r\nReceived: %d bytes xmit:%d\n", uip_len,xmit_idle);  
        memcpy(uip_buf, recv_data, uip_len);

#if 0	
#if NETSTACK_CONF_WITH_IPV6
        if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
        uip_neighbor_add(&IPBUF->srcipaddr, &BUF->src);
        tcpip_input();
        } else 
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif
        if(loopback)
        {
          uip_len -= sizeof(struct uip_eth_hdr);
          if ((uip_len > 0) && (xmit_idle != 0)) 
          {
            memcpy(xmit_data, uip_buf, uip_len);
            xmit_buffer[0].next = NULL;
            xmit_buffer[0].data = xmit_data;
            xmit_buffer[0].left = uip_len;
            xmit_buffer[0].flags = USB_BUFFER_IN | USB_BUFFER_NOTIFY;
            xmit_idle = 0;
            usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
            printf("\r\nSent: %d bytes\r\n", uip_len);
          }
        }
        else
        {
          if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
            uip_len -= sizeof(struct uip_eth_hdr);
            tcpip_input();
          } 
          else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP))
          {
            //uip_arp_arpin();
            /* If the above function invocation resulted in data that
            should be sent out on the network, the global variable
            uip_len is set to a value > 0. */
            if (uip_len > 0) 
            {
              memcpy(xmit_data, uip_buf, uip_len);
              xmit_buffer[0].next = NULL;
              xmit_buffer[0].data = xmit_data;
              xmit_buffer[0].left = uip_len;
              xmit_buffer[0].flags = USB_BUFFER_SHORT_END;

              usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
              /* printf("Sent: %d bytes\n", uip_len); */
            }
          }
        }
        init_recv_buffer();
        usb_submit_recv_buffer(DATA_OUT, &recv_buffer);
      }
    }
  }
  PROCESS_END();
}

void
usb_cdc_eth_setup()
{
  process_start(&usb_eth_process, NULL);
}

void
usb_cdc_eth_set_ifaddr(uip_ipaddr_t *addr)
{
  //usbethif.ipaddr = *addr;
}
  
void
dummy(uip_ipaddr_t *addr1,  uip_ipaddr_t *addr2)
{
  *addr1 = *addr2;
}
