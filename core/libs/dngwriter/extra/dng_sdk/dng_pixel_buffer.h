/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_pixel_buffer.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Support for holding buffers of sample data.
 */

/*****************************************************************************/

#ifndef __dng_pixel_buffer__
#define __dng_pixel_buffer__

/*****************************************************************************/

#include "dng_assertions.h"
#include "dng_rect.h"
#include "dng_tag_types.h"

/*****************************************************************************/

/// Compute best set of step values for a given source and destination area and stride.

void OptimizeOrder (const void *&sPtr,
					void *&dPtr,
					uint32 sPixelSize,
					uint32 dPixelSize,
					uint32 &count0,
					uint32 &count1,
					uint32 &count2,
					int32 &sStep0,
					int32 &sStep1,
					int32 &sStep2,
					int32 &dStep0,
					int32 &dStep1,
					int32 &dStep2);

void OptimizeOrder (const void *&sPtr,
					uint32 sPixelSize,
					uint32 &count0,
					uint32 &count1,
					uint32 &count2,
					int32 &sStep0,
					int32 &sStep1,
					int32 &sStep2);

void OptimizeOrder (void *&dPtr,
					uint32 dPixelSize,
					uint32 &count0,
					uint32 &count1,
					uint32 &count2,
					int32 &dStep0,
					int32 &dStep1,
					int32 &dStep2);

/*****************************************************************************/

#define qDebugPixelType 0

#if qDebugPixelType

#define ASSERT_PIXEL_TYPE(typeVal) CheckPixelType (typeVal)

#else

#define ASSERT_PIXEL_TYPE(typeVal) DNG_ASSERT (fPixelType == typeVal, "Pixel type access mismatch")

#endif

/*****************************************************************************/

/// \brief Holds a buffer of pixel data with "pixel geometry" metadata.
///
/// The pixel geometry describes the layout in terms of how many planes, rows and columns
/// plus the steps (in bytes) between each column, row and plane.

