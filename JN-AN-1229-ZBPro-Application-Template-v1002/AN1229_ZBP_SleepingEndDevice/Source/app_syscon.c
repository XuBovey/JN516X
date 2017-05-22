/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_syscon.c
 *
 * DESCRIPTION:			System Controller Event Handler
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

#include "DBG.h"
#include "pwrm.h"
#include "AppHardwareApi.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifndef TRACE_APP_SYSCON
#define TRACE_APP_SYSCON            FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_SysConISR
 *
 * DESCRIPTION:
 * Interrupt
 *
 ****************************************************************************/

#if JENNIC_CHIP_FAMILY == JN517x
void APP_isrSysCon ( uint32 u32Device, uint32 u32ItemBitmap )
{

    if ( u32Device == E_AHI_DEVICE_SYSCTRL)
    {
        if (u32ItemBitmap & E_AHI_SYSCTRL_WK1_MASK)
        {
            /* wake timer interrupt got us here */
            DBG_vPrintf(TRACE_APP_SYSCON, "APP: Wake Timer 1 Interrupt\n");
            PWRM_vWakeInterruptCallback();
        }

        if (u32ItemBitmap & E_AHI_SYSCTRL_WK0_MASK)
        {
            DBG_vPrintf(TRACE_APP_SYSCON, "APP: Wake Timer 0 Interrupt\n");
        }
    }

    if ( u32Device == E_AHI_DEVICE_SYSCTRL_GPIO_EXT &&
         u32ItemBitmap != 0 )
    {
        /* check whether interrupt source was DIO e.g. button presses,
         * or from the Comparator
         */
    }
}
#else
void APP_isrSysCon (void)
{

    /* clear pending DIO changed bits by reading register */
    uint32 u32IntSource = u32AHI_DioWakeStatus();
    uint8 u8WakeInt = u8AHI_WakeTimerFiredStatus();

    if (u8WakeInt & E_AHI_WAKE_TIMER_MASK_1)
    {
        /* wake timer interrupt got us here */
        DBG_vPrintf(TRACE_APP_SYSCON, "APP: Wake Timer 1 Interrupt\n");
        PWRM_vWakeInterruptCallback();
    }

    if (u8WakeInt & E_AHI_WAKE_TIMER_MASK_0)
    {
        DBG_vPrintf(TRACE_APP_SYSCON, "APP: Wake Timer 0 Interrupt\n");
    }

    if (u32IntSource)
    {
        /* check whether interrupt source was DIO e.g. button presses,
         * or from the Comparator
         */
    }
}
#endif
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
