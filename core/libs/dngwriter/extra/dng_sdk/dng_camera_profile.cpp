/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_camera_profile.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

#include "dng_camera_profile.h"

#include "dng_assertions.h"
#include "dng_host.h"
#include "dng_exceptions.h"
#include "dng_image_writer.h"
#include "dng_info.h"
#include "dng_parse_utils.h"
#include "dng_tag_codes.h"
#include "dng_tag_types.h"
#include "dng_temperature.h"
#include "dng_xy_coord.h"

/*****************************************************************************/

const char * kProfileName_Embedded = "Embedded";

const char * kAdobeCalibrationSignature = "com.adobe";

/*****************************************************************************/

dng_camera_profile::dng_camera_profile ()

	:	fName ()
	,	fCalibrationIlluminant1 (lsUnknown)
	,	fCalibrationIlluminant2 (lsUnknown)
	,	fColorMatrix1 ()
	,	fColorMatrix2 ()
	,	fForwardMatrix1 ()
	,	fForwardMatrix2 ()
	,	fReductionMatrix1 ()
	,	fReductionMatrix2 ()
	,	fFingerprint ()
	,	fCopyright ()
	,	fEmbedPolicy (pepAllowCopying)
	,	fHueSatDeltas1 ()
	,	fHueSatDeltas2 ()
	,	fLookTable ()
	,	fToneCurve ()
	,	fProfileCalibrationSignature ()
	,	fUniqueCameraModelRestriction ()
	,	fWasReadFromDNG (false)
	,	fWasStubbed (false)

	{

	fToneCurve.SetInvalid ();

	}

/*****************************************************************************/

dng_camera_profile::~dng_camera_profile ()
	{

	}

/*****************************************************************************/

real64 dng_camera_profile::IlluminantToTemperature (uint32 light)
	{

	switch (light)
		{

		case lsStandardLightA:
		case lsTungsten:
			{
			return 2850.0;
			}

		case lsISOStudioTungsten:
			{
			return 3200.0;
			}

		case lsD50:
			{
			return 5000.0;
			}

		case lsD55:
		case lsDaylight:
		case lsFineWeather:
		case lsFlash:
		case lsStandardLightB:
			{
			return 5500.0;
			}

		case lsD65:
		case lsStandardLightC:
		case lsCloudyWeather:
			{
			return 6500.0;
			}

		case lsD75:
		case lsShade:
			{
			return 7500.0;
			}

		case lsDaylightFluorescent:
			{
			return (5700.0 + 7100.0) * 0.5;
			}

		case lsDayWhiteFluorescent:
			{
			return (4600.0 + 5400.0) * 0.5;
			}

		case lsCoolWhiteFluorescent:
		case lsFluorescent:
			{
			return (3900.0 + 4500.0) * 0.5;
			}

		case lsWhiteFluorescent:
			{
			return (3200.0 + 3700.0) * 0.5;
			}

		default:
			{
			return 0.0;
			}

		}

	}

/******************************************************************************/

void dng_camera_profile::NormalizeColorMatrix (dng_matrix &m)
	{

	if (m.NotEmpty ())
		{

		// Find scale factor to normalize the matrix.

		dng_vector coord = m * PCStoXYZ ();

		real64 maxCoord = coord.MaxEntry ();

		if (maxCoord > 0.0 && (maxCoord < 0.99 || maxCoord > 1.01))
			{

			m.Scale (1.0 / maxCoord);

			}

		// Round to four decimal places.

		m.Round (10000);

		}

	}

/******************************************************************************/

void dng_camera_profile::SetColorMatrix1 (const dng_matrix &m)
	{

	fColorMatrix1 = m;

	NormalizeColorMatrix (fColorMatrix1);

	ClearFingerprint ();

	}

/******************************************************************************/

void dng_camera_profile::SetColorMatrix2 (const dng_matrix &m)
	{

	fColorMatrix2 = m;

	NormalizeColorMatrix (fColorMatrix2);

	ClearFingerprint ();

	}

/******************************************************************************/

// Make sure the forward matrix maps to exactly the PCS.

void dng_camera_profile::NormalizeForwardMatrix (dng_matrix &m)
	{

	if (m.NotEmpty ())
		{

		dng_vector cameraOne;

		cameraOne.SetIdentity (m.Cols ());

		dng_vector xyz = m * cameraOne;

		m = PCStoXYZ ().AsDiagonal () *
			Invert (xyz.AsDiagonal ()) *
			m;

		}

	}