class dng_pixel_buffer
	{

	public:

		// Area this buffer holds.

		dng_rect fArea;

		// Range of planes this buffer holds.

		uint32 fPlane;
		uint32 fPlanes;

		// Steps between pixels.

		int32 fRowStep;
		int32 fColStep;
		int32 fPlaneStep;

		// Basic pixel type (TIFF tag type code).

		uint32 fPixelType;

		// Size of pixel type in bytes.

		uint32 fPixelSize;

		// Pointer to buffer's data.

		void *fData;

		// Do we have write-access to this data?

		bool fDirty;

	private:

		void * InternalPixel (int32 row,
							  int32 col,
					  	      uint32 plane = 0) const
			{

			return (void *)
				   (((uint8 *) fData) + (int32)fPixelSize *
					(fRowStep   * (row   - fArea.t) +
					 fColStep   * (col   - fArea.l) +
					 fPlaneStep * (int32)(plane - fPlane )));

			}

		#if qDebugPixelType

		void CheckPixelType (uint32 pixelType) const;

		#endif

	public:

		dng_pixel_buffer ();

		dng_pixel_buffer (const dng_pixel_buffer &buffer);

		dng_pixel_buffer & operator= (const dng_pixel_buffer &buffer);

		virtual ~dng_pixel_buffer ();

		/// Get the range of pixel values.
		/// \retval Range of value a pixel can take. (Meaning [0, max] for unsigned case. Signed case is biased so [-32768, max - 32768].)

		uint32 PixelRange () const;

		/// Get extent of pixels in buffer
		/// \retval Rectangle giving valid extent of buffer.

		const dng_rect & Area () const
			{
			return fArea;
			}

		/// Number of planes of image data.
		/// \retval Number of planes held in buffer.

		uint32 Planes () const
			{
			return fPlanes;
			}

		/// Step, in pixels not bytes, between rows of data in buffer.
		/// \retval row step in pixels. May be negative.

		int32 RowStep () const
			{
			return fRowStep;
			}

		/// Step, in pixels not bytes, between planes of data in buffer.
		/// \retval plane step in pixels. May be negative.

		int32 PlaneStep () const
			{
			return fPlaneStep;
			}

		/// Get read-only untyped (void *) pointer to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as void *.

		const void * ConstPixel (int32 row,
					  			 int32 col,
					  			 uint32 plane = 0) const
			{

			return InternalPixel (row, col, plane);

			}

		/// Get a writable untyped (void *) pointer to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as void *.

		void * DirtyPixel (int32 row,
					  	   int32 col,
					  	   uint32 plane = 0)
			{

			DNG_ASSERT (fDirty, "Dirty access to const pixel buffer");

			return InternalPixel (row, col, plane);

			}

		/// Get read-only uint8 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as uint8 *.

		const uint8 * ConstPixel_uint8 (int32 row,
										int32 col,
										uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttByte);

			return (const uint8 *) ConstPixel (row, col, plane);

			}

		/// Get a writable uint8 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as uint8 *.

		uint8 * DirtyPixel_uint8 (int32 row,
								  int32 col,
								  uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttByte);

			return (uint8 *) DirtyPixel (row, col, plane);

			}

		/// Get read-only int8 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as int8 *.

		const int8 * ConstPixel_int8 (int32 row,
									  int32 col,
									  uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttSByte);

			return (const int8 *) ConstPixel (row, col, plane);

			}

		/// Get a writable int8 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as int8 *.

		int8 * DirtyPixel_int8 (int32 row,
								int32 col,
								uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttSByte);

			return (int8 *) DirtyPixel (row, col, plane);

			}

		/// Get read-only uint16 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as uint16 *.

		const uint16 * ConstPixel_uint16 (int32 row,
										  int32 col,
										  uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttShort);

			return (const uint16 *) ConstPixel (row, col, plane);

			}

		/// Get a writable uint16 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as uint16 *.

		uint16 * DirtyPixel_uint16 (int32 row,
								    int32 col,
								    uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttShort);

			return (uint16 *) DirtyPixel (row, col, plane);

			}

		/// Get read-only int16 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as int16 *.

		const int16 * ConstPixel_int16 (int32 row,
										int32 col,
										uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttSShort);

			return (const int16 *) ConstPixel (row, col, plane);

			}

		/// Get a writable int16 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as int16 *.

		int16 * DirtyPixel_int16 (int32 row,
								  int32 col,
								  uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttSShort);

			return (int16 *) DirtyPixel (row, col, plane);

			}

		/// Get read-only uint32 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as uint32 *.

		const uint32 * ConstPixel_uint32 (int32 row,
										  int32 col,
										  uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttLong);

			return (const uint32 *) ConstPixel (row, col, plane);

			}

		/// Get a writable uint32 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as uint32 *.

		uint32 * DirtyPixel_uint32 (int32 row,
								    int32 col,
								    uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttLong);

			return (uint32 *) DirtyPixel (row, col, plane);

			}

		/// Get read-only int32 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as int32 *.

		const int32 * ConstPixel_int32 (int32 row,
										int32 col,
										uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttSLong);

			return (const int32 *) ConstPixel (row, col, plane);

			}

		/// Get a writable int32 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as int32 *.

		int32 * DirtyPixel_int32 (int32 row,
								  int32 col,
								  uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttSLong);

			return (int32 *) DirtyPixel (row, col, plane);

			}

		/// Get read-only real32 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as real32 *.

		const real32 * ConstPixel_real32 (int32 row,
										  int32 col,
										  uint32 plane = 0) const
			{

			ASSERT_PIXEL_TYPE (ttFloat);

			return (const real32 *) ConstPixel (row, col, plane);

			}

		/// Get a writable real32 * to pixel data starting at a specific pixel in the buffer.
		/// \param row Start row for buffer pointer.
		/// \param col Start column for buffer pointer.
		/// \param plane Start plane for buffer pointer.
		/// \retval Pointer to pixel data as real32 *.

		real32 * DirtyPixel_real32 (int32 row,
									int32 col,
									uint32 plane = 0)
			{

			ASSERT_PIXEL_TYPE (ttFloat);

			return (real32 *) DirtyPixel (row, col, plane);

			}

		/// Initialize a rectangular area of pixel buffer to a constant.
		/// \param area Rectangle of pixel buffer to set.
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.
		/// \param value Constant value to set pixels to.

		void SetConstant (const dng_rect &area,
					      uint32 plane,
					      uint32 planes,
					      uint32 value);

		/// Initialize a rectangular area of pixel buffer to a constant unsigned 8-bit value.
		/// \param area Rectangle of pixel buffer to set.
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.
		/// \param value Constant uint8 value to set pixels to.

		void SetConstant_uint8 (const dng_rect &area,
								uint32 plane,
								uint32 planes,
								uint8 value)
			{

			DNG_ASSERT (fPixelType == ttByte, "Mismatched pixel type");

			SetConstant (area, plane, planes, (uint32) value);

			}

		/// Initialize a rectangular area of pixel buffer to a constant unsigned 16-bit value.
		/// \param area Rectangle of pixel buffer to set.
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.
		/// \param value Constant uint16 value to set pixels to.

		void SetConstant_uint16 (const dng_rect &area,
								 uint32 plane,
								 uint32 planes,
								 uint16 value)
			{

			DNG_ASSERT (fPixelType == ttShort, "Mismatched pixel type");

			SetConstant (area, plane, planes, (uint32) value);

			}

		/// Initialize a rectangular area of pixel buffer to a constant signed 16-bit value.
		/// \param area Rectangle of pixel buffer to set.
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.
		/// \param value Constant int16 value to set pixels to.

		void SetConstant_int16 (const dng_rect &area,
								uint32 plane,
								uint32 planes,
								int16 value)
			{

			DNG_ASSERT (fPixelType == ttSShort, "Mismatched pixel type");

			SetConstant (area, plane, planes, (uint32) (uint16) value);

			}

		/// Initialize a rectangular area of pixel buffer to a constant unsigned 32-bit value.
		/// \param area Rectangle of pixel buffer to set.
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.
		/// \param value Constant uint32 value to set pixels to.

		void SetConstant_uint32 (const dng_rect &area,
								 uint32 plane,
								 uint32 planes,
								 uint32 value)
			{

			DNG_ASSERT (fPixelType == ttLong, "Mismatched pixel type");

			SetConstant (area, plane, planes, value);

			}

		/// Initialize a rectangular area of pixel buffer to a constant real 32-bit value.
		/// \param area Rectangle of pixel buffer to set.
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.
		/// \param value Constant real32 value to set pixels to.

		void SetConstant_real32 (const dng_rect &area,
								 uint32 plane,
								 uint32 planes,
								 real32 value)
			{

			DNG_ASSERT (fPixelType == ttFloat, "Mismatched pixel type");

			union
				{
				uint32 i;
				real32 f;
				} x;

			x.f = value;

			SetConstant (area, plane, planes, x.i);

			}

		/// Initialize a rectangular area of pixel buffer to zeros.
		/// \param area Rectangle of pixel buffer to zero.
		/// \param area Area to zero
		/// \param plane Plane to start filling on.
		/// \param planes Number of planes to fill.

		void SetZero (const dng_rect &area,
					  uint32 plane,
					  uint32 planes);

		/// Copy image data from an area of one pixel buffer to same area of another.
		/// \param src Buffer to copy from.
		/// \param area Rectangle of pixel buffer to copy.
		/// \param srcPlane Plane to start copy in src.
		/// \param dstPlane Plane to start copy in dst.
		/// \param planes Number of planes to copy.

		void CopyArea (const dng_pixel_buffer &src,
					   const dng_rect &area,
					   uint32 srcPlane,
					   uint32 dstPlane,
					   uint32 planes);

		/// Copy image data from an area of one pixel buffer to same area of another.
		/// \param src Buffer to copy from.
		/// \param area Rectangle of pixel buffer to copy.
		/// \param plane Plane to start copy in src and this.
		/// \param planes Number of planes to copy.

		void CopyArea (const dng_pixel_buffer &src,
					   const dng_rect &area,
					   uint32 plane,
					   uint32 planes)
			{

			CopyArea (src, area, plane, plane, planes);

			}

		/// Calculate the offset phase of destination rectangle relative to source rectangle.
		/// Phase is based on a 0,0 origin and the notion of repeating srcArea across dstArea.
		/// It is the number of pixels into srcArea to start repeating from when tiling dstArea.
		/// \retval dng_point containing horizontal and vertical phase.

		static dng_point RepeatPhase (const dng_rect &srcArea,
					   			   	  const dng_rect &dstArea);

		/// Repeat the image data in srcArea across dstArea.
		/// (Generally used for padding operations.)
		/// \param srcArea Area to repeat from.
		/// \param dstArea Area to fill with data from srcArea.

		void RepeatArea (const dng_rect &srcArea,
						 const dng_rect &dstArea);

		/// Replicates a sub-area of a buffer to fill the entire buffer.

		void RepeatSubArea (const dng_rect subArea,
						    uint32 repeatV = 1,
						    uint32 repeatH = 1);

		/// Apply a right shift (C++ oerpator >>) to all pixel values. Only implemented for 16-bit (signed or unsigned) pixel buffers.
		/// \param shift Number of bits by which to right shift each pixel value.

		void ShiftRight (uint32 shift);

		/// Change metadata so pixels are iterated in opposite horizontal order.
		/// This operation does not require movement of actual pixel data.

		void FlipH ();

		/// Change metadata so pixels are iterated in opposite vertical order.
		/// This operation does not require movement of actual pixel data.

		void FlipV ();

		/// Change metadata so pixels are iterated in opposite plane order.
		/// This operation does not require movement of actual pixel data.

		void FlipZ ();	// Flip planes

		/// Return true if the contents of an area of the pixel buffer area are the same as those of another.
		/// \param rhs Buffer to compare against.
		/// \param area Rectangle of pixel buffer to test.
		/// \param plane Plane to start comparing.
		/// \param planes Number of planes to compare.
		/// \retval bool true if areas are equal, false otherwise.

		bool EqualArea (const dng_pixel_buffer &rhs,
					    const dng_rect &area,
					    uint32 plane,
					    uint32 planes) const;

		/// Return the absolute value of the maximum difference between two pixel buffers. Used for comparison testing with tolerance
		/// \param rhs Buffer to compare against.
		/// \param area Rectangle of pixel buffer to test.
		/// \param plane Plane to start comparing.
		/// \param planes Number of planes to compare.
		/// \retval larges absolute value difference between the corresponding pixels each buffer across area.

		real64 MaximumDifference (const dng_pixel_buffer &rhs,
								  const dng_rect &area,
								  uint32 plane,
								  uint32 planes) const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
