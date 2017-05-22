/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_router.c
 *
 * DESCRIPTION:			Router Application
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
#include <string.h>
#include <dbg.h>
#include <zps_apl_af.h>
#include <zps_apl_aib.h>
#include <pdm.h>
#include <PDM_IDs.h>
#include "app_common.h"
#include "app_router.h"
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
PRIVATE void vWaitForNetworkDiscovery(ZPS_tsAfEvent sStackEvent);
PRIVATE void vWaitForNetworkJoin(ZPS_tsAfEvent sStackEvent);
PRIVATE void vHandleStackEvent(ZPS_tsAfEvent sStackEvent);
PRIVATE void vClearDiscNT(void);

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
 * NAME: APP_vInitialiseRouter
 *
 * DESCRIPTION:
 * Initialises the router application
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitialiseRouter(void)
{
    bool_t bDeleteRecords = TRUE /*FALSE*/;
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
    PDM_eReadDataFromRecord(PDM_ID_APP_ROUTER,
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

    /* Always initialise any peripherals used by the applicatio
     * HERE
     */

    /* If the device state has been restored from flash, re-start the stack
     * and set the application running again. Note that if there is more than 1 state
     * where the network has already joined, then the other states should also be included
     * in the test below
     * E.g. E_RUNNING_1, E_RUNNING_2......
     * if (E_RUNNING_1 ==s_eDeviceState || E_RUNNING_2 ==s_eDeviceState)
     */
    if (E_RUNNING ==s_eDeviceState.eNodeState)
    {
        ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();
        DBG_vPrintf(TRACE_APP, "APP: Re-starting Stack....\r\n");
        if (ZPS_E_SUCCESS != eStatus)
        {
            DBG_vPrintf(TRACE_APP, "APP: ZPS_eZdoStartStack() failed error %d", eStatus);
        }
        /* turn on joining */
        ZPS_eAplZdoPermitJoining(0xff);

        /* Re-start any other application software modules
         * HERE
         */
    }
    else
        /* perform any actions require on initial start-up */
    {
        /* Return the device to the start-up start if it was reset during the network formation stage */
       s_eDeviceState.eNodeState = E_STARTUP;
    }
}

/****************************************************************************
 *
 * NAME: APP_taskRouter
 *
 * DESCRIPTION:
 * Main state machine
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vtaskRouter ( void )
{
    ZPS_tsAfEvent sStackEvent;

    sStackEvent.eType = ZPS_EVENT_NONE;
    if (ZQ_bQueueReceive (&APP_msgZpsEvents, &sStackEvent) )
    {
        DBG_vPrintf(TRACE_APP, "APP: No event to process!\n");
    }

    if (ZTIMER_eGetState(u8App_tmr1sec) == E_ZTIMER_STATE_EXPIRED)
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

        case E_DISCOVERY:
        {
            vWaitForNetworkDiscovery(sStackEvent);
        }
        break;

        case E_JOINING_NETWORK:
        {
        	vWaitForNetworkJoin(sStackEvent);
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
 * Start the process of network discovery
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vStartup(void)
{
    PRIVATE uint8 u8Channel = 11;

    vClearDiscNT();

    /* Set channel to scan and start stack */
    ZPS_psAplAibGetAib()->apsChannelMask = 1 << u8Channel;

    ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();
    if (ZPS_E_SUCCESS == eStatus)
    {
       s_eDeviceState.eNodeState = E_DISCOVERY;

       /* Move channel counter to next channel */
       u8Channel++;

       if (27 == u8Channel)
       {
           u8Channel = 11;
       }
    }

}

