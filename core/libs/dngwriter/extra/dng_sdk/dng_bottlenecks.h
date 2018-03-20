/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_bottlenecks.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Indirection mechanism for performance-critical routines that might be replaced
 * with hand-optimized or hardware-specific implementations.
 */

/*****************************************************************************/

#ifndef __dng_bottlenecks__
#define __dng_bottlenecks__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"

/*****************************************************************************/

typedef void (ZeroBytesProc)
			 (void *dPtr,
			  uint32 count);

typedef void (CopyBytesProc)
			 (const void *sPtr,
			  void *dPtr,
			  uint32 count);

/*****************************************************************************/

typedef void (SwapBytes16Proc)
			 (uint16 *dPtr,
			  uint32 count);

typedef void (SwapBytes32Proc)
			 (uint32 *dPtr,
			  uint32 count);

/*****************************************************************************/

typedef void (SetArea8Proc)
			 (uint8 *dPtr,
			  uint8 value,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep);

typedef void (SetArea16Proc)
			 (uint16 *dPtr,
			  uint16 value,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep);

typedef void (SetArea32Proc)
			 (uint32 *dPtr,
			  uint32 value,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep);

/*****************************************************************************/

typedef void (CopyArea8Proc)
			 (const uint8 *sPtr,
			  uint8 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea16Proc)
			 (const uint16 *sPtr,
			  uint16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea32Proc)
			 (const uint32 *sPtr,
			  uint32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea8_16Proc)
			 (const uint8 *sPtr,
			  uint16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea8_S16Proc)
			 (const uint8 *sPtr,
			  int16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea8_32Proc)
			 (const uint8 *sPtr,
			  uint32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea16_S16Proc)
			 (const uint16 *sPtr,
			  int16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea16_32Proc)
			 (const uint16 *sPtr,
			  uint32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef void (CopyArea8_R32Proc)
			 (const uint8 *sPtr,
			  real32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep,
			  uint32 pixelRange);

typedef void (CopyArea16_R32Proc)
			 (const uint16 *sPtr,
			  real32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep,
			  uint32 pixelRange);

typedef void (CopyAreaS16_R32Proc)
			 (const int16 *sPtr,
			  real32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep,
			  uint32 pixelRange);

typedef void (CopyAreaR32_8Proc)
			 (const real32 *sPtr,
			  uint8 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep,
			  uint32 pixelRange);

typedef void (CopyAreaR32_16Proc)
			 (const real32 *sPtr,
			  uint16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep,
			  uint32 pixelRange);

typedef void (CopyAreaR32_S16Proc)
			 (const real32 *sPtr,
			  int16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep,
			  uint32 pixelRange);

/*****************************************************************************/

typedef void (RepeatArea8Proc)
			 (const uint8 *sPtr,
			  uint8 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep,
			  uint32 repeatV,
			  uint32 repeatH,
			  uint32 phaseV,
			  uint32 phaseH);

typedef void (RepeatArea16Proc)
			 (const uint16 *sPtr,
			  uint16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep,
			  uint32 repeatV,
			  uint32 repeatH,
			  uint32 phaseV,
			  uint32 phaseH);

typedef void (RepeatArea32Proc)
			 (const uint32 *sPtr,
			  uint32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep,
			  uint32 repeatV,
			  uint32 repeatH,
			  uint32 phaseV,
			  uint32 phaseH);

/*****************************************************************************/

typedef void (ShiftRight16Proc)
			 (uint16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 rowStep,
			  int32 colStep,
			  int32 planeStep,
			  uint32 shift);

/*****************************************************************************/

typedef void (BilinearRow16Proc)
			 (const uint16 *sPtr,
			  uint16 *dPtr,
			  uint32 cols,
			  uint32 patPhase,
			  uint32 patCount,
			  const uint32 * kernCounts,
			  const int32  * const * kernOffsets,
			  const uint16 * const * kernWeights,
			  uint32 sShift);

typedef void (BilinearRow32Proc)
			 (const real32 *sPtr,
			  real32 *dPtr,
			  uint32 cols,
			  uint32 patPhase,
			  uint32 patCount,
			  const uint32 * kernCounts,
			  const int32  * const * kernOffsets,
			  const real32 * const * kernWeights,
			  uint32 sShift);

/*****************************************************************************/

typedef void (BaselineABCtoRGBProc)
			 (const real32 *sPtrA,
			  const real32 *sPtrB,
			  const real32 *sPtrC,
			  real32 *dPtrR,
			  real32 *dPtrG,
			  real32 *dPtrB,
			  uint32 count,
			  const dng_vector &cameraWhite,
			  const dng_matrix &cameraToRGB);

typedef void (BaselineABCDtoRGBProc)
			 (const real32 *sPtrA,
			  const real32 *sPtrB,
			  const real32 *sPtrC,
			  const real32 *sPtrD,
			  real32 *dPtrR,
			  real32 *dPtrG,
			  real32 *dPtrB,
			  uint32 count,
			  const dng_vector &cameraWhite,
			  const dng_matrix &cameraToRGB);

/*****************************************************************************/

typedef void (BaselineHueSatMapProc)
			 (const real32 *sPtrR,
			  const real32 *sPtrG,
			  const real32 *sPtrB,
			  real32 *dPtrR,
			  real32 *dPtrG,
			  real32 *dPtrB,
			  uint32 count,
			  const dng_hue_sat_map &lut);

/*****************************************************************************/

typedef void (BaselineGrayToRGBProc)
			 (const real32 *sPtrR,
			  const real32 *sPtrG,
			  const real32 *sPtrB,
			  real32 *dPtrG,
			  uint32 count,
			  const dng_matrix &matrix);

typedef void (BaselineRGBtoRGBProc)
			 (const real32 *sPtrR,
			  const real32 *sPtrG,
			  const real32 *sPtrB,
			  real32 *dPtrR,
			  real32 *dPtrG,
			  real32 *dPtrB,
			  uint32 count,
			  const dng_matrix &matrix);

/*****************************************************************************/

typedef void (Baseline1DTableProc)
			 (const real32 *sPtr,
			  real32 *dPtr,
			  uint32 count,
			  const dng_1d_table &table);

/*****************************************************************************/

typedef void (BaselineRGBToneProc)
			 (const real32 *sPtrR,
			  const real32 *sPtrG,
			  const real32 *sPtrB,
			  real32 *dPtrR,
			  real32 *dPtrG,
			  real32 *dPtrB,
			  uint32 count,
			  const dng_1d_table &table);

/*****************************************************************************/

typedef void (ResampleDown16Proc)
			 (const uint16 *sPtr,
			  uint16 *dPtr,
			  uint32 sCount,
			  int32 sRowStep,
			  const int16 *wPtr,
			  uint32 wCount,
			  uint32 pixelRange);

typedef void (ResampleDown32Proc)
			 (const real32 *sPtr,
			  real32 *dPtr,
			  uint32 sCount,
			  int32 sRowStep,
			  const real32 *wPtr,
			  uint32 wCount);

/*****************************************************************************/

typedef void (ResampleAcross16Proc)
			 (const uint16 *sPtr,
			  uint16 *dPtr,
			  uint32 dCount,
			  const int32 *coord,
			  const int16 *wPtr,
			  uint32 wCount,
			  uint32 wStep,
			  uint32 pixelRange);

typedef void (ResampleAcross32Proc)
			 (const real32 *sPtr,
			  real32 *dPtr,
			  uint32 dCount,
			  const int32 *coord,
			  const real32 *wPtr,
			  uint32 wCount,
			  uint32 wStep);

/*****************************************************************************/

typedef bool (EqualBytesProc)
			 (const void *sPtr,
			  const void *dPtr,
			  uint32 count);

typedef bool (EqualArea8Proc)
			 (const uint8 *sPtr,
			  const uint8 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef bool (EqualArea16Proc)
			 (const uint16 *sPtr,
			  const uint16 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

typedef bool (EqualArea32Proc)
			 (const uint32 *sPtr,
			  const uint32 *dPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sColStep,
			  int32 sPlaneStep,
			  int32 dRowStep,
			  int32 dColStep,
			  int32 dPlaneStep);

/*****************************************************************************/

typedef void (VignetteMask16Proc)
			 (uint16 *mPtr,
			  uint32 rows,
			  uint32 cols,
			  int32 rowStep,
			  int64 offsetH,
			  int64 offsetV,
			  int64 stepH,
			  int64 stepV,
			  uint32 tBits,
			  const uint16 *table);

typedef void (Vignette16Proc)
			 (int16 *sPtr,
			  const uint16 *mPtr,
			  uint32 rows,
			  uint32 cols,
			  uint32 planes,
			  int32 sRowStep,
			  int32 sPlaneStep,
			  int32 mRowStep,
			  uint32 mBits);

/*****************************************************************************/

typedef void (MapArea16Proc)
			 (uint16 *dPtr,
			  uint32 count0,
			  uint32 count1,
			  uint32 count2,
			  int32 step0,
			  int32 step1,
			  int32 step2,
			  const uint16 *map);

/*****************************************************************************/

struct dng_suite
	{
	ZeroBytesProc			*ZeroBytes;
	CopyBytesProc			*CopyBytes;
	SwapBytes16Proc			*SwapBytes16;
	SwapBytes32Proc			*SwapBytes32;
	SetArea8Proc			*SetArea8;
	SetArea16Proc			*SetArea16;
	SetArea32Proc			*SetArea32;
	CopyArea8Proc			*CopyArea8;
	CopyArea16Proc			*CopyArea16;
	CopyArea32Proc			*CopyArea32;
	CopyArea8_16Proc		*CopyArea8_16;
	CopyArea8_S16Proc		*CopyArea8_S16;
	CopyArea8_32Proc		*CopyArea8_32;
	CopyArea16_S16Proc		*CopyArea16_S16;
	CopyArea16_32Proc		*CopyArea16_32;
	CopyArea8_R32Proc		*CopyArea8_R32;
	CopyArea16_R32Proc		*CopyArea16_R32;
	CopyAreaS16_R32Proc		*CopyAreaS16_R32;
	CopyAreaR32_8Proc		*CopyAreaR32_8;
	CopyAreaR32_16Proc		*CopyAreaR32_16;
	CopyAreaR32_S16Proc		*CopyAreaR32_S16;
	RepeatArea8Proc			*RepeatArea8;
	RepeatArea16Proc		*RepeatArea16;
	RepeatArea32Proc		*RepeatArea32;
	ShiftRight16Proc		*ShiftRight16;
	BilinearRow16Proc		*BilinearRow16;
	BilinearRow32Proc		*BilinearRow32;
	BaselineABCtoRGBProc	*BaselineABCtoRGB;
	BaselineABCDtoRGBProc	*BaselineABCDtoRGB;
	BaselineHueSatMapProc	*BaselineHueSatMap;
	BaselineGrayToRGBProc	*BaselineRGBtoGray;
	BaselineRGBtoRGBProc	*BaselineRGBtoRGB;
	Baseline1DTableProc		*Baseline1DTable;
	BaselineRGBToneProc		*BaselineRGBTone;
	ResampleDown16Proc		*ResampleDown16;
	ResampleDown32Proc		*ResampleDown32;
	ResampleAcross16Proc	*ResampleAcross16;
	ResampleAcross32Proc	*ResampleAcross32;
	EqualBytesProc			*EqualBytes;
	EqualArea8Proc			*EqualArea8;
	EqualArea16Proc			*EqualArea16;
	EqualArea32Proc			*EqualArea32;
	VignetteMask16Proc		*VignetteMask16;
	Vignette16Proc			*Vignette16;
	MapArea16Proc			*MapArea16;
	};

/*****************************************************************************/

extern dng_suite gDNGSuite;

/*****************************************************************************/

inline void DoZeroBytes (void *dPtr,
						 uint32 count)
	{

	(gDNGSuite.ZeroBytes) (dPtr,
						   count);

	}

inline void DoCopyBytes (const void *sPtr,
						 void *dPtr,
						 uint32 count)
	{

	(gDNGSuite.CopyBytes) (sPtr,
						   dPtr,
						   count);

	}

/*****************************************************************************/

inline void DoSwapBytes16 (uint16 *dPtr,
						   uint32 count)
	{

	(gDNGSuite.SwapBytes16) (dPtr,
						     count);

	}

inline void DoSwapBytes32 (uint32 *dPtr,
						   uint32 count)
	{

	(gDNGSuite.SwapBytes32) (dPtr,
						     count);

	}

/*****************************************************************************/

inline void DoSetArea8 (uint8 *dPtr,
					    uint8 value,
						uint32 rows,
						uint32 cols,
						uint32 planes,
						int32 rowStep,
						int32 colStep,
						int32 planeStep)
	{

	(gDNGSuite.SetArea8) (dPtr,
						  value,
						  rows,
						  cols,
						  planes,
						  rowStep,
						  colStep,
						  planeStep);

	}

inline void DoSetArea16 (uint16 *dPtr,
						 uint16 value,
						 uint32 rows,
						 uint32 cols,
						 uint32 planes,
						 int32 rowStep,
						 int32 colStep,
						 int32 planeStep)
	{

	(gDNGSuite.SetArea16) (dPtr,
						   value,
						   rows,
						   cols,
						   planes,
						   rowStep,
						   colStep,
						   planeStep);

	}

inline void DoSetArea32 (uint32 *dPtr,
						 uint32 value,
						 uint32 rows,
						 uint32 cols,
						 uint32 planes,
						 int32 rowStep,
						 int32 colStep,
						 int32 planeStep)
	{

	(gDNGSuite.SetArea32) (dPtr,
						   value,
						   rows,
						   cols,
						   planes,
						   rowStep,
						   colStep,
						   planeStep);

	}

/*****************************************************************************/

inline void DoCopyArea8 (const uint8 *sPtr,
						 uint8 *dPtr,
						 uint32 rows,
						 uint32 cols,
						 uint32 planes,
						 int32 sRowStep,
						 int32 sColStep,
						 int32 sPlaneStep,
						 int32 dRowStep,
						 int32 dColStep,
						 int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea8) (sPtr,
						   dPtr,
						   rows,
						   cols,
						   planes,
						   sRowStep,
						   sColStep,
						   sPlaneStep,
						   dRowStep,
						   dColStep,
						   dPlaneStep);

	}

inline void DoCopyArea16 (const uint16 *sPtr,
						  uint16 *dPtr,
						  uint32 rows,
						  uint32 cols,
						  uint32 planes,
						  int32 sRowStep,
						  int32 sColStep,
						  int32 sPlaneStep,
						  int32 dRowStep,
						  int32 dColStep,
						  int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea16) (sPtr,
							dPtr,
							rows,
							cols,
							planes,
							sRowStep,
							sColStep,
							sPlaneStep,
							dRowStep,
							dColStep,
							dPlaneStep);

	}

inline void DoCopyArea32 (const uint32 *sPtr,
						  uint32 *dPtr,
						  uint32 rows,
						  uint32 cols,
						  uint32 planes,
						  int32 sRowStep,
						  int32 sColStep,
						  int32 sPlaneStep,
						  int32 dRowStep,
						  int32 dColStep,
						  int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea32) (sPtr,
							dPtr,
							rows,
							cols,
							planes,
							sRowStep,
							sColStep,
							sPlaneStep,
							dRowStep,
							dColStep,
							dPlaneStep);

	}

