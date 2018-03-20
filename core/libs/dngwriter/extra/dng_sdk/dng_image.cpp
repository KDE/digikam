/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_image.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_image.h"

#include "dng_assertions.h"
#include "dng_exceptions.h"
#include "dng_orientation.h"
#include "dng_pixel_buffer.h"
#include "dng_tag_types.h"
#include "dng_tile_iterator.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_tile_buffer::dng_tile_buffer (const dng_image &image,
						 		  const dng_rect &tile,
						 		  bool dirty)

	:	fImage   (image)
	,	fRefData (NULL)

	{

	fImage.AcquireTileBuffer (*this,
							  tile,
							  dirty);

	}

/*****************************************************************************/

dng_tile_buffer::~dng_tile_buffer ()
	{

	fImage.ReleaseTileBuffer (*this);

	}

/*****************************************************************************/

dng_const_tile_buffer::dng_const_tile_buffer (const dng_image &image,
						 		  			  const dng_rect &tile)

	: 	dng_tile_buffer (image, tile, false)

	{

	}

/*****************************************************************************/

dng_const_tile_buffer::~dng_const_tile_buffer ()
	{

	}

/*****************************************************************************/

dng_dirty_tile_buffer::dng_dirty_tile_buffer (dng_image &image,
						 		  			  const dng_rect &tile)

	: 	dng_tile_buffer (image, tile, true)

	{

	}

/*****************************************************************************/

dng_dirty_tile_buffer::~dng_dirty_tile_buffer ()
	{

	}

/*****************************************************************************/

dng_image::dng_image (const dng_rect &bounds,
				      uint32 planes,
				      uint32 pixelType)

	: 	fBounds    (bounds)
	,	fPlanes    (planes)
	,	fPixelType (pixelType)

	{

	}

/*****************************************************************************/

dng_image::~dng_image ()
	{

	}

/*****************************************************************************/

dng_image * dng_image::Clone () const
	{

	ThrowProgramError ("Clone is not supported by this dng_image subclass");

	return NULL;

	}

/*****************************************************************************/

void dng_image::SetPixelType (uint32 pixelType)
	{

	if (TagTypeSize (pixelType) != PixelSize ())
		{

		ThrowProgramError ("Cannot change pixel size for existing image");

		}

	fPixelType = pixelType;

	}

/*****************************************************************************/

uint32 dng_image::PixelSize () const
	{

	return TagTypeSize (PixelType ());

	}

/*****************************************************************************/

uint32 dng_image::PixelRange () const
	{

	switch (fPixelType)
		{

		case ttByte:
		case ttSByte:
			{
			return 0x0FF;
			}

		case ttShort:
		case ttSShort:
			{
			return 0x0FFFF;
			}

		case ttLong:
		case ttSLong:
			{
			return 0xFFFFFFFF;
			}

		default:
			break;

		}

	return 0;

	}

/*****************************************************************************/

dng_rect dng_image::RepeatingTile () const
	{

	return fBounds;

	}

/*****************************************************************************/

void dng_image::AcquireTileBuffer (dng_tile_buffer & /* buffer */,
								   const dng_rect & /* area */,
								   bool /* dirty */) const
	{

	ThrowProgramError ();

	}

/*****************************************************************************/

void dng_image::ReleaseTileBuffer (dng_tile_buffer & /* buffer */) const
	{

	}

/*****************************************************************************/

void dng_image::DoGet (dng_pixel_buffer &buffer) const
	{

	dng_rect tile;

	dng_tile_iterator iter (*this, buffer.fArea);

	while (iter.GetOneTile (tile))
		{

		dng_const_tile_buffer tileBuffer (*this, tile);

		buffer.CopyArea (tileBuffer,
				  		 tile,
				  		 buffer.fPlane,
				  		 buffer.fPlanes);

		}

	}

/*****************************************************************************/

