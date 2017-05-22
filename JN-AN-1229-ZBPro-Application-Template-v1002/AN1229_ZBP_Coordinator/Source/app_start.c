/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_start.c
 *
 * DESCRIPTION:			Coordinator Initialization and Startup
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
#include "app_common.h"
#include <app_coordinator.h>
#include <zps_nwk_pub.h>
#include "pdum_gen.h"
#include "ZQueue.h"
#include "portmacro.h"
#include "zps_apl_af.h"
#include "mac_vs_sap.h"
#include "AppHardwareApi.h"
#include "dbg.h"
#include "ZTimer.h"
#include "app_blink_led.h"
#include <string.h>

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

#if JENNIC_CHIP_FAMILY == JN517x
#define NVIC_INT_PRIO_LEVEL_BBC     (7)
#endif
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void APP_vInitResources(void);
PUBLIC void APP_vSetUpHardware(void);
PUBLIC void app_vMainloop(void);
PRIVATE void vInitialiseApp(void);
PRIVATE void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus);
PRIVATE void app_vCmdSend(uint8* buffer, uint16 length, uint16 u16NwkAddr);
PRIVATE void app_vGetUartCmd(void);
PRIVATE void app_vCmdListPrint(void);
PRIVATE uint8 app_u8MacAddrAndIeeeAddrPrint(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC uint8 u8App_tmr1sec;
PUBLIC uint8 u8TimerBlink;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/*local variables*/
tszQueue APP_msgLedsEvents;

PRIVATE zps_tsTimeEvent asTimeEvent[TIMER_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMlmeVsDcfmInd  asMacMlmeVsDcfmInd[MLME_QUEQUE_SIZE];
PRIVATE ZPS_tsAfEvent asAppEvents[APP_QUEUE_SIZE];
PRIVATE ZPS_tsAfEvent asStackEvents[ZPS_QUEUE_SIZE];
PRIVATE APP_tsLedsEvent asAppLedsEvents[APP_LED_QUEUE_SIZE];

PRIVATE ZTIMER_tsTimer asTimers[4];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern void vAHI_WatchdogRestart(void);
extern void PWRM_vManagePower(void);
extern void zps_taskZPS(void);

/*****************************************************************************
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

#if JENNIC_CHIP_FAMILY==JN516x
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
    /* Define HIGH_POWER_ENABLE to enable high power module */
    #ifdef HIGH_POWER_ENABLE
        vAHI_HighPowerModuleEnable(TRUE, TRUE);
    #endif

    /* Initialise various components */
    vInitialiseApp();
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
    /* nothing to register as device does not sleep */
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/*****************************************************************************
 *
 * NAME: vInitialiseApp
 *
 * DESCRIPTION:
 * Initialises JenOS modules and application.
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

    APP_vInitResources();
    APP_vSetUpHardware();
    /* initialise application */
    APP_vInitialiseCoordinator();
    APP_vInitLeds();
    app_vMainloop();

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
    u32AHI_Init();
    vAHI_InterruptSetPriority(MICRO_ISR_MASK_BBC,     NVIC_INT_PRIO_LEVEL_BBC );
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
    ZTIMER_eOpen(&u8App_tmr1sec,    NULL,    NULL, ZTIMER_FLAG_PREVENT_SLEEP);
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
    /* idle task commences on exit from OS start call */
    while (TRUE) {
        zps_taskZPS();
        APP_vtaskCoordinator();
        APP_vtaskMyEndPoint();

        App_vLedControl();
        app_vGetUartCmd();

        ZTIMER_vTask();
        /* kick the watchdog timer */
        vAHI_WatchdogRestart();
        PWRM_vManagePower();
    }
}

PRIVATE uint8 app_u8MacAddrAndIeeeAddrPrint(void)
{
	uint16 u16DstAddr;
	uint16 nodeNum = 0;

	do
	{
		u16DstAddr = ZPS_u16AplZdoLookupAddr(ZPS_u64NwkNibGetMappedIeeeAddr(ZPS_pvAplZdoGetNwkHandle(),nodeNum));
		DBG_vPrintf(TRACE_APP, "NodeNum: %d, NwkAddr: 0x%04x, IeeeAddr:0x%08x.\n",
					nodeNum,
					u16DstAddr,
					ZPS_u64NwkNibGetMappedIeeeAddr(ZPS_pvAplZdoGetNwkHandle(),nodeNum));
		nodeNum ++;
	}while (0xFFFE != u16DstAddr);

	return nodeNum;
}


PRIVATE void app_vCmdListPrint(void)
{
	DBG_vPrintf(TRACE_APP, ">>>:-----------------------------\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED1_ON        [1].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED1_OFF       [2].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED2_ON        [3].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED2_OFF       [4].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED3_ON        [5].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED3_OFF       [6].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED1_BLINK_ON  [7].\n");
	DBG_vPrintf(TRACE_APP, ">>>:APP_EVENT_LED1_BLINK_OFF [8].\n");
	DBG_vPrintf(TRACE_APP, ">>>:-----------------------------\n");
}


typedef enum {
	APP_CMD_IDLE,                                   /*  0, 0x00 */
	APP_CMD_DEVICE_SEL,                             /*  1, 0x01 */
    APP_CMD_DEVICE_HANDLE,                          /*  2, 0x02 */
} APP_teCmdStep;

PRIVATE void app_vGetUartCmd(void)
{
	static uint16 u16_NodeNwkAddr;
	static APP_teCmdStep te_cmdStep = APP_CMD_IDLE;
	uint8 u8_InputData;
	static uint8 u8_NodeCount;
	uint8 u8_NodeNum;
	uint8 u8_CmdCode;

	u8_InputData = DBG_iGetChar();

	if(u8_InputData <= 0x7F)
	{
		DBG_vPrintf(TRACE_APP, "u input %c.\n", u8_InputData);
		if(u8_InputData == '#')
		{
			te_cmdStep = APP_CMD_DEVICE_SEL;
			u8_NodeCount = app_u8MacAddrAndIeeeAddrPrint();
			DBG_vPrintf(TRACE_APP, "\n");
			DBG_vPrintf(TRACE_APP, "<<<: Please select the Node Device[0-%d].\n",u8_NodeCount-1);
			DBG_vPrintf(TRACE_APP, "<<<: [%d] is the current Node.\n",u8_NodeCount-1);
		}

		switch (te_cmdStep)
		{
		case APP_CMD_DEVICE_SEL:
			u8_NodeNum = u8_InputData - '0';
			if((u8_NodeNum <= u8_NodeCount) && (u8_NodeNum >= 0))
			{
				u16_NodeNwkAddr = ZPS_u16AplZdoLookupAddr(ZPS_u64NwkNibGetMappedIeeeAddr(ZPS_pvAplZdoGetNwkHandle(),u8_NodeNum));
				te_cmdStep = APP_CMD_DEVICE_HANDLE;
				app_vCmdListPrint();
				DBG_vPrintf(TRACE_APP, "<<<: Please select the Cmd[0-9].\n");
			}
		break;
		case APP_CMD_DEVICE_HANDLE:
			u8_CmdCode = u8_InputData - '0';
			if((u8_CmdCode < 9) && (u8_CmdCode > 0))
			{
				if(0xFFFE != u16_NodeNwkAddr)
				{
				app_vCmdSend(&u8_CmdCode, 1, u16_NodeNwkAddr);
				te_cmdStep = APP_CMD_IDLE;
				}else
				{
					APP_tsLedsEvent ledEvent;
					ledEvent.eEvent =(APP_teLedsEventType)(u8_CmdCode);
					ZQ_bQueueSend(&APP_msgLedsEvents, &ledEvent);
				}
			}else
			{
				DBG_vPrintf(TRACE_APP, "<<<: ERR: Get an error Cmd %d. Retry or input '#' to restart.\n", u8_CmdCode);
			}
		break;
		case APP_CMD_IDLE:
		default:
			DBG_vPrintf(TRACE_APP, "<<<: Please input '#' to continue.\n");
		}
	}
}


PRIVATE void app_vCmdSend(uint8* buffer, uint16 length, uint16 u16NwkAddr)
{
	PDUM_thAPduInstance hAPduInst;
	hAPduInst = PDUM_hAPduAllocateAPduInstance(apduMyData);

	if (hAPduInst == PDUM_INVALID_HANDLE)
	{
		DBG_vPrintf(TRACE_APP, ">>>cmdSend: Cmd Data Send ERR_AlloCateAPduInstance!!\n");
	}else
	{
		ZPS_teStatus eStatus;

		uint16 u16ClusterId = 0x13;
		uint8 u8SrcEndpoint = 1;
		uint8 u8DstEndpoint = 1; //MyEndPoint
		uint8 u8Radius = 0;
		uint16 u16Offset = 0;
		uint16 i = 0;
		ZPS_teAplAfSecurityMode  eSecurityMode = (ZPS_E_APL_AF_SECURE_NWK);

		DBG_vPrintf(TRACE_APP, "%d chars need to send.\n", length);
		if (length < 100) {
		  for (i = 0; i < length; i++) {
			   u16Offset += PDUM_u16APduInstanceWriteNBO(hAPduInst, u16Offset,"b", *(buffer + i));
			   DBG_vPrintf(TRACE_APP, "0x%02x  ", *(buffer + i));
		  }
		  PDUM_eAPduInstanceSetPayloadSize(hAPduInst, u16Offset);
		} else {
		  DBG_vPrintf(TRACE_APP, "ERR: Payload length exceeded\n");
		}

		eStatus= ZPS_eAplAfUnicastDataReq(
										hAPduInst,
										u16ClusterId,
										u8SrcEndpoint,
										u8DstEndpoint,
										u16NwkAddr,
										eSecurityMode,
										u8Radius,
										NULL
				                     );

		if (eStatus)
		{
			PDUM_eAPduFreeAPduInstance(hAPduInst);
			DBG_vPrintf(TRACE_APP, ">>>cmdSend:Cmd Data Semd Error: 0x%02x\n", eStatus);
		} else { DBG_vPrintf(TRACE_APP, ">>>cmdSend: Cmd Data Send OK\n"); }
	}
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
