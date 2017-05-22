/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_common.h
 *
 * DESCRIPTION:			Application Device Type Includes
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
#ifndef APP_COMMON_H_
#define APP_COMMON_H_

/* Device specific includes */
#ifdef Coordinator
    #include "App_Coordinator.h"
#endif

#ifdef Router
    #include "App_Router.h"
#endif

#ifdef SleepingEndDevice
    #include "App_SED.h"
#endif

#include "ZQueue.h"
#include "ZTimer.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
//#define POLL_TIME               APP_TIME_MS(1000)
//#define POLL_TIME_FAST          APP_TIME_MS(100)
//#define TEN_HZ_TICK_TIME        APP_TIME_MS(100)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_STARTUP,
    E_LEAVE_WAIT,
    E_LEAVE_RESET,
    E_NFN_START,
    E_DISCOVERY,
    E_NETWORK_FORMATION,
    E_JOINING_NETWORK,
    E_REJOINING,
    E_NETWORK_INIT,
    E_RESCAN,
    E_RUNNING
} teNODE_STATES;

typedef struct
{
    teNODE_STATES   eNodeState;
    teNODE_STATES   eNodePrevState;
}tsDeviceDesc;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void APP_vtaskMyEndPoint ( void );
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tszQueue zps_msgMlmeDcfmInd;
extern tszQueue zps_msgMcpsDcfmInd;
extern tszQueue zps_TimeEvents;
extern tszQueue APP_msgMyEndPointEvents;
extern tszQueue APP_msgZpsEvents;
extern tszQueue APP_msgLedsEvents;
extern uint8 u8App_tmr1sec;
extern uint8 u8TimerBlink;
extern void *_stack_low_water_mark;
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

#endif /*APP_COMMON_H_*/
