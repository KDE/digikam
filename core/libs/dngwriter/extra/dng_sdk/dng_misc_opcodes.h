/*****************************************************************************/
// Copyright 2008-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_misc_opcodes.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_misc_opcodes__
#define __dng_misc_opcodes__

/*****************************************************************************/

#include "dng_opcodes.h"

/*****************************************************************************/

class dng_opcode_TrimBounds: public dng_opcode
	{

	private:

		dng_rect fBounds;

	public:

		dng_opcode_TrimBounds (const dng_rect &bounds);

		dng_opcode_TrimBounds (dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image);

	};

/*****************************************************************************/

class dng_area_spec
	{

	public:

		enum
			{
			kDataSize = 32
			};

	private:

		dng_rect fArea;

		uint32 fPlane;
		uint32 fPlanes;

		uint32 fRowPitch;
		uint32 fColPitch;

	public:

		dng_area_spec (const dng_rect &area = dng_rect (),
					   uint32 plane = 0,
					   uint32 planes = 1,
					   uint32 rowPitch = 1,
					   uint32 colPitch = 1)

			:	fArea     (area)
			,	fPlane    (plane)
			,	fPlanes   (planes)
			,	fRowPitch (rowPitch)
			,	fColPitch (colPitch)

			{
			}

		const dng_rect & Area () const
			{
			return fArea;
			}

		const uint32 Plane () const
			{
			return fPlane;
			}

		const uint32 Planes () const
			{
			return fPlanes;
			}

		const uint32 RowPitch () const
			{
			return fRowPitch;
			}

		const uint32 ColPitch () const
			{
			return fColPitch;
			}

		void GetData (dng_stream &stream);

		void PutData (dng_stream &stream) const;

		dng_rect Overlap (const dng_rect &tile) const;

	};

/*****************************************************************************/

class dng_opcode_MapTable: public dng_inplace_opcode
	{

	private:

		dng_area_spec fAreaSpec;

		AutoPtr<dng_memory_block> fTable;

		uint32 fCount;

	public:

		dng_opcode_MapTable (dng_host &host,
							 const dng_area_spec &areaSpec,
							 const uint16 *table,
							 uint32 count = 0x10000);

		dng_opcode_MapTable (dng_host &host,
							 dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 imagePixelType);

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	private:

		void ReplicateLastEntry ();

	};

/*****************************************************************************/

class dng_opcode_MapPolynomial: public dng_inplace_opcode
	{

	public:

		enum
			{
			kMaxDegree = 8
			};

	private:

		dng_area_spec fAreaSpec;

		uint32 fDegree;

		real64 fCoefficient [kMaxDegree + 1];

		real32 fCoefficient32 [kMaxDegree + 1];

	public:

		dng_opcode_MapPolynomial (const dng_area_spec &areaSpec,
								  uint32 degree,
								  const real64 *coefficient);

		dng_opcode_MapPolynomial (dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 imagePixelType);

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	};

/*****************************************************************************/

class dng_opcode_DeltaPerRow: public dng_inplace_opcode
	{

	private:

		dng_area_spec fAreaSpec;

		AutoPtr<dng_memory_block> fTable;

		real32 fScale;

	public:

		dng_opcode_DeltaPerRow (const dng_area_spec &areaSpec,
								AutoPtr<dng_memory_block> &table);

		dng_opcode_DeltaPerRow (dng_host &host,
								dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 imagePixelType);

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	};

/*****************************************************************************/

class dng_opcode_DeltaPerColumn: public dng_inplace_opcode
	{

	private:

		dng_area_spec fAreaSpec;

		AutoPtr<dng_memory_block> fTable;

		real32 fScale;

	public:

		dng_opcode_DeltaPerColumn (const dng_area_spec &areaSpec,
								   AutoPtr<dng_memory_block> &table);

		dng_opcode_DeltaPerColumn (dng_host &host,
								   dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 imagePixelType);

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	};

/*****************************************************************************/

class dng_opcode_ScalePerRow: public dng_inplace_opcode
	{

	private:

		dng_area_spec fAreaSpec;

		AutoPtr<dng_memory_block> fTable;

	public:

		dng_opcode_ScalePerRow (const dng_area_spec &areaSpec,
								AutoPtr<dng_memory_block> &table);

		dng_opcode_ScalePerRow (dng_host &host,
								dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 imagePixelType);

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	};

/*****************************************************************************/

class dng_opcode_ScalePerColumn: public dng_inplace_opcode
	{

	private:

		dng_area_spec fAreaSpec;

		AutoPtr<dng_memory_block> fTable;

	public:

		dng_opcode_ScalePerColumn (const dng_area_spec &areaSpec,
								   AutoPtr<dng_memory_block> &table);

		dng_opcode_ScalePerColumn (dng_host &host,
								   dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 imagePixelType);

		virtual dng_rect ModifiedBounds (const dng_rect &imageBounds);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
