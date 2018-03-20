/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_linearization_info.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_linearization_info.h"

#include "dng_area_task.h"
#include "dng_exceptions.h"
#include "dng_host.h"
#include "dng_image.h"
#include "dng_info.h"
#include "dng_negative.h"
#include "dng_pixel_buffer.h"
#include "dng_tag_types.h"
#include "dng_tile_iterator.h"
#include "dng_utils.h"

/*****************************************************************************/

class dng_linearize_plane
	{

	private:

		const dng_image & fSrcImage;
		      dng_image & fDstImage;

		uint32 fPlane;

		dng_rect fActiveArea;

		uint32 fSrcPixelType;
		uint32 fDstPixelType;

		bool fReal32;

		real32 fScale;

		AutoPtr<dng_memory_block> fScale_buffer;

		uint32 fBlack_2D_rows;
		uint32 fBlack_2D_cols;

		AutoPtr<dng_memory_block> fBlack_2D_buffer;

		uint32 fBlack_1D_rows;

		AutoPtr<dng_memory_block> fBlack_1D_buffer;

	public:

		dng_linearize_plane (dng_host &host,
							 dng_linearization_info &info,
							 const dng_image &srcImage,
							 dng_image &dstImage,
							 uint32 plane);

		~dng_linearize_plane ();

		void Process (const dng_rect &tile);

	};

/*****************************************************************************/