void dng_image::DoPut (const dng_pixel_buffer &buffer)
	{

	dng_rect tile;

	dng_tile_iterator iter (*this, buffer.fArea);

	while (iter.GetOneTile (tile))
		{

		dng_dirty_tile_buffer tileBuffer (*this, tile);

		tileBuffer.CopyArea (buffer,
				  		     tile,
				  		     buffer.fPlane,
				  		     buffer.fPlanes);

		}

	}

/*****************************************************************************/

void dng_image::GetRepeat (dng_pixel_buffer &buffer,
				           const dng_rect &srcArea,
				           const dng_rect &dstArea) const
	{

	// If we already have the entire srcArea in the
	// buffer, we can just repeat that.

	if ((srcArea & buffer.fArea) == srcArea)
		{

		buffer.RepeatArea (srcArea,
						   dstArea);

		}

	// Else we first need to get the srcArea into the buffer area.

	else
		{

		// Find repeating pattern size.

		dng_point repeat = srcArea.Size ();

		// Find pattern phase at top-left corner of destination area.

		dng_point phase = dng_pixel_buffer::RepeatPhase (srcArea,
													     dstArea);

		// Find new source area at top-left of dstArea.

		dng_rect newArea = srcArea + (dstArea.TL () -
								      srcArea.TL ());

		// Find quadrant split coordinates.

		int32 splitV = newArea.t + repeat.v - phase.v;
		int32 splitH = newArea.l + repeat.h - phase.h;

		// Top-left quadrant.

		dng_rect dst1 (dng_rect (newArea.t,
					   			 newArea.l,
					   			 splitV,
					   			 splitH) & dstArea);

		if (dst1.NotEmpty ())
			{

			dng_pixel_buffer temp (buffer);

			temp.fArea = dst1 + (srcArea.TL () -
								 dstArea.TL () +
								 dng_point (phase.v, phase.h));

			temp.fData = buffer.DirtyPixel (dst1.t,
									        dst1.l,
									        buffer.fPlane);

			DoGet (temp);

			}

		// Top-right quadrant.

		dng_rect dst2 (dng_rect (newArea.t,
								 splitH,
								 splitV,
								 newArea.r) & dstArea);

		if (dst2.NotEmpty ())
			{

			dng_pixel_buffer temp (buffer);

			temp.fArea = dst2 + (srcArea.TL () -
								 dstArea.TL () +
								 dng_point (phase.v, -phase.h));

			temp.fData = buffer.DirtyPixel (dst2.t,
									        dst2.l,
									        buffer.fPlane);

			DoGet (temp);

			}

		// Bottom-left quadrant.

		dng_rect dst3 (dng_rect (splitV,
								 newArea.l,
								 newArea.b,
								 splitH) & dstArea);

		if (dst3.NotEmpty ())
			{

			dng_pixel_buffer temp (buffer);

			temp.fArea = dst3 + (srcArea.TL () -
								 dstArea.TL () +
								 dng_point (-phase.v, phase.h));

			temp.fData = buffer.DirtyPixel (dst3.t,
									        dst3.l,
									        buffer.fPlane);

			DoGet (temp);

			}

		// Bottom-right quadrant.

		dng_rect dst4 (dng_rect (splitV,
								 splitH,
								 newArea.b,
								 newArea.r) & dstArea);

		if (dst4.NotEmpty ())
			{

			dng_pixel_buffer temp (buffer);

			temp.fArea = dst4 + (srcArea.TL () -
								 dstArea.TL () +
								 dng_point (-phase.v, -phase.h));

			temp.fData = buffer.DirtyPixel (dst4.t,
									        dst4.l,
									        buffer.fPlane);

			DoGet (temp);

			}

		// Replicate this new source area.

		buffer.RepeatArea (newArea,
						   dstArea);

		}

	}

/*****************************************************************************/

