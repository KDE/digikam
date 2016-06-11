/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_color_spec.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

#include "dng_color_spec.h"

#include "dng_assertions.h"
#include "dng_camera_profile.h"
#include "dng_exceptions.h"
#include "dng_matrix.h"
#include "dng_negative.h"
#include "dng_temperature.h"
#include "dng_utils.h"
#include "dng_xy_coord.h"

/*****************************************************************************/

dng_matrix_3by3 MapWhiteMatrix (const dng_xy_coord &white1,
						        const dng_xy_coord &white2)
	{

	// Use the linearized Bradford adaptation matrix.

	dng_matrix_3by3 Mb ( 0.8951,  0.2664, -0.1614,
		 		        -0.7502,  1.7135,  0.0367,
		  			     0.0389, -0.0685,  1.0296);

	dng_vector_3 w1 = Mb * XYtoXYZ (white1);
	dng_vector_3 w2 = Mb * XYtoXYZ (white2);

	// Negative white coordinates are kind of meaningless.

	w1 [0] = Max_real64 (w1 [0], 0.0);
	w1 [1] = Max_real64 (w1 [1], 0.0);
	w1 [2] = Max_real64 (w1 [2], 0.0);

	w2 [0] = Max_real64 (w2 [0], 0.0);
	w2 [1] = Max_real64 (w2 [1], 0.0);
	w2 [2] = Max_real64 (w2 [2], 0.0);

	// Limit scaling to something reasonable.

	dng_matrix_3by3 A;

	A [0] [0] = Pin_real64 (0.1, w1 [0] > 0.0 ? w2 [0] / w1 [0] : 10.0, 10.0);
	A [1] [1] = Pin_real64 (0.1, w1 [1] > 0.0 ? w2 [1] / w1 [1] : 10.0, 10.0);
	A [2] [2] = Pin_real64 (0.1, w1 [2] > 0.0 ? w2 [2] / w1 [2] : 10.0, 10.0);

	dng_matrix_3by3 B = Invert (Mb) * A * Mb;

	return B;

	}

/******************************************************************************/

dng_color_spec::dng_color_spec (const dng_negative &negative,
							    const dng_camera_profile *profile)

	:	fChannels (negative.ColorChannels ())

	,	fTemperature1 (0.0)
	,	fTemperature2 (0.0)

	,	fColorMatrix1 ()
	,	fColorMatrix2 ()

	,	fForwardMatrix1 ()
	,	fForwardMatrix2 ()

	,	fReductionMatrix1 ()
	,	fReductionMatrix2 ()

	,	fCameraCalibration1 ()
	,	fCameraCalibration2 ()

	,	fAnalogBalance ()

	,	fWhiteXY ()

	,	fCameraWhite ()
	,	fCameraToPCS ()

	{

	if (fChannels > 1)
		{

		if (!profile || !profile->IsValid (fChannels))
			{
			ThrowBadFormat ();
			}

		if (profile->WasStubbed ())
			{
			ThrowProgramError ("Using stubbed profile");
			}

		fTemperature1 = profile->CalibrationTemperature1 ();
		fTemperature2 = profile->CalibrationTemperature2 ();

		fColorMatrix1 = profile->ColorMatrix1 ();
		fColorMatrix2 = profile->ColorMatrix2 ();

		fForwardMatrix1 = profile->ForwardMatrix1 ();
		fForwardMatrix2 = profile->ForwardMatrix2 ();

		fReductionMatrix1 = profile->ReductionMatrix1 ();
		fReductionMatrix2 = profile->ReductionMatrix2 ();

		fCameraCalibration1.SetIdentity (fChannels);
		fCameraCalibration2.SetIdentity (fChannels);

		if (negative. CameraCalibrationSignature () ==
			profile->ProfileCalibrationSignature ())
			{

			if (negative.CameraCalibration1 ().Rows () == fChannels &&
				negative.CameraCalibration1 ().Cols () == fChannels)
				{

				fCameraCalibration1 = negative.CameraCalibration1 ();

				}

			if (negative.CameraCalibration2 ().Rows () == fChannels &&
				negative.CameraCalibration2 ().Cols () == fChannels)
				{

				fCameraCalibration2 = negative.CameraCalibration2 ();

				}

			}

		fAnalogBalance = dng_matrix (fChannels, fChannels);

		for (uint32 j = 0; j < fChannels; j++)
			{

			fAnalogBalance [j] [j] = negative.AnalogBalance (j);

			}

		dng_camera_profile::NormalizeForwardMatrix (fForwardMatrix1);

		fColorMatrix1 = fAnalogBalance * fCameraCalibration1 * fColorMatrix1;

		if (!profile->HasColorMatrix2 () ||
				fTemperature1 <= 0.0 ||
				fTemperature2 <= 0.0 ||
				fTemperature1 == fTemperature2)
			{

			fTemperature1 = 5000.0;
			fTemperature2 = 5000.0;

			fColorMatrix2       = fColorMatrix1;
			fForwardMatrix2     = fForwardMatrix1;
			fReductionMatrix2   = fReductionMatrix1;
			fCameraCalibration2 = fCameraCalibration1;

			}

		else
			{

			dng_camera_profile::NormalizeForwardMatrix (fForwardMatrix2);

			fColorMatrix2 = fAnalogBalance * fCameraCalibration2 * fColorMatrix2;

			// Swap values if temperatures are out of order.

			if (fTemperature1 > fTemperature2)
				{

				real64 temp   = fTemperature1;
				fTemperature1 = fTemperature2;
				fTemperature2 = temp;

				dng_matrix T  = fColorMatrix1;
				fColorMatrix1 = fColorMatrix2;
				fColorMatrix2 = T;

				T               = fForwardMatrix1;
				fForwardMatrix1 = fForwardMatrix2;
				fForwardMatrix2 = T;

				T                 = fReductionMatrix1;
				fReductionMatrix1 = fReductionMatrix2;
				fReductionMatrix2 = T;

				T                   = fCameraCalibration1;
				fCameraCalibration1 = fCameraCalibration2;
				fCameraCalibration2 = T;

				}

			}

		}

	}

