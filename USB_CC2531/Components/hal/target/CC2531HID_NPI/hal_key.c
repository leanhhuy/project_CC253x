/*
  Filename:       hal_key.c
*/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_board.h"
#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_types.h"
#include "osal.h"
#include "usb_interrupt.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Macros  
 * ------------------------------------------------------------------------------------------------
 */

#define HAL_KEY_CLR_INT() \
st ( \
  /* PxIFG has to be cleared before PxIF. */\
  P1IFG = 0; \
  P1IF = 0; \
)

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */

uint8 Hal_KeyIntEnable;

/* ------------------------------------------------------------------------------------------------
 *                                       Global Functions
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */

static halKeyCBack_t pHalKeyProcessFunction;
static volatile uint8 isrKeys;
static uint8 halKeys;

/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          HalKeyInit
 *
 * @brief       This function is called by HalDriverInit to initialize the H/W keys.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalKeyInit(void)
{
}

/**************************************************************************************************
 * @fn          HalKeyConfig
 *
 * @brief       This function is called by HalDriverInit to initialize the H/W keys.
 *
 * input parameters
 *
 * @param       interruptEnable - TRUE/FALSE to enable the key interrupt.
 * @param       cback - The callback function for the key change event.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalKeyConfig(bool interruptEnable, halKeyCBack_t cback)
{
  if ((Hal_KeyIntEnable = interruptEnable))
  {
    HAL_KEY_CLR_INT();             // Clear spurious ints.
    PICTL |= 0x01;                 // P1ICONL: Falling edge ints on pins 0-3.
    P1IEN |= PUSH1_BV | PUSH2_BV;  // Enable specific P1 bits for ints by bit mask.
    IEN2  |= 0x10;                 // Enable general P1 interrupts.
  }
  else
  {
    (void)osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
  }

  pHalKeyProcessFunction = cback;
}

/**************************************************************************************************
 * @fn          HalKeyPoll
 *
 * @brief       This function is called by Hal_ProcessEvent() on a HAL_KEY_EVENT.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalKeyPoll(void)
{
  uint8 newKeys;

  if (Hal_KeyIntEnable)
  {
    halIntState_t intState;
    HAL_ENTER_CRITICAL_SECTION(intState);
    newKeys = isrKeys;
    isrKeys = 0;
    HAL_EXIT_CRITICAL_SECTION(intState);
  }
  else
  {
    uint8 keys = HalKeyRead();
    newKeys = (halKeys ^ keys) & keys;
    halKeys = keys;
  }

  if (newKeys && pHalKeyProcessFunction)
  {
    (pHalKeyProcessFunction)(newKeys, HAL_KEY_STATE_NORMAL);
  }
}

/**************************************************************************************************
 * @fn          HalKeyRead
 *
 * @brief       This function is called anywhere.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The bit mask of all keys pressed.
 **************************************************************************************************
 */
uint8 HalKeyRead(void)
{
  uint8 keys = 0;

  if (HAL_PUSH_BUTTON1())
  {
    keys |= HAL_KEY_SW_1;
  }

  if (HAL_PUSH_BUTTON2())
  {
    keys |= HAL_KEY_SW_2;
  }

  return keys;
}

/**************************************************************************************************
 * @fn      HalKeyEnterSleep
 *
 * @brief  - Get called to enter sleep mode
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void HalKeyEnterSleep ( void )
{
}

/**************************************************************************************************
 * @fn      HalKeyExitSleep
 *
 * @brief   - Get called when sleep is over
 *
 * @param
 *
 * @return  - return saved keys
 **************************************************************************************************/
uint8 HalKeyExitSleep ( void )
{
  /* Wake up and read keys */
  return ( HalKeyRead () );
}

/**************************************************************************************************
 * @fn          usbKeyISR
 *
 * @brief       This function is the ISR for the Port2 USB/Key interrupt.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
HAL_ISR_FUNCTION( usbKeyISR, P1INT_VECTOR )
{
  HAL_ENTER_ISR();

  if (P1IFG & PUSH1_BV)
  {
    isrKeys |= HAL_KEY_SW_1;
  }

  if (P1IFG & PUSH2_BV)
  {
    isrKeys |= HAL_KEY_SW_2;
  }

  HAL_KEY_CLR_INT();
  (void)osal_set_event(Hal_TaskID, HAL_KEY_EVENT);

  HAL_EXIT_ISR();
}

/**************************************************************************************************
*/