void dng_image::GetEdge (dng_pixel_buffer &buffer,
					     edge_option edgeOption,
				         const dng_rect &srcArea,
				         const dng_rect &dstArea) const
	{

	switch (edgeOption)
		{

		case edge_zero:
			{

			buffer.SetZero (dstArea,
							buffer.fPlane,
							buffer.fPlanes);

			break;

			}

		case edge_repeat:
			{

			GetRepeat (buffer,
					   srcArea,
					   dstArea);

			break;

			}

		case edge_repeat_zero_last:
			{

			if (buffer.fPlanes > 1)
				{

				dng_pixel_buffer buffer1 (buffer);

				buffer1.fPlanes--;

				GetEdge (buffer1,
						 edge_repeat,
						 srcArea,
						 dstArea);

				}

			dng_pixel_buffer buffer2 (buffer);

			buffer2.fPlane  = buffer.fPlanes - 1;
			buffer2.fPlanes = 1;

			buffer2.fData = buffer.DirtyPixel (buffer2.fArea.t,
										  	   buffer2.fArea.l,
										  	   buffer2.fPlane);

			GetEdge (buffer2,
					 edge_zero,
					 srcArea,
					 dstArea);

			break;

			}

		default:
			{

			ThrowProgramError ();
			break;
			}

		}

	}

/*****************************************************************************/

void dng_image::Get (dng_pixel_buffer &buffer,
					 edge_option edgeOption,
				     uint32 repeatV,
				     uint32 repeatH) const
	{

	// Find the overlap with the image bounds.

	dng_rect overlap = buffer.fArea & fBounds;

	// Move the overlapping pixels.

	if (overlap.NotEmpty ())
		{

		dng_pixel_buffer temp (buffer);

		temp.fArea = overlap;

		temp.fData = buffer.DirtyPixel (overlap.t,
								   		overlap.l,
								   		buffer.fPlane);

		DoGet (temp);

		}

	// See if we need to pad the edge values.

	if ((edgeOption != edge_none) && (overlap != buffer.fArea))
		{

		dng_rect areaT (buffer.fArea);
		dng_rect areaL (buffer.fArea);
		dng_rect areaB (buffer.fArea);
		dng_rect areaR (buffer.fArea);

		areaT.b = Min_int32 (areaT.b, fBounds.t);
		areaL.r = Min_int32 (areaL.r, fBounds.l);
		areaB.t = Max_int32 (areaB.t, fBounds.b);
		areaR.l = Max_int32 (areaR.l, fBounds.r);

		dng_rect areaH (buffer.fArea);
		dng_rect areaV (buffer.fArea);

		areaH.l = Max_int32 (areaH.l, fBounds.l);
		areaH.r = Min_int32 (areaH.r, fBounds.r);

		areaV.t = Max_int32 (areaV.t, fBounds.t);
		areaV.b = Min_int32 (areaV.b, fBounds.b);

		// Top left.

		dng_rect areaTL = areaT & areaL;

		if (areaTL.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (fBounds.t,
					 		   fBounds.l,
					 		   fBounds.t + repeatV,
					 		   fBounds.l + repeatH),
					 areaTL);

			}

		// Top middle.

		dng_rect areaTM = areaT & areaH;

		if (areaTM.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (fBounds.t,
					 		   areaTM.l,
					 		   fBounds.t + repeatV,
					 		   areaTM.r),
					 areaTM);

			}

		// Top right.

		dng_rect areaTR = areaT & areaR;

		if (areaTR.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (fBounds.t,
					 		   fBounds.r - repeatH,
					 		   fBounds.t + repeatV,
					 		   fBounds.r),
					 areaTR);

			}

		// Left middle.

		dng_rect areaLM = areaL & areaV;

		if (areaLM.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (areaLM.t,
					 		   fBounds.l,
					 		   areaLM.b,
					 		   fBounds.l + repeatH),
					 areaLM);

			}

		// Right middle.

		dng_rect areaRM = areaR & areaV;

		if (areaRM.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (areaRM.t,
					 		   fBounds.r - repeatH,
					 		   areaRM.b,
					 		   fBounds.r),
					 areaRM);

			}

		// Bottom left.

		dng_rect areaBL = areaB & areaL;

		if (areaBL.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (fBounds.b - repeatV,
					 		   fBounds.l,
					 		   fBounds.b,
					 		   fBounds.l + repeatH),
					 areaBL);

			}

		// Bottom middle.

		dng_rect areaBM = areaB & areaH;

		if (areaBM.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (fBounds.b - repeatV,
					 		   areaBM.l,
					 		   fBounds.b,
					 		   areaBM.r),
					 areaBM);

			}

		// Bottom right.

		dng_rect areaBR = areaB & areaR;

		if (areaBR.NotEmpty ())
			{

			GetEdge (buffer,
					 edgeOption,
					 dng_rect (fBounds.b - repeatV,
					 		   fBounds.r - repeatH,
					 		   fBounds.b,
					 		   fBounds.r),
					 areaBR);

			}

		}

	}

