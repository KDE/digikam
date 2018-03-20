/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_simple_image.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_simple_image.h"

#include "dng_memory.h"
#include "dng_orientation.h"
#include "dng_tag_types.h"

/*****************************************************************************/

dng_simple_image::dng_simple_image (const dng_rect &bounds,
									uint32 planes,
								    uint32 pixelType,
								    dng_memory_allocator &allocator)

	:	dng_image (bounds,
				   planes,
				   pixelType)

	,	fMemory    ()
	,	fBuffer    ()
	,	fAllocator (allocator)

	{

	uint32 pixelSize = TagTypeSize (pixelType);

	uint32 bytes = bounds.H () * bounds.W () * planes * pixelSize;

	fMemory.Reset (allocator.Allocate (bytes));

	fBuffer.fArea = bounds;

	fBuffer.fPlane  = 0;
	fBuffer.fPlanes = planes;

	fBuffer.fRowStep   = planes * bounds.W ();
	fBuffer.fColStep   = planes;
	fBuffer.fPlaneStep = 1;

	fBuffer.fPixelType = pixelType;
	fBuffer.fPixelSize = pixelSize;

	fBuffer.fData = fMemory->Buffer ();

	}

/*****************************************************************************/

dng_simple_image::~dng_simple_image ()
	{

	}

/*****************************************************************************/

dng_image * dng_simple_image::Clone () const
	{

	AutoPtr<dng_simple_image> result (new dng_simple_image (Bounds (),
															Planes (),
															PixelType (),
															fAllocator));

	result->fBuffer.CopyArea (fBuffer,
							  Bounds (),
							  0,
							  Planes ());

	return result.Release ();

	}

/*****************************************************************************/

void dng_simple_image::SetPixelType (uint32 pixelType)
	{

	dng_image::SetPixelType (pixelType);

	fBuffer.fPixelType = pixelType;

	}

/*****************************************************************************/

void dng_simple_image::Trim (const dng_rect &r)
	{

	fBounds.t = 0;
	fBounds.l = 0;

	fBounds.b = r.H ();
	fBounds.r = r.W ();

	fBuffer.fData = fBuffer.DirtyPixel (r.t, r.l);

	fBuffer.fArea = fBounds;

	}

/*****************************************************************************/

void dng_simple_image::Rotate (const dng_orientation &orientation)
	{

	int32 originH = fBounds.l;
	int32 originV = fBounds.t;

	int32 colStep = fBuffer.fColStep;
	int32 rowStep = fBuffer.fRowStep;

	uint32 width  = fBounds.W ();
	uint32 height = fBounds.H ();

	if (orientation.FlipH ())
		{

		originH += width - 1;

		colStep = -colStep;

		}

	if (orientation.FlipV ())
		{

		originV += height - 1;

		rowStep = -rowStep;

		}

	if (orientation.FlipD ())
		{

		int32 temp = colStep;

		colStep = rowStep;
		rowStep = temp;

		width  = fBounds.H ();
		height = fBounds.W ();

		}

	fBuffer.fData = fBuffer.DirtyPixel (originV, originH);

	fBuffer.fColStep = colStep;
	fBuffer.fRowStep = rowStep;

	fBounds.r = fBounds.l + width;
	fBounds.b = fBounds.t + height;

	fBuffer.fArea = fBounds;

	}

/*****************************************************************************/

void dng_simple_image::AcquireTileBuffer (dng_tile_buffer &buffer,
										  const dng_rect &area,
										  bool dirty) const
	{

	buffer.fArea = area;

	buffer.fPlane      = fBuffer.fPlane;
	buffer.fPlanes     = fBuffer.fPlanes;
	buffer.fRowStep    = fBuffer.fRowStep;
	buffer.fColStep    = fBuffer.fColStep;
	buffer.fPlaneStep  = fBuffer.fPlaneStep;
	buffer.fPixelType  = fBuffer.fPixelType;
	buffer.fPixelSize  = fBuffer.fPixelSize;

	buffer.fData = (void *) fBuffer.ConstPixel (buffer.fArea.t,
								  				buffer.fArea.l,
								  				buffer.fPlane);

	buffer.fDirty = dirty;

	}

/*****************************************************************************/