/*****************************************************************************/

dng_matrix dng_color_spec::FindXYZtoCamera (const dng_xy_coord &white,
											dng_matrix *forwardMatrix,
									        dng_matrix *reductionMatrix,
											dng_matrix *cameraCalibration)
	{

	// Convert to temperature/offset space.

	dng_temperature td (white);

	// Find fraction to weight the first calibration.

	real64 g;

	if (td.Temperature () <= fTemperature1)
		g = 1.0;

	else if (td.Temperature () >= fTemperature2)
		g = 0.0;

	else
		{

		real64 invT = 1.0 / td.Temperature ();

		g = (invT                  - (1.0 / fTemperature2)) /
		    ((1.0 / fTemperature1) - (1.0 / fTemperature2));

		}

	// Interpolate the color matrix.

	dng_matrix colorMatrix;

	if (g >= 1.0)
		colorMatrix = fColorMatrix1;

	else if (g <= 0.0)
		colorMatrix = fColorMatrix2;

	else
		colorMatrix = (g      ) * fColorMatrix1 +
					  (1.0 - g) * fColorMatrix2;

	// Interpolate forward matrix, if any.

	if (forwardMatrix)
		{

		bool has1 = fForwardMatrix1.NotEmpty ();
		bool has2 = fForwardMatrix2.NotEmpty ();

		if (has1 && has2)
			{

			if (g >= 1.0)
				*forwardMatrix = fForwardMatrix1;

			else if (g <= 0.0)
				*forwardMatrix = fForwardMatrix2;

			else
				*forwardMatrix = (g      ) * fForwardMatrix1 +
								 (1.0 - g) * fForwardMatrix2;

			}

		else if (has1)
			{

			*forwardMatrix = fForwardMatrix1;

			}

		else if (has2)
			{

			*forwardMatrix = fForwardMatrix2;

			}

		else
			{

			forwardMatrix->Clear ();

			}

		}

	// Interpolate reduction matrix, if any.

	if (reductionMatrix)
		{

		bool has1 = fReductionMatrix1.NotEmpty ();
		bool has2 = fReductionMatrix2.NotEmpty ();

		if (has1 && has2)
			{

			if (g >= 1.0)
				*reductionMatrix = fReductionMatrix1;

			else if (g <= 0.0)
				*reductionMatrix = fReductionMatrix2;

			else
				*reductionMatrix = (g      ) * fReductionMatrix1 +
								   (1.0 - g) * fReductionMatrix2;

			}

		else if (has1)
			{

			*reductionMatrix = fReductionMatrix1;

			}

		else if (has2)
			{

			*reductionMatrix = fReductionMatrix2;

			}

		else
			{

			reductionMatrix->Clear ();

			}

		}

	// Interpolate camera calibration matrix.

	if (cameraCalibration)
		{

		if (g >= 1.0)
			*cameraCalibration = fCameraCalibration1;

		else if (g <= 0.0)
			*cameraCalibration = fCameraCalibration2;

		else
			*cameraCalibration = (g      ) * fCameraCalibration1 +
							     (1.0 - g) * fCameraCalibration2;

		}

	// Return the interpolated color matrix.

	return colorMatrix;

	}

