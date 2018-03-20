/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_mosaic_info.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Support for descriptive information about color filter array patterns.
 */

/*****************************************************************************/

#ifndef __dng_mosaic_info__
#define __dng_mosaic_info__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_rect.h"
#include "dng_sdk_limits.h"
#include "dng_types.h"

/*****************************************************************************/

/// \brief Support for describing color filter array patterns and manipulating mosaic sample data.
///
/// See CFAPattern tag in \ref spec_tiff_ep "TIFF/EP specification" and CFAPlaneColor, CFALayout, and BayerGreenSplit
/// tags in the \ref spec_dng "DNG 1.1.0 specification".

class dng_mosaic_info
	{

	public:

		/// Size of fCFAPattern.

		dng_point fCFAPatternSize;

		/// CFA pattern from CFAPattern tag in the \ref spec_tiff_ep "TIFF/EP specification."

		uint8 fCFAPattern [kMaxCFAPattern] [kMaxCFAPattern];

		/// Number of color planes in DNG input.

		uint32 fColorPlanes;

		uint8 fCFAPlaneColor [kMaxColorPlanes];

		/// Value of CFALayout tag in the \ref spec_dng "DNG 1.3 specification."
		/// CFALayout describes the spatial layout of the CFA. The currently defined values are:
		///    - 1 = Rectangular (or square) layout.
		///    - 2 = Staggered layout A: even columns are offset down by 1/2 row.
		///    - 3 = Staggered layout B: even columns are offset up by 1/2 row.
		///    - 4 = Staggered layout C: even rows are offset right by 1/2 column.
		///    - 5 = Staggered layout D: even rows are offset left by 1/2 column.
		///    - 6 = Staggered layout E: even rows are offset up by 1/2 row, even columns are offset left by 1/2 column.
		///    - 7 = Staggered layout F: even rows are offset up by 1/2 row, even columns are offset right by 1/2 column.
		///    - 8 = Staggered layout G: even rows are offset down by 1/2 row, even columns are offset left by 1/2 column.
		///    - 9 = Staggered layout H: even rows are offset down by 1/2 row, even columns are offset right by 1/2 column.

		uint32 fCFALayout;

		/// Value of BayerGreeSplit tag in DNG file.
		/// BayerGreenSplit only applies to CFA images using a Bayer pattern filter array. This tag
		/// specifies, in arbitrary units, how closely the values of the green pixels in the blue/green rows
		/// track the values of the green pixels in the red/green rows.
		///
		/// A value of zero means the two kinds of green pixels track closely, while a non-zero value
		/// means they sometimes diverge. The useful range for this tag is from 0 (no divergence) to about
		/// 5000 (large divergence).

		uint32 fBayerGreenSplit;

	protected:

		dng_point fSrcSize;

		dng_point fCroppedSize;

		real64 fAspectRatio;

	public:

		dng_mosaic_info ();

		virtual ~dng_mosaic_info ();

		virtual void Parse (dng_host &host,
						    dng_stream &stream,
						    dng_info &info);

		virtual void PostParse (dng_host &host,
								dng_negative &negative);

		/// Returns whether the RAW data in this DNG file from a color filter array (mosaiced) source.
		/// \retval true if this DNG file is from a color filter array (mosiaced) source.

		bool IsColorFilterArray () const
			{
			return fCFAPatternSize != dng_point (0, 0);
			}

		/// Enable generating four-plane output from three-plane Bayer input.
		/// Extra plane is a second version of the green channel. First green is produced
		/// using green mosaic samples from one set of rows/columns (even/odd) and the second
		/// green channel is produced using the other set of rows/columns. One can compare the
		/// two versions to judge whether BayerGreenSplit needs to be set for a given input source.

		virtual bool SetFourColorBayer ();

		/// Returns scaling factor relative to input size needed to capture output data.
		/// Staggered (or rotated) sensing arrays are produced to a larger output than the number of input samples.
		/// This method indicates how much larger.
		/// \retval a point with integer scaling factors for the horizotal and vertical dimensions.

		virtual dng_point FullScale () const;

		/// Returns integer factors by which mosaic data must be downsampled to produce an image which is as close
		/// to prefSize as possible in longer dimension, but no smaller than minSize.
		/// \param minSize Number of pixels as minium for longer dimension of downsampled image.
		/// \param prefSize Number of pixels as target for longer dimension of downsampled image.
		/// \param cropFactor Faction of the image to be used after cropping.
		/// \retval Point containing integer factors by which image must be downsampled.

		virtual dng_point DownScale (uint32 minSize,
									 uint32 prefSize,
									 real64 cropFactor) const;

		/// Return size of demosaiced image for passed in downscaling factor.
		/// \param downScale Integer downsampling factor obtained from DownScale method.
		/// \retval Size of resulting demosaiced image.

		virtual dng_point DstSize (const dng_point &downScale) const;

		/// Demosaic interpolation of a single plane for non-downsampled case.
		/// \param host dng_host to use for buffer allocation requests, user cancellation testing, and progress updates.
		/// \param negative DNG negative of mosaiced data.
		/// \param srcImage Source image for mosaiced data.
		/// \param dstImage Destination image for resulting interpolated data.
		/// \param srcPlane Which plane to interpolate.

		virtual void InterpolateGeneric (dng_host &host,
										 dng_negative &negative,
								  		 const dng_image &srcImage,
								  		 dng_image &dstImage,
								  		 uint32 srcPlane = 0) const;

		/// Demosaic interpolation of a single plane for downsampled case.
		/// \param host dng_host to use for buffer allocation requests, user cancellation testing, and progress updates.
		/// \param negative DNG negative of mosaiced data.
		/// \param srcImage Source image for mosaiced data.
		/// \param dstImage Destination image for resulting interpolated data.
		/// \param downScale Amount (in horizontal and vertical) by which to subsample image.
		/// \param srcPlane Which plane to interpolate.

		virtual void InterpolateFast (dng_host &host,
									  dng_negative &negative,
							  	   	  const dng_image &srcImage,
								   	  dng_image &dstImage,
								      const dng_point &downScale,
								      uint32 srcPlane = 0) const;

		/// Demosaic interpolation of a single plane. Chooses between generic and fast interpolators based on parameters.
		/// \param host dng_host to use for buffer allocation requests, user cancellation testing, and progress updates.
		/// \param negative DNG negative of mosaiced data.
		/// \param srcImage Source image for mosaiced data.
		/// \param dstImage Destination image for resulting interpolated data.
		/// \param downScale Amount (in horizontal and vertical) by which to subsample image.
		/// \param srcPlane Which plane to interpolate.

		virtual void Interpolate (dng_host &host,
								  dng_negative &negative,
								  const dng_image &srcImage,
								  dng_image &dstImage,
								  const dng_point &downScale,
								  uint32 srcPlane = 0) const;

	protected:

		virtual bool IsSafeDownScale (const dng_point &downScale) const;

		uint32 SizeForDownScale (const dng_point &downScale) const;

		virtual bool ValidSizeDownScale (const dng_point &downScale,
									     uint32 minSize) const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
