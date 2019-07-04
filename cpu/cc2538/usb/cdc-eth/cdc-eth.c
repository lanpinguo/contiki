#include <cdc-eth.h>
#include "cdc.h"
#include <usb-api.h>
#include "usb-core.h"

//#include <uip_arp.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


#include "packetbuf.h"
#include "net/netstack.h"
#include "crc32.h"
#include "crc16.h"


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

struct uip_ipv4_hdr {
  /* IP header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ip4addr_t srcipaddr, destipaddr;
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
  /*PRINTF("\r\nget_string_descriptor: Index %d \r\n", string);  */
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


static void
set_packet_filter(uint16_t value,  uint16_t intf)
{
  if(value & PACKET_TYPE_PROMISCUOUS){
    PRINTF("\r\nEnter promiscuous Mode\r\n");
    
    //void cc2538_rf_set_promiscous_mode(char p);
  }

  //notify_user(USB_CDC_ACM_LINE_CODING);
  usb_send_ctrl_status();
}

 
static unsigned int
handle_cdc_eth_requests()
{
  PRINTF("\r\nCDC request %02x %02x\r\n", usb_setup_buffer.bmRequestType,
         usb_setup_buffer.bRequest);
  switch (usb_setup_buffer.bmRequestType)
  {
    case 0x21: /* CDC interface OUT requests */
      /* Check if it's the right interface */
      if(usb_setup_buffer.wIndex != 0)
        return 0;
      switch (usb_setup_buffer.bRequest) {
      case SET_CONTROL_LINE_STATE:
        //notify_user(USB_CDC_ACM_LINE_STATE);
        usb_send_ctrl_status();
        return 1;

      case SET_ETHERNET_PACKET_FILTER:
        {
          uint16_t filter = usb_setup_buffer.wValue;
          uint16_t intf =  usb_setup_buffer.wIndex;
          set_packet_filter(filter,intf);
          return 1;
        }
      }
      break;
    case 0xa1:                   /* CDC interface IN requests */
      if(usb_setup_buffer.wIndex != 0)
        return 0;
      switch (usb_setup_buffer.bRequest) {
      case GET_ENCAPSULATED_RESPONSE:
        PRINTF("\r\nCDC response\r\n");
        usb_send_ctrl_status();
        return 1;
      }
  }
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
static void
init_recv_buffer()
{
  recv_buffer.next = NULL;
  recv_buffer.data = recv_data;
  recv_buffer.left = UIP_BUFSIZE;
  recv_buffer.flags = USB_BUFFER_SHORT_END | USB_BUFFER_NOTIFY;
}




#if NXP_SNIFFER_MODE
struct nxp_sniffer_hdr {
  uint8_t timestamp[5];
  uint8_t sniffer_id[6];
  uint8_t chl;
  uint8_t lqi;
  uint8_t len;

};


#define XPKT_MAC_LEN        14
#define XPKT_ETH_FCS_LEN    4
#define XPKT_UDPIP_LEN      (sizeof(struct uip_udpipv4_hdr))
#define XPKT_SNIFFER_LEN    (sizeof(struct nxp_sniffer_hdr))
#define XPKT_HEADER_LEN     (XPKT_MAC_LEN + XPKT_UDPIP_LEN + XPKT_SNIFFER_LEN)
#define XPKT_MAX_LEN        (UIP_BUFSIZE + XPKT_HEADER_LEN ) 
#define ETHERTYPE_6LOWPAN		0xA0ED	/* RFC 4944: Transmission of IPv6 Packets over IEEE 802.15.4 Networks */

#else
#define XPKT_HEADER_LEN     (14)
#define XPKT_MAX_LEN        (UIP_BUFSIZE + XPKT_HEADER_LEN ) 
#endif


static USBBuffer xmit_buffer[3];
static uint8_t xmit_idle = 1;

#if NXP_SNIFFER_MODE

static uint8_t xmit_data[XPKT_MAX_LEN] = 
{
  0x02,0x00,0x00,0x00,0x00,0x12,0x98,0x54,0x1b,0xa2,0x87,0xd0,0x08,0x00
};
#else
static uint8_t xmit_data[XPKT_MAX_LEN] = 
{
  0x02,0x00,0x00,0x00,0x00,0x12,0x98,0x54,0x1b,0xa2,0x87,0xd0,0xA0,0xED
};
#endif

#if NXP_SNIFFER_MODE
struct uip_udpipv4_hdr *udpip_hdr = (void*)(xmit_data + XPKT_MAC_LEN);
struct nxp_sniffer_hdr *sniffer_hdr = (void*)(xmit_data + XPKT_MAC_LEN + XPKT_UDPIP_LEN);
#endif

int16_t usb_xmit_buffer_init(void)
{
#if NXP_SNIFFER_MODE
  uint16_t tmp;

  
  /*ip init*/
  udpip_hdr->vhl = 0x45;
  udpip_hdr->tos = 0;
  udpip_hdr->ttl = 0xff;
  udpip_hdr->proto = 0x11; /*UDP*/
  udpip_hdr->destipaddr.u8[0] = 192;
  udpip_hdr->destipaddr.u8[1] = 168;
  udpip_hdr->destipaddr.u8[2] = 3;
  udpip_hdr->destipaddr.u8[3] = 100;

  udpip_hdr->srcipaddr.u8[0] = 192;
  udpip_hdr->srcipaddr.u8[1] = 168;
  udpip_hdr->srcipaddr.u8[2] = 3;
  udpip_hdr->srcipaddr.u8[3] = 200;


  /*udp init*/
  tmp = 1024;
  udpip_hdr->destport[0] = (tmp>>8) & 0xFF;
  udpip_hdr->destport[1] = tmp & 0xFF;

  tmp = 1000;
  udpip_hdr->srcport[0] = (tmp>>8) & 0xFF;
  udpip_hdr->srcport[1] = tmp & 0xFF;

  sniffer_hdr->sniffer_id[0] = '1';
  sniffer_hdr->sniffer_id[1] = '2';
  sniffer_hdr->sniffer_id[2] = '3';
  sniffer_hdr->sniffer_id[3] = '4';
  sniffer_hdr->sniffer_id[4] = '5';
  sniffer_hdr->sniffer_id[5] = '\0';

#endif   
  return 0;
}

 static uint16_t
chksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
  uint16_t t;
  const uint8_t *dataptr;
  const uint8_t *last_byte;

  dataptr = data;
  last_byte = data + len - 1;

  while(dataptr < last_byte) {   /* At least two more bytes */
    t = (dataptr[0] << 8) + dataptr[1];
    sum += t;
    if(sum < t) {
      sum++;      /* carry */
    }
    dataptr += 2;
  }

  if(dataptr == last_byte) {
    t = (dataptr[0] << 8) + 0;
    sum += t;
    if(sum < t) {
      sum++;      /* carry */
    }
  }

  /* Return sum in host byte order. */
  return sum;
}

uint16_t ip_hdr_chksum(uint8_t *ip_buf, uint16_t len)
{
	return 	 uip_htons(chksum(0, ip_buf, len));
}


uint16_t udp_hdr_chksum(uint8_t *ip_buf, uint16_t udp_len)
{
	uint16_t sum;
	struct uip_ipv4_hdr *pIpHdr = (struct uip_ipv4_hdr *)ip_buf;

	/* First sum pseudoheader. */

	/* IP protocol and length fields. This addition cannot carry. */
	sum = udp_len + 17;
	/* Sum IP source and destination addresses. */
	sum = chksum(sum, (uint8_t *)&pIpHdr->srcipaddr, 2 * sizeof(uip_ip4addr_t));

	/* Sum UDP header and data. */
	sum = chksum(sum, &ip_buf[sizeof(struct uip_ipv4_hdr)],udp_len);

	return (sum == 0) ? 0xffff : sum;

}



uint16_t usb_xmit_buf_hdr_update(uint8_t* data,uint16_t len)
{
#if NXP_SNIFFER_MODE
  uint16_t tmp;
  radio_value_t  chl;
  uint32_t *fcs;
  uint16_t *fcs16;
  
  sniffer_hdr->lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  sniffer_hdr->timestamp[3] = (packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP) >> 8) & 0xFF;
  sniffer_hdr->timestamp[4] = packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP) & 0xFF;
  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL,&chl);
  sniffer_hdr->chl = chl;
  /*packetbuf_attr(PACKETBUF_ATTR_CHANNEL);*/
  sniffer_hdr->len = packetbuf_totlen();
  tmp = sizeof(struct uip_udpipv4_hdr) + sizeof(struct nxp_sniffer_hdr) +len;
  udpip_hdr->len[0] = (tmp>>8) & 0xFF;
  udpip_hdr->len[1] = tmp & 0xFF;

  udpip_hdr->ipchksum = 0;
  tmp = ~(ip_hdr_chksum(xmit_data + XPKT_MAC_LEN,sizeof(struct uip_ipv4_hdr)));
  udpip_hdr->ipchksum = tmp;


  tmp = sizeof(struct uip_udp_hdr) + sizeof(struct nxp_sniffer_hdr) +len;
  udpip_hdr->udplen[0] = (tmp>>8) & 0xFF;
  udpip_hdr->udplen[1] = tmp & 0xFF;

  udpip_hdr->udpchksum[0] = 0;
  udpip_hdr->udpchksum[1] = 0;
  tmp = ~(udp_hdr_chksum(xmit_data + XPKT_MAC_LEN,tmp));
  udpip_hdr->udpchksum[0] = (tmp>>8) & 0xFF;
  udpip_hdr->udpchksum[1] = tmp & 0xFF;

  memcpy(xmit_data + XPKT_HEADER_LEN , data, len);

  
  /* update 802.15.4 fcs */
  fcs16 = (uint16_t *)(xmit_data + XPKT_HEADER_LEN + len  - 2);
  *fcs16 = crc16_data(xmit_data + XPKT_HEADER_LEN, len - 2 , 0);


  /* update fcs */
  fcs = (uint32_t *)(xmit_data + len + XPKT_HEADER_LEN);
  *fcs = crc32_data(xmit_data, len + XPKT_HEADER_LEN , 0xFFFFFFFF);

  return len + XPKT_HEADER_LEN  + XPKT_ETH_FCS_LEN;

  
