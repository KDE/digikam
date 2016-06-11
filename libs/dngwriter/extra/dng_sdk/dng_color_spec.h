/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_color_spec.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Class for holding a specific color transform.
*/

#ifndef __dng_color_spec__
#define __dng_color_spec__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_matrix.h"
#include "dng_types.h"
#include "dng_xy_coord.h"

/*****************************************************************************/

/// \brief Compute a 3x3 matrix which maps colors from white point white1 to
/// white point white2
///
/// Uses linearized Bradford adaptation matrix to compute a mapping from
/// colors measured with one white point (white1) to another (white2).

dng_matrix_3by3 MapWhiteMatrix (const dng_xy_coord &white1,
						   		const dng_xy_coord &white2);

/*****************************************************************************/

/// Color transform taking into account white point and camera calibration and
/// individual calibration from DNG negative.

class dng_color_spec
	{

	private:

		uint32 fChannels;

		real64 fTemperature1;
		real64 fTemperature2;

		dng_matrix fColorMatrix1;
		dng_matrix fColorMatrix2;

		dng_matrix fForwardMatrix1;
		dng_matrix fForwardMatrix2;

		dng_matrix fReductionMatrix1;
		dng_matrix fReductionMatrix2;

		dng_matrix fCameraCalibration1;
		dng_matrix fCameraCalibration2;

		dng_matrix fAnalogBalance;

		dng_xy_coord fWhiteXY;

		dng_vector fCameraWhite;
		dng_matrix fCameraToPCS;

	public:

		/// Read calibration info from DNG negative and construct a
		/// dng_color_spec.

		dng_color_spec (const dng_negative &negative,
					    const dng_camera_profile *profile);

		virtual ~dng_color_spec ()
			{
			}

		/// Number of channels used for this color transform. Three
		/// for most cameras.

		uint32 Channels () const
			{
			return fChannels;
			}

		/// Setter for white point. Value is as XY colorspace coordinate.
		/// \param white White point to set as an XY value.

		void SetWhiteXY (const dng_xy_coord &white);

		/// Getter for white point. Value is as XY colorspace coordinate.
		/// \retval XY value of white point.

		const dng_xy_coord & WhiteXY () const;

		/// Return white point in camera native color coordinates.
		/// \retval A dng_vector with components ranging from 0.0 to 1.0
		/// that is normalized such that one component is equal to 1.0 .

		const dng_vector & CameraWhite () const;

		/// Getter for camera to Profile Connection Space color transform.
		/// \retval A transform that takes into account all camera calibration
		/// transforms and white point.

		const dng_matrix & CameraToPCS () const;

		/// Return the XY value to use for SetWhiteXY for a given camera color
		/// space coordinate as the white point.
		/// \param neutral A camera color space value to use for white point.
		/// Components range from 0.0 to 1.0 and should be normalized such that
		/// the largest value is 1.0 .
		/// \retval White point in XY space that makes neutral map to this
		/// XY value as closely as possible.

		dng_xy_coord NeutralToXY (const dng_vector &neutral);

	private:

		dng_matrix FindXYZtoCamera (const dng_xy_coord &white,
									dng_matrix *forwardMatrix = NULL,
									dng_matrix *reductionMatrix = NULL,
									dng_matrix *cameraCalibration = NULL);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
