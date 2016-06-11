/*****************************************************************************/
// Copyright 2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_lens_correction.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_lens_correction__
#define __dng_lens_correction__

/*****************************************************************************/

#include "dng_1d_function.h"
#include "dng_matrix.h"
#include "dng_opcodes.h"
#include "dng_pixel_buffer.h"
#include "dng_point.h"
#include "dng_resample.h"
#include "dng_sdk_limits.h"

#include <vector>

/*****************************************************************************/

/// \brief Abstract base class holding common warp opcode parameters (e.g., number of
/// planes, optical center) and common warp routines.

class dng_warp_params
	{

	public:

		// Number of planes to be warped. Must be either 1 or equal to the number of
		// planes of the image to be processed. If set to 1, then a single set of
		// warp parameters applies to all planes of the image. fPlanes must be at
		// least 1 and no greater than kMaxColorPlanes (see dng_sdk_limits.h).

		uint32 fPlanes;

		// The optical center of the lens in normalized [0,1] coordinates with
		// respect to the image's active area. For example, a value of (0.5, 0.5)
		// indicates that the optical center of the lens is at the center of the
		// image's active area. A normalized radius of 1.0 corresponds to the
		// distance from fCenter to the farthest corner of the image's active area.
		// Each component of fCenter must lie in the range [0,1].

		dng_point_real64 fCenter;

	public:

		dng_warp_params ();

		// planes is the number of planes of parameters specified: It must be either
		// 1 or equal to the number of planes of the image to be processed.

		dng_warp_params (uint32 planes,
						 const dng_point_real64 &fCenter);

		virtual ~dng_warp_params ();

		// Is the entire correction a NOP for all planes?

		virtual bool IsNOPAll () const;

		// Is the entire correction a NOP for the specified plane?

		virtual bool IsNOP (uint32 plane) const;

		// Is the radial correction a NOP for all planes?

		virtual bool IsRadNOPAll () const;

		// Is the radial correction a NOP for the specified plane?

		virtual bool IsRadNOP (uint32 plane) const;

		// Is the tangential correction a NOP for all planes?

		virtual bool IsTanNOPAll () const;

		// Is the tangential correction a NOP for the specified plane?

		virtual bool IsTanNOP (uint32 plane) const;

		// Do these warp params appear valid?

		virtual bool IsValid () const;

		// Are these warp params valid for the specified negative?

		virtual bool IsValidForNegative (const dng_negative &negative) const;

		// Propagate warp parameters from first plane to all other planes.

		virtual void PropagateToAllPlanes (uint32 totalPlanes) = 0;

		// Evaluate the 1D radial warp function for the specified plane. Parameter r
		// is the destination (i.e., corrected) normalized radius, i.e., the
		// normalized Euclidean distance between a corrected pixel position and the
		// optical center in the image. r lies in the range [0,1]. The returned
		// result is non-negative.

		virtual real64 Evaluate (uint32 plane,
								 real64 r) const = 0;

		// Compute and return the inverse of Evaluate () above. The base
		// implementation uses Newton's method to perform the inversion. Parameter r
		// is the source (i.e., uncorrected) normalized radius, i.e., normalized
		// Euclidean distance between a corrected pixel position and the optical
		// center in the image. Both r and the computed result are non-negative.

		virtual real64 EvaluateInverse (uint32 plane,
										real64 r) const;

		// Evaluate the 1D radial warp ratio function for the specified plane.
		// Parameter r2 is the square of the destination (i.e., corrected) normalized
		// radius, i.e., the square of the normalized Euclidean distance between a
		// corrected pixel position and the optical center in the image. r2 must lie
		// in the range [0,1]. Note that this is different than the Evaluate ()
		// function, above, in that the argument to EvaluateRatio () is the square of
		// the radius, not the radius itself. The returned result is non-negative.
		// Mathematically, EvaluateRatio (r * r) is the same as Evaluate (r) / r.

		virtual real64 EvaluateRatio (uint32 plane,
									  real64 r2) const = 0;

		// Evaluate the 2D tangential warp for the specified plane. Parameter r2 is
		// the square of the destination (i.e., corrected) normalized radius, i.e.,
		// the square of the normalized Euclidean distance between a corrected pixel
		// position P and the optical center in the image. r2 must lie in the range
		// [0,1]. diff contains the vertical and horizontal Euclidean distances (in
		// pixels) between P and the optical center. diff2 contains the squares of
		// the vertical and horizontal Euclidean distances (in pixels) between P and
		// the optical center. The returned result is the tangential warp offset,
		// measured in pixels.

		virtual dng_point_real64 EvaluateTangential (uint32 plane,
													 real64 r2,
													 const dng_point_real64 &diff,
													 const dng_point_real64 &diff2) const = 0;

		// Evaluate the 2D tangential warp for the specified plane. diff contains the
		// vertical and horizontal Euclidean distances (in pixels) between the
		// destination (i.e., corrected) pixel position and the optical center in the
		// image. The returned result is the tangential warp offset, measured in
		// pixels.

		dng_point_real64 EvaluateTangential2 (uint32 plane,
											  const dng_point_real64 &diff) const;

		// Evaluate the 2D tangential warp for the specified plane. Parameter r2 is
		// the square of the destination (i.e., corrected) normalized radius, i.e.,
		// the square of the normalized Euclidean distance between a corrected pixel
		// position P and the optical center in the image. r2 must lie in the range
		// [0,1]. diff contains the vertical and horizontal Euclidean distances (in
		// pixels) between P and the optical center. The returned result is the
		// tangential warp offset, measured in pixels.

		dng_point_real64 EvaluateTangential3 (uint32 plane,
											  real64 r2,
											  const dng_point_real64 &diff) const;

		// Compute and return the maximum warped radius gap. Let D be a rectangle in
		// a destination (corrected) image. Let rDstFar and rDstNear be the farthest
		// and nearest points to the image center, respectively. Then the specified
		// parameter maxDstGap is the Euclidean distance between rDstFar and
		// rDstNear. Warp D through this warp function to a closed and bounded
		// (generally not rectangular) region S. Let rSrcfar and rSrcNear be the
		// farthest and nearest points to the image center, respectively. This
		// routine returns a value that is at least (rSrcFar - rSrcNear).

		virtual real64 MaxSrcRadiusGap (real64 maxDstGap) const = 0;

		// Compute and return the maximum warped tangential gap. minDst is the
		// top-left pixel of the image in normalized pixel coordinates. maxDst is the
		// bottom-right pixel of the image in normalized pixel coordinates.
		// MaxSrcTanGap () computes the maximum absolute shift in normalized pixels
		// in the horizontal and vertical directions that can occur as a result of
		// the tangential warp.

		virtual dng_point_real64 MaxSrcTanGap (dng_point_real64 minDst,
											   dng_point_real64 maxDst) const = 0;

		// Debug parameters.

		virtual void Dump () const;

	};