inline void DoCopyArea8_16 (const uint8 *sPtr,
						    uint16 *dPtr,
						    uint32 rows,
						    uint32 cols,
						    uint32 planes,
						    int32 sRowStep,
						    int32 sColStep,
						    int32 sPlaneStep,
						    int32 dRowStep,
						    int32 dColStep,
						    int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea8_16) (sPtr,
							  dPtr,
							  rows,
							  cols,
							  planes,
							  sRowStep,
							  sColStep,
							  sPlaneStep,
							  dRowStep,
							  dColStep,
							  dPlaneStep);

	}

inline void DoCopyArea8_S16 (const uint8 *sPtr,
						     int16 *dPtr,
						     uint32 rows,
						     uint32 cols,
						     uint32 planes,
						     int32 sRowStep,
						     int32 sColStep,
						     int32 sPlaneStep,
						     int32 dRowStep,
						     int32 dColStep,
						     int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea8_S16) (sPtr,
							   dPtr,
							   rows,
							   cols,
							   planes,
							   sRowStep,
							   sColStep,
							   sPlaneStep,
							   dRowStep,
							   dColStep,
							   dPlaneStep);

	}

inline void DoCopyArea8_32 (const uint8 *sPtr,
						    uint32 *dPtr,
						    uint32 rows,
						    uint32 cols,
						    uint32 planes,
						    int32 sRowStep,
						    int32 sColStep,
						    int32 sPlaneStep,
						    int32 dRowStep,
						    int32 dColStep,
						    int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea8_32) (sPtr,
							  dPtr,
							  rows,
							  cols,
							  planes,
							  sRowStep,
							  sColStep,
							  sPlaneStep,
							  dRowStep,
							  dColStep,
							  dPlaneStep);

	}

