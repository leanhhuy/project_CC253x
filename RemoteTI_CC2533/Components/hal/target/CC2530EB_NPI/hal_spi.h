#include "hal_board.h"

#if (defined HAL_SPI) && (HAL_SPI == TRUE)

/**************************************************************************************************
  Filename:       hal_spi.h
  Revised:        $Date: 2012-10-02 15:13:51 -0700 (Tue, 02 Oct 2012) $
  Revision:       $Revision: 31675 $

  Description:    This file contains the interface for the NP SPI Service.


  Copyright 2005-2012 Texas Instruments Incorporated. All rights reserved.

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

#ifndef HAL_SPI_H
#define HAL_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_rpc.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define HAL_SPI_BUF_LEN   256
#define HAL_SPI_BUF_MAX   253

/* maximum length of data in the general frame format */
#define RPC_DATA_MAX      (HAL_SPI_BUF_LEN - RPC_FRAME_HDR_SZ)

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */
/* The following function is not used */
#define HalSpiPoll()
  
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          HalSpiInit
 *
 * @brief       This function is called to set up the SPI interface.
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
extern void HalSpiInit(void);

/**************************************************************************************************
 * @fn          halSpiRxIsr
 *
 * @brief       This function handles the DMA Rx complete interrupt.
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
extern void HalSpiRxIsr(void);

/**************************************************************************************************
 * @fn          halSpiTxIsr
 *
 * @brief       This function handles the DMA Tx complete interrupt.
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
extern void HalSpiTxIsr(void);

/**************************************************************************************************
 * @fn          halSpiPollCallback
 *
 * @brief       This function is called by the SPI driver when a POLL frame is received.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      A pointer to an OSAL message buffer containing the next AREQ frame to transmit,
 *              if any; NULL otherwise.
 **************************************************************************************************
 */
extern uint8 *npSpiPollCallback(void);

/**************************************************************************************************
 * @fn          halSpiPollReadyback
 *
 * @brief       This function is called by the SPI driver to check if any data is ready to send.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if data is ready to send; FALSE otherwise.
 **************************************************************************************************
 */
extern bool npSpiReadyCallback(void);

/**************************************************************************************************
 * @fn          npSpiTxCompleteCallback
 *
 * @brief       This function is called by the SPI driver when frame transmission is complete.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 **************************************************************************************************
 */
extern void npSpiTxCompleteCallback(void);

/**************************************************************************************************
 * @fn          halSpiSRspAlloc
 *
 * @brief       This function is called by MT to allocate a buffer in which to build an SRSP frame.
 *              MT must only call this function after processing a received SREQ frame.
 *
 * input parameters
 *
 * @param       len - Length of the buffer required.
 *
 * output parameters
 *
 * None.
 *
 * @return      NULL for failure; a pointer to the halSpiBuf on success. Success is determined by
 *              the correct halSpiState and H/W signals as well as a valid length request.
 **************************************************************************************************
 */
extern uint8 *npSpiSRspAlloc(uint8 len);

/**************************************************************************************************
 * @fn          halSpiAReqAlloc
 *
 * @brief       This function is called by MT to allocate a buffer in which to buld an AREQ frame.
 *
 * input parameters
 *
 * @param       len - Length of the buffer required.
 *
 * output parameters
 *
 * None.
 *
 * @return      NULL for failure; otherwise a pointer to the data of an osal message.
 **************************************************************************************************
 */
extern uint8 *npSpiAReqAlloc(uint8 len);

/**************************************************************************************************
 * @fn          halSpiSRspReady
 *
 * @brief       This function is called by MT to notify SPI driver that an SRSP is ready to Tx.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the buffer to transmit on the SPI.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
extern void npSpiSRspReady(uint8 *pBuf);

/**************************************************************************************************
 * @fn          halSpiAReqReady
 *
 * @brief       This function is called by MT to notify the SPI driver that an AREQ frame is ready
 *              to be transmitted.
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
extern void npSpiAReqReady(void);

/**************************************************************************************************
 * @fn          halSpiAReqComplete
 *
 * @brief       This function is called by MT to notify the SPI driver that the processing of a
 *              received AREQ is complete.
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
extern void npSpiAReqComplete(void);

/**************************************************************************************************
 * @fn          halSpiGetReqBuf
 *
 * @brief       This function is called by the application to get the buffer containing the
 *              currently received AREQ or SREQ.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to the buffer containing the currently received AREQ or SREQ.
 **************************************************************************************************
 */
extern uint8 *npSpiGetReqBuf(void);

/**************************************************************************************************
 * @fn          halSpiMrdyIsr
 *
 * @brief       This function is called when a GPIO falling-edge interrupt occurs on the MRDY.
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
extern void npSpiMrdyIsr(void);

/**************************************************************************************************
 * @fn          halSpiIdle
 *
 * @brief       This function returns true if SPI is idle and there is no queued data.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      True if SPI is idle with no quequed data.
 **************************************************************************************************
 */
extern bool npSpiIdle(void);

/**************************************************************************************************
 * @fn          npSpiMonitor
 *
 * @brief       This function monitors the SPI signals for error conditions and for the end of a
 *              transaction. If an error is detected it attempts to recover.
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
extern void npSpiMonitor(void);

/**************************************************************************************************
 * @fn          HalSpiAssertSrdy
 *
 * @brief       This function is called to assert the SRDY signal.
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
extern void HalSpiAssertSrdy(void);

/**************************************************************************************************
*/

#ifdef __cplusplus
};
#endif

#endif /* HAL_SPI_H */

/******************************************************************************
******************************************************************************/

#endif  // #if ((defined HAL_SPI) && (HAL_SPI == TRUE))
