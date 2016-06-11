/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_image.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 *  Support for working with image data in DNG SDK.
 */

/*****************************************************************************/

#ifndef __dng_image__
#define __dng_image__

/*****************************************************************************/

#include "dng_assertions.h"
#include "dng_classes.h"
#include "dng_pixel_buffer.h"
#include "dng_point.h"
#include "dng_rect.h"
#include "dng_tag_types.h"
#include "dng_types.h"

/*****************************************************************************/

/// \brief Class to get resource acquisition is instantiation behavior for tile
/// buffers. Can be dirty or constant tile access.

class dng_tile_buffer: public dng_pixel_buffer
	{

	protected:

		const dng_image &fImage;

		void *fRefData;

	protected:

		/// Obtain a tile from an image.
		/// \param image Image tile will come from.
		/// \param tile Rectangle denoting extent of tile.
		/// \param dirty Flag indicating whether this is read-only or read-write acesss.

		dng_tile_buffer (const dng_image &image,
						 const dng_rect &tile,
						 bool dirty);

		virtual ~dng_tile_buffer ();

	public:

		void SetRefData (void *refData)
			{
			fRefData = refData;
			}

		void * GetRefData () const
			{
			return fRefData;
			}

	private:

		// Hidden copy constructor and assignment operator.

		dng_tile_buffer (const dng_tile_buffer &buffer);

		dng_tile_buffer & operator= (const dng_tile_buffer &buffer);

	};

/*****************************************************************************/

/// \brief Class to get resource acquisition is instantiation behavior for
/// constant (read-only) tile buffers.

class dng_const_tile_buffer: public dng_tile_buffer
	{

	public:

		/// Obtain a read-only tile from an image.
		/// \param image Image tile will come from.
		/// \param tile Rectangle denoting extent of tile.

		dng_const_tile_buffer (const dng_image &image,
							   const dng_rect &tile);

		virtual ~dng_const_tile_buffer ();

	};

/*****************************************************************************/

/// \brief Class to get resource acquisition is instantiation behavior for
/// dirty (writable) tile buffers.

class dng_dirty_tile_buffer: public dng_tile_buffer
	{

	public:

		/// Obtain a writable tile from an image.
		/// \param image Image tile will come from.
		/// \param tile Rectangle denoting extent of tile.

		dng_dirty_tile_buffer (dng_image &image,
							   const dng_rect &tile);

		virtual ~dng_dirty_tile_buffer ();

	};

/*****************************************************************************/

/// \brief Base class for holding image data in DNG SDK. See dng_simple_image
/// for derived class most often used in DNG SDK.

