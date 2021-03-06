/**************************************************************************************************
    Filename:       np_main.c
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

#if (defined HAL_MCU_CC2530 || defined HAL_MCU_CC2531 || defined HAL_MCU_CC2533)
// NV system migration utilities
#include "nvmigr.h"
#endif
#include "zid.h"
#include "zid_adaptor.h"
#include "zid_dongle.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void osalInitTasks(void);

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

const pTaskEventHandlerFn tasksArr[] = {
  macEventLoop,
  RCN_ProcessEvent,
  RTI_ProcessEvent,
  Dongle_ProcessEvent,
  zidAda_ProcessEvent,
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
  Dongle_Init( taskID++ );
  zidAda_Init( taskID++ );
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

#if (defined HAL_MCU_CC2530 || defined HAL_MCU_CC2531 || defined HAL_MCU_CC2533)
  /* Clean up obsolete NV pages in case the software was updated from an old
   * NV system software */
  NVMIGR_CLEAR_OBSOLETE_NV_PAGES();
#endif

  /* Initialize NV system */
  osal_snv_init();

  /* Initialize MAC */
  MAC_InitRf4ce();

  // initialize tasks
  osal_init_system();

  // enable interrupts
  osal_int_enable( INTS_ALL );

#if defined( POWER_SAVING )
  osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

  // and away we go!
  osal_start_system();

  // will never reach here but return value anyways to suppress warning
  return 0;
}

/**************************************************************************************************
**************************************************************************************************/