/******************************************************************************/

void dng_camera_profile::SetForwardMatrix1 (const dng_matrix &m)
	{

	fForwardMatrix1 = m;

	fForwardMatrix1.Round (10000);

	ClearFingerprint ();

	}

/******************************************************************************/

void dng_camera_profile::SetForwardMatrix2 (const dng_matrix &m)
	{

	fForwardMatrix2 = m;

	fForwardMatrix2.Round (10000);

	ClearFingerprint ();

	}

/*****************************************************************************/

void dng_camera_profile::SetReductionMatrix1 (const dng_matrix &m)
	{

	fReductionMatrix1 = m;

	fReductionMatrix1.Round (10000);

	ClearFingerprint ();

	}

/******************************************************************************/

void dng_camera_profile::SetReductionMatrix2 (const dng_matrix &m)
	{

	fReductionMatrix2 = m;

	fReductionMatrix2.Round (10000);

	ClearFingerprint ();

	}

/*****************************************************************************/

bool dng_camera_profile::HasColorMatrix1 () const
	{

	return fColorMatrix1.Cols () == 3 &&
		   fColorMatrix1.Rows ()  > 1;

	}

/*****************************************************************************/

bool dng_camera_profile::HasColorMatrix2 () const
	{

	return fColorMatrix2.Cols () == 3 &&
		   fColorMatrix2.Rows () == fColorMatrix1.Rows ();

	}

/*****************************************************************************/

void dng_camera_profile::SetHueSatDeltas1 (const dng_hue_sat_map &deltas1)
	{

	fHueSatDeltas1 = deltas1;

	ClearFingerprint ();

	}

/*****************************************************************************/

void dng_camera_profile::SetHueSatDeltas2 (const dng_hue_sat_map &deltas2)
	{

	fHueSatDeltas2 = deltas2;

	ClearFingerprint ();

	}

/*****************************************************************************/

void dng_camera_profile::SetLookTable (const dng_hue_sat_map &table)
	{

	fLookTable = table;

	ClearFingerprint ();

	}

/*****************************************************************************/

static void FingerprintMatrix (dng_md5_printer_stream &printer,
							   const dng_matrix &matrix)
	{

	tag_matrix tag (0, matrix);

	// Tag's Put routine doesn't write the header, only the data

	tag.Put (printer);

	}

/*****************************************************************************/

static void FingerprintHueSatMap (dng_md5_printer_stream &printer,
								  const dng_hue_sat_map &map)
	{

	if (map.IsNull ())
		return;

	uint32 hues;
	uint32 sats;
	uint32 vals;

	map.GetDivisions (hues, sats, vals);

	printer.Put_uint32 (hues);
	printer.Put_uint32 (sats);
	printer.Put_uint32 (vals);

	for (uint32 val = 0; val < vals; val++)
		for (uint32 hue = 0; hue < hues; hue++)
			for (uint32 sat = 0; sat < sats; sat++)
				{

				dng_hue_sat_map::HSBModify modify;

				map.GetDelta (hue, sat, val, modify);

				printer.Put_real32 (modify.fHueShift);
				printer.Put_real32 (modify.fSatScale);
				printer.Put_real32 (modify.fValScale);

				}

	}

/*****************************************************************************/