/*****************************************************************************/

/// \brief Warp parameters for pinhole perspective rectilinear (not fisheye) camera model.
/// Supports radial and tangential (decentering) distortion correction parameters.
///
/// Note the restrictions described below.

class dng_warp_params_rectilinear: public dng_warp_params
	{

	public:

		// Radial and tangential polynomial coefficients. These define a warp from
		// corrected pixel coordinates (xDst, yDst) to uncorrected pixel coordinates
		// (xSrc, ySrc) for each plane P as follows:
		//
		// Let kr0 = fRadParams [P][0]
		//	   kr1 = fRadParams [P][1]
		//	   kr2 = fRadParams [P][2]
		//	   kr3 = fRadParams [P][3]
		//
		//	   kt0 = fTanParams [P][0]
		//	   kt1 = fTanParams [P][1]
		//
		// Let (xCenter, yCenter) be the optical image center (see fCenter, below)
		// expressed in pixel coordinates. Let maxDist be the Euclidean distance (in
		// pixels) from (xCenter, yCenter) to the farthest image corner.
		//
		// First, compute the normalized distance of the corrected pixel position
		// (xDst, yDst) from the image center:
		//
		//	   dx = (xDst - xCenter) / maxDist
		//	   dy = (yDst - yCenter) / maxDist
		//
		//	   r^2 = dx^2 + dy^2
		//
		// Compute the radial correction term:
		//
		//	   ratio = kr0 + (kr1 * r^2) + (kr2 * r^4) + (kr3 * r^6)
		//
		//	   dxRad = dx * ratio
		//	   dyRad = dy * ratio
		//
		// Compute the tangential correction term:
		//
		//	   dxTan = (2 * kt0 * dx * dy) + kt1 * (r^2 + 2 * dx^2)
		//	   dyTan = (2 * kt1 * dx * dy) + kt0 * (r^2 + 2 * dy^2)
		//
		// Compute the uncorrected pixel position (xSrc, ySrc):
		//
		//	   xSrc = xCenter + (dxRad + dxTan) * maxDist
		//	   ySrc = yCenter + (dyRad + dyTan) * maxDist
		//
		// Mathematical definitions and restrictions:
		//
		// Let { xSrc, ySrc } = f (xDst, yDst) be the warp function defined above.
		//
		// Let xSrc = fx (xDst, yDst) be the x-component of the warp function.
		// Let ySrc = fy (xDst, yDst) be the y-component of the warp function.
		//
		// f (x, y) must be an invertible function.
		//
		// fx (x, y) must be an increasing function of x.
		// fy (x, y) must be an increasing function of x.
		//
		// The parameters kr0, kr1, kr2, and kr3 must define an increasing radial
		// warp function. Specifically, let w (r) be the radial warp function:
		//
		//	   w (r) = (kr0 * r) + (kr1 * r^3) + (kr2 * r^5) + (kr3 * r^7).
		//
		// w (r) must be an increasing function.

		dng_vector fRadParams [kMaxColorPlanes];
		dng_vector fTanParams [kMaxColorPlanes];

	public:

		dng_warp_params_rectilinear ();

		dng_warp_params_rectilinear (uint32 planes,
									 const dng_vector radParams [],
									 const dng_vector tanParams [],
									 const dng_point_real64 &fCenter);

		virtual ~dng_warp_params_rectilinear ();

		// Overridden methods.

		virtual bool IsRadNOP (uint32 plane) const;

		virtual bool IsTanNOP (uint32 plane) const;

		virtual bool IsValid () const;

		virtual void PropagateToAllPlanes (uint32 totalPlanes);

		virtual real64 Evaluate (uint32 plane,
								 real64 r) const;

		virtual real64 EvaluateRatio (uint32 plane,
									  real64 r2) const;

		virtual dng_point_real64 EvaluateTangential (uint32 plane,
													 real64 r2,
													 const dng_point_real64 &diff,
													 const dng_point_real64 &diff2) const;

		virtual real64 MaxSrcRadiusGap (real64 maxDstGap) const;

		virtual dng_point_real64 MaxSrcTanGap (dng_point_real64 minDst,
											   dng_point_real64 maxDst) const;

		virtual void Dump () const;

	};