#else

  memcpy(xmit_data + XPKT_HEADER_LEN , packetbuf_dataptr(), packetbuf_datalen());
  return packetbuf_datalen() + XPKT_HEADER_LEN + XPKT_ETH_FCS_LEN;
#endif   

}

void dump_buf(uint8_t *_p, int len)
{
	char buf[128];
	int i, j, i0;
	const unsigned char *p = (const unsigned char *)_p;

	printf("\r\n");
	/* hexdump routine */
	for (i = 0; i < len; ) {
		memset(buf, sizeof(buf), ' ');
		sprintf(buf, "%5d: ", i);
		i0 = i;
		for (j=0; j < 16 && i < len; i++, j++)
			sprintf(buf+7+j*3, "%02x ", (uint8_t)(p[i]));
		i = i0;
		for (j=0; j < 16 && i < len; i++, j++)
			sprintf(buf+7+j + 48, "%c",
				isprint(p[i]) ? p[i] : '.');
		printf("%s\n", buf);
	}
	
	printf("\r\n");
}



int16_t
usbeth_l2_send(uint8_t *pkt, uint16_t len)
{
  uint16_t offset = 0;
  uint16_t total_len = len + 4; /* add fcs field*/
  uint32_t *fcs;


  
  if(!(xmit_idle > 0)){
    return -1;
  }

  /* update fcs */
  fcs = (uint32_t *)(pkt + len);
  *fcs = crc32_data(pkt, len, 0xFFFFFFFF);
  
  memcpy(xmit_data , pkt, total_len);

  /* for debug only */
  dump_buf(xmit_data,total_len);

  
  if(total_len > XPKT_MAX_LEN)
  {
    return -1;
  }

  for(;offset < total_len;)
  {
    xmit_buffer[0].next = NULL;
    xmit_buffer[0].data = xmit_data + offset;
    if((total_len - offset) > DATA_IN_PKT_SIZE_MAX){
      xmit_buffer[0].left = DATA_IN_PKT_SIZE_MAX;
      xmit_buffer[0].flags = USB_BUFFER_IN ;
      offset += DATA_IN_PKT_SIZE_MAX;
    }
    else
    {
      /* the last segment*/
      xmit_buffer[0].left = total_len - offset;
      xmit_buffer[0].flags = USB_BUFFER_NOTIFY | USB_BUFFER_PACKET_END; ;
      offset = total_len;
    }
    /* printf("usbeth_send: %d\n", uip_len);  */
    usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
  }
  return len;
}

