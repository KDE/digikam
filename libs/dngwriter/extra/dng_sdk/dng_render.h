/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_render.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Classes for conversion of RAW data to final image.
 */

/*****************************************************************************/

#ifndef __dng_render__
#define __dng_render__

/*****************************************************************************/

#include "dng_1d_function.h"
#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_spline.h"
#include "dng_xy_coord.h"

/******************************************************************************/

/// \brief Curve for pre-exposure-compensation adjustment based on noise floor, shadows, and highlight level.

class dng_function_exposure_ramp: public dng_1d_function
	{

	public:

		real64 fSlope;		// Slope of straight segment.

		real64 fBlack;		// Intercept of straight segment.

		real64 fRadius;		// Rounding radius.

		real64 fQScale;		// Quadradic scale.

	public:

		dng_function_exposure_ramp (real64 white,
				   					real64 black,
				   					real64 minBlack);

		virtual real64 Evaluate (real64 x) const;

	};

/******************************************************************************/

/// \brief Exposure compensation curve for a given compensation amount in stops using quadric for roll-off.

class dng_function_exposure_tone: public dng_1d_function
	{

	protected:

		bool fIsNOP;		// Is this a NOP function?

		real64 fSlope;		// Slope for lower part of curve.

		real64 a;			// Quadradic parameters for upper two f-stops.
		real64 b;
		real64 c;

	public:

		dng_function_exposure_tone (real64 exposure);

		/// Returns output value for a given input tone.

		virtual real64 Evaluate (real64 x) const;

	};

/*****************************************************************************/

/// Default ACR3 tone curve.

class dng_tone_curve_acr3_default: public dng_1d_function
	{

	public:

		/// Returns output value for a given input tone.

		virtual real64 Evaluate (real64 x) const;

		/// Returns nearest input value for a given output tone.

		virtual real64 EvaluateInverse (real64 x) const;

		static const dng_1d_function & Get ();

	};

/*****************************************************************************/

/// \brief Encoding gamma curve for a given color space.

class dng_function_gamma_encode: public dng_1d_function
	{

	protected:

		const dng_color_space &fSpace;

	public:

		dng_function_gamma_encode (const dng_color_space &space);

		virtual real64 Evaluate (real64 x) const;

	};

/*****************************************************************************/

/// \brief Class used to render digital negative to displayable image.

class dng_render
	{

	protected:

		dng_host &fHost;

		const dng_negative &fNegative;

		dng_xy_coord fWhiteXY;

		real64 fExposure;

		real64 fShadows;

		const dng_1d_function *fToneCurve;

		const dng_color_space *fFinalSpace;

		uint32 fFinalPixelType;

		uint32 fMaximumSize;

	private:

		AutoPtr<dng_spline_solver> fProfileToneCurve;

	public:

		/// Construct a rendering instance that will be used to convert a given digital negative.
		/// \param host The host to use for memory allocation, progress updates, and abort testing.
		/// \param negative The digital negative to convert to a displayable image.

		dng_render (dng_host &host,
					const dng_negative &negative);

		virtual ~dng_render ()
			{
			}

		/// Set the white point to be used for conversion.
		/// \param white White point to use.

		void SetWhiteXY (const dng_xy_coord &white)
			{
			fWhiteXY = white;
			}

		/// Get the white point to be used for conversion.
		/// \retval White point to use.

		const dng_xy_coord WhiteXY () const
			{
			return fWhiteXY;
			}

		/// Set exposure compensation.
		/// \param exposure Compensation value in stops, positive or negative.

		void SetExposure (real64 exposure)
			{
			fExposure = exposure;
			}

		/// Get exposure compensation.
		/// \retval Compensation value in stops, positive or negative.

		real64 Exposure () const
			{
			return fExposure;
			}

		/// Set shadow clip amount.
		/// \param shadows Shadow clip amount.

		void SetShadows (real64 shadows)
			{
			fShadows = shadows;
			}

		/// Get shadow clip amount.
		/// \retval Shadow clip amount.

		real64 Shadows () const
			{
			return fShadows;
			}

		/// Set custom tone curve for conversion.
		/// \param curve 1D function that defines tone mapping to use during conversion.

		void SetToneCurve (const dng_1d_function &curve)
			{
			fToneCurve = &curve;
			}

		/// Get custom tone curve for conversion.
		/// \retval 1D function that defines tone mapping to use during conversion.

		const dng_1d_function & ToneCurve () const
			{
			return *fToneCurve;
			}

		/// Set final color space in which resulting image data should be represented.
		/// (See dng_color_space.h for possible values.)
		/// \param space Color space to use.

		void SetFinalSpace (const dng_color_space &space)
			{
			fFinalSpace = &space;
			}

		/// Get final color space in which resulting image data should be represented.
		/// \retval Color space to use.

		const dng_color_space & FinalSpace () const
			{
			return *fFinalSpace;
			}

		/// Set pixel type of final image data.
		/// Can be ttByte (default), ttShort, or ttFloat.
		/// \param type Pixel type to use.

		void SetFinalPixelType (uint32 type)
			{
			fFinalPixelType = type;
			}

		/// Get pixel type of final image data.
		/// Can be ttByte (default), ttShort, or ttFloat.
		/// \retval Pixel type to use.

		uint32 FinalPixelType () const
			{
			return fFinalPixelType;
			}

		/// Set maximum dimension, in pixels, of resulting image.
		/// If final image would have either dimension larger than maximum, the larger
		/// of the two dimensions is set to this maximum size and the smaller dimension
		/// is adjusted to preserve aspect ratio.
		/// \param size Maximum size to allow.

		void SetMaximumSize (uint32 size)
			{
			fMaximumSize = size;
			}

		/// Get maximum dimension, in pixels, of resulting image.
		/// If the final image would have either dimension larger than this maximum, the larger
		/// of the two dimensions is set to this maximum size and the smaller dimension
		/// is adjusted to preserve the image's aspect ratio.
		/// \retval Maximum allowed size.

		uint32 MaximumSize () const
			{
			return fMaximumSize;
			}

		/// Actually render a digital negative to a displayable image.
		/// Input digital negative is passed to the constructor of this dng_render class.
		/// \retval The final resulting image.

		virtual dng_image * Render ();

	private:

		// Hidden copy constructor and assignment operator.

		dng_render (const dng_render &render);

		dng_render & operator= (const dng_render &render);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