/*****************************************************************************/

/// \brief Warp parameters for fisheye camera model (radial component only). Note the
/// restrictions described below.

class dng_warp_params_fisheye: public dng_warp_params
	{

	public:

		// Radial warp coefficients. These define a warp from corrected pixel
		// coordinates (xDst, yDst) to uncorrected pixel coordinates (xSrc, ySrc) for
		// each plane P as follows:
		//
		// Let kr0 = fRadParams [P][0]
		//	   kr1 = fRadParams [P][1]
		//	   kr2 = fRadParams [P][2]
		//	   kr3 = fRadParams [P][3]
		//
		// Let (xCenter, yCenter) be the optical image center (see fCenter, below)
		// expressed in pixel coordinates. Let maxDist be the Euclidean distance (in
		// pixels) from (xCenter, yCenter) to the farthest image corner.
		//
		// First, compute the normalized distance of the corrected pixel position
		// (xDst, yDst) from the image center:
		//
		//	   dx = (xDst - xCenter) / maxDist
		//	   dy = (yDst - yCenter) / maxDist
		//
		//	   r = sqrt (dx^2 + dy^2)
		//
		// Compute the radial correction term:
		//
		//	   t = atan (r)
		//
		//	   rWarp = (kr0 * t) + (kr1 * t^3) + (kr2 * t^5) + (kr3 * t^7)
		//
		//	   ratio = rWarp / r
		//
		//	   dxRad = dx * ratio
		//	   dyRad = dy * ratio
		//
		// Compute the uncorrected pixel position (xSrc, ySrc):
		//
		//	   xSrc = xCenter + (dxRad * maxDist)
		//	   ySrc = yCenter + (dyRad * maxDist)
		//
		// The parameters kr0, kr1, kr2, and kr3 must define an increasing radial
		// warp function. Specifically, let w (r) be the radial warp function:
		//
		//	   t = atan (r)
		//
		//	   w (r) = (kr0 * t) + (kr1 * t^3) + (kr2 * t^5) + (kr3 * t^7).
		//
		// w (r) must be an increasing function.

		dng_vector fRadParams [kMaxColorPlanes];

	public:

		dng_warp_params_fisheye ();

		dng_warp_params_fisheye (uint32 planes,
								 const dng_vector radParams [],
								 const dng_point_real64 &fCenter);

		virtual ~dng_warp_params_fisheye ();

		// Overridden methods.

		virtual bool IsRadNOP (uint32 plane) const;

		virtual bool IsTanNOP (uint32 plane) const;

		virtual bool IsValid () const;

		virtual void PropagateToAllPlanes (uint32 totalPlanes);

		virtual real64 Evaluate (uint32 plane,
								 real64 r) const;

		virtual real64 EvaluateRatio (uint32 plane,
									  real64 r2) const;

		virtual dng_point_real64 EvaluateTangential (uint32 plane,
													 real64 r2,
													 const dng_point_real64 &diff,
													 const dng_point_real64 &diff2) const;

		virtual real64 MaxSrcRadiusGap (real64 maxDstGap) const;

		virtual dng_point_real64 MaxSrcTanGap (dng_point_real64 minDst,
											   dng_point_real64 maxDst) const;

		virtual void Dump () const;

	};