int16_t
usbeth_send(uint8_t* data,uint16_t len)
{
  uint16_t offset = 0;
  uint16_t total_len ;


  
  if(!(xmit_idle > 0)){
    return -1;
  }

  
  total_len = usb_xmit_buf_hdr_update(data,len);

  /* for debug only */
  dump_buf(xmit_data,total_len);

  
  if(total_len > XPKT_MAX_LEN)
  {
    return -1;
  }

  for(;offset < total_len;)
  {
    xmit_buffer[0].next = NULL;
    xmit_buffer[0].data = xmit_data + offset;
    if((total_len - offset) > DATA_IN_PKT_SIZE_MAX){
      xmit_buffer[0].left = DATA_IN_PKT_SIZE_MAX;
      xmit_buffer[0].flags = USB_BUFFER_IN ;
      offset += DATA_IN_PKT_SIZE_MAX;
    }
    else
    {
      /* the last segment*/
      xmit_buffer[0].left = total_len - offset;
      xmit_buffer[0].flags = USB_BUFFER_NOTIFY | USB_BUFFER_PACKET_END; ;
      offset = total_len;
    }
    /* printf("usbeth_send: %d\n", uip_len);  */
    usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
  }
  return len;
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

  usb_xmit_buffer_init();
  
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
            PRINTF("Configured\n");
            usb_setup_bulk_endpoint(DATA_IN);
            usb_setup_bulk_endpoint(DATA_OUT);
            usb_setup_interrupt_endpoint(INTERRUPT_IN);
            init_recv_buffer();
            usb_submit_recv_buffer(DATA_OUT, &recv_buffer);
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
        //PRINTF("\r\nReceived: %d bytes xmit:%d\n", uip_len,xmit_idle); 
        
        //dump_buf(recv_data, uip_len);
        //memcpy(uip_buf, recv_data, uip_len);

#if 0
        if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
        uip_neighbor_add(&IPBUF->srcipaddr, &BUF->src);
        tcpip_input();
        } else 
#endif /* NETSTACK_CONF_WITH_IPV6 */
#if 0
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
              xmit_buffer[0].left = 32;
              xmit_buffer[0].flags = USB_BUFFER_SHORT_END;

              usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
              /* printf("Sent: %d bytes\n", uip_len); */
            }
          }
        }
#endif        
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