void dng_camera_profile::CalculateFingerprint () const
	{

	DNG_ASSERT (!fWasStubbed, "CalculateFingerprint on stubbed profile");

	dng_md5_printer_stream printer;

	// MD5 hash is always calculated on little endian data.

	printer.SetLittleEndian ();

	// The data that we fingerprint closely matches that saved
	// by the profile_tag_set class in dng_image_writer.cpp, with
	// the exception of the fingerprint itself.

	if (HasColorMatrix1 ())
		{

		uint32 colorChannels = ColorMatrix1 ().Rows ();

		printer.Put_uint16 ((uint16) fCalibrationIlluminant1);

		FingerprintMatrix (printer, fColorMatrix1);

		if (fForwardMatrix1.Rows () == fColorMatrix1.Cols () &&
			fForwardMatrix1.Cols () == fColorMatrix1.Rows ())
			{

			FingerprintMatrix (printer, fForwardMatrix1);

			}

		if (colorChannels > 3 && fReductionMatrix1.Rows () *
								 fReductionMatrix1.Cols () == colorChannels * 3)
			{

			FingerprintMatrix (printer, fReductionMatrix1);

			}

		if (HasColorMatrix2 ())
			{

			printer.Put_uint16 ((uint16) fCalibrationIlluminant2);

			FingerprintMatrix (printer, fColorMatrix2);

			if (fForwardMatrix2.Rows () == fColorMatrix2.Cols () &&
				fForwardMatrix2.Cols () == fColorMatrix2.Rows ())
				{

				FingerprintMatrix (printer, fForwardMatrix2);

				}

			if (colorChannels > 3 && fReductionMatrix2.Rows () *
									 fReductionMatrix2.Cols () == colorChannels * 3)
				{

				FingerprintMatrix (printer, fReductionMatrix2);

				}

			}

		printer.Put (fName.Get    (),
					 fName.Length ());

		printer.Put (fProfileCalibrationSignature.Get    (),
					 fProfileCalibrationSignature.Length ());

		printer.Put_uint32 (fEmbedPolicy);

		printer.Put (fCopyright.Get    (),
					 fCopyright.Length ());

		bool haveHueSat1 = HueSatDeltas1 ().IsValid ();

		bool haveHueSat2 = HueSatDeltas2 ().IsValid () &&
						   HasColorMatrix2 ();

		if (haveHueSat1)
			{

			FingerprintHueSatMap (printer, fHueSatDeltas1);

			}

		if (haveHueSat2)
			{

			FingerprintHueSatMap (printer, fHueSatDeltas2);

			}

		if (fLookTable.IsValid ())
			{

			FingerprintHueSatMap (printer, fLookTable);

			}

		if (fToneCurve.IsValid ())
			{

			for (uint32 i = 0; i < fToneCurve.fCoord.size (); i++)
				{

				printer.Put_real32 ((real32) fToneCurve.fCoord [i].h);
				printer.Put_real32 ((real32) fToneCurve.fCoord [i].v);

				}

			}

		}

	fFingerprint = printer.Result ();

	}

/******************************************************************************/

bool dng_camera_profile::ValidForwardMatrix (const dng_matrix &m)
	{

	const real64 kThreshold = 0.01;

	if (m.NotEmpty ())
		{

		dng_vector cameraOne;

		cameraOne.SetIdentity (m.Cols ());

		dng_vector xyz = m * cameraOne;

		dng_vector pcs = PCStoXYZ ();

		if (Abs_real64 (xyz [0] - pcs [0]) > kThreshold ||
			Abs_real64 (xyz [1] - pcs [1]) > kThreshold ||
			Abs_real64 (xyz [2] - pcs [2]) > kThreshold)
			{

			return false;

			}

		}

	return true;

	}

/******************************************************************************/

