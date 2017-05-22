/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_endpoint.c
 *
 * DESCRIPTION:			End Point Event Handler
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
#include <dbg.h>
#include <zps_apl_af.h>
#include "app_common.h"

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

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
tszQueue APP_msgMyEndPointEvents;
tszQueue APP_msgZpsEvents;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: APP_taskEndpoint
 *
 * DESCRIPTION:
 * End Point event handling
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vtaskMyEndPoint ( void )
{
    ZPS_tsAfEvent sStackEvent;
    sStackEvent.eType = ZPS_EVENT_NONE;

    /* check if any messages to collect */
    if ( ZQ_bQueueReceive(&APP_msgMyEndPointEvents, &sStackEvent))
    {
        DBG_vPrintf(TRACE_APP, "APP: No event to process\n");
    }

    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        switch (sStackEvent.eType)
        {
            case ZPS_EVENT_APS_INTERPAN_DATA_INDICATION:
            {
                  DBG_vPrintf(TRACE_APP, "APP: event  ZPS_EVENT_APS_INTERPAN_DATA_INDICATION\n");
                  PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
            }
            break;

            case ZPS_EVENT_APS_ZGP_DATA_INDICATION:
            {
            	DBG_vPrintf(TRACE_APP, "APP: event  ZPS_EVENT_APS_ZGP_DATA_INDICATION\n");
                if (sStackEvent.uEvent.sApsZgpDataIndEvent.hAPduInst)
                {
                    PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsZgpDataIndEvent.hAPduInst);
                }
            }
            break;

            case ZPS_EVENT_APS_DATA_INDICATION:
            {
            	DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: ZPS_EVENT_AF_DATA_INDICATION\n");

                /* Process incoming cluster messages for this endpoint... */
                DBG_vPrintf(TRACE_APP, "    Data Indication:\r\n");
                DBG_vPrintf(TRACE_APP, "        Profile :%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u16ProfileId);
                DBG_vPrintf(TRACE_APP, "        Cluster :%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u16ClusterId);
                DBG_vPrintf(TRACE_APP, "        EndPoint:%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint);

                /* free the application protocol data unit (APDU) once it has been dealt with */
                PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
            }
            break;

            case ZPS_EVENT_APS_DATA_CONFIRM:
            {
            	DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: ZPS_EVENT_APS_DATA_CONFIRM Status %d, Address 0x%04x\n",
                            sStackEvent.uEvent.sApsDataConfirmEvent.u8Status,
                            sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u16Addr);
            }
            break;

            case ZPS_EVENT_APS_DATA_ACK:
            {
            	DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: ZPS_EVENT_APS_DATA_ACK Status %d, Address 0x%04x\n",
                            sStackEvent.uEvent.sApsDataAckEvent.u8Status,
                            sStackEvent.uEvent.sApsDataAckEvent.u16DstAddr);
            }
            break;

            default:
            {
            	DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: unhandled event %d\n", sStackEvent.eType);
            }
            break;
        }
    }
}
/****************************************************************************
 *
 * NAME: APP_vGenCallback
 *
 * DESCRIPTION:
 * Stack callback
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vGenCallback(uint8 u8Endpoint, ZPS_tsAfEvent *psStackEvent)
{
    if ( u8Endpoint == 0 )
    {
    	ZQ_bQueueSend(&APP_msgZpsEvents, (void*) psStackEvent);
    }
    else
    {
    	ZQ_bQueueSend(&APP_msgMyEndPointEvents, (void*) psStackEvent);
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
