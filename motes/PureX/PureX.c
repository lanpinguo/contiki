/******************************************************************************

  Copyright (C), 2001-2011, Pure Co., Ltd.

 ******************************************************************************
  File Name     : pureX.c
  Version       : Initial Draft
  Author        : lanpinguo
  Created       : 2018/9/14
  Last Modified :
  Description   : Pure Motes node main body
  Function List :
              broadcast_recv
              PROCESS_THREAD
              PROCESS_THREAD
              PROCESS_THREAD
              recv_uc
  History       :
  1.Date        : 2018/9/14
    Author      : lanpinguo
    Modification: Created file

******************************************************************************/

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/



#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "i2c.h"
#include <stdio.h>

#include <cdc-eth.h>
#include "cdc.h"
#include <usb-api.h>
#include "usb-core.h"
#include "shell-memdebug.h"


#define TEST  2

#if TEST == 1
static uint8_t pkt_index = 0;
static uint8_t test_data[UIP_BUFSIZE] = {
  0x14,0x75,0x90,0x73,0x55,0xb4,0x98,0x54,0x1b,0xa2,0x87,0xd0,0x08,0x00,0x45,0x00,
  0x00,0x34,0x1b,0x1c,0x40,0x00,0x40,0x06,0x4c,0x1b,0xc0,0xa8,0x02,0x64,0x0d,0xe6,
  0x02,0x9b,0xdb,0x17,0x00,0x16,0xfb,0x07,0x2b,0x89,0x00,0x00,0x00,0x00,0x80,0x02,
  0xfa,0xf0,0x9e,0xd3,0x00,0x00,0x02,0x04,0x05,0xb4,0x01,0x03,0x03,0x08,0x01,0x01,
  0x04,0x02
};

#elif TEST == 2
static uint8_t pkt_index = 0;
static uint8_t test_data[UIP_BUFSIZE] = {
  0x41, 0xd8, 0xf9, 0x34, 0x12, 0xff, 0xff, 0xf1, 0xfd, 0x05, 0x10, 0x00, 0x4b, 0x12, 0x00, 0x7a,
  0x3b, 0x3a, 0x1a, 0x9b, 0x00, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#elif TEST == 3
static uint8_t test_data[] = {
  0x02, 0x00, 0x00, 0x00, 0x00, 0x12, 0xf4, 0x8e, 0x38, 0x86, 0x3e, 0x8b, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x44, 0xc3, 0x61, 0x40, 0x00, 0x40, 0x11, 0xf0, 0x65, 0xc0, 0xa8, 0x02, 0xc9, 0xc0, 0xa8,
  0x02, 0xc8, 0x90, 0x41, 0x04, 0x00, 0x00, 0x30, 0xb4, 0x45, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x30, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x33, 0x34, 0x36, 0x37, 0x38,
  0x39, 0x30, 0x2d, 0x30, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x00, 0x00, 0x00,0x00
};

#endif
/*---------------------------------------------------------------------------*/
PROCESS(shell_debug_process, "pure");
SHELL_COMMAND(pure_command,
	      "pure",
	      "pure [num]: blink LEDs ([num] times)",
	      &shell_debug_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_debug_process, ev, data)
{


  PROCESS_BEGIN();

  
#if TEST == 1
  pkt_index++;
  test_data[0] = pkt_index;
  usbeth_send(test_data,66);
  printf("\r\nusbeth_send: %d\r\n", pkt_index);
#elif TEST == 2
  pkt_index++;
  usbeth_send(test_data,64);
  printf("\r\nusbeth_send: %d\r\n", pkt_index);
#elif TEST == 3
  usbeth_l2_send(test_data, sizeof(test_data) - 4);
#endif

  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_pure_init(void)
{
  shell_register_command(&pure_command);
}



/*---------------------------------------------------------------------------*/
/* We first declare our processes. */
PROCESS(pure_x_shell_process, "PureX Contiki shell");
PROCESS(humidity_sensor_process, "Temp & Humidity process");

/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(&pure_x_shell_process,\
	&humidity_sensor_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pure_x_shell_process, ev, data)
{
  PROCESS_BEGIN();
  serial_shell_init();
  shell_blink_init();
  shell_ps_init();
  shell_reboot_init();
  shell_text_init();
  shell_time_init();
  shell_memdebug_init();
  shell_pure_init();
#if COFFEE
  shell_coffee_init();
  shell_file_init();
#endif
  
  PROCESS_END();
}








/*---------------------------------------------------------------------------*/
#define CRC8_POLY 0x31 //Polynomial 0x31(X8 + X5 + X4 + 1)
uint8_t CRC8(uint8_t crc, uint8_t byte)
{
  uint8_t i;
 
  crc ^= byte;
  for(i = 0; i < 8; i++)
  {
    if(crc & 0x80)
    {
      crc = (crc<< 1)^CRC8_POLY;
    }
    else
    {
      crc = crc<< 1;
    }
  }
 
  return crc;
}
 


#define MAX_RANGE (65536.0)
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(humidity_sensor_process, ev, data)
{
	static uint8_t rc = 0;
  //static struct etimer et;
  

  PROCESS_BEGIN();

  rc += 1;
  //etimer_set(&et, CLOCK_SECOND * 2);
	printf("\r\nThis is OTA tester (%x)\r\n",rc);



	//EnterNvmApplication(0,5);
	
  //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