/*****************************************************************************/

/// \brief Warp opcode for pinhole perspective (rectilinear) camera model.

class dng_opcode_WarpRectilinear: public dng_opcode
	{

	protected:

		dng_warp_params_rectilinear fWarpParams;

	public:

		dng_opcode_WarpRectilinear (const dng_warp_params_rectilinear &params,
									uint32 flags);

		explicit dng_opcode_WarpRectilinear (dng_stream &stream);

		// Overridden methods.

		virtual bool IsNOP () const;

		virtual bool IsValidForNegative (const dng_negative &negative) const;

		virtual void PutData (dng_stream &stream) const;

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image);

	protected:

		static uint32 ParamBytes (uint32 planes);

	};

/*****************************************************************************/

/// \brief Warp opcode for fisheye camera model.

class dng_opcode_WarpFisheye: public dng_opcode
	{

	protected:

		dng_warp_params_fisheye fWarpParams;

	public:

		dng_opcode_WarpFisheye (const dng_warp_params_fisheye &params,
								uint32 flags);

		explicit dng_opcode_WarpFisheye (dng_stream &stream);

		// Overridden methods.

		virtual bool IsNOP () const;

		virtual bool IsValidForNegative (const dng_negative &negative) const;

		virtual void PutData (dng_stream &stream) const;

		virtual void Apply (dng_host &host,
							dng_negative &negative,
							AutoPtr<dng_image> &image);

	protected:

		static uint32 ParamBytes (uint32 planes);

	};

/*****************************************************************************/

/// \brief Radially-symmetric vignette (peripheral illuminational falloff) correction
/// parameters.

class dng_vignette_radial_params
	{

	public:

		static const uint32 kNumTerms = 5;

	public:

		// Let v be an uncorrected pixel value of a pixel p in linear space.
		//
		// Let r be the Euclidean distance between p and the optical center.
		//
		// Compute corrected pixel value v' = v * g, where g is the gain.
		//
		// Let k0 = fParams [0]
		// Let k1 = fParams [1]
		// Let k2 = fParams [2]
		// Let k3 = fParams [3]
		// Let k4 = fParams [4]
		//
		// Gain g = 1 + (k0 * r^2) + (k1 * r^4) + (k2 * r^6) + (k3 * r^8) + (k4 * r^10)

		std::vector<real64> fParams;

		dng_point_real64 fCenter;

	public:

		dng_vignette_radial_params ();

		dng_vignette_radial_params (const std::vector<real64> &params,
									const dng_point_real64 &center);

		bool IsNOP () const;

		bool IsValid () const;

		// For debugging.

		void Dump () const;

	};

/*****************************************************************************/

/// \brief Radially-symmetric lens vignette correction opcode.

class dng_opcode_FixVignetteRadial: public dng_inplace_opcode
	{

	protected:

		dng_vignette_radial_params fParams;

		uint32 fImagePlanes;

		int64 fSrcOriginH;
		int64 fSrcOriginV;

		int64 fSrcStepH;
		int64 fSrcStepV;

		uint32 fTableInputBits;
		uint32 fTableOutputBits;

		AutoPtr<dng_memory_block> fGainTable;

		AutoPtr<dng_memory_block> fMaskBuffers [kMaxMPThreads];

	public:

		dng_opcode_FixVignetteRadial (const dng_vignette_radial_params &params,
									  uint32 flags);

		explicit dng_opcode_FixVignetteRadial (dng_stream &stream);

		virtual bool IsNOP () const;

		virtual bool IsValidForNegative (const dng_negative &) const;

		virtual void PutData (dng_stream &stream) const;

		virtual uint32 BufferPixelType (uint32 /* imagePixelType */)
			{
			return ttSShort;
			}

		virtual void Prepare (dng_negative &negative,
							  uint32 threadCount,
							  const dng_point &tileSize,
							  const dng_rect &imageBounds,
							  uint32 imagePlanes,
							  uint32 bufferPixelType,
							  dng_memory_allocator &allocator);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &buffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	protected:

		static uint32 ParamBytes ();

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
