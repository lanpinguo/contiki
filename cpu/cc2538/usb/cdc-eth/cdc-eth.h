#ifndef CDC_ETH_H_NUI0ULFC7C__
#define CDC_ETH_H_NUI0ULFC7C__

#include <net/ip/uip.h>

/* Should be called before usb_cdc_eth_setup */
void
usb_cdc_eth_set_ifaddr(uip_ipaddr_t *addr);

void
usb_cdc_eth_setup();

int16_t
usbeth_send(uint8_t* data,uint16_t len);

int16_t
usbeth_l2_send(uint8_t *pkt, uint16_t len);

#endif /* CDC_ETH_H_NUI0ULFC7C__ */
