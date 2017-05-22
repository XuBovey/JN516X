/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_start_SED.c
 *
 * DESCRIPTION:			Sleeping EndDevice Initialization and Startup
 *
 *****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5169, JN5168,
 * JN5164, JN5161].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2015. All rights reserved
 *
 ****************************************************************************/
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppApi.h>
#include <pwrm.h>
#include <dbg.h>
#include <dbg_uart.h>
#include <app_pdm.h>
#include "app_sleeping_enddevice.h"
#include <zps_nwk_pub.h>
#include "app_common.h"
#include "pdum_gen.h"
#include "portmacro.h"
#include "zps_apl_af.h"
#include "mac_vs_sap.h"
#include "AppHardwareApi.h"
#include "dbg.h"
#include "ZTimer.h"
#include "app_blink_led.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_APP
	#define TRACE_APP 	FALSE
#else
	#define TRACE_APP 	TRUE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/*#defines*/
#define TIMER_QUEUE_SIZE             8
#define MLME_QUEQUE_SIZE             4
#define MCPS_QUEUE_SIZE             24
#define ZPS_QUEUE_SIZE               2
#define APP_QUEUE_SIZE               2
#define APP_LED_QUEUE_SIZE           2

#define APP_JOINING_BLINK_TIME          (ZTIMER_TIME_MSEC(1000))

#if JENNIC_CHIP_FAMILY == JN517x
#define NVIC_INT_PRIO_LEVEL_SYSCTRL (3)
#define NVIC_INT_PRIO_LEVEL_BBC     (7)
#define NVIC_INT_PRIO_LEVEL_UART0   (4)
#endif
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void APP_vInitResources(void);
PUBLIC void APP_vSetUpHardware(void);
PUBLIC void app_vMainloop(void);
PRIVATE void vInitialiseApp(void);
PRIVATE void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus);
PRIVATE PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);
PRIVATE PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);


PUBLIC uint8 u8App_tmr1sec;
PUBLIC uint8 u8TimerBlink;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/*local variables*/
PRIVATE zps_tsTimeEvent asTimeEvent[TIMER_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMlmeVsDcfmInd  asMacMlmeVsDcfmInd[MLME_QUEQUE_SIZE];
PRIVATE ZPS_tsAfEvent asAppEvents[APP_QUEUE_SIZE];
PRIVATE ZPS_tsAfEvent asStackEvents[ZPS_QUEUE_SIZE];
PRIVATE APP_tsLedsEvent asAppLedsEvents[APP_LED_QUEUE_SIZE];

PRIVATE ZTIMER_tsTimer asTimers[4];

tszQueue APP_msgLedsEvents;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern void vAHI_WatchdogRestart(void);
extern void PWRM_vManagePower(void);
extern void zps_taskZPS(void);

/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 * Entry point for application from a cold start.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
PUBLIC void vAppMain(void)
{
	/* Wait until FALSE i.e. on XTAL  - otherwise uart data will be at wrong speed */
    while (bAHI_GetClkSource() == TRUE);
    /* Now we are running on the XTAL, optimise the flash memory wait states. */
    vAHI_OptimiseWaitStates();

    /* Initialise the debug diagnostics module to use UART0 at 115K Baud;
     * Do not use UART 1 if LEDs are used, as it shares DIO with the LEDS
     */
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "\n\nAPP: Power Up\n");
#if JENNIC_CHIP_FAMILY == JN516x
    /*
     * Initialise the stack overflow exception to trigger if the end of the
     * stack is reached. See the linker command file to adjust the allocated
     * stack size.
     */
    vAHI_SetStackOverflow(TRUE, (uint32)&_stack_low_water_mark);
#endif
    /*
     * Catch resets due to watchdog timer expiry. Comment out to harden code.
     */
    if (bAHI_WatchdogResetEvent())
    {
        DBG_vPrintf(TRACE_APP, "APP: Watchdog timer has reset device!\n");
        vAHI_WatchdogStop();
        while (1);
    }

    /* Initialize ROM based software modules */
    #ifndef JENNIC_MAC_MiniMacShim
        (void)u32AppApiInit(NULL, NULL, NULL, NULL, NULL, NULL);
    #endif
    /* Define HIGH_POWER_ENABLE to enable high power module */
    #ifdef HIGH_POWER_ENABLE
        vAHI_HighPowerModuleEnable(TRUE, TRUE);
    #endif
    /* initialise application */
    APP_vInitLeds();
    APP_vInitResources();
    APP_vSetUpHardware();
    vInitialiseApp();

    app_vMainloop();

}

/*****************************************************************************
 *
 * NAME: vAppRegisterPWRMCallbacks
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
void vAppRegisterPWRMCallbacks(void)
{
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vInitialiseApp
 *
 * DESCRIPTION:
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInitialiseApp(void)
{
    /* Initialise JenOS modules. Initialise Power Manager even on non-sleeping nodes
     * as it allows the device to doze when in the idle task
     */
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);

    /*
     *  Initialise the PDM, use an application supplied key (g_sKey),
     *  The key value can be set to the desired value here, or the key in eFuse can be used.
     *  To use the key stored in eFuse set the pointer to the key to Null, and remove the
     *  key structure here.
     */
#ifdef PDM_EEPROM
	PDM_eInitialise(63);
	PDM_vRegisterSystemCallback(vPdmEventHandlerCallback);
#else
	PDM_vInit(7, 1, 64 * 1024 , NULL, NULL, NULL, &g_sKey);