dng_linearize_plane::dng_linearize_plane (dng_host &host,
										  dng_linearization_info &info,
										  const dng_image &srcImage,
										  dng_image &dstImage,
										  uint32 plane)

	:	fSrcImage (srcImage)
	,	fDstImage (dstImage)
	,	fPlane (plane)
	,	fActiveArea (info.fActiveArea)
	,	fSrcPixelType (srcImage.PixelType ())
	,	fDstPixelType (dstImage.PixelType ())
	,	fReal32 (false)
	,	fScale (0.0f)
	,	fScale_buffer ()
	,	fBlack_2D_rows (0)
	,	fBlack_2D_cols (0)
	,	fBlack_2D_buffer ()
	,	fBlack_1D_rows (0)
	,	fBlack_1D_buffer ()

	{

	uint32 j;
	uint32 k;

	// Make sure the source pixel type is supported.

	if (fSrcPixelType != ttByte  &&
		fSrcPixelType != ttShort &&
		fSrcPixelType != ttLong)
		{

		DNG_REPORT ("Unsupported source pixel type");

		ThrowProgramError ();

		}

	if (fDstPixelType != ttShort &&
		fDstPixelType != ttFloat)
		{

		DNG_REPORT ("Unsupported destination pixel type");

		ThrowProgramError ();

		}

	// Are we using floating point math?

	fReal32 = (fSrcPixelType == ttLong ||
			   fDstPixelType == ttFloat);

	// Find the scale for this plane.

	real64 maxBlack = info.MaxBlackLevel (plane);

	real64 minRange = info.fWhiteLevel [plane] - maxBlack;

	if (minRange <= 0.0)
		{
		ThrowBadFormat ();
		}

	real64 scale = 1.0 / minRange;

	fScale = (real32) scale;

	// Calculate two-dimensional black pattern, if any.

	if (info.fBlackDeltaH.Get ())
		{

		fBlack_2D_rows = info.fBlackLevelRepeatRows;
		fBlack_2D_cols = info.fActiveArea.W ();

		}

	else if (info.fBlackLevelRepeatCols > 1)
		{

		fBlack_2D_rows = info.fBlackLevelRepeatRows;
		fBlack_2D_cols = info.fBlackLevelRepeatCols;

		}

	if (fBlack_2D_rows)
		{

		fBlack_2D_buffer.Reset (host.Allocate (fBlack_2D_rows * fBlack_2D_cols * 4));

		for (j = 0; j < fBlack_2D_rows; j++)
			{

			for (k = 0;  k < fBlack_2D_cols; k++)
				{

				real64 x = info.fBlackLevel [j]
											[k % info.fBlackLevelRepeatCols]
											[plane];

				if (info.fBlackDeltaH.Get ())
					{

					x += info.fBlackDeltaH->Buffer_real64 () [k];

					}

				x *= scale;

				uint32 index = j * fBlack_2D_cols + k;

				if (fReal32)
					{

					fBlack_2D_buffer->Buffer_real32 () [index] = (real32) x;

					}

				else
					{

					x *= 0x0FFFF * 256.0;

					int32 y = Round_int32 (x);

					fBlack_2D_buffer->Buffer_int32 () [index] = y;

					}

				}

			}

		}

	// Calculate one-dimensional (per row) black pattern, if any.

	if (info.fBlackDeltaV.Get ())
		{

		fBlack_1D_rows = info.fActiveArea.H ();

		}

	else if (fBlack_2D_rows == 0 &&
			 (info.fBlackLevelRepeatRows > 1 || fSrcPixelType != ttShort))
		{

		fBlack_1D_rows = info.fBlackLevelRepeatRows;

		}

	if (fBlack_1D_rows)
		{

		fBlack_1D_buffer.Reset (host.Allocate (fBlack_1D_rows * 4));

		for (j = 0; j < fBlack_1D_rows; j++)
			{

			real64 x = 0.0;

			if (fBlack_2D_rows == 0)
				{

				x = info.fBlackLevel [j % info.fBlackLevelRepeatRows]
									 [0]
									 [plane];

				}

			if (info.fBlackDeltaV.Get ())
				{

				x += info.fBlackDeltaV->Buffer_real64 () [j];

				}

			x *= scale;

			if (fReal32)
				{

				fBlack_1D_buffer->Buffer_real32 () [j] = (real32) x;

				}

			else
				{

				x *= 0x0FFFF * 256.0;

				int32 y = Round_int32 (x);

				fBlack_1D_buffer->Buffer_int32 () [j] = y;

				}

			}

		}

	// Calculate scale table, if any.

	if (fSrcPixelType != ttLong)
		{

		// Find linearization table, if any.

		uint16 *lut = NULL;

		uint32 lutEntries = 0;

		if (info.fLinearizationTable.Get ())
			{

			lut = info.fLinearizationTable->Buffer_uint16 ();

			lutEntries = info.fLinearizationTable->LogicalSize () >> 1;

			}

		// If the black level does not vary from pixel to pixel, then
		// the entire process can be a single LUT.

		if (fBlack_1D_rows == 0 &&
		    fBlack_2D_rows == 0)
			{

			fScale_buffer.Reset (host.Allocate (0x10000 *
											    TagTypeSize (fDstPixelType)));

			for (j = 0; j < 0x10000; j++)
				{

				uint32 x = j;

				// Apply linearization table, if any.

				if (lut)
					{

					x = Min_uint32 (x, lutEntries - 1);

					x = lut [x];

					}

				// Subtract constant black level.

				real64 y = x - info.fBlackLevel [0] [0] [plane];

				// Apply scale.

				y *= scale;

				// We can burn in the clipping also.

				y = Pin_real64 (0.0, y, 1.0);

				// Store output value in table.

				if (fDstPixelType == ttShort)
					{

					uint16 z = (uint16) Round_uint32 (y * 0x0FFFF);

					fScale_buffer->Buffer_uint16 () [j] = z;

					}

				else
					{

					fScale_buffer->Buffer_real32 () [j] = (real32) y;

					}

				}

			}

		// Else we only do the scaling operation in the scale table.

		else
			{

			fScale_buffer.Reset (host.Allocate (0x10000 * 4));

			for (j = 0; j < 0x10000; j++)
				{

				uint32 x = j;

				// Apply linearization table, if any.

				if (lut)
					{

					x = Min_uint32 (x, lutEntries - 1);

					x = lut [x];

					}

				// Apply scale.

				real64 y = x * scale;

				// Store output value in table.

				if (fReal32)
					{

					fScale_buffer->Buffer_real32 () [j] = (real32) y;

					}

				else
					{

					int32 z = Round_int32 (y * 0x0FFFF * 256.0);

					fScale_buffer->Buffer_int32 () [j] = z;

					}

				}

			}

		}

	}

