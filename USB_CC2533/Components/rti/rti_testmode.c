/**************************************************************************************************
  Filename:       rti_testmode.c
**************************************************************************************************/

/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/

/* HAL includes */
#include "hal_types.h"
#include "hal_mcu.h"

/* MAC includes */
#include "mac_api.h"

/* RTI includes */
#include "rti.h"

/* RCN includes */
#include "rcn_nwk.h"

/**************************************************************************************************
 *                                           Constant
 **************************************************************************************************/

// Radio chip definition used by test mode function implementation
#if (defined HAL_MCU_CC2530 || defined HAL_MCU_CC2531 || defined HAL_MCU_CC2533)
# define MAC_RADIO_CC2530
#elif (defined HAL_MCU_MSP430)
# warning Need to implement radio-specific test modes.
#else
# warning Need to implement radio-specific test modes.
#endif

/**************************************************************************************************
 *                                        Type definitions
 **************************************************************************************************/

/**************************************************************************************************
 *                                        Global Variables
 **************************************************************************************************/

/**************************************************************************************************
 *                                        Local Variables
 **************************************************************************************************/

static uint16 rtiRxCounter; // Test mode receive packet counter

/**************************************************************************************************
 *                                     Local Function Prototypes
 **************************************************************************************************/


/**************************************************************************************************
 *
 * @fn          RTI_SwResetReq
 *
 * @brief       This function resets the radio processor CPU by way of software triggering.
 *              Implementation of this function is target (CPU) dependent.
 *              Note that in production platform, the reset could be done by chip reset signal
 *              and hence use of this function should be restricted to development phase.
 *
 * input parameters
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void RTI_SwResetReq( void )
{
  HAL_SYSTEM_RESET();
}

#ifdef FEATURE_TEST_MODE // global feature flag
/**************************************************************************************************
 *
 * @fn          RTI_TestModeReq
 *
 * @brief       This function is used to place the radio in test modes.
 *              Note that implementation is chip dependent. HAL is not used to reduce code
 *              size overhead.
 *
 * input parameters
 *
 * @param       mode - test mode: RTI_TEST_MODE_TX_RAW_CARRIER, RTI_TEST_MODE_TX_RANDOM_DATA
 *                     or RTI_TEST_MODE_RX_AT_FREQ
 * @param       txPower - transmit power in dBm.
 * @param       channel - MAC channel number
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void RTI_TestModeReq( uint8 mode, int8 txPower, uint8 channel )
{
  halIntState_t is;
  
  if (mode == RTI_TEST_MODE_RX_AT_FREQ)
  {
    uint8 trueBuf = TRUE;
    MAC_MlmeResetReq(TRUE);
    MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &trueBuf);
    MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &channel);
    FRMFILT0 &= 0xFE;
    rtiRxCounter = 0;
  }
  else
  {
    uint8 falseBuf = FALSE;
    MAC_MlmeResetReq(TRUE);
    MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &falseBuf);
    // MAC does not support full range of transmit power
    // Hence, use RCN interface to set transmit power level
    RCN_NlmeSetReq(RCN_NIB_TRANSMIT_POWER, 0, (uint8 *) &txPower);
    MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &channel);
  
    if (mode == RTI_TEST_MODE_TX_RAW_CARRIER)
    {
      HAL_ENTER_CRITICAL_SECTION(is);

#ifdef MAC_RADIO_CC2530
      FRMCTRL0 &= 0xfc; // clear TX_MODE
      FRMCTRL0 |= 0x3; // TX_MODE = 2; FIFO infinite transmission
      MDMTEST1 |= 0x10;
      RFST = 0xFF;   // clear instruction memory (ISCLEAR)
      // -- program begins --
      RFST = 0xD9;   // Turn on transmitter (STXON)
      RFST = 0xD2;   // Stop execution (STOP)
      // -- program ends --
      RFST = 0xE1;   // start program execution (ISSTART)
#endif
      
      HAL_EXIT_CRITICAL_SECTION(is);
    }
    else if (mode == RTI_TEST_MODE_TX_RANDOM_DATA)
    {
      HAL_ENTER_CRITICAL_SECTION(is);

#ifdef MAC_RADIO_CC2530
      FRMCTRL0 &= 0xfc; // clear TX_MODE
      FRMCTRL0 |= 3; // TX_MODE = 3; Pseudo-random infinite transmission     
      RFST = 0xFF;   // clear instruction memory (ISCLEAR)
      // -- program begins --
      RFST = 0xD9;   // Turn on transmitter (STXON)
      RFST = 0xD2;   // Stop execution (STOP)
      // -- program ends --
      RFST = 0xE1;   // start program execution (ISSTART)
#endif

      HAL_EXIT_CRITICAL_SECTION(is);
    }
  }
}


/**************************************************************************************************
 *
 * @fn          RTI_TestRxCounterGetReq
 *
 * @brief       This function is used to obtain received packet counter value.
 *
 * input parameters
 *
 * @param       resetFlag - whether or not to reset the counter after reading the value
 *
 * output parameters
 *
 * None.
 *
 * @return      counter value
 */
uint16 RTI_TestRxCounterGetReq(uint8 resetFlag)
{
  uint16 result = rtiRxCounter;
  
  if (resetFlag)
  {
    rtiRxCounter = 0;
  }
  
  return result;
}

#endif // FEATURE_TEST_MODE

/**************************************************************************************************
 * @fn          RCN_CbackRxCount
 *
 * @brief       This is a callback function from RF4CE network layer.
 *              This function is valid only when test mode feature is turned on at build time.
 *              Network layer shall call this callback function to indicate that a CRC passed
 *              MAC frame was received.
 *              Client should increment its receive counter.
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
 */
void RCN_CbackRxCount(void)
{
  rtiRxCounter++;
}


/**************************************************************************************************
 **************************************************************************************************/

