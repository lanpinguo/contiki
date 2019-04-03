
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

typedef struct {
  uint32_t imageCRC[2];
  uint32_t nvicJump[2];
} ibm_ledger_t;
//static_assert((sizeof(ibm_ledger_t) == 16), "Need to PACK the ibm_ledger_t");

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define HAL_IBM_LEDGER_PAGE        254

#define FLASH_BASE								 FLASH_FW_ADDR



__attribute__((__section__(".boot_data"), used))
static const ibm_ledger_t LedgerPageSignature = {
  .imageCRC = {0x01234567, 0x89ABCDEF},
  .nvicJump = {0x02468ACE, 0x13579BDF}
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
int boot_memcmp(const void *s1, const void *s2, size_t n)
{
	int rc;
	for(int i = 0 ; i < n; i++)
	{
		rc = ((char*)s1)[i] - ((char*)s2)[i];
		if(rc != 0)
			break;
	}
	return rc;
}


__attribute__((__section__(".boot_fw"), used))
int boot_main(void)
{
	uint32_t ledgerPageAddr = FLASH_BASE + (HAL_IBM_LEDGER_PAGE * FLASH_PAGE_SIZE);

	for (int pgCnt = 0; pgCnt < HAL_IBM_LEDGER_PAGE; pgCnt++, ledgerPageAddr -= FLASH_PAGE_SIZE)
	{
		ibm_ledger_t *pLedger = (ibm_ledger_t *)ledgerPageAddr;
		int ledgerCnt = 0;

		if (boot_memcmp(pLedger, &LedgerPageSignature, sizeof(ibm_ledger_t)))
		{
			continue;
		}

		for (pLedger++; ledgerCnt < (FLASH_PAGE_SIZE/sizeof(ibm_ledger_t)); ledgerCnt++, pLedger++)
		{
			if ( (pLedger->imageCRC[0] == 0xFFFFFFFF) || // Not expected except first 2-step programming.
					 ((pLedger->imageCRC[0] != 0) && (pLedger->imageCRC[0] == pLedger->imageCRC[1])) )
			{
				// Sanity check NVIC entries.
				if ((pLedger->nvicJump[0] > 0x20004000) &&
						(pLedger->nvicJump[0] < 0x27007FFF) &&
						(pLedger->nvicJump[1] > FLASH_BASE) &&
						(pLedger->nvicJump[1] < 0x0027EFFF))
				{
					EnterNvmApplication(pLedger->nvicJump[0], pLedger->nvicJump[1]);
				}
			}
		}
	}


	return -1;
}


#endif

