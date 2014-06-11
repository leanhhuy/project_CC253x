/**************************************************************************************************
    Filename:       np_main.c
    Revised:        $Date: 2012-10-02 15:13:51 -0700 (Tue, 02 Oct 2012) $
    Revision:       $Revision: 31675 $

    Description:

    This file contains the sample application for testing the Network Processor
    Interface API.

  Copyright 2008-2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

// HAL includes
#include "hal_types.h"
#include "hal_board.h"
#include "hal_drivers.h"
#include "hal_timer.h"
#if ((defined HAL_SPI) && (HAL_SPI == TRUE))
#include "hal_spi.h"
#endif

// OSAL includes
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "osal_snv.h"

// MAC API
#include "mac_api.h"
#include "mac_rf4ce.h"

// RCN API
#include "rcn_task.h"

// RTI API
#include "rti.h"

// NPI API
#include "npi.h"

// Application includes
#include "np_main.h"

#include "zid.h"
#if FEATURE_ZID_ADA
#include "zid_adaptor.h"
#endif
#if FEATURE_ZID_CLD
#include "zid_class_device.h"
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void          osalInitTasks( void );
#if !FEATURE_ZID
static void   SA_Init( uint8 taskId );
static uint16 SA_ProcessEvent(uint8 taskId, uint16 events);
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

// Sample Application Message Data
npiMsgData_t msgBuf;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

// Sample Application task related data
uint8 SA_TaskId;

const pTaskEventHandlerFn tasksArr[] = {
  macEventLoop,
  RCN_ProcessEvent,
  RTI_ProcessEvent,
  NPI_ProcessEvent,
#if defined FEATURE_ZID_ADA && FEATURE_ZID_ADA == TRUE
  zidAda_ProcessEvent,
#endif
#if defined FEATURE_ZID_CLD && FEATURE_ZID_CLD == TRUE
  zidCld_ProcessEvent,
#endif
#if !FEATURE_ZID
  SA_ProcessEvent,
#endif
  Hal_ProcessEvent
};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16 *tasksEvents;

/**************************************************************************************************
 * @fn      osalInitTasks
 *
 * @brief   This function is called by OSAL to invoke the initialization
 *          function for each task.
 *
 * @param   void
 *
 * @return  none
 */
void osalInitTasks( void )
{
  uint8 taskID = 0;

  tasksEvents = (uint16 *)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
  osal_memset( tasksEvents, 0, (sizeof( uint16 ) * tasksCnt));

  macTaskInit( taskID++ );
  RCN_Init( taskID++ );
  RTI_Init( taskID++ );
  NPI_Init( taskID++ );
#if defined FEATURE_ZID_ADA && FEATURE_ZID_ADA == TRUE
  zidAda_Init( taskID++ );
#endif
#if defined FEATURE_ZID_CLD && FEATURE_ZID_CLD == TRUE
  zidCld_Init( taskID++ );
#endif
#if !FEATURE_ZID
  SA_Init( taskID++ );
#endif
  Hal_Init( taskID );
}

/**************************************************************************************************
 * @fn          main
 *
 * @brief       This function is the C-code main entry function.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      Reason for exiting.
 **************************************************************************************************
 */
int main(void)
{
  // disable interrupts
  osal_int_disable( INTS_ALL );

  /* Initialize hardware */
  HAL_BOARD_INIT();

  // initialization for HAL including USART for NPI (SPI or UART)
  HalDriverInit();

  /* Initialize NV system */
  osal_snv_init();

  /* Initialize MAC */
  MAC_InitRf4ce();

  // start the RemoTI Surrogate
  RTIS_Init();

  // initialize tasks
  osal_init_system();

  // enable interrupts
  osal_int_enable( INTS_ALL );

#if defined( POWER_SAVING )
  osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

  // inform AP that NP has been reset
#if ((defined HAL_SPI) && (HAL_SPI == TRUE))
  HalSpiAssertSrdy();
#else
  RTI_ResetInd();
#endif

  // and away we go!
  osal_start_system();

  // will never reach here but return value anyways to suppress warning
  return 0;
}


#if !FEATURE_ZID
/**************************************************************************************************
 *
 * @fn          SA_Init
 *
 * @brief       This is the Sample Application task initialization called by OSAL.
 *
 * @param       taskId - task ID assigned after it was added in the OSAL task queue
 *
 * @return      none
 *
 **************************************************************************************************/
void SA_Init( uint8 taskId )
{
  // save task ID assigned by OSAL
  SA_TaskId = taskId;
}


/**************************************************************************************************
 * @fn          SA_ProcessEvent
 *
 * @brief       This function processes the OSAL events and messages for the
 *              Sample Application.
 *
 * input parameters
 *
 * @param taskId - The task ID assigned to this application by OSAL at system initialization.
 * @param events - A bit mask of the pending event(s).
 *
 * output parameters
 *
 * None.
 *
 * @return      The events bit map received via parameter with the bits cleared which correspond to
 *              the event(s) that were processed on this invocation.
 **************************************************************************************************
 */
static uint16 SA_ProcessEvent(uint8 taskId, uint16 events)
{
  (void)taskId;

  if ( events & SYS_EVENT_MSG )
  {
    osal_event_hdr_t  *pMsg;

    while ((pMsg = (osal_event_hdr_t *) osal_msg_receive(NPI_TaskId)) != NULL)
    {
      switch (pMsg->event)
      {
        default:
          break;
      }

      osal_msg_deallocate((uint8 *) pMsg);
    }

    return (events ^ SYS_EVENT_MSG);
  }

  return ( 0 );  /* Discard unknown events. */
}
#endif

/**************************************************************************************************
**************************************************************************************************/
