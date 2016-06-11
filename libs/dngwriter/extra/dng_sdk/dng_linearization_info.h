/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_linearization_info.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Support for linearization table and black level tags.
 */

/*****************************************************************************/

#ifndef __dng_linearization_info__
#define __dng_linearization_info__

/*****************************************************************************/

#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_memory.h"
#include "dng_rational.h"
#include "dng_rect.h"
#include "dng_sdk_limits.h"

/*****************************************************************************/

/// \brief Class for managing data values related to DNG linearization.
///
/// See LinearizationTable, BlackLevel, BlackLevelRepeatDim, BlackLevelDeltaH, BlackLevelDeltaV and WhiteLevel tags in the \ref spec_dng "DNG 1.1.0 specification".

class dng_linearization_info
	{

	public:

		/// This rectangle defines the active (non-masked) pixels of the sensor.
		/// The order of the rectangle coordinates is: top, left, bottom, right.

		dng_rect fActiveArea;

		/// Number of rectangles in fMaskedArea

		uint32   fMaskedAreaCount;

		/// List of non-overlapping rectangle coordinates of fully masked pixels.
		/// Can be optionally used by DNG readers to measure the black encoding level.
		/// The order of each rectangle's coordinates is: top, left, bottom, right.
		/// If the raw image data has already had its black encoding level subtracted, then this tag should
		/// not be used, since the masked pixels are no longer useful.
		/// Note that DNG writers are still required to include an estimate and store the black encoding level
		/// using the black level DNG tags. Support for the MaskedAreas tag is not required of DNG
		/// readers.

		dng_rect fMaskedArea [kMaxMaskedAreas];

		/// A lookup table that maps stored values into linear values.
		/// This tag is typically used to increase compression ratios by storing the raw data in a non-linear, more
		/// visually uniform space with fewer total encoding levels.
		/// If SamplesPerPixel is not equal to one, e.g. Fuji S3 type sensor, this single table applies to all the samples for each
		/// pixel.

		AutoPtr<dng_memory_block> fLinearizationTable;

		/// Actual number of rows in fBlackLevel pattern

		uint32 fBlackLevelRepeatRows;

		/// Actual number of columns in fBlackLevel pattern

		uint32 fBlackLevelRepeatCols;

		/// Repeating pattern of black level deltas fBlackLevelRepeatRows by fBlackLevelRepeatCols in size.

		real64 fBlackLevel [kMaxBlackPattern] [kMaxBlackPattern] [kMaxSamplesPerPixel];

		/// Memory block of double-precision floating point deltas between baseline black level and a given column's black level

		AutoPtr<dng_memory_block> fBlackDeltaH;

		/// Memory block of double-precision floating point deltas between baseline black level and a given row's black level

		AutoPtr<dng_memory_block> fBlackDeltaV;

		/// Single white level (maximum sensor value) for each sample plane.

		real64 fWhiteLevel [kMaxSamplesPerPixel];

	protected:

		int32 fBlackDenom;

	public:

		dng_linearization_info ();

		virtual ~dng_linearization_info ();

		void RoundBlacks ();

		virtual void Parse (dng_host &host,
						    dng_stream &stream,
						    dng_info &info);

		virtual void PostParse (dng_host &host,
								dng_negative &negative);

		/// Compute the maximum black level for a given sample plane taking into account base black level, repeated black level patter, and row/column delta maps.

		real64 MaxBlackLevel (uint32 plane) const;

		/// Convert raw data from in-file format to a true linear image using linearization data from DNG.
		/// \param host Used to allocate buffers, check for aborts, and post progress updates.
		/// \param srcImage Input pre-linearization RAW samples.
		/// \param dstImage Output linearized image.

		virtual void Linearize (dng_host &host,
								const dng_image &srcImage,
								dng_image &dstImage);

		/// Compute black level for one coordinate and sample plane in the image.
		/// \param row Row to compute black level for.
		/// \param col Column to compute black level for.
		/// \param plane Sample plane to compute black level for.

		dng_urational BlackLevel (uint32 row,
								  uint32 col,
								  uint32 plane) const;

		/// Number of per-row black level deltas in fBlackDeltaV.

		uint32 RowBlackCount () const;

		/// Lookup black level delta for a given row.
		/// \param row Row to get black level for.
		/// \retval black level for indicated row.

		dng_srational RowBlack (uint32 row) const;

		/// Number of per-column black level deltas in fBlackDeltaV.

		uint32 ColumnBlackCount () const;

		/// Lookup black level delta for a given column.
		/// \param col Column to get black level for.
		/// \retval black level for indicated column.

		dng_srational ColumnBlack (uint32 col) const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