inline void DoCopyArea16_S16 (const uint16 *sPtr,
						      int16 *dPtr,
						      uint32 rows,
						      uint32 cols,
						      uint32 planes,
						      int32 sRowStep,
						      int32 sColStep,
						      int32 sPlaneStep,
						      int32 dRowStep,
						      int32 dColStep,
						      int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea16_S16) (sPtr,
							    dPtr,
							    rows,
							    cols,
							    planes,
							    sRowStep,
							    sColStep,
							    sPlaneStep,
							    dRowStep,
							    dColStep,
							    dPlaneStep);

	}

inline void DoCopyArea16_32 (const uint16 *sPtr,
						     uint32 *dPtr,
						     uint32 rows,
						     uint32 cols,
						     uint32 planes,
						     int32 sRowStep,
						     int32 sColStep,
						     int32 sPlaneStep,
						     int32 dRowStep,
						     int32 dColStep,
						     int32 dPlaneStep)
	{

	(gDNGSuite.CopyArea16_32) (sPtr,
							   dPtr,
							   rows,
							   cols,
							   planes,
							   sRowStep,
							   sColStep,
							   sPlaneStep,
							   dRowStep,
							   dColStep,
							   dPlaneStep);

	}

inline void DoCopyArea8_R32 (const uint8 *sPtr,
							 real32 *dPtr,
							 uint32 rows,
							 uint32 cols,
							 uint32 planes,
							 int32 sRowStep,
							 int32 sColStep,
							 int32 sPlaneStep,
							 int32 dRowStep,
							 int32 dColStep,
							 int32 dPlaneStep,
							 uint32 pixelRange)
	{

	(gDNGSuite.CopyArea8_R32) (sPtr,
							   dPtr,
							   rows,
							   cols,
							   planes,
							   sRowStep,
							   sColStep,
							   sPlaneStep,
							   dRowStep,
							   dColStep,
							   dPlaneStep,
							   pixelRange);

	}