#endif
    /* Initialise Protocol Data Unit Manager */
    PDUM_vInit();

    /* Register callback that provides information about stack errors */
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    /* Initialize application */
    APP_vInitialiseSleepingEndDevice();
}

/****************************************************************************
 *
 * NAME: PreSleep
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRACE_APP, "APP: Going to sleep (CB)\n");
    vAppApiSaveMacSettings();
    ZTIMER_vSleep();
}

/****************************************************************************
 *
 * NAME: Wakeup
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
PWRM_CALLBACK(Wakeup)
{
	/* Wait until FALSE i.e. on XTAL  - otherwise uart data will be at wrong speed */
    while (bAHI_GetClkSource() == TRUE);
    /* Now we are running on the XTAL, optimise the flash memory wait states. */
    vAHI_OptimiseWaitStates();

    /* Initialise the debug diagnostics module to use UART0 at 115K Baud;
     * Do not use UART 1 if LEDs are used, as it shares DIO with the LEDS
     */
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "\n\nAPP: Woken up (CB)\n");

    /* Restore Mac settings (turns radio on) */
    vMAC_RestoreSettings();

    APP_vSetUpHardware();
    ZTIMER_vWake();
}

/****************************************************************************
 *
 * NAME: vfExtendedStatusCallBack
 *
 * DESCRIPTION:
 * ZPS extended error callback .
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
	DBG_vPrintf(TRACE_APP, "ERROR: Extended status %x\n", eExtendedStatus);
}


/****************************************************************************
 *
 * NAME: APP_vSetUpHardware
 *
 * DESCRIPTION:
 * Set up interrupts
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vSetUpHardware(void)
{
#if (JENNIC_CHIP_FAMILY == JN517x)
	extern void APP_isrSysCon ( uint32 u32Device, uint32 u32ItemBitmap );
    vAHI_SysCtrlRegisterCallback ( APP_isrSysCon );
    u32AHI_Init();
    vAHI_InterruptSetPriority(MICRO_ISR_MASK_BBC,     NVIC_INT_PRIO_LEVEL_BBC );
    vAHI_InterruptSetPriority(MICRO_ISR_MASK_SYSCTRL, NVIC_INT_PRIO_LEVEL_SYSCTRL );
#else
    TARGET_INITIALISE();
    /* clear interrupt priority level  */
    SET_IPL(0);
    portENABLE_INTERRUPTS();
#endif
}


/****************************************************************************
 *
 * NAME: APP_vInitResources
 *
 * DESCRIPTION:
 * Initialise resources (timers, queue's etc)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitResources(void)
{

    /* Initialise the Z timer module */
    ZTIMER_eInit(asTimers, sizeof(asTimers) / sizeof(ZTIMER_tsTimer));

    /* Create Z timers */
    ZTIMER_eOpen(&u8App_tmr1sec,    NULL,               NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerBlink,     vAPP_cbBlinkLED,    NULL, ZTIMER_FLAG_PREVENT_SLEEP);

    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd,         MLME_QUEQUE_SIZE,      sizeof(MAC_tsMlmeVsDcfmInd), (uint8*)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd,         MCPS_QUEUE_SIZE,       sizeof(MAC_tsMcpsVsDcfmInd), (uint8*)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_TimeEvents,             TIMER_QUEUE_SIZE,      sizeof(zps_tsTimeEvent),     (uint8*)asTimeEvent);
    ZQ_vQueueCreate(&APP_msgZpsEvents,           APP_QUEUE_SIZE,        sizeof(ZPS_tsAfEvent),       (uint8*)asAppEvents);
    ZQ_vQueueCreate(&APP_msgMyEndPointEvents,    ZPS_QUEUE_SIZE,        sizeof(ZPS_tsAfEvent),       (uint8*)asStackEvents);
    ZQ_vQueueCreate(&APP_msgLedsEvents,          APP_LED_QUEUE_SIZE,    sizeof(APP_tsLedsEvent),     (uint8*)asAppLedsEvents);
}

/****************************************************************************
 *
 * NAME: APP_vMainLoop
 *
 * DESCRIPTION:
 * Main application loop
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void app_vMainloop(void)
{
	//int dbg_GetChar;
    /* idle task commences on exit from OS start call */
    while (TRUE) {
        zps_taskZPS();
        APP_vtaskSleepingEndDevice();
        APP_vtaskMyEndPoint();
        App_vLedControl();
        ZTIMER_vTask();

        /*dbg_GetChar = DBG_iGetChar();
		if(dbg_GetChar != -1)
		{
			DBG_vPrintf(TRACE_APP, ">>>:close LED3 [1].\n");
			DBG_vPrintf(TRACE_APP, ">>>:Open  LED3 [2].\n");
			DBG_vPrintf(TRACE_APP, ">>>:Blink LED1 [3].\n");
			DBG_vPrintf(TRACE_APP, ">>>:Stop Blink LED1 [4].\n");
			DBG_vPrintf(TRACE_APP, ">>>:The input cmd was %c ^_^.\n", dbg_GetChar);
			switch (dbg_GetChar)
			{
			case '1': APP_vSetLED(LED3, FALSE); break;
			case '2': APP_vSetLED(LED3, TRUE); break;
			case '3': vStartBlinkTimer(APP_JOINING_BLINK_TIME); break;
			case '4': vStopBlinkTimer(); break;
			default : break;
			}
		}*/

        /* kick the watchdog timer */
        vAHI_WatchdogRestart();
        PWRM_vManagePower();
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
