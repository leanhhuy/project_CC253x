/*
  Filename:       OnBoard.c
*/

/*********************************************************************
 * INCLUDES
 */

#include "OnBoard.h"

/*********************************************************************
 * @fn      Onboard_soft_reset
 *
 * @brief   Effect a soft reset.
 *
 * @param   none
 *
 * @return  none
 *
 *********************************************************************/
__near_func void Onboard_soft_reset( void )
{
  HAL_DISABLE_INTERRUPTS();
  // Abort all DMA channels to insure that ongoing operations do not
  // interfere with re-configuration.
  DMAARM = 0x80 | 0x1F;
  asm("LJMP 0x0");
}

/*********************************************************************
*********************************************************************/
