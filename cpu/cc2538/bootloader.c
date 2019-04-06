
#include "contiki.h"
#include "reg.h"
#include "flash.h"
#include "sys-ctrl.h"
#include "rom-util.h"

#include <stdint.h>



#if FLASH_OTA_BOOT_MANAGER
/* ------------------------------------------------------------------------------------------------
 *                                          Typedefs
 * ------------------------------------------------------------------------------------------------
 */
	// OTA Header constants
#define OTA_HDR_MAGIC_NUMBER                0x0BEEF11E
#define OTA_HDR_BLOCK_SIZE                  128
#define OTA_HDR_STACK_VERSION               2
#define OTA_HDR_HEADER_VERSION              0x0100
#define OTA_HDR_FIELD_CTRL                  0
	
#define OTA_HEADER_LEN_MIN                  56
#define OTA_HEADER_LEN_MAX                  69
#define OTA_HEADER_LEN_MIN_ECDSA            166
#define OTA_HEADER_STR_LEN                  32
	
#define OTA_HEADER_IMG_VALID              	0x5A5A
	
#define OTA_FC_SCV_PRESENT                  (0x1 << 0)
#define OTA_FC_DSF_PRESENT                  (0x1 << 1)
#define OTA_FC_HWV_PRESENT                  (0x1 << 2)

	
#define HAL_IBM_LEDGER_PAGE        					254

#define NVIC_VECTOR_SIZE								 		(163*4)
	
#define FLASH_BASE								 					FLASH_FW_ADDR
	
#define GPIO_A_DIR				0x400D9400
#define GPIO_A_DATA				0x400D9000
#define GPIO_C_DIR				0x400DB400
#define GPIO_C_DATA				0x400DB000

#define HWREG(x)                                                              \
	(*((volatile unsigned long *)(x)))

 
typedef struct
{
	uint16_t manufacturer;
	uint16_t type;
	uint32_t version;
} zclOTA_FileID_t;

typedef struct
{
	uint16_t tag;
	uint32_t length;
} OTA_SubElementHdr_t;

typedef struct
{
	uint32_t magicNumber;
	uint32_t imageSize;
	uint16_t imageValid;
	uint16_t headerLength;
	uint16_t headerVersion;
	uint16_t fieldControl;
	zclOTA_FileID_t fileId;
#if 0
}__attribute__ ((aligned(4))) OTA_ImageHeader_t;
#else
}__attribute__ ((packed)) OTA_ImageHeader_t;
#endif

typedef struct {
  uint32_t stackPtr;
  uint32_t nvicReset;
} ibm_ledger_t;
//static_assert((sizeof(ibm_ledger_t) == 16), "Need to PACK the ibm_ledger_t");

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */
__attribute__((__section__(".image_data"), used))
static const OTA_ImageHeader_t ImageHeader = {
  .magicNumber = OTA_HDR_MAGIC_NUMBER,
  .headerVersion = OTA_HDR_HEADER_VERSION,
  .headerLength = sizeof(OTA_ImageHeader_t),
  .imageValid = OTA_HEADER_IMG_VALID,
};




__attribute__((__section__(".boot_fw"), used))
void EnterNvmApplication(uint32_t spInit, uint32_t resetVector)
{
	// Set the LR register to a value causing a processor fault in case of
	// an accidental return from the application.
	asm("mov r2, #0xffffffff");
	asm("mov lr, r2");

	asm("mov sp, r0");	// Setup the initial stack pointer value.

	asm("bx r1");  // Branch to application reset ISR.
}


__attribute__((__section__(".boot_fw"), used))
int boot_main(void)
{
	volatile uint32_t ledgerPageAddr = 0x200000;
	volatile unsigned long ulLoopCount;

	// Set direction output and initial value for PC2 and PC0
	// Greed LED on PA2
	// Red LED on PA4
	// Blue LED on PA5
	HWREG(GPIO_A_DIR) |= 0x34;
	HWREG(GPIO_A_DATA + (0x34 << 2)) = 0;
	

	
#if 0	
	// Loop forever.
	while(1)
	{
			// Turn on both LED's.
			HWREG(GPIO_A_DATA + (0x34 << 2)) ^= 0x34;
	
			// Delay for a bit
			for(ulLoopCount = 200000; ulLoopCount > 0; ulLoopCount--)
			{
			}
	};
#endif

	for (int pgCnt = 0; pgCnt < HAL_IBM_LEDGER_PAGE; pgCnt++, ledgerPageAddr += FLASH_PAGE_SIZE)
	{
		OTA_ImageHeader_t *pLedger = (OTA_ImageHeader_t *)(ledgerPageAddr + NVIC_VECTOR_SIZE);

		if (pLedger->magicNumber != OTA_HDR_MAGIC_NUMBER)
		{
			continue;
		}
		// Turn on both LED's.
		HWREG(GPIO_A_DATA + (0x30 << 2)) = 0x30;

		if (pLedger->imageValid == OTA_HEADER_IMG_VALID)
		{
			HWREG(GPIO_A_DATA + (0x04 << 2)) = 0x04;
			ibm_ledger_t* img = (ibm_ledger_t*)(ledgerPageAddr);
			// Sanity check NVIC entries.
			if ((img->stackPtr > 0x20000000) &&
					(img->stackPtr < 0x27007FFF) &&
					(img->nvicReset > 0x200000) &&
					(img->nvicReset < 0x0027EFFF))
			{
				HWREG(GPIO_A_DATA + (0x20 << 2)) = 0x00;
				EnterNvmApplication(img->stackPtr, img->nvicReset);
			}
		}
	}

	// Loop forever.
	while(1)
	{
			// Turn Blue  LED.
			HWREG(GPIO_A_DATA + (0x10 << 2)) ^= 0x10;
	
			// Delay for a bit
			for(ulLoopCount = 200000; ulLoopCount > 0; ulLoopCount--)
			{
			}
	};

	return -1;
}


#endif

