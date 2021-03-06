/*
  Filename:       zid_ada_app_helper.c
*/


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "comdef.h"
#include "OSAL.h"
#include "rti.h"
#include "zid_common.h"
#include "zid_profile.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          zidAdaAppHlp_ReadNonStdDescComp
 *
 * @brief       Read a (possibly fragmented) non standard descriptor component from NV
 *
 * input parameters
 *
 * @param  pairIdx  - pairing index of class device whose non-std descriptor is to be read
 * @param  descNum  - non-std descriptor number (0 to (aplcMaxNonStdDescCompsPerHID - 1))
 * @param  pBuf     - Pointer to buffer to place non-std descriptor component as defined in Table 9 of ZID Profile, r15
 *
 * output parameters
 *
 * None.
 *
 * @return
 *
 * None.
 *
 */
rStatus_t zidAdaAppHlp_ReadNonStdDescComp( uint8 pairIdx, uint8 descNum, zid_non_std_desc_comp_t *pBuf )
{
  uint16 len;
  uint8 localDescNum = descNum;
  uint8 localPairIdx = pairIdx;
  uint8 fragNum = 0;
  uint8 *pData;
  zid_non_std_desc_comp_t *pDesc;
  zid_frag_non_std_desc_comp_t *pFrag;

  /* Allocate memory for temporary storage of fragments */
  pData = osal_mem_alloc( ZID_NON_STD_DESC_SPEC_FRAG_T_MAX );
  pDesc = (zid_non_std_desc_comp_t *)pData;
  pFrag = (zid_frag_non_std_desc_comp_t *)pData;

  /* Validate input data */
  if ((pBuf == NULL) || (pData == NULL) || (descNum >= aplcMaxNonStdDescCompsPerHID))
  {
    return RTI_ERROR_INVALID_PARAMETER;
  }

  /* First store current descriptor number in ZID */
  (void)RTI_WriteItemEx( RTI_PROFILE_ZID, ZID_ITEM_CURRENT_PXY_NUM, 1, &localPairIdx );
  (void)RTI_WriteItemEx( RTI_PROFILE_ZID, ZID_ITEM_NON_STD_DESC_NUM, 1, &localDescNum );
  (void)RTI_WriteItemEx( RTI_PROFILE_ZID, ZID_ITEM_NON_STD_DESC_FRAG_NUM, 1, &fragNum );

  /* Read "header" of first fragment to determine total length */
  if (RTI_ReadItemEx( RTI_PROFILE_ZID, ZID_ITEM_NON_STD_DESC_FRAG, sizeof(zid_non_std_desc_comp_t), pData ) != RTI_SUCCESS)
  {
    return RTI_ERROR_INVALID_PARAMETER;
  }

  /* Store "header" in dest buf */
  pBuf = osal_memcpy( pBuf, pData, sizeof( zid_non_std_desc_comp_t ) );

  /* Unpack descriptor length */
  len = (pDesc->size_h << 8) | pDesc->size_l;

  /* Determine if non-std descriptor component needs to be fragmented or not */
  if (len > aplcMaxNonStdDescFragmentSize)
  {
    /* Store each fragment */
    while (len > 0)
    {
      uint8 fragLen;

      if (len > aplcMaxNonStdDescFragmentSize)
      {
        fragLen = aplcMaxNonStdDescFragmentSize;
      }
      else
      {
        fragLen = len;
      }

      /* Read fragment data */
      if (RTI_ReadItemEx( RTI_PROFILE_ZID, ZID_ITEM_NON_STD_DESC_FRAG, fragLen + sizeof( zid_frag_non_std_desc_comp_t ), pData ) != RTI_SUCCESS)
      {
        return RTI_ERROR_INVALID_PARAMETER;
      }

      /* Copy fragment data into dest buf */
      pBuf = osal_memcpy( pBuf, pFrag->data, fragLen );

      /* Bump pointers for next fragment */
      len -= fragLen;
      fragNum++;

      /* Write next fragment number */
      (void)RTI_WriteItemEx( RTI_PROFILE_ZID, ZID_ITEM_NON_STD_DESC_FRAG_NUM, 1, &fragNum );
    }
  }
  else
  {
    /* Not fragmented, so just read buffer as is */
    if (RTI_ReadItemEx( RTI_PROFILE_ZID, ZID_ITEM_NON_STD_DESC_FRAG, len + sizeof( zid_non_std_desc_comp_t ), pData ) != RTI_SUCCESS)
    {
      return RTI_ERROR_INVALID_PARAMETER;
    }

    /* Copy fragment data into dest buf */
    pBuf = osal_memcpy( pBuf, pDesc->data, len );
  }

  /* Free up memory */
  osal_mem_free( pData );

  return RTI_SUCCESS;
}

/**************************************************************************************************
*/
