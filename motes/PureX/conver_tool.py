#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Apr  4 15:33:06 2019

@author: lanpinguo
"""

import sys
import struct
import argparse
from intelhex import IntelHex

'''
typedef struct
{
	uint32_t magicNumber;
	uint32_t imageSize;
	uint16_t imageValid;
	uint16_t headerLength;
	uint16_t headerVersion;
	uint16_t fieldControl;
	zclOTA_FileID_t fileId;
}__attribute__ ((packed)) OTA_ImageHeader_t;
#endif
'''

NVIC_VECTOR_SIZE = (163*4)

IMG_HDR_OFFSET = NVIC_VECTOR_SIZE
IMG_LEN_POS = 4






if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Extract images from .hex file.')
    parser.add_argument('-m', '--mode', choices=['A', 'B'], default='A')
    parser.add_argument('-i', '--input', help='Specify a input file', default = '')
    parser.add_argument('-o', '--output', help='Specify a output file name', default = 'image$(mode).bin')
    
    args = parser.parse_args(sys.argv[1:])


    if args.input == '':
        print("Please specify a valid file!")
        sys.exit(-1)

    #Default range is for A image
    img_addr_start = 0x00200000
    img_addr_end = 0x0023DFFF

    img_hdr_start = img_addr_start + IMG_HDR_OFFSET

    if args.mode == 'B':
        img_addr_start = 0x0023E000
        img_addr_end = 0x0027F7FF
        
    img_out = "image_%s" % ( args.mode ) 

#    try:
    ih = IntelHex(args.input)
    img_tmp = ih[img_addr_start : img_addr_end]
    segs = img_tmp.segments()
    img_len = 0
    for s in segs:
        img_len += s[1] - s[0] + 1
    #print(img_len)
    formatstring = 'I' # see Python docs for full list of valid struct formats
    img_tmp.puts((img_hdr_start + IMG_LEN_POS), struct.pack(formatstring,img_len)) # put image len  in hex file
    
    #there is a bug when formatstring='IHI', so split into two part: 'IH' and 'I'.
    formatstring = 'IIH'
    (magic_num,img_size,img_valid) = struct.unpack(formatstring, img_tmp.gets(img_hdr_start,10))
    #formatstring = 'H'
    #(img_valid,) = struct.unpack(formatstring, img_tmp.gets((img_hdr_start + IMG_LEN_POS_OFFSET),2))

    print("magic_num = 0x%08X,img_valid = 0x%04X,img_size = 0x%08X" % (magic_num,img_valid,img_size))
    
    img_tmp.tofile("%s.hex" % img_out, format='hex')
    img_tmp.tobinfile("%s.bin" % img_out)
#    except:
#        print("Failed !!")
#        sys.exit(-1)






