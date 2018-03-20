/*****************************************************************************/
// Copyright 2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_bad_pixels.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_bad_pixels.h"

#include "dng_filter_task.h"
#include "dng_globals.h"
#include "dng_host.h"
#include "dng_image.h"
#include "dng_negative.h"

#include <algorithm>

/*****************************************************************************/

dng_opcode_FixBadPixelsConstant::dng_opcode_FixBadPixelsConstant
								 (uint32 constant,
								  uint32 bayerPhase)

	:	dng_filter_opcode (dngOpcode_FixBadPixelsConstant,
						   dngVersion_1_3_0_0,
						   0)

	,	fConstant   (constant)
	,	fBayerPhase (bayerPhase)

	{

	}

/*****************************************************************************/

dng_opcode_FixBadPixelsConstant::dng_opcode_FixBadPixelsConstant
								 (dng_stream &stream)

	:	dng_filter_opcode (dngOpcode_FixBadPixelsConstant,
						   stream,
						   "FixBadPixelsConstant")

	,	fConstant   (0)
	,	fBayerPhase (0)

	{

	if (stream.Get_uint32 () != 8)
		{
		ThrowBadFormat ();
		}

	fConstant   = stream.Get_uint32 ();
	fBayerPhase = stream.Get_uint32 ();

	#if qDNGValidate

	if (gVerbose)
		{

		printf ("Constant: %u\n", (unsigned) fConstant);

		printf ("Bayer Phase: %u\n", (unsigned) fBayerPhase);

		}

	#endif

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsConstant::PutData (dng_stream &stream) const
	{

	stream.Put_uint32 (8);

	stream.Put_uint32 (fConstant  );
	stream.Put_uint32 (fBayerPhase);

	}

/*****************************************************************************/

dng_point dng_opcode_FixBadPixelsConstant::SrcRepeat ()
	{

	return dng_point (2, 2);

	}

/*****************************************************************************/

dng_rect dng_opcode_FixBadPixelsConstant::SrcArea (const dng_rect &dstArea,
												   const dng_rect & /* imageBounds */)
	{

	dng_rect srcArea = dstArea;

	srcArea.t -= 2;
	srcArea.l -= 2;

	srcArea.b += 2;
	srcArea.r += 2;

	return srcArea;

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsConstant::Prepare (dng_negative & /* negative */,
											   uint32 /* threadCount */,
											   const dng_point & /* tileSize */,
											   const dng_rect & /* imageBounds */,
											   uint32 imagePlanes,
											   uint32 bufferPixelType,
											   dng_memory_allocator & /* allocator */)
	{

	// This opcode is restricted to single channel images.

	if (imagePlanes != 1)
		{

		ThrowBadFormat ();

		}

	// This opcode is restricted to 16-bit images.

	if (bufferPixelType != ttShort)
		{

		ThrowBadFormat ();

		}

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsConstant::ProcessArea (dng_negative & /* negative */,
												   uint32 /* threadIndex */,
												   dng_pixel_buffer &srcBuffer,
												   dng_pixel_buffer &dstBuffer,
												   const dng_rect &dstArea,
												   const dng_rect & /* imageBounds */)
	{

	dstBuffer.CopyArea (srcBuffer,
						dstArea,
						0,
						dstBuffer.fPlanes);

	uint16 badPixel = (uint16) fConstant;

	for (int32 dstRow = dstArea.t; dstRow < dstArea.b; dstRow++)
		{

		const uint16 *sPtr = srcBuffer.ConstPixel_uint16 (dstRow, dstArea.l, 0);
			  uint16 *dPtr = dstBuffer.DirtyPixel_uint16 (dstRow, dstArea.l, 0);

		for (int32 dstCol = dstArea.l; dstCol < dstArea.r; dstCol++)
			{

			if (*sPtr == badPixel)
				{

				uint32 count = 0;
				uint32 total = 0;

				uint16 value;

				if (IsGreen (dstRow, dstCol))	// Green pixel
					{

					value = sPtr [-srcBuffer.fRowStep - 1];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					value = sPtr [-srcBuffer.fRowStep + 1];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					value = sPtr [srcBuffer.fRowStep - 1];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					value = sPtr [srcBuffer.fRowStep + 1];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					}

				else	// Red/blue pixel.
					{

					value = sPtr [-srcBuffer.fRowStep * 2];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					value = sPtr [srcBuffer.fRowStep * 2];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					value = sPtr [-2];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					value = sPtr [2];

					if (value != badPixel)
						{
						count += 1;
						total += value;
						}

					}

				if (count == 4)		// Most common case.
					{

					*dPtr = (uint16) ((total + 2) >> 2);

					}

				else if (count > 0)
					{

					*dPtr = (uint16) ((total + (count >> 1)) / count);

					}

				}

			sPtr++;
			dPtr++;

			}

		}

	}

/*****************************************************************************/

dng_bad_pixel_list::dng_bad_pixel_list ()

	:	fBadPoints ()
	,	fBadRects  ()

	{

	}

/*****************************************************************************/

void dng_bad_pixel_list::AddPoint (const dng_point &pt)
	{

	fBadPoints.push_back (pt);

	}

/*****************************************************************************/

void dng_bad_pixel_list::AddRect (const dng_rect &r)
	{

	fBadRects.push_back (r);

	}

/*****************************************************************************/

static bool SortBadPoints (const dng_point &a,
						   const dng_point &b)
	{

	if (a.v < b.v)
		return true;

	if (a.v > b.v)
		return false;

	return a.h < b.h;

	}

/*****************************************************************************/

static bool SortBadRects (const dng_rect &a,
						  const dng_rect &b)
	{

	if (a.t < b.t)
		return true;

	if (a.t > b.t)
		return false;

	if (a.l < b.l)
		return true;

	if (a.l > b.l)
		return false;

	if (a.b < b.b)
		return true;

	if (a.b > b.b)
		return false;

	return a.r < b.r;

	}

/*****************************************************************************/

void dng_bad_pixel_list::Sort ()
	{

	if (PointCount () > 1)
		{

		std::sort (fBadPoints.begin (),
				   fBadPoints.end   (),
				   SortBadPoints);

		}

	if (RectCount () > 1)
		{

		std::sort (fBadRects.begin (),
				   fBadRects.end   (),
				   SortBadRects);

		}

	}

/*****************************************************************************/

bool dng_bad_pixel_list::IsPointIsolated (uint32 index,
										  uint32 radius) const
	{

	dng_point pt = Point (index);

	// Search backward through bad point list.

	for (int32 j = index - 1; j >= 0; j--)
		{

		const dng_point &pt2 = Point (j);

		if (pt2.v < pt.v - (int32) radius)
			{
			break;
			}

		if (Abs_int32 (pt2.h - pt.h) <= radius)
			{
			return false;
			}

		}

	// Search forward through bad point list.

	for (uint32 k = index + 1; k < PointCount (); k++)
		{

		const dng_point &pt2 = Point (k);

		if (pt2.v > pt.v + (int32) radius)
			{
			break;
			}

		if (Abs_int32 (pt2.h - pt.h) <= radius)
			{
			return false;
			}

		}

	// Search through bad rectangle list.

	dng_rect testRect (pt.v - radius,
					   pt.h - radius,
					   pt.v + radius + 1,
					   pt.h + radius + 1);

	for (uint32 n = 0; n < RectCount (); n++)
		{

		if ((testRect & Rect (n)).NotEmpty ())
			{
			return false;
			}

		}

	// Did not find point anywhere, so bad pixel is isolated.

	return true;

	}

/*****************************************************************************/

bool dng_bad_pixel_list::IsRectIsolated (uint32 index,
										 uint32 radius) const
	{

	dng_rect testRect = Rect (index);

	testRect.t -= radius;
	testRect.l -= radius;
	testRect.b += radius;
	testRect.r += radius;

	for (uint32 n = 0; n < RectCount (); n++)
		{

		if (n != index)
			{

			if ((testRect & Rect (n)).NotEmpty ())
				{
				return false;
				}

			}

		}

	return true;

	}

/*****************************************************************************/

bool dng_bad_pixel_list::IsPointValid (const dng_point &pt,
									   const dng_rect &imageBounds,
									   uint32 index) const
	{

	// The point must be in the image bounds to be valid.

	if (pt.v <  imageBounds.t ||
		pt.h <  imageBounds.l ||
		pt.v >= imageBounds.b ||
		pt.h >= imageBounds.r)
		{
		return false;
		}

	// Only search the bad point list if we have a starting search index.

	if (index != kNoIndex)
		{

		// Search backward through bad point list.

		for (int32 j = index - 1; j >= 0; j--)
			{

			const dng_point &pt2 = Point (j);

			if (pt2.v < pt.v)
				{
				break;
				}

			if (pt2 == pt)
				{
				return false;
				}

			}

		// Search forward through bad point list.

		for (uint32 k = index + 1; k < PointCount (); k++)
			{

			const dng_point &pt2 = Point (k);

			if (pt2.v > pt.v)
				{
				break;
				}

			if (pt2 == pt)
				{
				return false;
				}

			}

		}

	// Search through bad rectangle list.

	for (uint32 n = 0; n < RectCount (); n++)
		{

		const dng_rect &r = Rect (n);

		if (pt.v >= r.t &&
			pt.h >= r.l &&
			pt.v <  r.b &&
			pt.h <  r.r)
			{
			return false;
			}

		}

	// Did not find point anywhere, so pixel is valid.

	return true;

	}

/*****************************************************************************/

dng_opcode_FixBadPixelsList::dng_opcode_FixBadPixelsList
							 (AutoPtr<dng_bad_pixel_list> &list,
							  uint32 bayerPhase)


	:	dng_filter_opcode (dngOpcode_FixBadPixelsList,
						   dngVersion_1_3_0_0,
						   0)

	,	fList ()

	,	fBayerPhase (bayerPhase)

	{

	fList.Reset (list.Release ());

	fList->Sort ();

	}

/*****************************************************************************/

dng_opcode_FixBadPixelsList::dng_opcode_FixBadPixelsList (dng_stream &stream)

	:	dng_filter_opcode (dngOpcode_FixBadPixelsList,
						   stream,
						   "FixBadPixelsList")

	,	fList ()

	,	fBayerPhase (0)

	{

	uint32 size = stream.Get_uint32 ();

	fBayerPhase = stream.Get_uint32 ();

	uint32 pCount = stream.Get_uint32 ();
	uint32 rCount = stream.Get_uint32 ();

	if (size != 12 + pCount * 8 + rCount * 16)
		{
		ThrowBadFormat ();
		}

	fList.Reset (new dng_bad_pixel_list);

	uint32 index;

	for (index = 0; index < pCount; index++)
		{

		dng_point pt;

		pt.v = stream.Get_int32 ();
		pt.h = stream.Get_int32 ();

		fList->AddPoint (pt);

		}

	for (index = 0; index < rCount; index++)
		{

		dng_rect r;

		r.t = stream.Get_int32 ();
		r.l = stream.Get_int32 ();
		r.b = stream.Get_int32 ();
		r.r = stream.Get_int32 ();

		fList->AddRect (r);

		}

	fList->Sort ();

	#if qDNGValidate

	if (gVerbose)
		{

		printf ("Bayer Phase: %u\n", (unsigned) fBayerPhase);

		printf ("Bad Pixels: %u\n", (unsigned) pCount);

		for (index = 0; index < pCount && index < gDumpLineLimit; index++)
			{
			printf ("    Pixel [%u]: v=%d, h=%d\n",
					(unsigned) index,
					(int) fList->Point (index).v,
					(int) fList->Point (index).h);
			}

		if (pCount > gDumpLineLimit)
			{
			printf ("    ... %u bad pixels skipped\n", pCount - gDumpLineLimit);
			}

		printf ("Bad Rects: %u\n", (unsigned) rCount);

		for (index = 0; index < rCount && index < gDumpLineLimit; index++)
			{
			printf ("    Rect [%u]: t=%d, l=%d, b=%d, r=%d\n",
					(unsigned) index,
					(int) fList->Rect (index).t,
					(int) fList->Rect (index).l,
					(int) fList->Rect (index).b,
					(int) fList->Rect (index).r);
			}

		if (rCount > gDumpLineLimit)
			{
			printf ("    ... %u bad rects skipped\n", rCount - gDumpLineLimit);
			}

		}

	#endif

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::PutData (dng_stream &stream) const
	{

	uint32 pCount = fList->PointCount ();
	uint32 rCount = fList->RectCount  ();

	stream.Put_uint32 (12 + pCount * 8 + rCount * 16);

	stream.Put_uint32 (fBayerPhase);

	stream.Put_uint32 (pCount);
	stream.Put_uint32 (rCount);

	uint32 index;

	for (index = 0; index < pCount; index++)
		{

		const dng_point &pt (fList->Point (index));

		stream.Put_int32 (pt.v);
		stream.Put_int32 (pt.h);

		}

	for (index = 0; index < rCount; index++)
		{

		const dng_rect &r (fList->Rect (index));

		stream.Put_int32 (r.t);
		stream.Put_int32 (r.l);
		stream.Put_int32 (r.b);
		stream.Put_int32 (r.r);

		}

	}

/*****************************************************************************/

dng_rect dng_opcode_FixBadPixelsList::SrcArea (const dng_rect &dstArea,
											   const dng_rect & /* imageBounds */)
	{

	int32 padding = 0;

	if (fList->PointCount ())
		{
		padding += kBadPointPadding;
		}

	if (fList->RectCount ())
		{
		padding += kBadRectPadding;
		}

	dng_rect srcArea = dstArea;

	srcArea.t -= padding;
	srcArea.l -= padding;

	srcArea.b += padding;
	srcArea.r += padding;

	return srcArea;

	}

/*****************************************************************************/

dng_point dng_opcode_FixBadPixelsList::SrcRepeat ()
	{

	return dng_point (2, 2);

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::Prepare (dng_negative & /* negative */,
										   uint32 /* threadCount */,
										   const dng_point & /* tileSize */,
										   const dng_rect & /* imageBounds */,
										   uint32 imagePlanes,
										   uint32 bufferPixelType,
										   dng_memory_allocator & /* allocator */)
	{

	// This opcode is restricted to single channel images.

	if (imagePlanes != 1)
		{

		ThrowBadFormat ();

		}

	// This opcode is restricted to 16-bit images.

	if (bufferPixelType != ttShort)
		{

		ThrowBadFormat ();

		}

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::FixIsolatedPixel (dng_pixel_buffer &buffer,
													dng_point &badPoint)
	{

	uint16 *p0 = buffer.DirtyPixel_uint16 (badPoint.v - 2, badPoint.h - 2, 0);
	uint16 *p1 = buffer.DirtyPixel_uint16 (badPoint.v - 1, badPoint.h - 2, 0);
	uint16 *p2 = buffer.DirtyPixel_uint16 (badPoint.v    , badPoint.h - 2, 0);
	uint16 *p3 = buffer.DirtyPixel_uint16 (badPoint.v + 1, badPoint.h - 2, 0);
	uint16 *p4 = buffer.DirtyPixel_uint16 (badPoint.v + 2, badPoint.h - 2, 0);

	uint32 est0;
	uint32 est1;
	uint32 est2;
	uint32 est3;

	uint32 grad0;
	uint32 grad1;
	uint32 grad2;
	uint32 grad3;

	if (IsGreen (badPoint.v, badPoint.h))		// Green pixel
		{

		// g00 b01 g02 b03 g04
		// r10 g11 r12 g13 r14
		// g20 b21 g22 b23 g24
		// r30 g31 r32 g33 r34
		// g40 b41 g42 b43 g44

		int32 b01 = p0 [1];
		int32 g02 = p0 [2];
		int32 b03 = p0 [3];

		int32 r10 = p1 [0];
		int32 g11 = p1 [1];
		int32 r12 = p1 [2];
		int32 g13 = p1 [3];
		int32 r14 = p1 [4];

		int32 g20 = p2 [0];
		int32 b21 = p2 [1];
		int32 b23 = p2 [3];
		int32 g24 = p2 [4];

		int32 r30 = p3 [0];
		int32 g31 = p3 [1];
		int32 r32 = p3 [2];
		int32 g33 = p3 [3];
		int32 r34 = p3 [4];

		int32 b41 = p4 [1];
		int32 g42 = p4 [2];
		int32 b43 = p4 [3];

		est0 = g02 + g42;

		grad0 = Abs_int32 (g02 - g42) +
				Abs_int32 (g11 - g31) +
				Abs_int32 (g13 - g33) +
				Abs_int32 (b01 - b21) +
				Abs_int32 (b03 - b23) +
				Abs_int32 (b21 - b41) +
				Abs_int32 (b23 - b43);

		est1 = g11 + g33;

		grad1 = Abs_int32 (g11 - g33) +
				Abs_int32 (g02 - g24) +
				Abs_int32 (g20 - g42) +
				Abs_int32 (b01 - b23) +
				Abs_int32 (r10 - r32) +
				Abs_int32 (r12 - r34) +
				Abs_int32 (b21 - b43);

		est2 = g20 + g24;

		grad2 = Abs_int32 (g20 - g24) +
				Abs_int32 (g11 - g13) +
				Abs_int32 (g31 - g33) +
				Abs_int32 (r10 - r12) +
				Abs_int32 (r30 - r32) +
				Abs_int32 (r12 - r14) +
				Abs_int32 (r32 - r34);

		est3 = g13 + g31;

		grad3 = Abs_int32 (g13 - g31) +
				Abs_int32 (g02 - g20) +
				Abs_int32 (g24 - g42) +
				Abs_int32 (b03 - b21) +
				Abs_int32 (r14 - r32) +
				Abs_int32 (r12 - r30) +
				Abs_int32 (b23 - b41);

		}

	else		// Red/blue pixel
		{

		// b00 g01 b02 g03 b04
		// g10 r11 g12 r13 g14
		// b20 g21 b22 g23 b24
		// g30 r31 g32 r33 g34
		// b40 g41 b42 g43 b44

		int32 b00 = p0 [0];
		int32 g01 = p0 [1];
		int32 b02 = p0 [2];
		int32 g03 = p0 [3];
		int32 b04 = p0 [4];

		int32 g10 = p1 [0];
		int32 r11 = p1 [1];
		int32 g12 = p1 [2];
		int32 r13 = p1 [3];
		int32 g14 = p1 [4];

		int32 b20 = p2 [0];
		int32 g21 = p2 [1];
		int32 g23 = p2 [3];
		int32 b24 = p2 [4];

		int32 g30 = p3 [0];
		int32 r31 = p3 [1];
		int32 g32 = p3 [2];
		int32 r33 = p3 [3];
		int32 g34 = p3 [4];

		int32 b40 = p4 [0];
		int32 g41 = p4 [1];
		int32 b42 = p4 [2];
		int32 g43 = p4 [3];
		int32 b44 = p4 [4];

		est0 = b02 + b42;

		grad0 = Abs_int32 (b02 - b42) +
				Abs_int32 (g12 - g32) +
				Abs_int32 (g01 - g21) +
				Abs_int32 (g21 - g41) +
				Abs_int32 (g03 - g23) +
				Abs_int32 (g23 - g43) +
				Abs_int32 (r11 - r31) +
				Abs_int32 (r13 - r33);

		est1 = b00 + b44;

		grad1 = Abs_int32 (b00 - b44) +
				Abs_int32 (r11 - r33) +
				Abs_int32 (g01 - g23) +
				Abs_int32 (g10 - g32) +
				Abs_int32 (g12 - g34) +
				Abs_int32 (g21 - g43) +
				Abs_int32 (b02 - b24) +
				Abs_int32 (b20 - b42);

		est2 = b20 + b24;

		grad2 = Abs_int32 (b20 - b24) +
				Abs_int32 (g21 - g23) +
				Abs_int32 (g10 - g12) +
				Abs_int32 (g12 - g14) +
				Abs_int32 (g30 - g32) +
				Abs_int32 (g32 - g34) +
				Abs_int32 (r11 - r13) +
				Abs_int32 (r31 - r33);

		est3 = b04 + b40;

		grad3 = Abs_int32 (b04 - b40) +
				Abs_int32 (r13 - r31) +
				Abs_int32 (g03 - g21) +
				Abs_int32 (g14 - g32) +
				Abs_int32 (g12 - g30) +
				Abs_int32 (g23 - g41) +
				Abs_int32 (b02 - b20) +
				Abs_int32 (b24 - b42);

		}

	uint32 minGrad = Min_uint32 (grad0, grad1);

	minGrad = Min_uint32 (minGrad, grad2);
	minGrad = Min_uint32 (minGrad, grad3);

	uint32 limit = (minGrad * 3) >> 1;

	uint32 total = 0;
	uint32 count = 0;

	if (grad0 <= limit)
		{
		total += est0;
		count += 2;
		}

	if (grad1 <= limit)
		{
		total += est1;
		count += 2;
		}

	if (grad2 <= limit)
		{
		total += est2;
		count += 2;
		}

	if (grad3 <= limit)
		{
		total += est3;
		count += 2;
		}

	uint32 estimate = (total + (count >> 1)) / count;

	p2 [2] = (uint16) estimate;

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::FixClusteredPixel (dng_pixel_buffer &buffer,
													 uint32 pointIndex,
													 const dng_rect &imageBounds)
	{

	const uint32 kNumSets = 3;
	const uint32 kSetSize = 4;

	static const int32 kOffset [kNumSets] [kSetSize] [2] =
		{
			{
				{ -1,  1 },
				{ -1, -1 },
				{  1, -1 },
				{  1,  1 }
			},
			{
				{ -2,  0 },
				{  2,  0 },
				{  0, -2 },
				{  0,  2 }
			},
			{
				{ -2, -2 },
				{ -2,  2 },
				{  2, -2 },
				{  2,  2 }
			}
		};

	dng_point badPoint = fList->Point (pointIndex);

	bool isGreen = IsGreen (badPoint.v, badPoint.h);

	uint16 *p = buffer.DirtyPixel_uint16 (badPoint.v, badPoint.h, 0);

	for (uint32 set = 0; set < kNumSets; set++)
		{

		if (!isGreen && (kOffset [set] [0] [0] & 1) == 1)
			{
			continue;
			}

		uint32 total = 0;
		uint32 count = 0;

		for (uint32 entry = 0; entry < kSetSize; entry++)
			{

			dng_point offset (kOffset [set] [entry] [0],
							  kOffset [set] [entry] [1]);

			if (fList->IsPointValid (badPoint + offset,
									 imageBounds,
									 pointIndex))
				{

				total += p [offset.v * buffer.fRowStep +
							offset.h * buffer.fColStep];

				count++;

				}

			}

		if (count)
			{

			uint32 estimate = (total + (count >> 1)) / count;

			p [0] = (uint16) estimate;

			return;

			}

		}

	// Unable to patch bad pixel.  Leave pixel as is.

	#if qDNGValidate

	char s [256];

	sprintf (s, "Unable to repair bad pixel, row %d, column %d",
			 (int) badPoint.v,
			 (int) badPoint.h);

	ReportWarning (s);

	#endif

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::FixSingleColumn (dng_pixel_buffer &buffer,
												   const dng_rect &badRect)
	{

	int32 cs = buffer.fColStep;

	for (int32 row = badRect.t; row < badRect.b; row++)
		{

		uint16 *p0 = buffer.DirtyPixel_uint16 (row - 4, badRect.l - 4, 0);
		uint16 *p1 = buffer.DirtyPixel_uint16 (row - 3, badRect.l - 4, 0);
		uint16 *p2 = buffer.DirtyPixel_uint16 (row - 2, badRect.l - 4, 0);
		uint16 *p3 = buffer.DirtyPixel_uint16 (row - 1, badRect.l - 4, 0);
		uint16 *p4 = buffer.DirtyPixel_uint16 (row    , badRect.l - 4, 0);
		uint16 *p5 = buffer.DirtyPixel_uint16 (row + 1, badRect.l - 4, 0);
		uint16 *p6 = buffer.DirtyPixel_uint16 (row + 2, badRect.l - 4, 0);
		uint16 *p7 = buffer.DirtyPixel_uint16 (row + 3, badRect.l - 4, 0);
		uint16 *p8 = buffer.DirtyPixel_uint16 (row + 4, badRect.l - 4, 0);

		uint32 est0;
		uint32 est1;
		uint32 est2;
		uint32 est3;
		uint32 est4;
		uint32 est5;
		uint32 est6;

		uint32 grad0;
		uint32 grad1;
		uint32 grad2;
		uint32 grad3;
		uint32 grad4;
		uint32 grad5;
		uint32 grad6;

		uint32 lower = 0;
		uint32 upper = 0x0FFFF;

		if (IsGreen (row, badRect.l))		// Green pixel
			{

			// g00 b01 g02 b03 g04 b05 g06 b07 g08
			// r10 g11 r12 g13 r14 g15 r16 g17 r18
			// g20 b21 g22 b23 g24 b25 g26 b27 g28
			// r30 g31 r32 g33 r34 g35 r36 g37 r38
			// g40 b41 g42 b43 g44 b45 g46 b47 g48
			// r50 g51 r52 g53 r54 g55 r56 g57 r58
			// g60 b61 g62 b63 g64 b65 g66 b67 g68
			// r70 g71 r72 g73 r74 g75 r76 g77 r78
			// g80 b81 g82 b83 g84 b85 g86 b87 g88

			int32 b03 = p0 [3 * cs];
			int32 b05 = p0 [5 * cs];

			int32 g13 = p1 [3 * cs];
			int32 g15 = p1 [5 * cs];

			int32 g22 = p2 [2 * cs];
			int32 b23 = p2 [3 * cs];
			int32 b25 = p2 [5 * cs];
			int32 g26 = p2 [6 * cs];

			int32 r30 = p3 [0 * cs];
			int32 g31 = p3 [1 * cs];
			int32 r32 = p3 [2 * cs];
			int32 g33 = p3 [3 * cs];
			int32 g35 = p3 [5 * cs];
			int32 r36 = p3 [6 * cs];
			int32 g37 = p3 [7 * cs];
			int32 r38 = p3 [8 * cs];

			int32 g40 = p4 [0 * cs];
			int32 g42 = p4 [2 * cs];
			int32 b43 = p4 [3 * cs];
			int32 b45 = p4 [5 * cs];
			int32 g46 = p4 [6 * cs];
			int32 g48 = p4 [8 * cs];

			int32 r50 = p5 [0 * cs];
			int32 g51 = p5 [1 * cs];
			int32 r52 = p5 [2 * cs];
			int32 g53 = p5 [3 * cs];
			int32 g55 = p5 [5 * cs];
			int32 r56 = p5 [6 * cs];
			int32 g57 = p5 [7 * cs];
			int32 r58 = p5 [8 * cs];

			int32 g62 = p6 [2 * cs];
			int32 b63 = p6 [3 * cs];
			int32 b65 = p6 [5 * cs];
			int32 g66 = p6 [6 * cs];

			int32 g73 = p7 [3 * cs];
			int32 g75 = p7 [5 * cs];

			int32 b83 = p8 [3 * cs];
			int32 b85 = p8 [5 * cs];

			est0 = g13 + g75;

			grad0 = Abs_int32 (g13 - g75) +
					Abs_int32 (g15 - g46) +
					Abs_int32 (g22 - g53) +
					Abs_int32 (g35 - g66) +
					Abs_int32 (g42 - g73) +
					Abs_int32 (b03 - b65) +
					Abs_int32 (b23 - b85);

			est1 = g33 + g55;

			grad1 = Abs_int32 (g33 - g55) +
					Abs_int32 (g22 - g55) +
					Abs_int32 (g33 - g66) +
					Abs_int32 (g13 - g35) +
					Abs_int32 (g53 - g75) +
					Abs_int32 (b23 - b45) +
					Abs_int32 (b43 - b65);

			est2 = g31 + g57;

			grad2 = Abs_int32 (g31 - g57) +
					Abs_int32 (g33 - g46) +
					Abs_int32 (g35 - g48) +
					Abs_int32 (g40 - g53) +
					Abs_int32 (g42 - g55) +
					Abs_int32 (r30 - r56) +
					Abs_int32 (r32 - r58);

			est3 = g42 + g46;

			grad3 = Abs_int32 (g42 - g46) * 2 +
					Abs_int32 (g33 - g35) +
					Abs_int32 (g53 - g55) +
					Abs_int32 (b23 - b25) +
					Abs_int32 (b43 - b45) +
					Abs_int32 (b63 - b65);

			est4 = g37 + g51;

			grad4 = Abs_int32 (g37 - g51) +
					Abs_int32 (g35 - g42) +
					Abs_int32 (g33 - g40) +
					Abs_int32 (g48 - g55) +
					Abs_int32 (g46 - g53) +
					Abs_int32 (r38 - r52) +
					Abs_int32 (r36 - r50);

			est5 = g35 + g53;

			grad5 = Abs_int32 (g35 - g53) +
					Abs_int32 (g26 - g53) +
					Abs_int32 (g35 - g62) +
					Abs_int32 (g15 - g33) +
					Abs_int32 (g55 - g73) +
					Abs_int32 (b25 - b43) +
					Abs_int32 (b45 - b63);

			est6 = g15 + g73;

			grad6 = Abs_int32 (g15 - g73) +
					Abs_int32 (g13 - g42) +
					Abs_int32 (g26 - g55) +
					Abs_int32 (g33 - g62) +
					Abs_int32 (g46 - g75) +
					Abs_int32 (b05 - b63) +
					Abs_int32 (b25 - b83);

			lower = Min_uint32 (Min_uint32 (g33, g35),
								Min_uint32 (g53, g55));

			upper = Max_uint32 (Max_uint32 (g33, g35),
								Max_uint32 (g53, g55));

			}

		else		// Red/blue pixel
			{

			// b00 g01 b02 g03 b04 g05 b06 g07 b08
			// g10 r11 g12 r13 g14 r15 g16 r17 g18
			// b20 g21 b22 g23 b24 g25 b26 g27 b28
			// g30 r31 g32 r33 g34 r35 g36 r37 g38
			// b40 g41 b42 g43 b44 g45 b46 g47 b48
			// g50 r51 g52 r53 g54 r55 g56 r57 g58
			// b60 g61 b62 g63 b64 g65 b66 g67 b68
			// g70 r71 g72 r73 g74 r75 g76 r77 g78
			// b80 g81 b82 g83 b84 g85 b86 g87 b88

			int32 b02 = p0 [2 * cs];
			int32 g03 = p0 [3 * cs];
			int32 g05 = p0 [5 * cs];
			int32 b06 = p0 [6 * cs];

			int32 r13 = p1 [3 * cs];
			int32 r15 = p1 [5 * cs];

			int32 b20 = p2 [0 * cs];
			int32 b22 = p2 [2 * cs];
			int32 g23 = p2 [3 * cs];
			int32 g25 = p2 [5 * cs];
			int32 b26 = p2 [6 * cs];
			int32 b28 = p2 [8 * cs];

			int32 r31 = p3 [1 * cs];
			int32 g32 = p3 [2 * cs];
			int32 r33 = p3 [3 * cs];
			int32 r35 = p3 [5 * cs];
			int32 g36 = p3 [6 * cs];
			int32 r37 = p3 [7 * cs];

			int32 g41 = p4 [1 * cs];
			int32 b42 = p4 [2 * cs];
			int32 g43 = p4 [3 * cs];
			int32 g45 = p4 [5 * cs];
			int32 b46 = p4 [6 * cs];
			int32 g47 = p4 [7 * cs];

			int32 r51 = p5 [1 * cs];
			int32 g52 = p5 [2 * cs];
			int32 r53 = p5 [3 * cs];
			int32 r55 = p5 [5 * cs];
			int32 g56 = p5 [6 * cs];
			int32 r57 = p5 [7 * cs];

			int32 b60 = p6 [0 * cs];
			int32 b62 = p6 [2 * cs];
			int32 g63 = p6 [3 * cs];
			int32 g65 = p6 [5 * cs];
			int32 b66 = p6 [6 * cs];
			int32 b68 = p6 [8 * cs];

			int32 r73 = p7 [3 * cs];
			int32 r75 = p7 [5 * cs];

			int32 b82 = p8 [2 * cs];
			int32 g83 = p8 [3 * cs];
			int32 g85 = p8 [5 * cs];
			int32 b86 = p8 [6 * cs];

			est0 = b02 + b86;

			grad0 = Abs_int32 (b02 - b86) +
					Abs_int32 (r13 - r55) +
					Abs_int32 (r33 - r75) +
					Abs_int32 (g03 - g45) +
					Abs_int32 (g23 - g65) +
					Abs_int32 (g43 - g85);

			est1 = b22 + b66;

			grad1 = Abs_int32 (b22 - b66) +
					Abs_int32 (r13 - r35) +
					Abs_int32 (r33 - r55) +
					Abs_int32 (r53 - r75) +
					Abs_int32 (g23 - g45) +
					Abs_int32 (g43 - g65);

			est2 = b20 + b68;

			grad2 = Abs_int32 (b20 - b68) +
					Abs_int32 (r31 - r55) +
					Abs_int32 (r33 - r57) +
					Abs_int32 (g23 - g47) +
					Abs_int32 (g32 - g56) +
					Abs_int32 (g41 - g65);

			est3 = b42 + b46;

			grad3 = Abs_int32 (b42 - b46) +
					Abs_int32 (r33 - r35) +
					Abs_int32 (r53 - r55) +
					Abs_int32 (g32 - g36) +
					Abs_int32 (g43 - g43) +
					Abs_int32 (g52 - g56);

			est4 = b28 + b60;

			grad4 = Abs_int32 (b28 - b60) +
					Abs_int32 (r37 - r53) +
					Abs_int32 (r35 - r51) +
					Abs_int32 (g25 - g41) +
					Abs_int32 (g36 - g52) +
					Abs_int32 (g47 - g63);

			est5 = b26 + b62;

			grad5 = Abs_int32 (b26 - b62) +
					Abs_int32 (r15 - r33) +
					Abs_int32 (r35 - r53) +
					Abs_int32 (r55 - r73) +
					Abs_int32 (g25 - g43) +
					Abs_int32 (g45 - g63);

			est6 = b06 + b82;

			grad6 = Abs_int32 (b06 - b82) +
					Abs_int32 (r15 - r53) +
					Abs_int32 (r35 - r73) +
					Abs_int32 (g05 - g43) +
					Abs_int32 (g25 - g63) +
					Abs_int32 (g45 - g83);

			lower = Min_uint32 (b42, b46);
			upper = Max_uint32 (b42, b46);

			}

		uint32 minGrad = Min_uint32 (grad0, grad1);

		minGrad = Min_uint32 (minGrad, grad2);
		minGrad = Min_uint32 (minGrad, grad3);
		minGrad = Min_uint32 (minGrad, grad4);
		minGrad = Min_uint32 (minGrad, grad5);
		minGrad = Min_uint32 (minGrad, grad6);

		uint32 limit = (minGrad * 3) >> 1;

		uint32 total = 0;
		uint32 count = 0;

		if (grad0 <= limit)
			{
			total += est0;
			count += 2;
			}

		if (grad1 <= limit)
			{
			total += est1;
			count += 2;
			}

		if (grad2 <= limit)
			{
			total += est2;
			count += 2;
			}

		if (grad3 <= limit)
			{
			total += est3;
			count += 2;
			}

		if (grad4 <= limit)
			{
			total += est4;
			count += 2;
			}

		if (grad5 <= limit)
			{
			total += est5;
			count += 2;
			}

		if (grad6 <= limit)
			{
			total += est6;
			count += 2;
			}

		uint32 estimate = (total + (count >> 1)) / count;

		p4 [4] = (uint16) Pin_uint32 (lower, estimate, upper);

		}

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::FixSingleRow (dng_pixel_buffer &buffer,
												const dng_rect &badRect)
	{

	dng_pixel_buffer tBuffer = buffer;

	tBuffer.fArea = Transpose (buffer.fArea);

	tBuffer.fRowStep = buffer.fColStep;
	tBuffer.fColStep = buffer.fRowStep;

	dng_rect tBadRect = Transpose (badRect);

	FixSingleColumn (tBuffer, tBadRect);

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::FixClusteredRect (dng_pixel_buffer &buffer,
												    const dng_rect &badRect,
													const dng_rect &imageBounds)
	{

	const uint32 kNumSets = 8;
	const uint32 kSetSize = 8;

	static const int32 kOffset [kNumSets] [kSetSize] [2] =
		{
			{
				{ -1,  1 },
				{ -1, -1 },
				{  1, -1 },
				{  1,  1 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 }
			},
			{
				{ -2,  0 },
				{  2,  0 },
				{  0, -2 },
				{  0,  2 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 }
			},
			{
				{ -2, -2 },
				{ -2,  2 },
				{  2, -2 },
				{  2,  2 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 }
			},
			{
				{ -1, -3 },
				{ -3, -1 },
				{  1, -3 },
				{  3, -1 },
				{ -1,  3 },
				{ -3,  1 },
				{  1,  3 },
				{  3,  1 }
			},
			{
				{ -4,  0 },
				{  4,  0 },
				{  0, -4 },
				{  0,  4 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 }
			},
			{
				{ -3, -3 },
				{ -3,  3 },
				{  3, -3 },
				{  3,  3 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 }
			},
			{
				{ -2, -4 },
				{ -4, -2 },
				{  2, -4 },
				{  4, -2 },
				{ -2,  4 },
				{ -4,  2 },
				{  2,  4 },
				{  4,  2 }
			},
			{
				{ -4, -4 },
				{ -4,  4 },
				{  4, -4 },
				{  4,  4 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 },
				{  0,  0 }
			}
		};

	bool didFail = false;

	for (int32 row = badRect.t; row < badRect.b; row++)
		{

		for (int32 col = badRect.l; col < badRect.r; col++)
			{

			uint16 *p = buffer.DirtyPixel_uint16 (row, col, 0);

			bool isGreen = IsGreen (row, col);

			bool didFixPixel = false;

			for (uint32 set = 0; set < kNumSets && !didFixPixel; set++)
				{

				if (!isGreen && (kOffset [set] [0] [0] & 1) == 1)
					{
					continue;
					}

				uint32 total = 0;
				uint32 count = 0;

				for (uint32 entry = 0; entry < kSetSize; entry++)
					{

					dng_point offset (kOffset [set] [entry] [0],
									  kOffset [set] [entry] [1]);

					if (offset.v == 0 &&
						offset.h == 0)
						{
						break;
						}

					if (fList->IsPointValid (dng_point (row, col) + offset,
											 imageBounds))
						{

						total += p [offset.v * buffer.fRowStep +
									offset.h * buffer.fColStep];

						count++;

						}

					}

				if (count)
					{

					uint32 estimate = (total + (count >> 1)) / count;

					p [0] = (uint16) estimate;

					didFixPixel = true;

					}

				}

			if (!didFixPixel)
				{

				didFail = true;

				}

			}

		}

	#if qDNGValidate

	if (didFail)
		{

		ReportWarning ("Unable to repair bad rectangle");

		}

	#endif

	}

/*****************************************************************************/

void dng_opcode_FixBadPixelsList::ProcessArea (dng_negative & /* negative */,
											   uint32 /* threadIndex */,
											   dng_pixel_buffer &srcBuffer,
											   dng_pixel_buffer &dstBuffer,
											   const dng_rect &dstArea,
											   const dng_rect &imageBounds)
	{

	uint32 pointCount = fList->PointCount ();
	uint32 rectCount  = fList->RectCount  ();

	dng_rect fixArea = dstArea;

	if (rectCount)
		{
		fixArea.t -= kBadRectPadding;
		fixArea.l -= kBadRectPadding;
		fixArea.b += kBadRectPadding;
		fixArea.r += kBadRectPadding;
		}

	bool didFixPoint = false;

	if (pointCount)
		{

		for (uint32 pointIndex = 0; pointIndex < pointCount; pointIndex++)
			{

			dng_point badPoint = fList->Point (pointIndex);

			if (badPoint.v >= fixArea.t &&
				badPoint.h >= fixArea.l &&
				badPoint.v <  fixArea.b &&
				badPoint.h <  fixArea.r)
				{

				bool isIsolated = fList->IsPointIsolated (pointIndex,
														  kBadPointPadding);

				if (isIsolated &&
					badPoint.v >= imageBounds.t + kBadPointPadding &&
					badPoint.h >= imageBounds.l + kBadPointPadding &&
					badPoint.v <  imageBounds.b - kBadPointPadding &&
					badPoint.h <  imageBounds.r - kBadPointPadding)
					{

					FixIsolatedPixel (srcBuffer,
									  badPoint);

					}

				else
					{

					FixClusteredPixel (srcBuffer,
									   pointIndex,
									   imageBounds);

					}

				didFixPoint = true;

				}

			}

		}

	if (rectCount)
		{

		if (didFixPoint)
			{

			srcBuffer.RepeatSubArea (imageBounds,
									 SrcRepeat ().v,
									 SrcRepeat ().h);

			}

		for (uint32 rectIndex = 0; rectIndex < rectCount; rectIndex++)
			{

			dng_rect badRect = fList->Rect (rectIndex);

			dng_rect overlap = dstArea & badRect;

			if (overlap.NotEmpty ())
				{

				bool isIsolated = fList->IsRectIsolated (rectIndex,
														 kBadRectPadding);

				if (isIsolated &&
					badRect.r == badRect.l + 1 &&
					badRect.l >= imageBounds.l + SrcRepeat ().h &&
					badRect.r <= imageBounds.r - SrcRepeat ().v)
					{

					FixSingleColumn (srcBuffer,
									 overlap);

					}

				else if (isIsolated &&
						 badRect.b == badRect.t + 1 &&
						 badRect.t >= imageBounds.t + SrcRepeat ().h &&
						 badRect.b <= imageBounds.b - SrcRepeat ().v)
					{

					FixSingleRow (srcBuffer,
								  overlap);

					}

				else
					{

					FixClusteredRect (srcBuffer,
									  overlap,
									  imageBounds);

					}

				}

			}

		}

	dstBuffer.CopyArea (srcBuffer,
						dstArea,
						0,
						dstBuffer.fPlanes);

	}

/*****************************************************************************/
