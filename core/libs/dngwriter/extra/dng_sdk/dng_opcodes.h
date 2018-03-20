/*****************************************************************************/
// Copyright 2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_opcodes.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_opcodes__
#define __dng_opcodes__

/*****************************************************************************/

#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_rect.h"
#include "dng_types.h"

/*****************************************************************************/

enum dng_opcode_id
	{

	// Internal use only opcode.  Never written to DNGs.

	dngOpcode_Private				= 0,

	// Warp image to correct distortion and lateral chromatic aberration for
	// rectilinear lenses.

	dngOpcode_WarpRectilinear 		= 1,

	// Warp image to correction distortion for fisheye lenses (i.e., map the
	// fisheye projection to a perspective projection).

	dngOpcode_WarpFisheye			= 2,

	// Radial vignette correction.

	dngOpcode_FixVignetteRadial		= 3,

	// Patch bad Bayer pixels which are marked with a special value in the image.

	dngOpcode_FixBadPixelsConstant  = 4,

	// Patch bad Bayer pixels/rectangles at a list of specified coordinates.

	dngOpcode_FixBadPixelsList		= 5,

	// Trim image to specified bounds.

	dngOpcode_TrimBounds			= 6,

	// Map an area through a 16-bit LUT.

	dngOpcode_MapTable				= 7,

	// Map an area using a polynomial function.

	dngOpcode_MapPolynomial			= 8,

	// Apply a gain map to an area.

	dngOpcode_GainMap				= 9,

	// Apply a per-row delta to an area.

	dngOpcode_DeltaPerRow			= 10,

	// Apply a per-column delta to an area.

	dngOpcode_DeltaPerColumn		= 11,

	// Apply a per-row scale to an area.

	dngOpcode_ScalePerRow			= 12,

	// Apply a per-column scale to an area.

	dngOpcode_ScalePerColumn		= 13

	};

/*****************************************************************************/

class dng_opcode
	{

	public:

		enum
			{
			kFlag_None			= 0,
			kFlag_Optional      = 1,
			kFlag_SkipIfPreview = 2
			};

	private:

		uint32 fOpcodeID;

		uint32 fMinVersion;

		uint32 fFlags;

		bool fWasReadFromStream;

		uint32 fStage;

	protected:

		dng_opcode (uint32 opcodeID,
					uint32 minVersion,
					uint32 flags);

		dng_opcode (uint32 opcodeID,
					dng_stream &stream,
					const char *name);

	public:

		virtual ~dng_opcode ();

		uint32 OpcodeID () const
			{
			return fOpcodeID;
			}

		uint32 MinVersion () const
			{
			return fMinVersion;
			}

		uint32 Flags () const
			{
			return fFlags;
			}

		bool Optional () const
			{
			return (Flags () & kFlag_Optional) != 0;
			}

		bool SkipIfPreview () const
			{
			return (Flags () & kFlag_SkipIfPreview) != 0;
			}

		bool WasReadFromStream () const
			{
			return fWasReadFromStream;
			}

		uint32 Stage () const
			{
			return fStage;
			}

		void SetStage (uint32 stage)
			{
			fStage = stage;
			}

		virtual bool IsNOP () const
			{
			return false;
			}

		virtual bool IsValidForNegative (const dng_negative & /* negative */) const
			{
			return true;
			}

		virtual void PutData (dng_stream &stream) const;

		bool AboutToApply (dng_host &host,
						   dng_negative &negative);

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image) = 0;

	};

/*****************************************************************************/

class dng_opcode_Unknown: public dng_opcode
	{

	private:

		AutoPtr<dng_memory_block> fData;

	public:

		dng_opcode_Unknown (dng_host &host,
							uint32 opcodeID,
							dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image);

	};

/*****************************************************************************/

class dng_filter_opcode: public dng_opcode
	{

	protected:

		dng_filter_opcode (uint32 opcodeID,
						   uint32 minVersion,
						   uint32 flags);

		dng_filter_opcode (uint32 opcodeID,
						   dng_stream &stream,
						   const char *name);

	public:

		virtual uint32 BufferPixelType (uint32 imagePixelType)
			{
			return imagePixelType;
			}

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds)
			{
			return imageBounds;
			}

		virtual dng_point SrcRepeat ()
			{
			return dng_point (1, 1);
			}

		virtual dng_rect SrcArea (const dng_rect &dstArea,
								  const dng_rect & /* imageBounds */)
			{
			return dstArea;
			}

		virtual dng_point SrcTileSize (const dng_point &dstTileSize,
									   const dng_rect &imageBounds)
			{
			return SrcArea (dng_rect (dstTileSize),
							imageBounds).Size ();
			}

		virtual void Prepare (dng_negative & /* negative */,
							  uint32 /* threadCount */,
							  const dng_point & /* tileSize */,
							  const dng_rect & /* imageBounds */,
							  uint32 /* imagePlanes */,
							  uint32 /* bufferPixelType */,
							  dng_memory_allocator & /* allocator */)
			{
			}

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds) = 0;

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image);

	};

/*****************************************************************************/

class dng_inplace_opcode: public dng_opcode
	{

	protected:

		dng_inplace_opcode (uint32 opcodeID,
						    uint32 minVersion,
						    uint32 flags);

		dng_inplace_opcode (uint32 opcodeID,
						    dng_stream &stream,
						    const char *name);

	public:

		virtual uint32 BufferPixelType (uint32 imagePixelType)
			{
			return imagePixelType;
			}

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds)
			{
			return imageBounds;
			}

		virtual void Prepare (dng_negative & /* negative */,
							  uint32 /* threadCount */,
							  const dng_point & /* tileSize */,
							  const dng_rect & /* imageBounds */,
							  uint32 /* imagePlanes */,
							  uint32 /* bufferPixelType */,
							  dng_memory_allocator & /* allocator */)
			{
			}

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds) = 0;

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
