/*****************************************************************************
 *
 * MODULE:          JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:       app_blink_led.c
 *
 * DESCRIPTION:     ZLO Demo: Blinks an LED a number of pre defined ticks
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5179].
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
 * Copyright NXP B.V. 2016. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "dbg.h"
#include "app_blink_led.h"
#include "AppHardwareApi.h"
#include "ZTimer.h"
#include "pwrm.h"
#include "ZQueue.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DEBUG_BLINK_LED
    #define TRACE_BLINK_LED   TRUE
#else
    #define TRACE_BLINK_LED   FALSE
#endif
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#define BLINK_LED	LED1
#define APP_JOINING_BLINK_TIME          (ZTIMER_TIME_MSEC(1000))
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vToggleLED(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern PUBLIC uint8 u8TimerBlink;
extern tszQueue APP_msgLedsEvents;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE bool_t bDIO1State = FALSE;
PRIVATE uint32 u32BlinkTickTime = 0;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_vInitLeds
 *
 * DESCRIPTION:
 * Initialises LED's
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitLeds(void)
{

    vAHI_DioSetDirection(0, BOARD_LED_CTRL_MASK);

}

/****************************************************************************
 *
 * NAME: APP_vSetLED
 *
 * DESCRIPTION:
 * Initialises LED's
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vSetLED(uint8 u8Led, bool_t bOn)
{
    if ( u8Led == LED1)
    {
        if (bOn)
        {
        	/* leds on carrier driven low for on */
            vAHI_DioSetOutput(0, BOARD_LED_D1_PIN);
        }
        else
        {
            vAHI_DioSetOutput(BOARD_LED_D1_PIN, 0);
        }
    }
    else if ( u8Led == LED2)
    {
        if (bOn)
        {
        	/* leds on carrier driven low for on */
        	vAHI_DioSetOutput(0, BOARD_LED_D2_PIN);
        }
        else
        {

            vAHI_DioSetOutput(BOARD_LED_D2_PIN , 0);
        }
    }
    else if ( u8Led == LED3)
    {
        if (bOn)
        {
            /* leds on carrier driven low for on */
            vAHI_DioSetOutput( 0, BOARD_LED_D3_PIN);
        }
        else
        {
            vAHI_DioSetOutput(BOARD_LED_D3_PIN, 0);
        }
    }
}

/****************************************************************************
 *
 * NAME: APP_BlinkLED
 *
 * DESCRIPTION:
 * Toggles LED 2 and restarts the timer.
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAPP_cbBlinkLED( void *pvParams)
{
    DBG_vPrintf(TRACE_BLINK_LED, "\nAPP Blink LED: Task Started");
    vToggleLED();
    ZTIMER_eStart(u8TimerBlink, u32BlinkTickTime);
}

/****************************************************************************
 *
 * NAME: vStartBlinkTimer
 *
 * DESCRIPTION:
 * Starts the software timer and blinks it the desired amount of ticks. It also
 * stores the time so the Task can restart it with the defined time
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vStartBlinkTimer(uint32 u32Ticks)
{
    DBG_vPrintf(TRACE_BLINK_LED, "\nAPP Blink LED: Starting Blink Timer value = %d", u32Ticks);
    u32BlinkTickTime = u32Ticks;
    ZTIMER_eStart(u8TimerBlink, u32Ticks);
}

/****************************************************************************
 *
 * NAME: vStopBlinkTimer
 *
 * DESCRIPTION:
 * Stops the blink timer and turns the LED off
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vStopBlinkTimer(void)
{
    DBG_vPrintf(TRACE_BLINK_LED, "\nAPP Blink LED: Stopping Blink Timer");
    ZTIMER_eStop(u8TimerBlink);
    u32BlinkTickTime = 0;
    APP_vSetLED(BLINK_LED, FALSE);
}

/****************************************************************************/
/***        Local Function 			                                      ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vToggleLED
 *
 * DESCRIPTION:
 * Changes the state of LED 2
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vToggleLED(void)
{
    DBG_vPrintf(TRACE_BLINK_LED, "\nAPP Blink LED: Toggle LED to %b", !bDIO1State);
    bDIO1State = !bDIO1State;
    APP_vSetLED(BLINK_LED, bDIO1State);
}

/****************************************************************************
 *
 * NAME: App_vLedControl
 *
 * DESCRIPTION:
 * Control Leds Status
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void App_vLedControl ( void )
{
	APP_tsLedsEvent sLedsEvent;

	/* check if any messages to collect */
	if ( ZQ_bQueueReceive(&APP_msgLedsEvents, &sLedsEvent) )
	{
		DBG_vPrintf(TRACE_BLINK_LED, "APP LEDs: Get a message to process: %d.\n", (int)sLedsEvent.eEvent);
		switch (sLedsEvent.eEvent)
		{
			case APP_EVENT_LED1_ON:  APP_vSetLED(LED1, TRUE); DBG_vPrintf(TRACE_BLINK_LED, "LED1 ON\n");break;
			case APP_EVENT_LED1_OFF: APP_vSetLED(LED1, FALSE); vStopBlinkTimer(); DBG_vPrintf(TRACE_BLINK_LED, "LED1 OFF\n");break;
			case APP_EVENT_LED2_ON:  APP_vSetLED(LED2, TRUE); DBG_vPrintf(TRACE_BLINK_LED, "LED2 ON\n");break;
			case APP_EVENT_LED2_OFF: APP_vSetLED(LED2, FALSE); DBG_vPrintf(TRACE_BLINK_LED, "LED2 OFF\n");break;
			case APP_EVENT_LED3_ON:  APP_vSetLED(LED3, TRUE); DBG_vPrintf(TRACE_BLINK_LED, "LED3 ON\n");break;
			case APP_EVENT_LED3_OFF: APP_vSetLED(LED3, FALSE); DBG_vPrintf(TRACE_BLINK_LED, "LED3 OFF\n");break;
			case APP_EVENT_LED1_BLINK_ON:  vStartBlinkTimer(APP_JOINING_BLINK_TIME); DBG_vPrintf(TRACE_BLINK_LED, "LED1 blink\n");break;
			case APP_EVENT_LED1_BLINK_OFF: vStopBlinkTimer(); DBG_vPrintf(TRACE_BLINK_LED, "LED1 OFF\n");break;
			default : DBG_vPrintf(TRACE_BLINK_LED, "Sorry the LED Cmd[%d] not support.\n",(int)sLedsEvent.eEvent);break;
		}
	}
	else
	{
		//DBG_vPrintf(TRACE_BLINK_LED, "APP LEDs: Get a message to process\n");
	}


}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