/*****************************************************************************/

void dng_image::Put (const dng_pixel_buffer &buffer)
	{

	// Move the overlapping pixels.

	dng_rect overlap = buffer.fArea & fBounds;

	if (overlap.NotEmpty ())
		{

		dng_pixel_buffer temp (buffer);

		temp.fArea = overlap;

		temp.fData = (void *) buffer.ConstPixel (overlap.t,
								   		 		 overlap.l,
								   		 		 buffer.fPlane);

		// Move the overlapping planes.

		if (temp.fPlane < Planes ())
			{

			temp.fPlanes = Min_uint32 (temp.fPlanes,
									   Planes () - temp.fPlane);

			DoPut (temp);

			}

		}

	}

/*****************************************************************************/

void dng_image::Trim (const dng_rect &r)
	{

	if (r != Bounds ())
		{

		ThrowProgramError ("Trim is not support by this dng_image subclass");

		}

	}

/*****************************************************************************/

void dng_image::Rotate (const dng_orientation &orientation)
	{

	if (orientation != dng_orientation::Normal ())
		{

		ThrowProgramError ("Rotate is not support by this dng_image subclass");

		}

	}

/*****************************************************************************/

void dng_image::CopyArea (const dng_image &src,
						  const dng_rect &area,
						  uint32 srcPlane,
						  uint32 dstPlane,
						  uint32 planes)
	{

	if (&src == this)
		return;

	dng_tile_iterator destIter(*this, area);
	dng_rect destTileArea;

	while (destIter.GetOneTile(destTileArea))
		{
		dng_tile_iterator srcIter(src, destTileArea);
		dng_rect srcTileArea;

		while (srcIter.GetOneTile(srcTileArea))
			{

			dng_dirty_tile_buffer destTile(*this, srcTileArea);
			dng_const_tile_buffer srcTile(src, srcTileArea);

			destTile.CopyArea (srcTile, srcTileArea, srcPlane, dstPlane, planes);

			}

		}

	}

/*****************************************************************************/

bool dng_image::EqualArea (const dng_image &src,
						   const dng_rect &area,
						   uint32 plane,
						   uint32 planes) const
	{

	if (&src == this)
		return true;

	dng_tile_iterator destIter (*this, area);

	dng_rect destTileArea;

	while (destIter.GetOneTile (destTileArea))
		{

		dng_tile_iterator srcIter (src, destTileArea);

		dng_rect srcTileArea;

		while (srcIter.GetOneTile (srcTileArea))
			{

			dng_const_tile_buffer destTile (*this, srcTileArea);
			dng_const_tile_buffer srcTile  (src  , srcTileArea);

			if (!destTile.EqualArea (srcTile, srcTileArea, plane, planes))
				{
				return false;
				}

			}

		}

	return true;

	}

/*****************************************************************************/

void dng_image::SetConstant (uint32 value,
							 const dng_rect &area)
	{

	dng_tile_iterator iter (*this, area);

	dng_rect tileArea;

	while (iter.GetOneTile (tileArea))
		{

		dng_dirty_tile_buffer buffer (*this, tileArea);

		buffer.SetConstant (tileArea,
							0,
							fPlanes,
							value);

		}

	}

/*****************************************************************************/