/*****************************************************************************/

void dng_color_spec::SetWhiteXY (const dng_xy_coord &white)
	{

	fWhiteXY = white;

	// Deal with monochrome cameras.

	if (fChannels == 1)
		{

		fCameraWhite.SetIdentity (1);

		fCameraToPCS = PCStoXYZ ().AsColumn ();

		return;

		}

	// Interpolate an matric values for this white point.

	dng_matrix colorMatrix;
	dng_matrix forwardMatrix;
	dng_matrix reductionMatrix;
	dng_matrix cameraCalibration;

	colorMatrix = FindXYZtoCamera (fWhiteXY,
								   &forwardMatrix,
								   &reductionMatrix,
								   &cameraCalibration);

	// Find the camera white values.

	fCameraWhite = colorMatrix * XYtoXYZ (fWhiteXY);

	real64 whiteScale = 1.0 / MaxEntry (fCameraWhite);

	for (uint32 j = 0; j < fChannels; j++)
		{

		// We don't support non-positive values for camera neutral values.

		fCameraWhite [j] = Pin_real64 (0.001,
									   whiteScale * fCameraWhite [j],
									   1.0);

		}

	// If we have a forward matrix, then just use that.

	if (forwardMatrix.NotEmpty ())
		{

		dng_matrix individualToReference = Invert (fAnalogBalance * cameraCalibration);

		dng_vector refCameraWhite = individualToReference * fCameraWhite;

		fCameraToPCS = forwardMatrix *
					   Invert (refCameraWhite.AsDiagonal ()) *
					   individualToReference;

		}

	// Else we need to use the adapt in XYZ method.

	else
		{

		dng_matrix pcsToCamera = colorMatrix * MapWhiteMatrix (PCStoXY (), fWhiteXY);

		// Scale matrix so PCS white can just be reached when the first camera
		// channel saturates.

		real64 scale = MaxEntry (pcsToCamera * PCStoXYZ ());

		pcsToCamera = (1.0 / scale) * pcsToCamera;

		// Invert this matrix.  Note that if there are more than three
		// camera channels, this inversion is non-unique.

		fCameraToPCS = Invert (pcsToCamera, reductionMatrix);

		}

	}

/*****************************************************************************/

const dng_xy_coord & dng_color_spec::WhiteXY () const
	{

	DNG_ASSERT (fWhiteXY.IsValid (), "Using invalid WhiteXY");

	return fWhiteXY;

	}

/*****************************************************************************/

const dng_vector & dng_color_spec::CameraWhite () const
	{

	DNG_ASSERT (fCameraWhite.NotEmpty (), "Using invalid CameraWhite");

	return fCameraWhite;

	}

/*****************************************************************************/

const dng_matrix & dng_color_spec::CameraToPCS () const
	{

	DNG_ASSERT (fCameraToPCS.NotEmpty (), "Using invalid CameraToPCS");

	return fCameraToPCS;

	}

/*****************************************************************************/

dng_xy_coord dng_color_spec::NeutralToXY (const dng_vector &neutral)
	{

	const uint32 kMaxPasses = 30;

	if (fChannels == 1)
		{

		return PCStoXY ();

		}

	dng_xy_coord last = D50_xy_coord ();

	for (uint32 pass = 0; pass < kMaxPasses; pass++)
		{

		dng_matrix xyzToCamera = FindXYZtoCamera (last);

		dng_xy_coord next = XYZtoXY (Invert (xyzToCamera) * neutral);

		if (Abs_real64 (next.x - last.x) +
			Abs_real64 (next.y - last.y) < 0.0000001)
			{

			return next;

			}

		// If we reach the limit without converging, we are most likely
		// in a two value oscillation.  So take the average of the last
		// two estimates and give up.

		if (pass == kMaxPasses - 1)
			{

			next.x = (last.x + next.x) * 0.5;
			next.y = (last.y + next.y) * 0.5;

			}

		last = next;

		}

	return last;

	}

/*****************************************************************************/