/*****************************************************************************/

dng_linearize_plane::~dng_linearize_plane ()
	{

	}

/*****************************************************************************/

void dng_linearize_plane::Process (const dng_rect &srcTile)
	{

	// Process tile.

	dng_rect dstTile = srcTile - fActiveArea.TL ();

	dng_const_tile_buffer srcBuffer (fSrcImage, srcTile);
	dng_dirty_tile_buffer dstBuffer (fDstImage, dstTile);

	int32 sStep = srcBuffer.fColStep;
	int32 dStep = dstBuffer.fColStep;

	uint32 count = srcTile.W ();

	uint32 dstCol = dstTile.l;

	uint32 rows = srcTile.H ();

	for (uint32 row = 0; row < rows; row++)
		{

		uint32 dstRow = dstTile.t + row;

		const void *sPtr = srcBuffer.ConstPixel (srcTile.t + row,
											     srcTile.l,
											     fPlane);

		void *dPtr = dstBuffer.DirtyPixel (dstRow,
										   dstCol,
										   fPlane);

		// Simple LUT case.

		if (fBlack_1D_rows == 0 &&
			fBlack_2D_rows == 0 && fSrcPixelType != ttLong)
			{

			if (fDstPixelType == ttShort)
				{

				const uint16 *lut = fScale_buffer->Buffer_uint16 ();

				uint16 *dstPtr = (uint16 *) dPtr;

				if (fSrcPixelType == ttByte)
					{

					const uint8 *srcPtr = (const uint8 *) sPtr;

					for (uint32 j = 0; j < count; j++)
						{

						*dstPtr = lut [*srcPtr];

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				else
					{

					const uint16 *srcPtr = (const uint16 *) sPtr;

					for (uint32 j = 0; j < count; j++)
						{

						*dstPtr = lut [*srcPtr];

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				}

			else
				{

				const real32 *lut = fScale_buffer->Buffer_real32 ();

				real32 *dstPtr = (real32 *) dPtr;

				if (fSrcPixelType == ttByte)
					{

					const uint8 *srcPtr = (const uint8 *) sPtr;

					for (uint32 j = 0; j < count; j++)
						{

						*dstPtr = lut [*srcPtr];

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				else
					{

					const uint16 *srcPtr = (const uint16 *) sPtr;

					for (uint32 j = 0; j < count; j++)
						{

						*dstPtr = lut [*srcPtr];

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				}

			}

		// Integer math case.

		else if (!fReal32)
			{

			const int32 *lut = fScale_buffer->Buffer_int32 ();

			int32 b1 = 0;

			if (fBlack_1D_rows)
				{
				b1 = fBlack_1D_buffer->Buffer_int32 () [dstRow % fBlack_1D_rows];
				}

			const int32 *b2 = NULL;

			uint32 b2_count = fBlack_2D_cols;
			uint32 b2_phase = 0;

			if (b2_count)
				{

				b2 = fBlack_2D_buffer->Buffer_int32 () +
					 b2_count * (dstRow % fBlack_2D_rows);

				b2_phase = dstCol % b2_count;

				}

			uint16 *dstPtr = (uint16 *) dPtr;

			b1 -= 128;		// Rounding for 8 bit shift

			if (fSrcPixelType == ttByte)
				{

				const uint8 *srcPtr = (const uint8 *) sPtr;

				for (uint32 j = 0; j < count; j++)
					{

					int32 x = lut [*srcPtr] - b1;

					if (b2_count)
						{

						x -= b2 [b2_phase];

						if (++b2_phase == b2_count)
							{
							b2_phase = 0;
							}

						}

					x >>= 8;

					*dstPtr = Pin_uint16 (x);

					srcPtr += sStep;
					dstPtr += dStep;

					}

				}

			else
				{

				const uint16 *srcPtr = (const uint16 *) sPtr;

				for (uint32 j = 0; j < count; j++)
					{

					int32 x = lut [*srcPtr] - b1;

					if (b2_count)
						{

						x -= b2 [b2_phase];

						if (++b2_phase == b2_count)
							{
							b2_phase = 0;
							}

						}

					x >>= 8;

					*dstPtr = Pin_uint16 (x);

					srcPtr += sStep;
					dstPtr += dStep;

					}

				}

			}

		// Floating point math cases.

		else
			{

			real32 b1 = 0.0f;

			if (fBlack_1D_rows)
				{
				b1 = fBlack_1D_buffer->Buffer_real32 () [dstRow % fBlack_1D_rows];
				}

			const real32 *b2 = NULL;

			uint32 b2_count = fBlack_2D_cols;
			uint32 b2_phase = 0;

			if (b2_count)
				{

				b2 = fBlack_2D_buffer->Buffer_real32 () +
					 b2_count * (dstRow % fBlack_2D_rows);

				b2_phase = dstCol % b2_count;

				}

			// Case 1: uint8/uint16 -> real32

			if (fSrcPixelType != ttLong)
				{

				const real32 *lut = fScale_buffer->Buffer_real32 ();

				real32 *dstPtr = (real32 *) dPtr;

				if (fSrcPixelType == ttByte)
					{

					const uint8 *srcPtr = (const uint8 *) sPtr;

					for (uint32 j = 0; j < count; j++)
						{

						real32 x = lut [*srcPtr] - b1;

						if (b2_count)
							{

							x -= b2 [b2_phase];

							if (++b2_phase == b2_count)
								{
								b2_phase = 0;
								}

							}

						x = Pin_real32 (0.0f, x, 1.0f);

						*dstPtr = x;

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				else
					{

					const uint16 *srcPtr = (const uint16 *) sPtr;

					for (uint32 j = 0; j < count; j++)
						{

						real32 x = lut [*srcPtr] - b1;

						if (b2_count)
							{

							x -= b2 [b2_phase];

							if (++b2_phase == b2_count)
								{
								b2_phase = 0;
								}

							}

						x = Pin_real32 (0.0f, x, 1.0f);

						*dstPtr = x;

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				}

			// Otherwise source is uint32

			else
				{

				real32 scale = fScale;

				const uint32 *srcPtr = (const uint32 *) sPtr;

				// Case 2: uint32 -> real32

				if (fDstPixelType == ttFloat)
					{

					real32 *dstPtr = (real32 *) dPtr;

					for (uint32 j = 0; j < count; j++)
						{

						real32 x = ((real32) *srcPtr) * scale - b1;

						if (b2_count)
							{

							x -= b2 [b2_phase];

							if (++b2_phase == b2_count)
								{
								b2_phase = 0;
								}

							}

						x = Pin_real32 (0.0f, x, 1.0f);

						*dstPtr = x;

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				// Case 3: uint32 -> uint16

				else
					{

					uint16 *dstPtr = (uint16 *) dPtr;

					real32 dstScale = (real32) 0x0FFFF;

					for (uint32 j = 0; j < count; j++)
						{

						real32 x = ((real32) *srcPtr) * scale - b1;

						if (b2_count)
							{

							x -= b2 [b2_phase];

							if (++b2_phase == b2_count)
								{
								b2_phase = 0;
								}

							}

						x = Pin_real32 (0.0f, x, 1.0f);

						*dstPtr = (uint16) (x * dstScale + 0.5f);

						srcPtr += sStep;
						dstPtr += dStep;

						}

					}

				}

			}

		}

	}

/*****************************************************************************/

class dng_linearize_image: public dng_area_task
	{

	private:

		const dng_image & fSrcImage;
		      dng_image & fDstImage;

		dng_rect fActiveArea;

		AutoPtr<dng_linearize_plane> fPlaneTask [kMaxColorPlanes];

	public:

		dng_linearize_image (dng_host &host,
							 dng_linearization_info &info,
							 const dng_image &srcImage,
							 dng_image &dstImage);

		virtual ~dng_linearize_image ();

		virtual dng_rect RepeatingTile1 () const;

		virtual dng_rect RepeatingTile2 () const;

		virtual void Process (uint32 threadIndex,
							  const dng_rect &tile,
							  dng_abort_sniffer *sniffer);

	};

/*****************************************************************************/

dng_linearize_image::dng_linearize_image (dng_host &host,
										  dng_linearization_info &info,
										  const dng_image &srcImage,
										  dng_image &dstImage)

	:	fSrcImage   (srcImage)
	,	fDstImage   (dstImage)
	,	fActiveArea (info.fActiveArea)

	{

	// Build linearization table for each plane.

	for (uint32 plane = 0; plane < srcImage.Planes (); plane++)
		{

		fPlaneTask [plane].Reset (new dng_linearize_plane (host,
														   info,
														   srcImage,
														   dstImage,
														   plane));

		}

	// Adjust maximum tile size.

	fMaxTileSize = dng_point (1024, 1024);

	}

/*****************************************************************************/

dng_linearize_image::~dng_linearize_image ()
	{

	}

/*****************************************************************************/

dng_rect dng_linearize_image::RepeatingTile1 () const
	{

	return fSrcImage.RepeatingTile ();

	}

/*****************************************************************************/

dng_rect dng_linearize_image::RepeatingTile2 () const
	{

	return fDstImage.RepeatingTile () + fActiveArea.TL ();

	}

/*****************************************************************************/

void dng_linearize_image::Process (uint32 /* threadIndex */,
							  	   const dng_rect &srcTile,
							  	   dng_abort_sniffer * /* sniffer */)
	{

	// Process each plane.

	for (uint32 plane = 0; plane < fSrcImage.Planes (); plane++)
		{

		fPlaneTask [plane]->Process (srcTile);

		}

	}

/*****************************************************************************/

dng_linearization_info::dng_linearization_info ()

	:	fActiveArea ()
	,	fMaskedAreaCount (0)
	,	fLinearizationTable ()
	,	fBlackLevelRepeatRows (1)
	,	fBlackLevelRepeatCols (1)
	,	fBlackDeltaH ()
	,	fBlackDeltaV ()
	,	fBlackDenom (256)

	{

	uint32 j;
	uint32 k;
	uint32 n;

	for (j = 0; j < kMaxBlackPattern; j++)
		for (k = 0; k < kMaxBlackPattern; k++)
			for (n = 0; n < kMaxSamplesPerPixel; n++)
				{
				fBlackLevel [j] [k] [n] = 0.0;
				}

	for (n = 0; n < kMaxSamplesPerPixel; n++)
		{
		fWhiteLevel [n] = 65535.0;
		}

	}

/*****************************************************************************/

dng_linearization_info::~dng_linearization_info ()
	{

	}

/*****************************************************************************/

void dng_linearization_info::RoundBlacks ()
	{

	uint32 j;
	uint32 k;
	uint32 n;

	real64 maxAbs = 0.0;

	for (j = 0; j < fBlackLevelRepeatRows; j++)
		for (k = 0; k < fBlackLevelRepeatCols; k++)
			for (n = 0; n < kMaxSamplesPerPixel; n++)
				{

				maxAbs = Max_real64 (maxAbs,
									 Abs_real64 (fBlackLevel [j] [k] [n]));

				}

	uint32 count = RowBlackCount ();

	for (j = 0; j < count; j++)
		{

		maxAbs = Max_real64 (maxAbs,
							 Abs_real64 (fBlackDeltaV->Buffer_real64 () [j]));

		}

	count = ColumnBlackCount ();

	for (j = 0; j < count; j++)
		{

		maxAbs = Max_real64 (maxAbs,
							 Abs_real64 (fBlackDeltaH->Buffer_real64 () [j]));


		}

	fBlackDenom = 256;

	while (fBlackDenom > 1 && (maxAbs * fBlackDenom) >= 30000.0 * 65536.0)
		{
		fBlackDenom >>= 1;
		}

	for (j = 0; j < fBlackLevelRepeatRows; j++)
		for (k = 0; k < fBlackLevelRepeatCols; k++)
			for (n = 0; n < kMaxSamplesPerPixel; n++)
				{

				fBlackLevel [j] [k] [n] = BlackLevel (j, k, n).As_real64 ();

				}

	count = RowBlackCount ();

	for (j = 0; j < count; j++)
		{

		fBlackDeltaV->Buffer_real64 () [j] = RowBlack (j).As_real64 ();

		}

	count = ColumnBlackCount ();

	for (j = 0; j < count; j++)
		{

		fBlackDeltaH->Buffer_real64 () [j] = ColumnBlack (j).As_real64 ();

		}

	}

/*****************************************************************************/

void dng_linearization_info::Parse (dng_host &host,
								    dng_stream &stream,
								    dng_info &info)
	{

	uint32 j;
	uint32 k;
	uint32 n;

	// Find main image IFD.

	dng_ifd &rawIFD = *info.fIFD [info.fMainIndex].Get ();

	// Copy active area.

	fActiveArea = rawIFD.fActiveArea;

	// Copy masked areas.

	fMaskedAreaCount = rawIFD.fMaskedAreaCount;

	for (j = 0; j < fMaskedAreaCount; j++)
		{
		fMaskedArea [j] = rawIFD.fMaskedArea [j];
		}

	// Read linearization LUT.

	if (rawIFD.fLinearizationTableCount)
		{

		uint32 size = rawIFD.fLinearizationTableCount * sizeof (uint16);

		fLinearizationTable.Reset (host.Allocate (size));

		uint16 *table = fLinearizationTable->Buffer_uint16 ();

		stream.SetReadPosition (rawIFD.fLinearizationTableOffset);

		for (j = 0; j < rawIFD.fLinearizationTableCount; j++)
			{
			table [j] = stream.Get_uint16 ();
			}

		}

	// Copy black level pattern.

	fBlackLevelRepeatRows = rawIFD.fBlackLevelRepeatRows;
	fBlackLevelRepeatCols = rawIFD.fBlackLevelRepeatCols;

	for (j = 0; j < kMaxBlackPattern; j++)
		for (k = 0; k < kMaxBlackPattern; k++)
			for (n = 0; n < kMaxSamplesPerPixel; n++)
				{
				fBlackLevel [j] [k] [n] = rawIFD.fBlackLevel [j] [k] [n];
				}

	// Read BlackDeltaH.

	if (rawIFD.fBlackLevelDeltaHCount)
		{

		uint32 size = rawIFD.fBlackLevelDeltaHCount * sizeof (real64);

		fBlackDeltaH.Reset (host.Allocate (size));

		real64 *blacks = fBlackDeltaH->Buffer_real64 ();

		stream.SetReadPosition (rawIFD.fBlackLevelDeltaHOffset);

		for (j = 0; j < rawIFD.fBlackLevelDeltaHCount; j++)
			{
			blacks [j] = stream.TagValue_real64 (rawIFD.fBlackLevelDeltaHType);
			}

		}

	// Read BlackDeltaV.

	if (rawIFD.fBlackLevelDeltaVCount)
		{

		uint32 size = rawIFD.fBlackLevelDeltaVCount * sizeof (real64);

		fBlackDeltaV.Reset (host.Allocate (size));

		real64 *blacks = fBlackDeltaV->Buffer_real64 ();

		stream.SetReadPosition (rawIFD.fBlackLevelDeltaVOffset);

		for (j = 0; j < rawIFD.fBlackLevelDeltaVCount; j++)
			{
			blacks [j] = stream.TagValue_real64 (rawIFD.fBlackLevelDeltaVType);
			}

		}

	// Copy white level.

	for (n = 0; n < kMaxSamplesPerPixel; n++)
		{
		fWhiteLevel [n] = rawIFD.fWhiteLevel [n];
		}

	// Round off black levels.

	RoundBlacks ();

	}

/*****************************************************************************/

void dng_linearization_info::PostParse (dng_host & /* host */,
										dng_negative &negative)
	{

	if (fActiveArea.IsEmpty ())
		{

		fActiveArea = negative.Stage1Image ()->Bounds ();

		}

	}

/*****************************************************************************/

real64 dng_linearization_info::MaxBlackLevel (uint32 plane) const
	{

	uint32 j;
	uint32 k;

	// Find maximum value of fBlackDeltaH for each phase of black pattern.

	real64 maxDeltaH [kMaxBlackPattern];

	for (j = 0; j < fBlackLevelRepeatCols; j++)
		{
		maxDeltaH [j] = 0.0;
		}

	if (fBlackDeltaH.Get ())
		{

		real64 *table = fBlackDeltaH->Buffer_real64 ();

		uint32 entries = fBlackDeltaH->LogicalSize () / sizeof (table [0]);

		for (j = 0; j < entries; j++)
			{

			real64 &entry = maxDeltaH [j % fBlackLevelRepeatCols];

			if (j < fBlackLevelRepeatCols)
				{
				entry = table [j];
				}
			else
				{
				entry = Max_real64 (entry, table [j]);
				}

			}

		}

	// Find maximum value of fBlackDeltaV for each phase of black pattern.

	real64 maxDeltaV [kMaxBlackPattern];

	for (j = 0; j < fBlackLevelRepeatRows; j++)
		{
		maxDeltaV [j] = 0.0;
		}

	if (fBlackDeltaV.Get ())
		{

		real64 *table = fBlackDeltaV->Buffer_real64 ();

		uint32 entries = fBlackDeltaV->LogicalSize () / sizeof (table [0]);

		for (j = 0; j < entries; j++)
			{

			real64 &entry = maxDeltaV [j % fBlackLevelRepeatRows];

			if (j < fBlackLevelRepeatRows)
				{
				entry = table [j];
				}
			else
				{
				entry = Max_real64 (entry, table [j]);
				}

			}

		}

	// Now scan the pattern and find the maximum value after row and column
	// deltas.

	real64 maxBlack = 0.0;

	for (j = 0; j < fBlackLevelRepeatRows; j++)
		{

		for (k = 0; k < fBlackLevelRepeatCols; k++)
			{

			real64 black = fBlackLevel [j] [k] [plane];

			black += maxDeltaH [k];
			black += maxDeltaV [j];

			if (j == 0 && k == 0)
				{
				maxBlack = black;
				}
			else
				{
				maxBlack = Max_real64 (maxBlack, black);
				}

			}

		}

	return maxBlack;

	}

/*****************************************************************************/

void dng_linearization_info::Linearize (dng_host &host,
										const dng_image &srcImage,
										dng_image &dstImage)
	{

	dng_linearize_image processor (host,
								   *this,
								   srcImage,
								   dstImage);

	host.PerformAreaTask (processor,
						  fActiveArea);

	}

/*****************************************************************************/

dng_urational dng_linearization_info::BlackLevel (uint32 row,
												  uint32 col,
												  uint32 plane) const
	{

	dng_urational r;

	r.Set_real64 (fBlackLevel [row] [col] [plane], fBlackDenom);

	return r;

	}

/*****************************************************************************/

uint32 dng_linearization_info::RowBlackCount () const
	{

	if (fBlackDeltaV.Get ())
		{

		return fBlackDeltaV->LogicalSize () >> 3;

		}

	return 0;

	}

/*****************************************************************************/

dng_srational dng_linearization_info::RowBlack (uint32 row) const
	{

	if (fBlackDeltaV.Get ())
		{

		dng_srational r;

		r.Set_real64 (fBlackDeltaV->Buffer_real64 () [row], fBlackDenom);

		return r;

		}

	return dng_srational (0, 1);

	}

/*****************************************************************************/

uint32 dng_linearization_info::ColumnBlackCount () const
	{

	if (fBlackDeltaH.Get ())
		{

		return fBlackDeltaH->LogicalSize () >> 3;

		}

	return 0;

	}

/*****************************************************************************/

dng_srational dng_linearization_info::ColumnBlack (uint32 col) const
	{

	if (fBlackDeltaH.Get ())
		{

		dng_srational r;

		r.Set_real64 (fBlackDeltaH->Buffer_real64 () [col], fBlackDenom);

		return r;

		}

	return dng_srational (0, 1);

	}

/*****************************************************************************/
