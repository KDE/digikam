/*****************************************************************************/
// Copyright 2008-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_gain_map.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_gain_map__
#define __dng_gain_map__

/*****************************************************************************/

#include "dng_memory.h"
#include "dng_misc_opcodes.h"
#include "dng_tag_types.h"

/*****************************************************************************/

class dng_gain_map
	{

	private:

		dng_point fPoints;

		dng_point_real64 fSpacing;

		dng_point_real64 fOrigin;

		uint32 fPlanes;

		uint32 fRowStep;

		AutoPtr<dng_memory_block> fBuffer;

	public:

		dng_gain_map (dng_memory_allocator &allocator,
					  const dng_point &points,
					  const dng_point_real64 &spacing,
					  const dng_point_real64 &origin,
					  uint32 planes);

		const dng_point & Points () const
			{
			return fPoints;
			}

		const dng_point_real64 & Spacing () const
			{
			return fSpacing;
			}

		const dng_point_real64 & Origin () const
			{
			return fOrigin;
			}

		uint32 Planes () const
			{
			return fPlanes;
			}

		real32 & Entry (uint32 rowIndex,
						uint32 colIndex,
						uint32 plane)
			{

			return *(fBuffer->Buffer_real32 () +
				     rowIndex * fRowStep +
				     colIndex * fPlanes  +
				     plane);

			}

		const real32 & Entry (uint32 rowIndex,
							  uint32 colIndex,
							  uint32 plane) const
			{

			return *(fBuffer->Buffer_real32 () +
				     rowIndex * fRowStep +
				     colIndex * fPlanes  +
				     plane);

			}

		real32 Interpolate (int32 row,
							int32 col,
							uint32 plane,
							const dng_rect &bounds) const;

		uint32 PutStreamSize () const;

		void PutStream (dng_stream &stream) const;

		static dng_gain_map * GetStream (dng_host &host,
										 dng_stream &stream);

	private:

		// Hidden copy constructor and assignment operator.

		dng_gain_map (const dng_gain_map &map);

		dng_gain_map & operator= (const dng_gain_map &map);

	};

/*****************************************************************************/

class dng_opcode_GainMap: public dng_inplace_opcode
	{

	private:

		dng_area_spec fAreaSpec;

		AutoPtr<dng_gain_map> fGainMap;

	public:

		dng_opcode_GainMap (const dng_area_spec &areaSpec,
							AutoPtr<dng_gain_map> &gainMap);

		dng_opcode_GainMap (dng_host &host,
							dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 /* imagePixelType */)
			{
			return ttFloat;
			}

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds)
			{
			return fAreaSpec.Overlap (imageBounds);
			}

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	private:

		// Hidden copy constructor and assignment operator.

		dng_opcode_GainMap (const dng_opcode_GainMap &opcode);

		dng_opcode_GainMap & operator= (const dng_opcode_GainMap &opcode);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
