/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_coordinator.c
 *
 * DESCRIPTION:			Coordinator Application
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
#include <pdm.h>
#include <pwrm.h>
#include <zps_apl_af.h>
#include <PDM_IDs.h>
#include "app_common.h"
#include "app_coordinator.h"
#include "ZTimer.h"
#include "ZQueue.h"
#include "pdum_gen.h"

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
PRIVATE void vStartup(void);
PRIVATE void vWaitForNetworkFormation(ZPS_tsAfEvent sStackEvent);
PRIVATE void vHandleStackEvent(ZPS_tsAfEvent sStackEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsDeviceDesc s_eDeviceState;
PUBLIC uint8 au8DefaultTCLinkKey[16]    = {0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c,
                                           0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39};

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: APP_vInitialiseCoordinator
 *
 * DESCRIPTION:
 * Initialises the Coordinator application
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitialiseCoordinator(void)
{
    bool_t bDeleteRecords = TRUE;
    uint16 u16DataBytesRead;

    /* If required, at this point delete the network context from flash, perhaps upon some condition
     * For example, check if a button is being held down at reset, and if so request the Persistent
     * Data Manager to delete all its records:
     * e.g. bDeleteRecords = vCheckButtons();
     * Alternatively, always call PDM_vDelete() if context saving is not required.
     */
    if (bDeleteRecords)
    {
        DBG_vPrintf(TRACE_APP, "APP: Deleting all records from flash\n");
        PDM_vDeleteAllDataRecords();
    }

    /* Restore any application data previously saved to flash
     * All Application records must be loaded before the call to
     * ZPS_eAplAfInit
     */
    s_eDeviceState.eNodeState = E_STARTUP;
    PDM_eReadDataFromRecord(PDM_ID_APP_COORD,
                      		&s_eDeviceState,
                       		sizeof(s_eDeviceState),
                       		&u16DataBytesRead);

    /* Initialise ZBPro stack */
    ZPS_eAplAfInit();
    ZPS_vAplSecSetInitialSecurityState(ZPS_ZDO_PRECONFIGURED_LINK_KEY,
                                       au8DefaultTCLinkKey,
                                       0x00,
                                       ZPS_APS_GLOBAL_LINK_KEY);

    /* Initialise other software modules
     * HERE
     */

    /* Always initialise any peripherals used by the application
     * HERE
     */

    /* If the device state has been restored from flash, re-start the stack
     * and set the application running again. Note that if there is more than 1 state
     * where the network has already joined, then the other states should also be included
     * in the test below
     * E.g. E_RUNNING_1, E_RUNNING_2......
     * if (E_RUNNING_1 == s_sDevice || E_RUNNING_2 == s_sDevice)
     */
    if (E_RUNNING == s_eDeviceState.eNodeState)
    {
        ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();

        DBG_vPrintf(TRACE_APP, "APP: Re-starting Stack....\r\n");

        if (ZPS_E_SUCCESS != eStatus)
        {
            DBG_vPrintf(TRACE_APP, "APP: ZPS_eZdoStartStack() failed error %d", eStatus);
        }

        /* Turn on joining */
        ZPS_eAplZdoPermitJoining(0xff);

        /* Re-start any other application software modules
         * HERE
         */
    }
    else /* perform any actions require on initial start-up */
    {
        /* Return the device to the start-up start if it was reset during the network formation stage */
        s_eDeviceState.eNodeState = E_STARTUP;
    }
}

/****************************************************************************
 *
 * NAME: APP_vtaskCoordinator
 *
 * DESCRIPTION:
 * Main state machine
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vtaskCoordinator ( void )
{
    ZPS_tsAfEvent sStackEvent;
    sStackEvent.eType = ZPS_EVENT_NONE;

    /* check if any messages to collect */
	if (ZQ_bQueueReceive (&APP_msgZpsEvents, &sStackEvent) )
	{
		DBG_vPrintf(TRACE_APP, "APP: No event to process!\n");
	}

    if ( ZTIMER_eGetState(u8App_tmr1sec) == E_ZTIMER_STATE_EXPIRED )
    {
    	ZTIMER_eStart (u8App_tmr1sec, ZTIMER_TIME_SEC(1) );
    }

    switch (s_eDeviceState.eNodeState)
    {
        case E_STARTUP:
        {
            vStartup();
        }
        break;

        case E_NETWORK_FORMATION:
        {
        	vWaitForNetworkFormation(sStackEvent);
        }
        break;

        case E_RUNNING:
        {
        	vHandleStackEvent(sStackEvent);
        }
        break;

        default:
        {
        	/* Do nothing */
        }
        break;
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vStartup
 *
 * DESCRIPTION:
 * Initiates network formation
 *
 * RETURNS:
 * Sequence number
 *
 ****************************************************************************/
PRIVATE void vStartup(void)
{
    /* display the startup splash screen */
    DBG_vPrintf(TRACE_APP, "APP: Application startup\n");

    /* Stops Coordinator from sleeping ever - but does allow doze */
    PWRM_eStartActivity();

    ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();
    if (ZPS_E_SUCCESS == eStatus)
    {
        s_eDeviceState.eNodeState = E_NETWORK_FORMATION;
    }
    else
    {
        DBG_vPrintf(TRACE_APP, "APP: ZPS_eZdoStartStack() failed error %d", eStatus);
    }
    ZTIMER_eStart ( u8App_tmr1sec, ZTIMER_TIME_SEC(1) );
}

/****************************************************************************
 *
 * NAME: vWaitForNetworkFormation
 *
 * DESCRIPTION:
 * Handles stack events during network formation
 *
 * PARAMETERS: Name         RW  Usage
 *             sStackEvent  R   Contains details of the stack event
 *
 * RETURNS:
 * Sequence number
 *
 ****************************************************************************/
PRIVATE void vWaitForNetworkFormation(ZPS_tsAfEvent sStackEvent)
{
    /* wait for network stack to start up as a coordinator */
    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        if (ZPS_EVENT_NWK_STARTED == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Network Started\r\n");
            DBG_vPrintf(TRACE_APP, "APP: Channel - %d\r\n", ZPS_u8AplZdoGetRadioChannel());

            /* turn on joining */
            ZPS_eAplZdoPermitJoining(0xff);

            s_eDeviceState.eNodeState = E_RUNNING;

            /* Save the application state to flash. Note that all records may be saved at any time by the PDM:
             * if a module has called PDM_vSaveRecord(), but there is insufficient spare memory, an erase is performed
             * followed by a write of all records.
             */
            PDM_eSaveRecordData(PDM_ID_APP_COORD,
                            	&s_eDeviceState,
                            	sizeof(s_eDeviceState));
        }
    }
}

/****************************************************************************
 *
 * NAME: vHandleStackEvent
 *
 * DESCRIPTION:
 * Handles stack events after the Coordinator has started a network and is in
 * its running state
 *
 * PARAMETERS: Name         RW  Usage
 *             sStackEvent  R   Contains details of the stack event
 *
 * RETURNS:
 * Sequence number
 *
 ****************************************************************************/
PRIVATE void vHandleStackEvent(ZPS_tsAfEvent sStackEvent)
{
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
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_AF_DATA_INDICATION\n");

                /* Process incoming cluster messages ... */
                DBG_vPrintf(TRACE_APP, "        Profile :%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u16ProfileId);
                DBG_vPrintf(TRACE_APP, "        Cluster :%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u16ClusterId);
                DBG_vPrintf(TRACE_APP, "        EndPoint:%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint);

                /* free the application protocol data unit (APDU) once it has been dealt with */
                PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
            }
            break;

            case ZPS_EVENT_APS_DATA_CONFIRM:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_APS_DATA_CONFIRM Status %d, Address 0x%04x\n",
                            sStackEvent.uEvent.sApsDataConfirmEvent.u8Status,
                            sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u16Addr);
            }
            break;

            case ZPS_EVENT_APS_DATA_ACK:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_APS_DATA_ACK Status %d, Address 0x%04x\n",
                            sStackEvent.uEvent.sApsDataAckEvent.u8Status,
                            sStackEvent.uEvent.sApsDataAckEvent.u16DstAddr);
            }
            break;

            case ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: vCheckStackEvent: ZPS_EVENT_NEW_NODE_HAS_JOINED, Nwk Addr=0x%04x\n",
                            sStackEvent.uEvent.sNwkJoinIndicationEvent.u16NwkAddr);
            }
            break;

            case ZPS_EVENT_NWK_LEAVE_INDICATION:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_LEAVE_INDICATION\n");
            }
            break;

            case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_LEAVE_CONFIRM\n");
            }
            break;

            case ZPS_EVENT_NWK_STATUS_INDICATION:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_NWK_STATUS_INDICATION\n");
            }
            break;

            case ZPS_EVENT_NWK_ROUTE_DISCOVERY_CONFIRM:
            {
            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_ROUTE_DISCOVERY_CFM\n");
            }
            break;

            case ZPS_EVENT_ERROR:
            {
            	DBG_vPrintf(TRACE_APP, "APP: Monitor Sensors ZPS_EVENT_ERROR\n");
                DBG_vPrintf(TRACE_APP, "    Error Code %d\n", sStackEvent.uEvent.sAfErrorEvent.eError);

                if (ZPS_ERROR_OS_MESSAGE_QUEUE_OVERRUN == sStackEvent.uEvent.sAfErrorEvent.eError)
                {
                	DBG_vPrintf(TRACE_APP, "    Queue handle %d\n",sStackEvent.uEvent.sAfErrorEvent.uErrorData.sAfErrorOsMessageOverrun.hMessage);
                }
            }
            break;

            case ZPS_EVENT_NWK_POLL_CONFIRM:
            {
                DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_NEW_POLL_COMPLETE, status = %d\n",
                            sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status );
            }
            break;

            case ZPS_EVENT_NWK_JOINED_AS_ROUTER:
            case ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE:
            case ZPS_EVENT_NWK_STARTED:
            case ZPS_EVENT_NWK_FAILED_TO_START:
            case ZPS_EVENT_NWK_FAILED_TO_JOIN:
            case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
                /* Deliberate fall through */
            default:
            {
                DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: unhandled event %d\n", sStackEvent.eType);
            }
            break;
        }
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
