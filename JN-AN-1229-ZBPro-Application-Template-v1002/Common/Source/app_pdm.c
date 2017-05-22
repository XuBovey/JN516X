/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:          	app_pdm.c
 *
 * DESCRIPTION:        	Application PDM Interface
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
#include <pdm.h>
#include <dbg_uart.h>
#include <PDM_IDs.h>
#include <app_common.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_PDM
	#define TRACE_PDM 	FALSE
#else
	#define TRACE_PDM 	TRUE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
#ifdef PDM_EEPROM
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
#endif

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern PDM_tsRecordDescriptor sDevicePDDesc;
extern tsDeviceDesc sDeviceDesc;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

#ifdef PDM_EEPROM
/****************************************************************************
 *
 * NAME: vPdmEventHandlerCallback
 *
 * DESCRIPTION:
 * Handles PDM callback, information the application of PDM conditions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vPdmEventHandlerCallback(uint32 u32EventNumber, PDM_eSystemEventCode eSystemEventCode)
{
    switch (eSystemEventCode)
    {
        /*
         * The next three events will require the application to take some action
         */
        case E_PDM_SYSTEM_EVENT_WEAR_COUNT_TRIGGER_VALUE_REACHED:
        {
            DBG_vPrintf(TRACE_PDM, "PDM: Segment %d reached trigger wear level\n", u32EventNumber);
        }
        break;

        case E_PDM_SYSTEM_EVENT_DESCRIPTOR_SAVE_FAILED:
        {
            DBG_vPrintf(TRACE_PDM, "PDM: Record Id %d failed to save\n", u32EventNumber);
            DBG_vPrintf(TRACE_PDM, "PDM: Capacity %d\n", u8PDM_CalculateFileSystemCapacity());
            DBG_vPrintf(TRACE_PDM, "PDM: Occupancy %d\n", u8PDM_GetFileSystemOccupancy());
        }
        break;

        case E_PDM_SYSTEM_EVENT_PDM_NOT_ENOUGH_SPACE:
        {
        	DBG_vPrintf(TRACE_PDM, "PDM: Record %d not enough space\n", u32EventNumber);
            DBG_vPrintf(TRACE_PDM, "PDM: Capacity %d\n", u8PDM_CalculateFileSystemCapacity());
            DBG_vPrintf(TRACE_PDM, "PDM: Occupancy %d\n", u8PDM_GetFileSystemOccupancy());
        }
        break;

        /*
         *  The following events are really for information only
         */
        case E_PDM_SYSTEM_EVENT_EEPROM_SEGMENT_HEADER_REPAIRED:
        {
            DBG_vPrintf(TRACE_PDM, "PDM: Segment %d header repaired\n", u32EventNumber);
        }
        break;

        case E_PDM_SYSTEM_EVENT_SYSTEM_INTERNAL_BUFFER_WEAR_COUNT_SWAP:
        {
        	DBG_vPrintf(TRACE_PDM, "PDM: Segment %d buffer wear count swap\n", u32EventNumber);
        }
        break;

        case E_PDM_SYSTEM_EVENT_SYSTEM_DUPLICATE_FILE_SEGMENT_DETECTED:
        {
        	DBG_vPrintf(TRACE_PDM, "PDM: Segement %d duplicate selected\n", u32EventNumber);
        }
        break;

        default:
        {
        	DBG_vPrintf(TRACE_PDM, "PDM: Unexpected call back Code %d Number %d\n", eSystemEventCode, u32EventNumber);
        }
        break;
    }
}
#endif

/****************************************************************************
 *
 * NAME: eRestoreDeviceState
 *
 * DESCRIPTION:
 * Retrieves device state from the PDM
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC PDM_teStatus eRestoreDeviceState(void)
{
	 /* Restore any application data previously saved to flash */
	uint16 u16ByteRead;

    PDM_teStatus eDeviceStatus = PDM_eReadDataFromRecord(PDM_ID_APP_ROUTER,
                                                        &sDeviceDesc,
                                                        sizeof(tsDeviceDesc),
                                                        &u16ByteRead);

   DBG_vPrintf(TRACE_PDM, "\nAPP: PDM Load APP_ROUTER returned %d, RecState=%d", eDeviceStatus, sDevicePDDesc.eState);
   DBG_vPrintf(TRACE_PDM, "\nAPP State %d", sDeviceDesc.eNodeState);

   return (eDeviceStatus);
}

/****************************************************************************
 *
 * NAME: vDisplayPDMUsage
 *
 * DESCRIPTION:
 * Displays the PDM capacity and current occupancy
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplayPDMUsage(void)
{
	#ifdef PDM_EEPROM
		/*
		 * The functions u8PDM_CalculateFileSystemCapacity and u8PDM_GetFileSystemOccupancy
		 * may be called at any time to monitor space available in  the eeprom
		 */
		DBG_vPrintf(TRACE_PDM, "\r\nPDM: Capacity %d\n", u8PDM_CalculateFileSystemCapacity());
		DBG_vPrintf(TRACE_PDM, "\r\nPDM: Occupancy %d\n", u8PDM_GetFileSystemOccupancy());
	#endif
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
