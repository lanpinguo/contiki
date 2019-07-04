
/* 
 * Based upon the work of Grant P. Maizels and Ross Williams.
 * Their original licence details are in the header file.
 * This amalgamation is the work of James Bensley Copyright (c) 2018.
 */ 


#include <stdio.h>     // EOF, FILE, fclose(), fprintf(), fscanf(), perror(), printf(), stderr, stdin
#include <stdlib.h>    // calloc(), exit()
#include <string.h>    // memset()

#include "crc32.h"


cm_t g_cm = {
  .cm_width  = 32,
  .cm_poly   = 0x04C11DB7,
  .cm_init   = 0xFFFFFFFF,
  .cm_refin  = 1,
  .cm_refot  = 1,
  .cm_xorot  = 0xFFFFFFFF,
};
p_cm_t p_cm = &g_cm;


uint32_t crc32_data(const unsigned char *data, int len, uint32_t crc_init)
{

  int max = len;
  int j;


  if (data == NULL) {
     return 0;
  }
    
  p_cm->cm_init   = crc_init;
  cm_ini(p_cm);
   

  for (j = 0; j < max; j++ ) {
    cm_nxt(p_cm, data[j]);
  }

  return cm_crc(p_cm);
}


static uint32_t reflect(uint32_t v, uint32_t b) {

  static int   i;
  static uint32_t t;

  t = v;

  for (i=0; i < b; i++) {

    if (t & 1L) {
       v|=  BITMASK((b - 1) - i);
    } else {
       v&= ~BITMASK((b - 1) - i);
    }

    t>>=1;

  }

  return v;

}


static uint32_t widmask(p_cm_t p_cm) {

  return (((1L << (p_cm->cm_width - 1)) - 1L) << 1) | 1L;

}


static void cm_ini(p_cm_t p_cm) {

  p_cm->cm_reg = p_cm->cm_init;

}


static void cm_nxt(p_cm_t p_cm, uint32_t ch) {

  static int   i;
  static uint32_t uch, topbit;

  uch    = ch;
  topbit = BITMASK(p_cm->cm_width - 1);

  if (p_cm->cm_refin) uch = reflect(uch, 8);

  p_cm->cm_reg ^= (uch << (p_cm->cm_width - 8));

  for (i=0; i < 8; i++) {

    if (p_cm->cm_reg & topbit) {
      p_cm->cm_reg = (p_cm->cm_reg << 1) ^ p_cm->cm_poly;
    } else {
       p_cm->cm_reg <<= 1;
    }

    p_cm->cm_reg &= widmask(p_cm);

  }

}


static void cm_blk(p_cm_t p_cm, uint8_t *blk_adr, uint32_t blk_len) {

  while (blk_len--) cm_nxt(p_cm, *blk_adr++);

}


static uint32_t cm_crc(p_cm_t p_cm) {

  if (p_cm->cm_refot) {
    return p_cm->cm_xorot ^ reflect(p_cm->cm_reg, p_cm->cm_width);
  } else {
    return p_cm->cm_xorot ^ p_cm->cm_reg;
  }

}


static uint32_t cm_tab(p_cm_t p_cm, uint32_t index) {

  static uint8_t  i;
  static uint32_t r, topbit, inbyte;

  topbit = BITMASK(p_cm->cm_width - 1);
  inbyte = index;

  if (p_cm->cm_refin) inbyte = reflect(inbyte, 8);

  r = inbyte << (p_cm->cm_width - 8);

  for (i=0; i < 8; i++) {

    if (r & topbit) {
      r = (r << 1) ^ p_cm->cm_poly;
    } else {
       r <<= 1;
     }
  }

  if (p_cm->cm_refin) r = reflect(r, p_cm->cm_width);

  return r & widmask(p_cm);

}