inline void DoCopyArea16_R32 (const uint16 *sPtr,
							  real32 *dPtr,
							  uint32 rows,
							  uint32 cols,
							  uint32 planes,
							  int32 sRowStep,
							  int32 sColStep,
							  int32 sPlaneStep,
							  int32 dRowStep,
							  int32 dColStep,
							  int32 dPlaneStep,
							  uint32 pixelRange)
	{

	(gDNGSuite.CopyArea16_R32) (sPtr,
							    dPtr,
							    rows,
							    cols,
							    planes,
							    sRowStep,
							    sColStep,
							    sPlaneStep,
							    dRowStep,
							    dColStep,
							    dPlaneStep,
							    pixelRange);

	}

inline void DoCopyAreaS16_R32 (const int16 *sPtr,
							   real32 *dPtr,
							   uint32 rows,
							   uint32 cols,
							   uint32 planes,
							   int32 sRowStep,
							   int32 sColStep,
							   int32 sPlaneStep,
							   int32 dRowStep,
							   int32 dColStep,
							   int32 dPlaneStep,
							   uint32 pixelRange)
	{

	(gDNGSuite.CopyAreaS16_R32) (sPtr,
							     dPtr,
							     rows,
							     cols,
							     planes,
							     sRowStep,
							     sColStep,
							     sPlaneStep,
							     dRowStep,
							     dColStep,
							     dPlaneStep,
							     pixelRange);

	}

