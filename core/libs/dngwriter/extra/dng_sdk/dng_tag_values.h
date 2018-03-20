/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_tag_values.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_tag_values__
#define __dng_tag_values__

/*****************************************************************************/

// Values for NewSubFileType tag.

enum
	{

	// The main image data.

	sfMainImage					= 0,

	// Preview image for the primary settings.

	sfPreviewImage				= 1,

	// Preview image for non-primary settings.

	sfAltPreviewImage			= 0x10001

	};

/******************************************************************************/

// Values for PhotometricInterpretation tag.

enum
	{

	piWhiteIsZero 				= 0,
	piBlackIsZero				= 1,
	piRGB						= 2,
	piRGBPalette				= 3,
	piTransparencyMask			= 4,
	piCMYK						= 5,
	piYCbCr						= 6,
	piCIELab					= 8,
	piICCLab					= 9,

	piCFA						= 32803,		// TIFF-EP spec

	piLinearRaw					= 34892

	};

/******************************************************************************/

// Values for PlanarConfiguration tag.

enum
	{

	pcInterleaved				= 1,
	pcPlanar					= 2,

	pcRowInterleaved			= 100000		// Internal use only

	};

/******************************************************************************/

// Values for ExtraSamples tag.

enum
	{

	esUnspecified				= 0,
	esAssociatedAlpha			= 1,
	esUnassociatedAlpha			= 2

	};

/******************************************************************************/

// Values for SampleFormat tag.

enum
	{

	sfUnsignedInteger			= 1,
	sfSignedInteger				= 2,
	sfFloatingPoint				= 3,
	sfUndefined					= 4

	};

/******************************************************************************/

// Values for Compression tag.

enum
	{

	ccUncompressed				= 1,
	ccLZW						= 5,
	ccOldJPEG					= 6,
	ccJPEG						= 7,
	ccDeflate					= 8,
	ccPackBits					= 32773,
	ccOldDeflate				= 32946

	};

/******************************************************************************/

// Values for Predictor tag.

enum
	{

	cpNullPredictor				= 1,
	cpHorizontalDifference		= 2

	};

/******************************************************************************/

// Values for ResolutionUnit tag.

enum
	{

	ruNone						= 1,
	ruInch						= 2,
	ruCM						= 3,
	ruMM						= 4,
	ruMicroM					= 5

	};

/******************************************************************************/

// Values for LightSource tag.

enum
	{

	lsUnknown					=  0,

	lsDaylight					=  1,
	lsFluorescent				=  2,
	lsTungsten					=  3,
	lsFlash						=  4,
	lsFineWeather				=  9,
	lsCloudyWeather				= 10,
	lsShade						= 11,
	lsDaylightFluorescent		= 12,		// D 5700 - 7100K
	lsDayWhiteFluorescent		= 13,		// N 4600 - 5400K
	lsCoolWhiteFluorescent		= 14,		// W 3900 - 4500K
	lsWhiteFluorescent			= 15,		// WW 3200 - 3700K
	lsStandardLightA			= 17,
	lsStandardLightB			= 18,
	lsStandardLightC			= 19,
	lsD55						= 20,
	lsD65						= 21,
	lsD75						= 22,
	lsD50						= 23,
	lsISOStudioTungsten			= 24,

	lsOther						= 255

	};

/******************************************************************************/

// Values for ExposureProgram tag.

enum
	{

	epUnidentified				= 0,
	epManual					= 1,
	epProgramNormal				= 2,
	epAperturePriority			= 3,
	epShutterPriority			= 4,
	epProgramCreative			= 5,
	epProgramAction				= 6,
	epPortraitMode				= 7,
	epLandscapeMode				= 8

	};

/******************************************************************************/

// Values for MeteringMode tag.

enum
	{

	mmUnidentified				= 0,
	mmAverage					= 1,
	mmCenterWeightedAverage		= 2,
	mmSpot						= 3,
	mmMultiSpot					= 4,
	mmPattern					= 5,
	mmPartial					= 6,

	mmOther						= 255

	};

/******************************************************************************/

// CFA color codes from the TIFF/EP specification.

enum ColorKeyCode
	{

	colorKeyRed					= 0,
	colorKeyGreen				= 1,
	colorKeyBlue				= 2,
	colorKeyCyan				= 3,
	colorKeyMagenta				= 4,
	colorKeyYellow				= 5,
	colorKeyWhite				= 6,

	colorKeyMaxEnum				= 0xFF

	};

/*****************************************************************************/

// Values for the ColorimetricReference tag.  It specifies the colorimetric
// reference used for images with PhotometricInterpretation values of CFA
// or LinearRaw.

enum
	{

	// Scene referred (default):

	crSceneReferred				= 0,

	// Output referred using the parameters of the ICC profile PCS.

	crICCProfilePCS				= 1

	};

/*****************************************************************************/

// Values for the ProfileEmbedPolicy tag.

enum
	{

	// Freely embedable and copyable into installations that encounter this
	// profile, so long as the profile is only used to process DNG files.

	pepAllowCopying				= 0,

	// Can be embeded in a DNG for portable processing, but cannot be used
	// to process other files that the profile is not embedded in.

	pepEmbedIfUsed				= 1,

	// Can only be used if installed on the machine processing the file.
	// Note that this only applies to stand-alone profiles.  Profiles that
	// are already embedded inside a DNG file allowed to remain embedded
	// in that DNG, even if the DNG is resaved.

	pepEmbedNever				= 2,

	// No restricts on profile use or embedding.

	pepNoRestrictions			= 3

	};

/*****************************************************************************/

// Values for the PreviewColorSpace tag.

enum PreviewColorSpaceEnum
	{

	previewColorSpace_Unknown		= 0,
	previewColorSpace_GrayGamma22	= 1,
	previewColorSpace_sRGB			= 2,
	previewColorSpace_AdobeRGB      = 3,
	previewColorSpace_ProPhotoRGB	= 4,

	previewColorSpace_LastValid		= previewColorSpace_ProPhotoRGB,

	previewColorSpace_MaxEnum		= 0xFFFFFFFF

	};

/*****************************************************************************/

// TIFF-style byte order markers.

enum
	{

	byteOrderII					= 0x4949,		// 'II'
	byteOrderMM					= 0x4D4D		// 'MM'

	};

/*****************************************************************************/

// "Magic" numbers.

enum
	{

	// DNG related.

	magicTIFF					= 42,			// TIFF (and DNG)
	magicExtendedProfile		= 0x4352,		// 'CR'

	// Other raw formats - included here so the DNG SDK can parse them.

	magicPanasonic				= 85,
	magicOlympusA				= 0x4F52,
	magicOlympusB				= 0x5352

	};

/*****************************************************************************/

// DNG Version numbers

enum
	{

	dngVersion_None				= 0,

	dngVersion_1_0_0_0			= 0x01000000,
	dngVersion_1_1_0_0			= 0x01010000,
	dngVersion_1_2_0_0			= 0x01020000,
	dngVersion_1_3_0_0			= 0x01030000,

	dngVersion_Current			= dngVersion_1_3_0_0,

	dngVersion_SaveDefault		= dngVersion_Current

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
