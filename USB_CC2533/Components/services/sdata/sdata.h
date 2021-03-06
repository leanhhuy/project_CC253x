/**************************************************************************************************
  Filename:       sdata.h
**************************************************************************************************/

#ifndef SDATA_H
#define SDATA_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct
{
  uint8     *p;
  uint8     len;
} sData_t;

#ifdef __cplusplus
}
#endif

#endif /* SDATA_H */