inline void DoCopyAreaR32_8 (const real32 *sPtr,
							 uint8 *dPtr,
							 uint32 rows,
							 uint32 cols,
							 uint32 planes,
							 int32 sRowStep,
							 int32 sColStep,
							 int32 sPlaneStep,
							 int32 dRowStep,
							 int32 dColStep,
							 int32 dPlaneStep,
							 uint32 pixelRange)
	{

	(gDNGSuite.CopyAreaR32_8) (sPtr,
							   dPtr,
							   rows,
							   cols,
							   planes,
							   sRowStep,
							   sColStep,
							   sPlaneStep,
							   dRowStep,
							   dColStep,
							   dPlaneStep,
							   pixelRange);

	}

inline void DoCopyAreaR32_16 (const real32 *sPtr,
							  uint16 *dPtr,
							  uint32 rows,
							  uint32 cols,
							  uint32 planes,
							  int32 sRowStep,
							  int32 sColStep,
							  int32 sPlaneStep,
							  int32 dRowStep,
							  int32 dColStep,
							  int32 dPlaneStep,
							  uint32 pixelRange)
	{

	(gDNGSuite.CopyAreaR32_16) (sPtr,
							    dPtr,
							    rows,
							    cols,
							    planes,
							    sRowStep,
							    sColStep,
							    sPlaneStep,
							    dRowStep,
							    dColStep,
							    dPlaneStep,
							    pixelRange);

	}