class dng_image
	{

	friend class dng_tile_buffer;

	protected:

		// Bounds for this image.

		dng_rect fBounds;

		// Number of image planes.

		uint32 fPlanes;

		// Basic pixel type (TIFF tag type code).

		uint32 fPixelType;

	public:

		/// How to handle requests to get image areas outside the image bounds.

		enum edge_option
			{

			/// Leave edge pixels unchanged.

			edge_none,

			/// Pad with zeros.

			edge_zero,

			/// Repeat edge pixels.

			edge_repeat,

			/// Repeat edge pixels, except for last plane which is zero padded.

			edge_repeat_zero_last

			};

	protected:

		dng_image (const dng_rect &bounds,
				   uint32 planes,
				   uint32 pixelType);

	public:

		virtual ~dng_image ();

		virtual dng_image * Clone () const;

		/// Getter method for bounds of an image.

		const dng_rect & Bounds () const
			{
			return fBounds;
			}

		/// Getter method for size of an image.

		dng_point Size () const
			{
			return Bounds ().Size ();
			}

		/// Getter method for width of an image.

		uint32 Width () const
			{
			return Bounds ().W ();
			}

		/// Getter method for height of an image.

		uint32 Height () const
			{
			return Bounds ().H ();
			}

		/// Getter method for number of planes in an image.

		uint32 Planes () const
			{
			return fPlanes;
			}

		/// Getter for pixel type.
		/// \retval See dng_tagtypes.h . Valid values are ttByte, ttShort, ttSShort,
		/// ttLong, ttFloat .

		uint32 PixelType () const
			{
			return fPixelType;
			}

		/// Setter for pixel type.
		/// \param pixelType The new pixel type .

		virtual void SetPixelType (uint32 pixelType);

		/// Getter for pixel size.
		/// \retval Size, in bytes, of pixel type for this image .

		uint32 PixelSize () const;

		/// Getter for pixel range.
		/// For unsigned types, range is 0 to return value.
		/// For signed types, range is return value - 0x8000U.
		/// For ttFloat type, pixel range is 0.0 to 1.0 and this routine returns 1.

		uint32 PixelRange () const;

		/// Getter for best "tile stride" for accessing image.

		virtual dng_rect RepeatingTile () const;

		/// Get a pixel buffer of data on image with proper edge padding.
		/// \param buffer Receives resulting pixel buffer.
		/// \param edgeOption edge_option describing how to pad edges.
		/// \param repeatV Amount of repeated padding needed in vertical for
		/// edge_repeat and edge_repeat_zero_last edgeOption cases.
		/// \param repeatH Amount of repeated padding needed in horizontal for
		/// edge_repeat and edge_repeat_zero_last edgeOption cases.

		void Get (dng_pixel_buffer &buffer,
				  edge_option edgeOption = edge_none,
				  uint32 repeatV = 1,
				  uint32 repeatH = 1) const;

		/// Put a pixel buffer into image.
		/// \param buffer Pixel buffer to copy from.

		void Put (const dng_pixel_buffer &buffer);

		/// Shrink bounds of image to given rectangle.
		/// \param r Rectangle to crop to.

		virtual void Trim (const dng_rect &r);

		/// Rotate image to reflect given orientation change.
		/// \param orientation Directive to rotate image in a certain way.

		virtual void Rotate (const dng_orientation &orientation);

		/// Copy image data from an area of one image to same area of another.
		/// \param src Image to copy from.
		/// \param area Rectangle of images to copy.
		/// \param srcPlane Plane to start copying in src.
		/// \param dstPlane Plane to start copying in this.
		/// \param planes Number of planes to copy.

		void CopyArea (const dng_image &src,
					   const dng_rect &area,
					   uint32 srcPlane,
					   uint32 dstPlane,
					   uint32 planes);

		/// Copy image data from an area of one image to same area of another.
		/// \param src Image to copy from.
		/// \param area Rectangle of images to copy.
		/// \param plane Plane to start copying in src and this.
		/// \param planes Number of planes to copy.

		void CopyArea (const dng_image &src,
					   const dng_rect &area,
					   uint32 plane,
					   uint32 planes)
			{

			CopyArea (src, area, plane, plane, planes);

			}

		/// Return true if the contents of an area of the image are the same as those of another.
		/// \param rhs Image to compare against.
		/// \param area Rectangle of image to test.
		/// \param plane Plane to start comparing.
		/// \param planes Number of planes to compare.

		bool EqualArea (const dng_image &rhs,
						const dng_rect &area,
						uint32 plane,
						uint32 planes) const;

		// Routines to set the entire image to a constant value.

		void SetConstant_uint8 (uint8 value,
								const dng_rect &area)
			{

			DNG_ASSERT (fPixelType == ttByte, "Mismatched pixel type");

			SetConstant ((uint32) value, area);

			}

		void SetConstant_uint8 (uint8 value)
			{
			SetConstant (value, Bounds ());
			}

		void SetConstant_uint16 (uint16 value,
								 const dng_rect &area)
			{

			DNG_ASSERT (fPixelType == ttShort, "Mismatched pixel type");

			SetConstant ((uint32) value, area);

			}

		void SetConstant_uint16 (uint16 value)
			{
			SetConstant_uint16 (value, Bounds ());
			}

		void SetConstant_int16 (int16 value,
								const dng_rect &area)
			{

			DNG_ASSERT (fPixelType == ttSShort, "Mismatched pixel type");

			SetConstant ((uint32) (uint16) value, area);

			}

		void SetConstant_int16 (int16 value)
			{
			SetConstant_int16 (value, Bounds ());
			}

		void SetConstant_uint32 (uint32 value,
								 const dng_rect &area)
			{

			DNG_ASSERT (fPixelType == ttLong, "Mismatched pixel type");

			SetConstant (value, area);

			}

		void SetConstant_uint32 (uint32 value)
			{
			SetConstant_uint32 (value, Bounds ());
			}

		void SetConstant_real32 (real32 value,
								 const dng_rect &area)
			{

			DNG_ASSERT (fPixelType == ttFloat, "Mismatched pixel type");

			union
				{
				uint32 i;
				real32 f;
				} x;

			x.f = value;

			SetConstant (x.i, area);

			}

		void SetConstant_real32 (real32 value)
			{
			SetConstant_real32 (value, Bounds ());
			}

		virtual void GetRepeat (dng_pixel_buffer &buffer,
								const dng_rect &srcArea,
								const dng_rect &dstArea) const;

	protected:

		virtual void AcquireTileBuffer (dng_tile_buffer &buffer,
										const dng_rect &area,
										bool dirty) const;

		virtual void ReleaseTileBuffer (dng_tile_buffer &buffer) const;

		virtual void DoGet (dng_pixel_buffer &buffer) const;

		virtual void DoPut (const dng_pixel_buffer &buffer);

		void GetEdge (dng_pixel_buffer &buffer,
					  edge_option edgeOption,
					  const dng_rect &srcArea,
					  const dng_rect &dstArea) const;

		virtual void SetConstant (uint32 value,
								  const dng_rect &area);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