/****************************************************************************
 *
 * NAME: vWaitForNetworkDiscovery
 *
 * DESCRIPTION:
 * Check for and act upon stack events during network discovery.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  sStackEvent     R   Contains details of stack event
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vWaitForNetworkDiscovery(ZPS_tsAfEvent sStackEvent)
{
    /* wait for the node to complete network discovery */
    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        if (ZPS_EVENT_NWK_DISCOVERY_COMPLETE == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Network discovery complete\n");

            if ((ZPS_E_SUCCESS == sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus) ||
                (ZPS_NWK_ENUM_NEIGHBOR_TABLE_FULL == sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus))
            {
                DBG_vPrintf(TRACE_APP, "APP: Found %d networks\n", sStackEvent.uEvent.sNwkDiscoveryEvent.u8NetworkCount);
            }
            else
            {
                DBG_vPrintf(TRACE_APP, "APP: Network discovery failed with error %d\n",sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus);
            }
            if (   0 == sStackEvent.uEvent.sNwkDiscoveryEvent.u8NetworkCount
                || 0xff == sStackEvent.uEvent.sNwkDiscoveryEvent.u8SelectedNetwork)
            {
                if (0 != sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels)
                {
                    /* continue to look for networks on unscanned channels */
                    DBG_vPrintf(TRACE_APP, "APP: No networks found, continue scanning ...\n");
                    ZPS_eAplZdoDiscoverNetworks(sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels);
                }
                else
                {
                    DBG_vPrintf(TRACE_APP, "APP: Exhausted channels to scan\n");
                    s_eDeviceState.eNodeState = E_STARTUP;
                }
            }
            else
            {
                ZPS_tsNwkNetworkDescr *psNwkDescr = &sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[sStackEvent.uEvent.sNwkDiscoveryEvent.u8SelectedNetwork];
                ZPS_teStatus eStatus;

                DBG_vPrintf(TRACE_APP, "APP: Unscanned channels %08x\n", sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels);
                DBG_vPrintf(TRACE_APP, "\tAPP: Ext PAN ID = %016llx\n", psNwkDescr->u64ExtPanId);
                DBG_vPrintf(TRACE_APP, "\tAPP: Channel = %d\n", psNwkDescr->u8LogicalChan);
                DBG_vPrintf(TRACE_APP, "\tAPP: Stack Profile = %d\n", psNwkDescr->u8StackProfile);
                DBG_vPrintf(TRACE_APP, "\tAPP: Zigbee Version = %d\n", psNwkDescr->u8ZigBeeVersion);
                DBG_vPrintf(TRACE_APP, "\tAPP: Permit Joining = %d\n", psNwkDescr->u8PermitJoining);
                DBG_vPrintf(TRACE_APP, "\tAPP: Router Capacity = %d\n", psNwkDescr->u8RouterCapacity);
                DBG_vPrintf(TRACE_APP, "\tAPP: End Device Capacity = %d\n", psNwkDescr->u8EndDeviceCapacity);

                eStatus = ZPS_eAplZdoJoinNetwork(psNwkDescr);
                DBG_vPrintf(TRACE_APP, "APP: Trying Joining network\n");

                if (ZPS_E_SUCCESS == eStatus)
                {
                    DBG_vPrintf(TRACE_APP, "APP: Joining network\n");
                   s_eDeviceState.eNodeState = E_JOINING_NETWORK;
                }
                else
                {
                    /* start scan again */
                    DBG_vPrintf(TRACE_APP, "APP: Failed to join network reason = %02x\n", eStatus);
                    s_eDeviceState.eNodeState = E_STARTUP;
                }
            }
        }
        else if (ZPS_EVENT_NWK_FAILED_TO_JOIN == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Network discovery failed error=%d\n", sStackEvent.uEvent.sNwkJoinFailedEvent.u8Status);
           s_eDeviceState.eNodeState = E_STARTUP;
        }
        else if (ZPS_EVENT_NWK_JOINED_AS_ROUTER == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Node rejoined network with Addr 0x%04x\n",
                        sStackEvent.uEvent.sNwkJoinedEvent.u16Addr);
            s_eDeviceState.eNodeState = E_RUNNING;

            /* Save the application state to flash. Note that all records may be saved at any time by the PDM:
             * if a module has called PDM_vSaveRecord(), but there is insufficient spare memory, an erase is performed
             * followed by a write of all records.
             */
            PDM_eSaveRecordData(PDM_ID_APP_ROUTER,
                            	&s_eDeviceState,
                            	sizeof(s_eDeviceState));
        }
        else
        {
            DBG_vPrintf(TRACE_APP, "APP: Unexpected event in E_NETWORK_DISCOVERY - %d\n", sStackEvent.eType);
        }
    }
}

/****************************************************************************
 *
 * NAME: vWaitForNetworkJoin
 *
 * DESCRIPTION:
 * Check for and act upon stack events during network join.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  sStackEvent     R   Contains details of stack event
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vWaitForNetworkJoin(ZPS_tsAfEvent sStackEvent)
{
     uint64 u64ExtPANID;
    /* wait for the node to complete network joining */
    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        if (ZPS_EVENT_NWK_JOINED_AS_ROUTER == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Node joined network with Addr 0x%04x\n",
                        sStackEvent.uEvent.sNwkJoinedEvent.u16Addr);
            /* save the EPID for rejoins */
            u64ExtPANID = ZPS_u64NwkNibGetEpid(ZPS_pvAplZdoGetNwkHandle());
            ZPS_eAplAibSetApsUseExtendedPanId(u64ExtPANID);
            s_eDeviceState.eNodeState = E_RUNNING;
            PDM_eSaveRecordData(PDM_ID_APP_ROUTER,
                            	&s_eDeviceState,
                            	sizeof(s_eDeviceState));

            /* Start timer to periodically active APP_taskRouter */
            ZTIMER_eStart (u8App_tmr1sec, ZTIMER_TIME_SEC(1));
        }
        else if (ZPS_EVENT_NWK_FAILED_TO_JOIN == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Node failed to join network. Retrying ...\n");
            s_eDeviceState.eNodeState = E_STARTUP;
            /* Save the application state to flash. Note that all records may be saved at
             * any time by the PDM: if a module has called PDM_vSaveRecord(), but there
             * is insufficient spare memory, an erase is performed followed by a write
             * of all records.
             */
        }
        else
        {
            DBG_vPrintf(TRACE_APP, "APP: Unexpected event in E_NETWORK_JOIN - %d\n", sStackEvent.eType);
        }
    }
}

/****************************************************************************
 *
 * NAME: vHandleStackEvent
 *
 * DESCRIPTION:
 * Check for and act upon any valid stack event, after the node has joined a
 * network and is in its running state
 *
 * PARAMETERS:      Name            RW  Usage
 *                  sStackEvent     R   Contains details of stack event
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleStackEvent(ZPS_tsAfEvent sStackEvent)
{
    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        switch (sStackEvent.eType)
        {
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
                	DBG_vPrintf(TRACE_APP, "    Queue handle %d\n", sStackEvent.uEvent.sAfErrorEvent.uErrorData.sAfErrorOsMessageOverrun.hMessage);
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

/****************************************************************************
 *
 * NAME: vClearDiscNT
 *
 * DESCRIPTION:
 * Handles the re-intialisation of the discovery table to 0
 *
 * PARAMETERS: None
 *
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vClearDiscNT(void)
{
    ZPS_tsNwkNib * thisNib;

    void * thisNet = ZPS_pvAplZdoGetNwkHandle();
    thisNib = ZPS_psNwkNibGetHandle(thisNet);

    memset(thisNib->sTbl.psNtDisc, 0, sizeof(ZPS_tsNwkDiscNtEntry) * thisNib->sTblSize.u8NtDisc);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