inline void DoCopyAreaR32_S16 (const real32 *sPtr,
							   int16 *dPtr,
							   uint32 rows,
							   uint32 cols,
							   uint32 planes,
							   int32 sRowStep,
							   int32 sColStep,
							   int32 sPlaneStep,
							   int32 dRowStep,
							   int32 dColStep,
							   int32 dPlaneStep,
							   uint32 pixelRange)
	{

	(gDNGSuite.CopyAreaR32_S16) (sPtr,
							     dPtr,
							     rows,
							     cols,
							     planes,
							     sRowStep,
							     sColStep,
							     sPlaneStep,
							     dRowStep,
							     dColStep,
							     dPlaneStep,
							     pixelRange);

	}

/*****************************************************************************/

inline void DoRepeatArea8 (const uint8 *sPtr,
						   uint8 *dPtr,
						   uint32 rows,
						   uint32 cols,
						   uint32 planes,
						   int32 rowStep,
						   int32 colStep,
						   int32 planeStep,
						   uint32 repeatV,
						   uint32 repeatH,
						   uint32 phaseV,
						   uint32 phaseH)
	{

	(gDNGSuite.RepeatArea8) (sPtr,
						     dPtr,
						     rows,
						     cols,
						     planes,
						     rowStep,
						     colStep,
						     planeStep,
						     repeatV,
						     repeatH,
						     phaseV,
						     phaseH);

	}

inline void DoRepeatArea16 (const uint16 *sPtr,
							uint16 *dPtr,
							uint32 rows,
							uint32 cols,
							uint32 planes,
							int32 rowStep,
							int32 colStep,
							int32 planeStep,
							uint32 repeatV,
							uint32 repeatH,
							uint32 phaseV,
							uint32 phaseH)
	{

	(gDNGSuite.RepeatArea16) (sPtr,
						      dPtr,
						      rows,
						      cols,
						      planes,
						      rowStep,
						      colStep,
						      planeStep,
						      repeatV,
						      repeatH,
						      phaseV,
						      phaseH);

	}

inline void DoRepeatArea32 (const uint32 *sPtr,
							uint32 *dPtr,
							uint32 rows,
							uint32 cols,
							uint32 planes,
							int32 rowStep,
							int32 colStep,
							int32 planeStep,
							uint32 repeatV,
							uint32 repeatH,
							uint32 phaseV,
							uint32 phaseH)
	{

	(gDNGSuite.RepeatArea32) (sPtr,
						      dPtr,
						      rows,
						      cols,
						      planes,
						      rowStep,
						      colStep,
						      planeStep,
						      repeatV,
						      repeatH,
						      phaseV,
						      phaseH);

	}

/*****************************************************************************/

inline void DoShiftRight16 (uint16 *dPtr,
						    uint32 rows,
						    uint32 cols,
						    uint32 planes,
						    int32 rowStep,
						    int32 colStep,
						    int32 planeStep,
						    uint32 shift)
	{

	(gDNGSuite.ShiftRight16) (dPtr,
							  rows,
							  cols,
							  planes,
							  rowStep,
							  colStep,
							  planeStep,
							  shift);

	}

/*****************************************************************************/

inline void DoBilinearRow16 (const uint16 *sPtr,
							 uint16 *dPtr,
							 uint32 cols,
							 uint32 patPhase,
							 uint32 patCount,
							 const uint32 * kernCounts,
							 const int32  * const * kernOffsets,
							 const uint16 * const * kernWeights,
							 uint32 sShift)
	{

	(gDNGSuite.BilinearRow16) (sPtr,
							   dPtr,
							   cols,
							   patPhase,
							   patCount,
							   kernCounts,
							   kernOffsets,
							   kernWeights,
							   sShift);

	}

inline void DoBilinearRow32 (const real32 *sPtr,
							 real32 *dPtr,
							 uint32 cols,
							 uint32 patPhase,
							 uint32 patCount,
							 const uint32 * kernCounts,
							 const int32  * const * kernOffsets,
							 const real32 * const * kernWeights,
							 uint32 sShift)
	{

	(gDNGSuite.BilinearRow32) (sPtr,
							   dPtr,
							   cols,
							   patPhase,
							   patCount,
							   kernCounts,
							   kernOffsets,
							   kernWeights,
							   sShift);

	}

/*****************************************************************************/