bool dng_camera_profile::IsValid (uint32 channels) const
	{

	// For Monochrome images, we ignore the camera profile.

	if (channels == 1)
		{

		return true;

		}

	// ColorMatrix1 is required for all color images.

	if (fColorMatrix1.Cols () != 3 ||
		fColorMatrix1.Rows () != channels)
		{

		#if qDNGValidate

		ReportError ("ColorMatrix1 is wrong size");

		#endif

		return false;

		}

	// ColorMatrix2 is optional, but it must be valid if present.

	if (fColorMatrix2.Cols () != 0 ||
		fColorMatrix2.Rows () != 0)
		{

		if (fColorMatrix2.Cols () != 3 ||
			fColorMatrix2.Rows () != channels)
			{

			#if qDNGValidate

			ReportError ("ColorMatrix2 is wrong size");

			#endif

			return false;

			}

		}

	// ForwardMatrix1 is optional, but it must be valid if present.

	if (fForwardMatrix1.Cols () != 0 ||
		fForwardMatrix1.Rows () != 0)
		{

		if (fForwardMatrix1.Rows () != 3 ||
			fForwardMatrix1.Cols () != channels)
			{

			#if qDNGValidate

			ReportError ("ForwardMatrix1 is wrong size");

			#endif

			return false;

			}

		// Make sure ForwardMatrix1 does a valid mapping.

		if (!ValidForwardMatrix (fForwardMatrix1))
			{

			#if qDNGValidate

			ReportError ("ForwardMatrix1 does not map equal camera values to XYZ D50");

			#endif

			return false;

			}

		}

	// ForwardMatrix2 is optional, but it must be valid if present.

	if (fForwardMatrix2.Cols () != 0 ||
		fForwardMatrix2.Rows () != 0)
		{

		if (fForwardMatrix2.Rows () != 3 ||
			fForwardMatrix2.Cols () != channels)
			{

			#if qDNGValidate

			ReportError ("ForwardMatrix2 is wrong size");

			#endif

			return false;

			}

		// Make sure ForwardMatrix2 does a valid mapping.

		if (!ValidForwardMatrix (fForwardMatrix2))
			{

			#if qDNGValidate

			ReportError ("ForwardMatrix2 does not map equal camera values to XYZ D50");

			#endif

			return false;

			}

		}

	// ReductionMatrix1 is optional, but it must be valid if present.

	if (fReductionMatrix1.Cols () != 0 ||
		fReductionMatrix1.Rows () != 0)
		{

		if (fReductionMatrix1.Cols () != channels ||
			fReductionMatrix1.Rows () != 3)
			{

			#if qDNGValidate

			ReportError ("ReductionMatrix1 is wrong size");

			#endif

			return false;

			}

		}

	// ReductionMatrix2 is optional, but it must be valid if present.

	if (fReductionMatrix2.Cols () != 0 ||
		fReductionMatrix2.Rows () != 0)
		{

		if (fReductionMatrix2.Cols () != channels ||
			fReductionMatrix2.Rows () != 3)
			{

			#if qDNGValidate

			ReportError ("ReductionMatrix2 is wrong size");

			#endif

			return false;

			}

		}

	// Make sure ColorMatrix1 is invertable.

	try
		{

		if (fReductionMatrix1.NotEmpty ())
			{

			(void) Invert (fColorMatrix1,
						   fReductionMatrix1);

			}

		else
			{

			(void) Invert (fColorMatrix1);

			}

		}

	catch (...)
		{

		#if qDNGValidate

		ReportError ("ColorMatrix1 is not invertable");

		#endif

		return false;

		}

	// Make sure ColorMatrix2 is invertable.

	if (fColorMatrix2.NotEmpty ())
		{

		try
			{

			if (fReductionMatrix2.NotEmpty ())
				{

				(void) Invert (fColorMatrix2,
							   fReductionMatrix2);

				}

			else
				{

				(void) Invert (fColorMatrix2);

				}

			}

		catch (...)
			{

			#if qDNGValidate

			ReportError ("ColorMatrix2 is not invertable");

			#endif

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

bool dng_camera_profile::EqualData (const dng_camera_profile &profile) const
	{

	return fCalibrationIlluminant1		== profile.fCalibrationIlluminant1		&&
		   fCalibrationIlluminant2		== profile.fCalibrationIlluminant2		&&
		   fColorMatrix1				== profile.fColorMatrix1				&&
		   fColorMatrix2				== profile.fColorMatrix2				&&
		   fForwardMatrix1				== profile.fForwardMatrix1			    &&
		   fForwardMatrix2				== profile.fForwardMatrix2			    &&
		   fReductionMatrix1			== profile.fReductionMatrix1			&&
		   fReductionMatrix2			== profile.fReductionMatrix2			&&
		   fHueSatDeltas1				== profile.fHueSatDeltas1				&&
		   fHueSatDeltas2				== profile.fHueSatDeltas2				&&
		   fLookTable					== profile.fLookTable					&&
		   fToneCurve					== profile.fToneCurve					&&
		   fProfileCalibrationSignature == profile.fProfileCalibrationSignature;

	}

/*****************************************************************************/

void dng_camera_profile::ReadHueSatMap (dng_stream &stream,
										dng_hue_sat_map &hueSatMap,
										uint32 hues,
										uint32 sats,
										uint32 vals,
										bool skipSat0)
	{

	hueSatMap.SetDivisions (hues, sats, vals);

	for (uint32 val = 0; val < vals; val++)
		{

		for (uint32 hue = 0; hue < hues; hue++)
			{

			for (uint32 sat = skipSat0 ? 1 : 0; sat < sats; sat++)
				{

				dng_hue_sat_map::HSBModify modify;

				modify.fHueShift = stream.Get_real32 ();
				modify.fSatScale = stream.Get_real32 ();
				modify.fValScale = stream.Get_real32 ();

				hueSatMap.SetDelta (hue, sat, val, modify);

				}

			}

		}

	}

/*****************************************************************************/

void dng_camera_profile::Parse (dng_stream &stream,
								dng_camera_profile_info &profileInfo)
	{

	SetUniqueCameraModelRestriction (profileInfo.fUniqueCameraModel.Get ());

	if (profileInfo.fProfileName.NotEmpty ())
		{

		SetName (profileInfo.fProfileName.Get ());

		}

	SetCopyright (profileInfo.fProfileCopyright.Get ());

	SetEmbedPolicy (profileInfo.fEmbedPolicy);

	SetCalibrationIlluminant1 (profileInfo.fCalibrationIlluminant1);

	SetColorMatrix1 (profileInfo.fColorMatrix1);

	if (profileInfo.fForwardMatrix1.NotEmpty ())
		{

		SetForwardMatrix1 (profileInfo.fForwardMatrix1);

		}

	if (profileInfo.fReductionMatrix1.NotEmpty ())
		{

		SetReductionMatrix1 (profileInfo.fReductionMatrix1);

		}

	if (profileInfo.fColorMatrix2.NotEmpty ())
		{

		SetCalibrationIlluminant2 (profileInfo.fCalibrationIlluminant2);

		SetColorMatrix2 (profileInfo.fColorMatrix2);

		if (profileInfo.fForwardMatrix2.NotEmpty ())
			{

			SetForwardMatrix2 (profileInfo.fForwardMatrix2);

			}

		if (profileInfo.fReductionMatrix2.NotEmpty ())
			{

			SetReductionMatrix2 (profileInfo.fReductionMatrix2);

			}

		}

	SetProfileCalibrationSignature (profileInfo.fProfileCalibrationSignature.Get ());

	if (profileInfo.fHueSatDeltas1Offset != 0 &&
		profileInfo.fHueSatDeltas1Count  != 0)
		{

		TempBigEndian setEndianness (stream, profileInfo.fBigEndian);

		stream.SetReadPosition (profileInfo.fHueSatDeltas1Offset);

		bool skipSat0 = (profileInfo.fHueSatDeltas1Count == profileInfo.fProfileHues *
														   (profileInfo.fProfileSats - 1) *
														    profileInfo.fProfileVals * 3);

		ReadHueSatMap (stream,
					   fHueSatDeltas1,
					   profileInfo.fProfileHues,
					   profileInfo.fProfileSats,
					   profileInfo.fProfileVals,
					   skipSat0);

		}

	if (profileInfo.fHueSatDeltas2Offset != 0 &&
		profileInfo.fHueSatDeltas2Count  != 0)
		{

		TempBigEndian setEndianness (stream, profileInfo.fBigEndian);

		stream.SetReadPosition (profileInfo.fHueSatDeltas2Offset);

		bool skipSat0 = (profileInfo.fHueSatDeltas2Count == profileInfo.fProfileHues *
														   (profileInfo.fProfileSats - 1) *
														    profileInfo.fProfileVals * 3);

		ReadHueSatMap (stream,
					   fHueSatDeltas2,
					   profileInfo.fProfileHues,
					   profileInfo.fProfileSats,
					   profileInfo.fProfileVals,
					   skipSat0);

		}

	if (profileInfo.fLookTableOffset != 0 &&
		profileInfo.fLookTableCount  != 0)
		{

		TempBigEndian setEndianness (stream, profileInfo.fBigEndian);

		stream.SetReadPosition (profileInfo.fLookTableOffset);

		bool skipSat0 = (profileInfo.fLookTableCount == profileInfo.fLookTableHues *
													   (profileInfo.fLookTableSats - 1) *
														profileInfo.fLookTableVals * 3);

		ReadHueSatMap (stream,
					   fLookTable,
					   profileInfo.fLookTableHues,
					   profileInfo.fLookTableSats,
					   profileInfo.fLookTableVals,
					   skipSat0);

		}

	if ((profileInfo.fToneCurveCount & 1) == 0)
		{

		TempBigEndian setEndianness (stream, profileInfo.fBigEndian);

		stream.SetReadPosition (profileInfo.fToneCurveOffset);

		uint32 points = profileInfo.fToneCurveCount / 2;

		fToneCurve.fCoord.resize (points);

		for (size_t i = 0; i < points; i++)
			{

			dng_point_real64 point;

			point.h = stream.Get_real32 ();
			point.v = stream.Get_real32 ();

			fToneCurve.fCoord [i] = point;

			}

		}

	}

/*****************************************************************************/

bool dng_camera_profile::ParseExtended (dng_stream &stream)
	{

	try
		{

		dng_camera_profile_info profileInfo;

		if (!profileInfo.ParseExtended (stream))
			{
			return false;
			}

		Parse (stream, profileInfo);

		return true;

		}

	catch (...)
		{

		// Eat parsing errors.

		}

	return false;

	}

/*****************************************************************************/

void dng_camera_profile::SetFourColorBayer ()
	{

	uint32 j;

	if (!IsValid (3))
		{
		ThrowProgramError ();
		}

	if (fColorMatrix1.NotEmpty ())
		{

		dng_matrix m (4, 3);

		for (j = 0; j < 3; j++)
			{
			m [0] [j] = fColorMatrix1 [0] [j];
			m [1] [j] = fColorMatrix1 [1] [j];
			m [2] [j] = fColorMatrix1 [2] [j];
			m [3] [j] = fColorMatrix1 [1] [j];
			}

		fColorMatrix1 = m;

		}

	if (fColorMatrix2.NotEmpty ())
		{

		dng_matrix m (4, 3);

		for (j = 0; j < 3; j++)
			{
			m [0] [j] = fColorMatrix2 [0] [j];
			m [1] [j] = fColorMatrix2 [1] [j];
			m [2] [j] = fColorMatrix2 [2] [j];
			m [3] [j] = fColorMatrix2 [1] [j];
			}

		fColorMatrix2 = m;

		}

	fReductionMatrix1.Clear ();
	fReductionMatrix2.Clear ();

	fForwardMatrix1.Clear ();
	fForwardMatrix2.Clear ();

	}

/*****************************************************************************/

dng_hue_sat_map * dng_camera_profile::HueSatMapForWhite (const dng_xy_coord &white) const
	{

	if (fHueSatDeltas1.IsValid ())
		{

		// If we only have the first table, just use it for any color temperature.

		if (!fHueSatDeltas2.IsValid ())
			{

			return new dng_hue_sat_map (fHueSatDeltas1);

			}

		// Else we need to interpolate based on color temperature.

		real64 temperature1 = CalibrationTemperature1 ();
		real64 temperature2 = CalibrationTemperature2 ();

		if (temperature1 <= 0.0 ||
			temperature2 <= 0.0 ||
			temperature1 == temperature2)
			{

			return new dng_hue_sat_map (fHueSatDeltas1);

			}

		bool reverseOrder = temperature1 > temperature2;

		if (reverseOrder)
			{
			real64 temp  = temperature1;
			temperature1 = temperature2;
			temperature2 = temp;
			}

		// Convert to temperature/offset space.

		dng_temperature td (white);

		// Find fraction to weight the first calibration.

		real64 g;

		if (td.Temperature () <= temperature1)
			g = 1.0;

		else if (td.Temperature () >= temperature2)
			g = 0.0;

		else
			{

			real64 invT = 1.0 / td.Temperature ();

			g = (invT                 - (1.0 / temperature2)) /
				((1.0 / temperature1) - (1.0 / temperature2));

			}

		// Fix up if we swapped the order.

		if (reverseOrder)
			{
			g = 1.0 - g;
			}

		// Do the interpolation.

		return dng_hue_sat_map::Interpolate (HueSatDeltas1 (),
											 HueSatDeltas2 (),
											 g);

		}

	return NULL;

	}

/*****************************************************************************/

void dng_camera_profile::Stub ()
	{

	(void) Fingerprint ();

	dng_hue_sat_map nullTable;

	fHueSatDeltas1 = nullTable;
	fHueSatDeltas2 = nullTable;

	fLookTable = nullTable;

	fToneCurve.SetInvalid ();

	fWasStubbed = true;

	}

/*****************************************************************************/

void SplitCameraProfileName (const dng_string &name,
							 dng_string &baseName,
							 int32 &version)
	{

	baseName = name;

	version = 0;

	uint32 len = baseName.Length ();

	if (len > 5 && baseName.EndsWith (" beta"))
		{

		baseName.Truncate (len - 5);

		version += -10;

		}

	else if (len > 7)
		{

		char lastChar = name.Get () [len - 1];

		if (lastChar >= '0' && lastChar <= '9')
			{

			dng_string temp = name;

			temp.Truncate (len - 1);

			if (temp.EndsWith (" beta "))
				{

				baseName.Truncate (len - 7);

				version += ((int32) (lastChar - '0')) - 10;

				}

			}

		}

	len = baseName.Length ();

	if (len > 3)
		{

		char lastChar = name.Get () [len - 1];

		if (lastChar >= '0' && lastChar <= '9')
			{

			dng_string temp = name;

			temp.Truncate (len - 1);

			if (temp.EndsWith (" v"))
				{

				baseName.Truncate (len - 3);

				version += ((int32) (lastChar - '0')) * 100;

				}

			}

		}

	}

/*****************************************************************************/
