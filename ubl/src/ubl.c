/* --------------------------------------------------------------------------
  FILE        : ubl.c
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : The main project file for the user boot loader
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// This module's header file
#include "ubl.h"

// Device specific CSL
#include "device.h"

// Misc. utility function include
#include "util.h"

// Project specific debug functionality
#include "debug.h"
#include "uartboot.h"


#ifdef UBL_NOR
// NOR driver include
#include "nor.h"
#include "norboot.h"
#endif

#ifdef UBL_NAND
// NAND driver include
#include "nand.h"
#include "nandboot.h"
#endif

#ifdef UBL_SD_MMC
// NAND driver include
#include "sd_mmc.h"
#include "sd_mmcboot.h"
#endif

/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static Uint32 LOCAL_boot(void);
static void LOCAL_bootAbort(void);
static void (*APPEntry)(void);


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/

Uint32 gEntryPoint;


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Main entry point
void main(void)
{
  Uint8 Val;

  // Call to real boot function code
  LOCAL_boot();

  // Jump to entry point
  DEBUG_printString("\r\nIPNC UBL Version: 2.2.0");
#ifdef IPNC_DM365
  DEBUG_printString("\r\nPlatform: DM365-297\r\n");
#elif defined(IPNC_DM368)
#ifdef EXTREME
  DEBUG_printString("\r\nPlatform: DM368-486\r\n");
#else
  DEBUG_printString("\r\nPlatform: DM368-432\r\n");
#endif
#elif defined(IPNC_EVM)
  DEBUG_printString("\r\nPlatform: DM365-EVM\r\n");
#endif

  Val=0xff;
  RTCIF_getreg(0x10,&Val);
  if(Val != 0)
	DEBUG_printString("\r\nReset RTC Fail.. \r\n");

  DEBUG_printString("\r\nUBL Executed Sucessfully\r\n");

  DEBUG_printString("\r\nJumping to entry point at ");
  DEBUG_printHexInt(gEntryPoint);

  APPEntry = (void (*)(void)) gEntryPoint;
  (*APPEntry)();
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 LOCAL_boot(void)
{
  DEVICE_BootMode bootMode;
  // Read boot mode
  bootMode = DEVICE_bootMode();

  if (bootMode == DEVICE_BOOTMODE_UART)
  {
    // Wait until the RBL is done using the UART.
    while((UART0->LSR & 0x40) == 0 );
  }

  // Platform Initialization
  if ( DEVICE_init() != E_PASS )
  {
    DEBUG_printString("\r\n");
    DEBUG_printString(devString);
    DEBUG_printString(" initialization failed!\r\n");
    asm(" MOV PC, #0");
  }
  else
  {
    DEBUG_printString("\r\n");
    DEBUG_printString(devString);
    DEBUG_printString(" initialization passed!\r\n");
  }

  // Set RAM pointer to beginning of RAM space
  UTIL_setCurrMemPtr(0);

  // Send some information to host
  DEBUG_printString("TI UBL Base Version: ");
  DEBUG_printString(UBL_VERSION_STRING);
  DEBUG_printString("\r\nBoot Loader BootMode = ");

   // Select Boot Mode
#if defined(UBL_NAND)
    {
      //Report Bootmode to host
      DEBUG_printString("NAND\r\n");

      // Copy binary image application from NAND to RAM
      if (NANDBOOT_copy() != E_PASS)
      {
        DEBUG_printString("NAND Boot failed.\r\n");
        LOCAL_bootAbort();
      }
    }
#elif defined(UBL_NOR)
    {
      //Report Bootmode to host
    DEBUG_printString("NOR \r\n");

    // Copy binary application image from NOR to RAM
    if (NORBOOT_copy() != E_PASS)
    {
      DEBUG_printString("NOR Boot failed.\r\n");
      LOCAL_bootAbort();
    }
    }
#elif defined(UBL_SD_MMC)
  {
    //Report Bootmode to host
    DEBUG_printString("SD/MMC \r\n");

    // Copy binary of application image from SD/MMC card to RAM
    if (SDMMCBOOT_copy() != E_PASS)
    {
      DEBUG_printString("SD/MMC Boot failed.\r\n");
      LOCAL_bootAbort();
    }
  }
#else
  {
    //Report Bootmode to host
    DEBUG_printString("UART\r\n");
    UARTBOOT_copy();
  }


#endif

  DEBUG_printString("Boot Mode Task Completed\r\n");

  DEVICE_TIMER0Stop();

  return E_PASS;
}

static void LOCAL_bootAbort(void)
{
  DEBUG_printString("Aborting...\r\n");
  while (TRUE);
}

/************************************************************
* End file                                                  *
************************************************************/