inline void DoBaselineABCtoRGB (const real32 *sPtrA,
								const real32 *sPtrB,
								const real32 *sPtrC,
								real32 *dPtrR,
								real32 *dPtrG,
								real32 *dPtrB,
								uint32 count,
								const dng_vector &cameraWhite,
								const dng_matrix &cameraToRGB)
	{

	(gDNGSuite.BaselineABCtoRGB) (sPtrA,
								  sPtrB,
								  sPtrC,
								  dPtrR,
								  dPtrG,
								  dPtrB,
								  count,
								  cameraWhite,
								  cameraToRGB);

	}

inline void DoBaselineABCDtoRGB (const real32 *sPtrA,
								 const real32 *sPtrB,
								 const real32 *sPtrC,
								 const real32 *sPtrD,
								 real32 *dPtrR,
								 real32 *dPtrG,
								 real32 *dPtrB,
								 uint32 count,
								 const dng_vector &cameraWhite,
								 const dng_matrix &cameraToRGB)
	{

	(gDNGSuite.BaselineABCDtoRGB) (sPtrA,
								   sPtrB,
								   sPtrC,
								   sPtrD,
								   dPtrR,
								   dPtrG,
								   dPtrB,
								   count,
								   cameraWhite,
								   cameraToRGB);

	}

/*****************************************************************************/

inline void DoBaselineHueSatMap (const real32 *sPtrR,
								 const real32 *sPtrG,
								 const real32 *sPtrB,
								 real32 *dPtrR,
								 real32 *dPtrG,
								 real32 *dPtrB,
								 uint32 count,
								 const dng_hue_sat_map &lut)
	{

	(gDNGSuite.BaselineHueSatMap) (sPtrR,
								   sPtrG,
								   sPtrB,
								   dPtrR,
								   dPtrG,
								   dPtrB,
								   count,
								   lut);

	}

/*****************************************************************************/

inline void DoBaselineRGBtoGray (const real32 *sPtrR,
								 const real32 *sPtrG,
								 const real32 *sPtrB,
								 real32 *dPtrG,
								 uint32 count,
								 const dng_matrix &matrix)
	{

	(gDNGSuite.BaselineRGBtoGray) (sPtrR,
								   sPtrG,
								   sPtrB,
								   dPtrG,
								   count,
								   matrix);

	}

inline void DoBaselineRGBtoRGB (const real32 *sPtrR,
								const real32 *sPtrG,
								const real32 *sPtrB,
								real32 *dPtrR,
								real32 *dPtrG,
								real32 *dPtrB,
								uint32 count,
								const dng_matrix &matrix)
	{

	(gDNGSuite.BaselineRGBtoRGB) (sPtrR,
								  sPtrG,
								  sPtrB,
								  dPtrR,
								  dPtrG,
								  dPtrB,
								  count,
								  matrix);

	}

/*****************************************************************************/

inline void DoBaseline1DTable (const real32 *sPtr,
							   real32 *dPtr,
							   uint32 count,
							   const dng_1d_table &table)
	{

	(gDNGSuite.Baseline1DTable) (sPtr,
								 dPtr,
								 count,
								 table);

	}

/*****************************************************************************/

inline void DoBaselineRGBTone (const real32 *sPtrR,
							   const real32 *sPtrG,
							   const real32 *sPtrB,
							   real32 *dPtrR,
							   real32 *dPtrG,
							   real32 *dPtrB,
							   uint32 count,
							   const dng_1d_table &table)
	{

	(gDNGSuite.BaselineRGBTone) (sPtrR,
								 sPtrG,
								 sPtrB,
								 dPtrR,
								 dPtrG,
								 dPtrB,
								 count,
								 table);

	}

/*****************************************************************************/

inline void DoResampleDown16 (const uint16 *sPtr,
							  uint16 *dPtr,
							  uint32 sCount,
							  int32 sRowStep,
							  const int16 *wPtr,
							  uint32 wCount,
							  uint32 pixelRange)
	{

	(gDNGSuite.ResampleDown16) (sPtr,
								dPtr,
								sCount,
								sRowStep,
								wPtr,
								wCount,
								pixelRange);

	}

inline void DoResampleDown32 (const real32 *sPtr,
							  real32 *dPtr,
							  uint32 sCount,
							  int32 sRowStep,
							  const real32 *wPtr,
							  uint32 wCount)
	{

	(gDNGSuite.ResampleDown32) (sPtr,
								dPtr,
								sCount,
								sRowStep,
								wPtr,
								wCount);

	}

/*****************************************************************************/

inline void DoResampleAcross16 (const uint16 *sPtr,
								uint16 *dPtr,
								uint32 dCount,
								const int32 *coord,
								const int16 *wPtr,
								uint32 wCount,
								uint32 wStep,
								uint32 pixelRange)
	{

	(gDNGSuite.ResampleAcross16) (sPtr,
								  dPtr,
								  dCount,
								  coord,
								  wPtr,
								  wCount,
								  wStep,
								  pixelRange);

	}

inline void DoResampleAcross32 (const real32 *sPtr,
								real32 *dPtr,
								uint32 dCount,
								const int32 *coord,
								const real32 *wPtr,
								uint32 wCount,
								uint32 wStep)
	{

	(gDNGSuite.ResampleAcross32) (sPtr,
								  dPtr,
								  dCount,
								  coord,
								  wPtr,
								  wCount,
								  wStep);

	}

/*****************************************************************************/

inline bool DoEqualBytes (const void *sPtr,
						  const void *dPtr,
						  uint32 count)
	{

	return (gDNGSuite.EqualBytes) (sPtr,
								   dPtr,
								   count);

	}

inline bool DoEqualArea8 (const uint8 *sPtr,
						  const uint8 *dPtr,
						  uint32 rows,
						  uint32 cols,
						  uint32 planes,
						  int32 sRowStep,
						  int32 sColStep,
						  int32 sPlaneStep,
						  int32 dRowStep,
						  int32 dColStep,
						  int32 dPlaneStep)
	{

	return (gDNGSuite.EqualArea8) (sPtr,
								   dPtr,
								   rows,
								   cols,
								   planes,
								   sRowStep,
								   sColStep,
								   sPlaneStep,
								   dRowStep,
								   dColStep,
								   dPlaneStep);

	}

inline bool DoEqualArea16 (const uint16 *sPtr,
						   const uint16 *dPtr,
						   uint32 rows,
						   uint32 cols,
						   uint32 planes,
						   int32 sRowStep,
						   int32 sColStep,
						   int32 sPlaneStep,
						   int32 dRowStep,
						   int32 dColStep,
						   int32 dPlaneStep)
	{

	return (gDNGSuite.EqualArea16) (sPtr,
									dPtr,
									rows,
									cols,
									planes,
									sRowStep,
									sColStep,
									sPlaneStep,
									dRowStep,
									dColStep,
									dPlaneStep);

	}

inline bool DoEqualArea32 (const uint32 *sPtr,
						   const uint32 *dPtr,
						   uint32 rows,
						   uint32 cols,
						   uint32 planes,
						   int32 sRowStep,
						   int32 sColStep,
						   int32 sPlaneStep,
						   int32 dRowStep,
						   int32 dColStep,
						   int32 dPlaneStep)
	{

	return (gDNGSuite.EqualArea32) (sPtr,
									dPtr,
									rows,
									cols,
									planes,
									sRowStep,
									sColStep,
									sPlaneStep,
									dRowStep,
									dColStep,
									dPlaneStep);

	}

/*****************************************************************************/

inline void DoVignetteMask16 (uint16 *mPtr,
							  uint32 rows,
							  uint32 cols,
							  int32 rowStep,
							  int64 offsetH,
							  int64 offsetV,
							  int64 stepH,
							  int64 stepV,
							  uint32 tBits,
							  const uint16 *table)
	{

	(gDNGSuite.VignetteMask16) (mPtr,
								rows,
								cols,
								rowStep,
								offsetH,
								offsetV,
								stepH,
								stepV,
								tBits,
								table);

	}

/*****************************************************************************/

inline void DoVignette16 (int16 *sPtr,
						  const uint16 *mPtr,
						  uint32 rows,
						  uint32 cols,
						  uint32 planes,
						  int32 sRowStep,
						  int32 sPlaneStep,
						  int32 mRowStep,
						  uint32 mBits)
	{

	(gDNGSuite.Vignette16) (sPtr,
							mPtr,
							rows,
							cols,
							planes,
							sRowStep,
							sPlaneStep,
							mRowStep,
							mBits);

	}

/*****************************************************************************/

inline void DoMapArea16 (uint16 *dPtr,
						 uint32 count0,
						 uint32 count1,
						 uint32 count2,
						 int32 step0,
						 int32 step1,
						 int32 step2,
						 const uint16 *map)
	{

	(gDNGSuite.MapArea16) (dPtr,
						   count0,
						   count1,
						   count2,
						   step0,
						   step1,
						   step2,
						   map);

	}

/*****************************************************************************/

#endif

/*****************************************************************************/
