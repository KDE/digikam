/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_exif.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_exif.h"

#include "dng_tag_codes.h"
#include "dng_tag_types.h"
#include "dng_parse_utils.h"
#include "dng_globals.h"
#include "dng_exceptions.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_exif::dng_exif ()

	:	fImageDescription ()
	,	fMake             ()
	,	fModel            ()
	,	fSoftware         ()
	,	fArtist           ()
	,	fCopyright        ()
	,	fCopyright2       ()
	,	fUserComment      ()

	,	fDateTime            ()
	,	fDateTimeStorageInfo ()

	,	fDateTimeOriginal  ()
	,	fDateTimeOriginalStorageInfo ()

	,	fDateTimeDigitized ()
	,	fDateTimeDigitizedStorageInfo ()

	,	fTIFF_EP_StandardID (0)
	,	fExifVersion        (0)
	,	fFlashPixVersion	(0)

	,	fExposureTime      ()
	,	fFNumber           ()
	,	fShutterSpeedValue ()
	,	fApertureValue     ()
	,	fBrightnessValue   ()
	,	fExposureBiasValue ()
	,	fMaxApertureValue  ()
	,	fFocalLength       ()
	,	fDigitalZoomRatio  ()
	,	fExposureIndex     ()
	,	fSubjectDistance   ()
	,	fGamma             ()

	,	fBatteryLevelR ()
	,	fBatteryLevelA ()

	,	fExposureProgram  	  (0xFFFFFFFF)
	,	fMeteringMode     	  (0xFFFFFFFF)
	,	fLightSource      	  (0xFFFFFFFF)
	,	fFlash			  	  (0xFFFFFFFF)
	,	fFlashMask 			  (0x0000FFFF)
	,	fSensingMethod    	  (0xFFFFFFFF)
	,	fColorSpace       	  (0xFFFFFFFF)
	,	fFileSource       	  (0xFFFFFFFF)
	,	fSceneType		  	  (0xFFFFFFFF)
	,	fCustomRendered   	  (0xFFFFFFFF)
	,	fExposureMode	  	  (0xFFFFFFFF)
	,	fWhiteBalance     	  (0xFFFFFFFF)
	,	fSceneCaptureType 	  (0xFFFFFFFF)
	,	fGainControl 		  (0xFFFFFFFF)
	,	fContrast 			  (0xFFFFFFFF)
	,	fSaturation 		  (0xFFFFFFFF)
	,	fSharpness 			  (0xFFFFFFFF)
	,	fSubjectDistanceRange (0xFFFFFFFF)
	,	fSelfTimerMode		  (0xFFFFFFFF)
	,	fImageNumber		  (0xFFFFFFFF)

	,	fFocalLengthIn35mmFilm (0)

	,	fSubjectAreaCount (0)

	,	fComponentsConfiguration (0)

	,	fCompresssedBitsPerPixel ()

	,	fPixelXDimension (0)
	,	fPixelYDimension (0)

	,	fFocalPlaneXResolution ()
	,	fFocalPlaneYResolution ()

	,	fFocalPlaneResolutionUnit (0xFFFFFFFF)

	,	fCFARepeatPatternRows (0)
	,	fCFARepeatPatternCols (0)

	,	fImageUniqueID ()

	,	fGPSVersionID        (0)
	,	fGPSLatitudeRef      ()
	,	fGPSLongitudeRef     ()
	,	fGPSAltitudeRef      (0xFFFFFFFF)
	,	fGPSAltitude         ()
	,	fGPSSatellites       ()
	,	fGPSStatus           ()
	,	fGPSMeasureMode      ()
	,	fGPSDOP              ()
	,	fGPSSpeedRef         ()
	,	fGPSSpeed            ()
	,	fGPSTrackRef         ()
	,	fGPSTrack            ()
	,	fGPSImgDirectionRef  ()
	,	fGPSImgDirection     ()
	,	fGPSMapDatum         ()
	,	fGPSDestLatitudeRef  ()
	,	fGPSDestLongitudeRef ()
	,	fGPSDestBearingRef   ()
	,	fGPSDestBearing      ()
	,	fGPSDestDistanceRef  ()
	,	fGPSDestDistance     ()
	,	fGPSProcessingMethod ()
	,	fGPSAreaInformation  ()
	,	fGPSDateStamp        ()
	,	fGPSDifferential     (0xFFFFFFFF)

	,	fInteroperabilityIndex ()

	,	fInteroperabilityVersion (0)

	,	fRelatedImageFileFormat ()

	,	fRelatedImageWidth  (0)
	,	fRelatedImageLength (0)

	,	fCameraSerialNumber ()

	,	fLensID           ()
	,	fLensName         ()
	,	fLensSerialNumber ()

	,	fFlashCompensation ()

	,	fOwnerName ()
	,	fFirmware  ()

	{

	uint32 j;
	uint32 k;

	fISOSpeedRatings [0] = 0;
	fISOSpeedRatings [1] = 0;
	fISOSpeedRatings [2] = 0;

	for (j = 0; j < kMaxCFAPattern; j++)
		for (k = 0; k < kMaxCFAPattern; k++)
			{
			fCFAPattern [j] [k] = 255;
			}

	}

/*****************************************************************************/

dng_exif::~dng_exif ()
	{

	}

/*****************************************************************************/

dng_exif * dng_exif::Clone () const
	{

	dng_exif *result = new dng_exif (*this);

	if (!result)
		{
		ThrowMemoryFull ();
		}

	return result;

	}

/*****************************************************************************/

// Fix up common errors and rounding issues with EXIF exposure times.

real64 dng_exif::SnapExposureTime (real64 et)
	{

	// Protection against invalid values.

	if (et <= 0.0)
		return 0.0;

	// If near a standard shutter speed, snap to it.

	static const real64 kStandardSpeed [] =
		{
		30.0,
		25.0,
		20.0,
		15.0,
		13.0,
		10.0,
		8.0,
		6.0,
		5.0,
		4.0,
		3.2,
		3.0,
		2.5,
		2.0,
		1.6,
		1.5,
		1.3,
		1.0,
		0.8,
		0.7,
		0.6,
		0.5,
		0.4,
		0.3,
		1.0 / 4.0,
		1.0 / 5.0,
		1.0 / 6.0,
		1.0 / 8.0,
		1.0 / 10.0,
		1.0 / 13.0,
		1.0 / 15.0,
		1.0 / 20.0,
		1.0 / 25.0,
		1.0 / 30.0,
		1.0 / 40.0,
		1.0 / 45.0,
		1.0 / 50.0,
		1.0 / 60.0,
		1.0 / 80.0,
		1.0 / 90.0,
		1.0 / 100.0,
		1.0 / 125.0,
		1.0 / 160.0,
		1.0 / 180.0,
		1.0 / 200.0,
		1.0 / 250.0,
		1.0 / 320.0,
		1.0 / 350.0,
		1.0 / 400.0,
		1.0 / 500.0,
		1.0 / 640.0,
		1.0 / 750.0,
		1.0 / 800.0,
		1.0 / 1000.0,
		1.0 / 1250.0,
		1.0 / 1500.0,
		1.0 / 1600.0,
		1.0 / 2000.0,
		1.0 / 2500.0,
		1.0 / 3000.0,
		1.0 / 3200.0,
		1.0 / 4000.0,
		1.0 / 5000.0,
		1.0 / 6000.0,
		1.0 / 6400.0,
		1.0 / 8000.0,
		1.0 / 10000.0,
		1.0 / 12000.0,
		1.0 / 12800.0,
		1.0 / 16000.0
		};

	uint32 count = sizeof (kStandardSpeed    ) /
				   sizeof (kStandardSpeed [0]);

	for (uint32 fudge = 0; fudge <= 1; fudge++)
		{

		real64 testSpeed = et;

		if (fudge == 1)
			{

			// Often APEX values are rounded to a power of two,
			// which results in non-standard shutter speeds.

			if (et >= 0.1)
				{

				// No fudging slower than 1/10 second

				break;

				}

			else if (et >= 0.01)
				{

				// Between 1/10 and 1/100 the commonly misrounded
				// speeds are 1/15, 1/30, 1/60, which are often encoded as
				// 1/16, 1/32, 1/64.  Try fudging and see if we get
				// near a standard speed.

				testSpeed *= 16.0 / 15.0;

				}

			else
				{

				// Faster than 1/100, the commonly misrounded
				// speeds are 1/125, 1/250, 1/500, etc., which
				// are often encoded as 1/128, 1/256, 1/512.

				testSpeed *= 128.0 / 125.0;

				}

			}

		for (uint32 index = 0; index < count; index++)
			{

			if (testSpeed >= kStandardSpeed [index] * 0.98 &&
				testSpeed <= kStandardSpeed [index] * 1.02)
				{

				return kStandardSpeed [index];

				}

			}

		}

	// We are not near any standard speeds.  Round the non-standard value to something
	// that looks reasonable.

	if (et >= 10.0)
		{

		// Round to nearest second.

		et = floor (et + 0.5);

		}

	else if (et >= 0.5)
		{

		// Round to nearest 1/10 second

		et = floor (et * 10.0 + 0.5) * 0.1;

		}

	else if (et >= 1.0 / 20.0)
		{

		// Round to an exact inverse.

		et = 1.0 / floor (1.0 / et + 0.5);

		}

	else if (et >= 1.0 / 130.0)
		{

		// Round inverse to multiple of 5

		et = 0.2 / floor (0.2 / et + 0.5);

		}

	else if (et >= 1.0 / 750.0)
		{

		// Round inverse to multiple of 10

		et = 0.1 / floor (0.1 / et + 0.5);

		}

	else if (et >= 1.0 / 1300.0)
		{

		// Round inverse to multiple of 50

		et = 0.02 / floor (0.02 / et + 0.5);

		}

	else if (et >= 1.0 / 15000.0)
		{

		// Round inverse to multiple of 100

		et = 0.01 / floor (0.01 / et + 0.5);

		}

	else
		{

		// Round inverse to multiple of 1000

		et = 0.001 / floor (0.001 / et + 0.5);

		}

	return et;

	}

/*****************************************************************************/

void dng_exif::SetExposureTime (real64 et, bool snap)
	{

	fExposureTime.Clear ();

	fShutterSpeedValue.Clear ();

	if (snap)
		{

		et = SnapExposureTime (et);

		}

	if (et >= 1.0 / 32768.0 && et <= 32768.0)
		{

		if (et >= 100.0)
			{

			fExposureTime.Set_real64 (et, 1);

			}

		else if (et >= 1.0)
			{

			fExposureTime.Set_real64 (et, 10);

			fExposureTime.ReduceByFactor (10);

			}

		else if (et <= 0.1)
			{

			fExposureTime = dng_urational (1, Round_uint32 (1.0 / et));

			}

		else
			{

			fExposureTime.Set_real64 (et, 100);

			fExposureTime.ReduceByFactor (10);

			for (uint32 f = 2; f <= 9; f++)
				{

				real64 z = 1.0 / (real64) f / et;

				if (z >= 0.99 && z <= 1.01)
					{

					fExposureTime = dng_urational (1, f);

					break;

					}

				}

			}

		// Now mirror this value to the ShutterSpeedValue field.

		et = fExposureTime.As_real64 ();

		fShutterSpeedValue.Set_real64 (-log (et) / log (2.0), 1000000);

		fShutterSpeedValue.ReduceByFactor (10);
		fShutterSpeedValue.ReduceByFactor (10);
		fShutterSpeedValue.ReduceByFactor (10);
		fShutterSpeedValue.ReduceByFactor (10);
		fShutterSpeedValue.ReduceByFactor (10);
		fShutterSpeedValue.ReduceByFactor (10);

		}

	}

/*****************************************************************************/

void dng_exif::SetShutterSpeedValue (real64 ss)
	{

	if (fExposureTime.NotValid ())
		{

		real64 et = pow (2.0, -ss);

		SetExposureTime (et, true);

		}

	}

/******************************************************************************/

dng_urational dng_exif::EncodeFNumber (real64 fs)
	{

	dng_urational y;

	if (fs > 10.0)
		{

		y.Set_real64 (fs, 1);

		}

	else
		{

		y.Set_real64 (fs, 10);

		y.ReduceByFactor (10);

		}

	return y;

	}

/*****************************************************************************/

void dng_exif::SetFNumber (real64 fs)
	{

	fFNumber.Clear ();

	fApertureValue.Clear ();

	if (fs >= 1.0 && fs <= 32768.0)
		{

		fFNumber = EncodeFNumber (fs);

		// Now mirror this value to the ApertureValue field.

		real64 av = 2.0 * log (fFNumber.As_real64 ()) / log (2.0);

		if (av >= 0.0 && av <= 99.99)
			{

			fApertureValue.Set_real64 (av, 1000000);

			fApertureValue.ReduceByFactor (10);
			fApertureValue.ReduceByFactor (10);
			fApertureValue.ReduceByFactor (10);
			fApertureValue.ReduceByFactor (10);
			fApertureValue.ReduceByFactor (10);
			fApertureValue.ReduceByFactor (10);

			}

		}

	}

/*****************************************************************************/

void dng_exif::SetApertureValue (real64 av)
	{

	if (fFNumber.NotValid ())
		{

		real64 fs = pow (2.0, av * 0.5);

		SetFNumber (fs);

		}

	}

/*****************************************************************************/

void dng_exif::UpdateDateTime (const dng_date_time_info &dt)
	{

	fDateTime = dt;

	}

/*****************************************************************************/

bool dng_exif::ParseTag (dng_stream &stream,
						 dng_shared &shared,
						 uint32 parentCode,
						 bool isMainIFD,
						 uint32 tagCode,
						 uint32 tagType,
						 uint32 tagCount,
						 uint64 tagOffset)
	{

	if (parentCode == 0)
		{

		if (Parse_ifd0 (stream,
		 				shared,
						parentCode,
						tagCode,
						tagType,
						tagCount,
						tagOffset))
			{

			return true;

			}

		}

	if (parentCode == 0 || isMainIFD)
		{

		if (Parse_ifd0_main (stream,
		 				     shared,
						 	 parentCode,
						 	 tagCode,
						 	 tagType,
						 	 tagCount,
						 	 tagOffset))
			{

			return true;

			}

		}

	if (parentCode == 0 ||
		parentCode == tcExifIFD)
		{

		if (Parse_ifd0_exif (stream,
		 				     shared,
						 	 parentCode,
						 	 tagCode,
						 	 tagType,
						 	 tagCount,
						 	 tagOffset))
			{

			return true;

			}

		}

	if (parentCode == tcGPSInfo)
		{

		if (Parse_gps (stream,
		 			   shared,
					   parentCode,
					   tagCode,
					   tagType,
					   tagCount,
					   tagOffset))
			{

			return true;

			}

		}

	if (parentCode == tcInteroperabilityIFD)
		{

		if (Parse_interoperability (stream,
		 			   				shared,
									parentCode,
									tagCode,
									tagType,
									tagCount,
									tagOffset))
			{

			return true;

			}

		}

	return false;

	}

/*****************************************************************************/

// Parses tags that should only appear in IFD 0.

bool dng_exif::Parse_ifd0 (dng_stream &stream,
		 			   	   dng_shared & /* shared */,
						   uint32 parentCode,
						   uint32 tagCode,
						   uint32 tagType,
						   uint32 tagCount,
						   uint64 /* tagOffset */)
	{

	switch (tagCode)
		{

		case tcImageDescription:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fImageDescription);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ImageDescription: ");

				DumpString (fImageDescription);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcMake:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fMake);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Make: ");

				DumpString (fMake);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcModel:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fModel);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Model: ");

				DumpString (fModel);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcSoftware:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fSoftware);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Software: ");

				DumpString (fSoftware);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcDateTime:
			{

			uint64 tagPosition = stream.PositionInOriginalFile ();

			dng_date_time dt;

			if (!ParseDateTimeTag (stream,
								   parentCode,
								   tagCode,
								   tagType,
								   tagCount,
								   dt))
				{
				return false;
				}

			fDateTime.SetDateTime (dt);

			fDateTimeStorageInfo = dng_date_time_storage_info (tagPosition,
															   dng_date_time_format_exif);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("DateTime: ");

				DumpDateTime (fDateTime.DateTime ());

				printf ("\n");

				}

			#endif

			break;

			}

		case tcArtist:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fArtist);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Artist: ");

				DumpString (fArtist);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcCopyright:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseDualStringTag (stream,
								parentCode,
								tagCode,
								tagCount,
								fCopyright,
								fCopyright2);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Copyright: ");

				DumpString (fCopyright);

				if (fCopyright2.Get () [0] != 0)
					{

					printf (" ");

					DumpString (fCopyright2);

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcTIFF_EP_StandardID:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fTIFF_EP_StandardID = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("TIFF/EPStandardID: %u.%u.%u.%u\n",
						(unsigned) b0,
						(unsigned) b1,
						(unsigned) b2,
						(unsigned) b3);
				}

			#endif

			break;

			}

		case tcCameraSerialNumber:
		case tcKodakCameraSerialNumber:		// Kodak uses a very similar tag.
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fCameraSerialNumber);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s: ", LookupTagCode (parentCode, tagCode));

				DumpString (fCameraSerialNumber);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcLensInfo:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			if (!CheckTagCount (parentCode, tagCode, tagCount, 4))
				return false;

			fLensInfo [0] = stream.TagValue_urational (tagType);
			fLensInfo [1] = stream.TagValue_urational (tagType);
			fLensInfo [2] = stream.TagValue_urational (tagType);
			fLensInfo [3] = stream.TagValue_urational (tagType);

			// Some third party software wrote zero rather and undefined values
			// for unknown entries.  Work around this bug.

			for (uint32 j = 0; j < 4; j++)
				{

				if (fLensInfo [j].IsValid () && fLensInfo [j].As_real64 () <= 0.0)
					{

					fLensInfo [j] = dng_urational (0, 0);

					#if qDNGValidate

					ReportWarning ("Zero entry in LensInfo tag--should be undefined");

					#endif

					}

				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("LensInfo: ");

				real64 minFL = fLensInfo [0].As_real64 ();
				real64 maxFL = fLensInfo [1].As_real64 ();

				if (minFL == maxFL)
					printf ("%0.1f mm", minFL);
				else
					printf ("%0.1f-%0.1f mm", minFL, maxFL);

				if (fLensInfo [2].d)
					{

					real64 minFS = fLensInfo [2].As_real64 ();
					real64 maxFS = fLensInfo [3].As_real64 ();

					if (minFS == maxFS)
						printf (" f/%0.1f", minFS);
					else
						printf (" f/%0.1f-%0.1f", minFS, maxFS);

					}

				printf ("\n");

				}

			#endif

			break;

			}

		default:
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

// Parses tags that should only appear in IFD 0 or the main image IFD.

bool dng_exif::Parse_ifd0_main (dng_stream &stream,
		 			   	        dng_shared & /* shared */,
						  	    uint32 parentCode,
						  	    uint32 tagCode,
						  	    uint32 tagType,
						  	    uint32 tagCount,
						  	    uint64 /* tagOffset */)
	{

	switch (tagCode)
		{

		case tcFocalPlaneXResolution:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalPlaneXResolution = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalPlaneXResolution: %0.4f\n",
						fFocalPlaneXResolution.As_real64 ());

				}

			#endif

			break;

			}

		case tcFocalPlaneYResolution:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalPlaneYResolution = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalPlaneYResolution: %0.4f\n",
						fFocalPlaneYResolution.As_real64 ());

				}

			#endif

			break;

			}

		case tcFocalPlaneResolutionUnit:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalPlaneResolutionUnit = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalPlaneResolutionUnit: %s\n",
					    LookupResolutionUnit (fFocalPlaneResolutionUnit));

				}

			#endif

			break;

			}

		case tcSensingMethod:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSensingMethod = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SensingMethod: %s\n",
						LookupSensingMethod (fSensingMethod));

				}

			#endif

			break;

			}

		default:
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

// Parses tags that should only appear in IFD 0 or EXIF IFD.

bool dng_exif::Parse_ifd0_exif (dng_stream &stream,
								dng_shared & /* shared */,
						  	   	uint32 parentCode,
						  	    uint32 tagCode,
						  	    uint32 tagType,
						  	    uint32 tagCount,
						  	    uint64 /* tagOffset */)
	{

	switch (tagCode)
		{

		case tcBatteryLevel:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational, ttAscii);

			if (tagType == ttAscii)
				{

				ParseStringTag (stream,
								parentCode,
								tagCode,
								tagCount,
								fBatteryLevelA);

				}

			else
				{

				CheckTagCount (parentCode, tagCode, tagCount, 1);

				fBatteryLevelR = stream.TagValue_urational (tagType);

				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("BatteryLevel: ");

				if (tagType == ttAscii)
					{

					DumpString (fBatteryLevelA);

					}

				else
					{

					printf ("%0.2f", fBatteryLevelR.As_real64 ());

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcExposureTime:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			dng_urational et = stream.TagValue_urational (tagType);

			SetExposureTime (et.As_real64 (), true);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ExposureTime: ");

				DumpExposureTime (et.As_real64 ());

				printf ("\n");

				}

			if (et.As_real64 () <= 0.0)
				{

				ReportWarning ("The ExposureTime is <= 0");

				}

			#endif

			break;

			}

		case tcFNumber:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			dng_urational fs = stream.TagValue_urational (tagType);

			// Sometimes "unknown" is recorded as zero.

			if (fs.As_real64 () <= 0.0)
				{
				fs.Clear ();
				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FNumber: f/%0.1f\n",
						fs.As_real64 ());

				}

			#endif

			SetFNumber (fs.As_real64 ());

			break;

			}

		case tcExposureProgram:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fExposureProgram = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ExposureProgram: %s\n",
						LookupExposureProgram (fExposureProgram));

				}

			#endif

			break;

			}

		case tcISOSpeedRatings:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1, 3);

			for (uint32 j = 0; j < tagCount && j < 3; j++)
				{

				fISOSpeedRatings [j] = stream.TagValue_uint32 (tagType);

				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ISOSpeedRatings:");

				for (uint32 j = 0; j < tagCount && j < 3; j++)
					{

					printf (" %u", (unsigned) fISOSpeedRatings [j]);

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcTimeZoneOffset:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1, 2);

			dng_time_zone zoneOriginal;

			zoneOriginal.SetOffsetHours (stream.TagValue_int32 (tagType));

			fDateTimeOriginal.SetZone (zoneOriginal);

			// Note that there is no "TimeZoneOffsetDigitized" field, so
			// we assume the same tone zone as the original.

			fDateTimeDigitized.SetZone (zoneOriginal);

			dng_time_zone zoneCurrent;

			if (tagCount >= 2)
				{

				zoneCurrent.SetOffsetHours (stream.TagValue_int32 (tagType));

				fDateTime.SetZone (zoneCurrent);

				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("TimeZoneOffset: DateTimeOriginal = %d",
						(int) zoneOriginal.ExactHourOffset ());

				if (tagCount >= 2)
					{

					printf (", DateTime = %d",
							(int) zoneCurrent.ExactHourOffset ());

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcSelfTimerMode:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSelfTimerMode = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SelfTimerMode: ");

				if (fSelfTimerMode)
					{

					printf ("%u sec", (unsigned) fSelfTimerMode);

					}

				else
					{

					printf ("Off");

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcExifVersion:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fExifVersion = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{

				real64 x = (b0 - '0') * 10.00 +
						   (b1 - '0') *  1.00 +
						   (b2 - '0') *  0.10 +
						   (b3 - '0') *  0.01;

				printf ("ExifVersion: %0.2f\n", x);

				}

			#endif

			break;

			}

		case tcDateTimeOriginal:
			{

			uint64 tagPosition = stream.PositionInOriginalFile ();

			dng_date_time dt;

			if (!ParseDateTimeTag (stream,
								   parentCode,
								   tagCode,
								   tagType,
								   tagCount,
								   dt))
				{
				return false;
				}

			fDateTimeOriginal.SetDateTime (dt);

			fDateTimeOriginalStorageInfo = dng_date_time_storage_info (tagPosition,
																	   dng_date_time_format_exif);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("DateTimeOriginal: ");

				DumpDateTime (fDateTimeOriginal.DateTime ());

				printf ("\n");

				}

			#endif

			break;

			}

		case tcDateTimeDigitized:
			{

			uint64 tagPosition = stream.PositionInOriginalFile ();

			dng_date_time dt;

			if (!ParseDateTimeTag (stream,
								   parentCode,
								   tagCode,
								   tagType,
								   tagCount,
								   dt))
				{
				return false;
				}

			fDateTimeDigitized.SetDateTime (dt);

			fDateTimeDigitizedStorageInfo = dng_date_time_storage_info (tagPosition,
																	    dng_date_time_format_exif);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("DateTimeDigitized: ");

				DumpDateTime (fDateTimeDigitized.DateTime ());

				printf ("\n");

				}

			#endif

			break;

			}

		case tcComponentsConfiguration:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fComponentsConfiguration = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ComponentsConfiguration: %s %s %s %s\n",
						LookupComponent (b0),
						LookupComponent (b1),
						LookupComponent (b2),
						LookupComponent (b3));

				}

			#endif

			break;

			}

		case tcCompressedBitsPerPixel:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fCompresssedBitsPerPixel = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CompressedBitsPerPixel: %0.2f\n",
						fCompresssedBitsPerPixel.As_real64 ());

				}

			#endif

			break;

			}

		case tcShutterSpeedValue:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			dng_srational ss = stream.TagValue_srational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ShutterSpeedValue: ");

				real64 x = pow (2.0, -ss.As_real64 ());

				DumpExposureTime (x);

				printf ("\n");

				}

			// The ExposureTime and ShutterSpeedValue tags should be consistent.

			if (fExposureTime.IsValid ())
				{

				real64 et = fExposureTime.As_real64 ();

				real64 tv1 = -1.0 * log (et) / log (2.0);

				real64 tv2 = ss.As_real64 ();

				// Make sure they are within 0.25 APEX values.

				if (Abs_real64 (tv1 - tv2) > 0.25)
					{

					ReportWarning ("The ExposureTime and ShutterSpeedValue tags have conflicting values");

					}

				}

			#endif

			SetShutterSpeedValue (ss.As_real64 ());

			break;

			}

		case tcApertureValue:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			dng_urational av = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				real64 x = pow (2.0, 0.5 * av.As_real64 ());

				printf ("ApertureValue: f/%0.1f\n", x);

				}

			// The FNumber and ApertureValue tags should be consistent.

			if (fFNumber.IsValid () && av.IsValid ())
				{

				real64 fs = fFNumber.As_real64 ();

				real64 av1 = 2.0 * log (fs) / log (2.0);

				real64 av2 = av.As_real64 ();

				if (Abs_real64 (av1 - av2) > 0.25)
					{

					ReportWarning ("The FNumber and ApertureValue tags have conflicting values");

					}

				}

			#endif

			SetApertureValue (av.As_real64 ());

			break;

			}

		case tcBrightnessValue:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fBrightnessValue = stream.TagValue_srational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("BrightnessValue: %0.2f\n",
						fBrightnessValue.As_real64 ());

				}

			#endif

			break;

			}

		case tcExposureBiasValue:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fExposureBiasValue = stream.TagValue_srational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ExposureBiasValue: %0.2f\n",
						fExposureBiasValue.As_real64 ());

				}

			#endif

			break;

			}

		case tcMaxApertureValue:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fMaxApertureValue = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				real64 x = pow (2.0, 0.5 * fMaxApertureValue.As_real64 ());

				printf ("MaxApertureValue: f/%0.1f\n", x);

				}

			#endif

			break;

			}

		case tcSubjectDistance:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSubjectDistance = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SubjectDistance: %u/%u\n",
						(unsigned) fSubjectDistance.n,
						(unsigned) fSubjectDistance.d);

				}

			#endif

			break;

			}

		case tcMeteringMode:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fMeteringMode = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("MeteringMode: %s\n",
						LookupMeteringMode (fMeteringMode));

				}

			#endif

			break;

			}

		case tcLightSource:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fLightSource = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("LightSource: %s\n",
						LookupLightSource (fLightSource));

				}

			#endif

			break;

			}

		case tcFlash:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFlash = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Flash: %u\n", (unsigned) fFlash);

				if ((fFlash >> 5) & 1)
					{
					printf ("    No flash function\n");
					}

				else
					{

					if (fFlash & 0x1)
						{

						printf ("    Flash fired\n");

						switch ((fFlash >> 1) & 0x3)
							{

							case 2:
								printf ("    Strobe return light not detected\n");
								break;

							case 3:
								printf ("    Strobe return light detected\n");
								break;

							}

						}

					else
						{
						printf ("    Flash did not fire\n");
						}

					switch ((fFlash >> 3) & 0x3)
						{

						case 1:
							printf ("    Compulsory flash firing\n");
							break;

						case 2:
							printf ("    Compulsory flash suppression\n");
							break;

						case 3:
							printf ("    Auto mode\n");
							break;

						}

					if ((fFlash >> 6) & 1)
						{
						printf ("    Red-eye reduction supported\n");
						}

					}

				}

			#endif

			break;

			}

		case tcFocalLength:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalLength = stream.TagValue_urational (tagType);

			// Sometimes "unknown" is recorded as zero.

			if (fFocalLength.As_real64 () <= 0.0)
				{
				fFocalLength.Clear ();
				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalLength: %0.1f mm\n",
						fFocalLength.As_real64 ());

				}

			#endif

			break;

			}

		case tcImageNumber:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fImageNumber = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("ImageNumber: %u\n", (unsigned) fImageNumber);
				}

			#endif

			break;

			}

		case tcExposureIndex:
		case tcExposureIndexExif:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fExposureIndex = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s: ISO %0.1f\n",
						LookupTagCode (parentCode, tagCode),
						fExposureIndex.As_real64 ());

				}

			#endif

			break;

			}

		case tcUserComment:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			ParseEncodedStringTag (stream,
								   parentCode,
								   tagCode,
				    			   tagCount,
				    			   fUserComment);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("UserComment: ");

				DumpString (fUserComment);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcSubsecTime:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			dng_string subsecs;

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							subsecs);

			fDateTime.SetSubseconds (subsecs);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SubsecTime: ");

				DumpString (subsecs);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcSubsecTimeOriginal:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			dng_string subsecs;

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							subsecs);

			fDateTimeOriginal.SetSubseconds (subsecs);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SubsecTimeOriginal: ");

				DumpString (subsecs);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcSubsecTimeDigitized:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			dng_string subsecs;

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							subsecs);

			fDateTimeDigitized.SetSubseconds (subsecs);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SubsecTimeDigitized: ");

				DumpString (subsecs);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcFlashPixVersion:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fFlashPixVersion = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{

				real64 x = (b0 - '0') * 10.00 +
						   (b1 - '0') *  1.00 +
						   (b2 - '0') *  0.10 +
						   (b3 - '0') *  0.01;

				printf ("FlashPixVersion: %0.2f\n", x);

				}

			#endif

			break;

			}

		case tcColorSpace:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fColorSpace = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ColorSpace: %s\n",
						LookupColorSpace (fColorSpace));

				}

			#endif

			break;

			}

		case tcPixelXDimension:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fPixelXDimension = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("PixelXDimension: %u\n", (unsigned) fPixelXDimension);
				}

			#endif

			break;

			}

		case tcPixelYDimension:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fPixelYDimension = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("PixelYDimension: %u\n", (unsigned) fPixelYDimension);
				}

			#endif

			break;

			}

		case tcFocalPlaneXResolutionExif:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalPlaneXResolution = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalPlaneXResolutionExif: %0.4f\n",
						fFocalPlaneXResolution.As_real64 ());

				}

			#endif

			break;

			}

		case tcFocalPlaneYResolutionExif:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalPlaneYResolution = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalPlaneYResolutionExif: %0.4f\n",
						fFocalPlaneYResolution.As_real64 ());

				}

			#endif

			break;

			}

		case tcFocalPlaneResolutionUnitExif:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalPlaneResolutionUnit = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalPlaneResolutionUnitExif: %s\n",
					    LookupResolutionUnit (fFocalPlaneResolutionUnit));

				}

			#endif

			break;

			}

		case tcSensingMethodExif:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSensingMethod = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SensingMethodExif: %s\n",
						LookupSensingMethod (fSensingMethod));

				}

			#endif

			break;

			}

		case tcFileSource:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFileSource = stream.Get_uint8 ();

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FileSource: %s\n",
						LookupFileSource (fFileSource));

				}

			#endif

			break;

			}

		case tcSceneType:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSceneType = stream.Get_uint8 ();

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SceneType: %s\n",
						LookupSceneType (fSceneType));

				}

			#endif

			break;

			}

		case tcCFAPatternExif:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			if (tagCount <= 4)
				{
				return false;
				}

			uint32 cols = stream.Get_uint16 ();
			uint32 rows = stream.Get_uint16 ();

			if (tagCount != 4 + cols * rows)
				{
				return false;
				}

			if (cols < 1 || cols > kMaxCFAPattern ||
				rows < 1 || rows > kMaxCFAPattern)
				{
				return false;
				}

			fCFARepeatPatternCols = cols;
			fCFARepeatPatternRows = rows;

			// Note that the Exif spec stores this array in a different
			// scan order than the TIFF-EP spec.

			for (uint32 j = 0; j < fCFARepeatPatternCols; j++)
				for (uint32 k = 0; k < fCFARepeatPatternRows; k++)
					{

					fCFAPattern [k] [j] = stream.Get_uint8 ();

					}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CFAPatternExif:\n");

				for (uint32 j = 0; j < fCFARepeatPatternRows; j++)
					{

					int32 spaces = 4;

					for (uint32 k = 0; k < fCFARepeatPatternCols; k++)
						{

						while (spaces-- > 0)
							{
							printf (" ");
							}

						const char *name = LookupCFAColor (fCFAPattern [j] [k]);

						spaces = 9 - (int32) strlen (name);

						printf ("%s", name);

						}

					printf ("\n");

					}

				}

			#endif

			break;

			}

		case tcCustomRendered:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fCustomRendered = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CustomRendered: %s\n",
						LookupCustomRendered (fCustomRendered));

				}

			#endif

			break;

			}

		case tcExposureMode:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fExposureMode = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ExposureMode: %s\n",
						LookupExposureMode (fExposureMode));

				}

			#endif

			break;

			}

		case tcWhiteBalance:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fWhiteBalance = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("WhiteBalance: %s\n",
						LookupWhiteBalance (fWhiteBalance));

				}

			#endif

			break;

			}

		case tcDigitalZoomRatio:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fDigitalZoomRatio = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("DigitalZoomRatio: ");

				if (fDigitalZoomRatio.n == 0 ||
					fDigitalZoomRatio.d == 0)
					{

					printf ("Not used\n");

					}

				else
					{

					printf ("%0.2f\n", fDigitalZoomRatio.As_real64 ());

					}

				}

			#endif

			break;

			}

		case tcFocalLengthIn35mmFilm:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fFocalLengthIn35mmFilm = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("FocalLengthIn35mmFilm: %u mm\n",
						(unsigned) fFocalLengthIn35mmFilm);

				}

			#endif

			break;

			}

		case tcSceneCaptureType:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSceneCaptureType = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SceneCaptureType: %s\n",
						LookupSceneCaptureType (fSceneCaptureType));

				}

			#endif

			break;

			}

		case tcGainControl:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fGainControl = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("GainControl: %s\n",
						LookupGainControl (fGainControl));

				}

			#endif

			break;

			}

		case tcContrast:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fContrast = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Contrast: %s\n",
						LookupContrast (fContrast));

				}

			#endif

			break;

			}

		case tcSaturation:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSaturation = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Saturation: %s\n",
						LookupSaturation (fSaturation));

				}

			#endif

			break;

			}

		case tcSharpness:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSharpness = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Sharpness: %s\n",
						LookupSharpness (fSharpness));

				}

			#endif

			break;

			}

		case tcSubjectDistanceRange:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fSubjectDistanceRange = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("SubjectDistanceRange: %s\n",
						LookupSubjectDistanceRange (fSubjectDistanceRange));

				}

			#endif

			break;

			}

		case tcSubjectArea:
		case tcSubjectLocation:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			if (!CheckTagCount (parentCode, tagCode, tagCount, 2, 4))
				{
				return false;
				}

			if (tagCode == tcSubjectLocation)
				{
				CheckTagCount (parentCode, tagCode, tagCount, 2);
				}

			fSubjectAreaCount = tagCount;

			for (uint32 j = 0; j < tagCount; j++)
				{

				fSubjectArea [j] = stream.TagValue_uint32 (tagType);

				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s:", LookupTagCode (parentCode, tagCode));

				for (uint32 j = 0; j < fSubjectAreaCount; j++)
					{

					printf (" %u", (unsigned) fSubjectArea [j]);

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcGamma:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fGamma = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("Gamma: %0.2f\n",
						fGamma.As_real64 ());

				}

			#endif

			break;

			}

		case tcImageUniqueID:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttAscii))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 33))
				return false;

			dng_string s;

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							s);

			if (s.Length () != 32)
				return false;

			dng_fingerprint f;

			for (uint32 j = 0; j < 32; j++)
				{

				char c = ForceUppercase (s.Get () [j]);

				uint32 digit;

				if (c >= '0' && c <= '9')
					{
					digit = c - '0';
					}

				else if (c >= 'A' && c <= 'F')
					{
					digit = c - 'A' + 10;
					}

				else
					return false;

				f.data [j >> 1] *= 16;
				f.data [j >> 1] += (uint8) digit;

				}

			fImageUniqueID = f;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ImageUniqueID: ");

				DumpFingerprint (fImageUniqueID);

				printf ("\n");

				}

			#endif

			break;

			}

		default:
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

// Parses tags that should only appear in GPS IFD

bool dng_exif::Parse_gps (dng_stream &stream,
						  dng_shared & /* shared */,
						  uint32 parentCode,
						  uint32 tagCode,
						  uint32 tagType,
						  uint32 tagCount,
						  uint64 /* tagOffset */)
	{

	switch (tagCode)
		{

		case tcGPSVersionID:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fGPSVersionID = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("GPSVersionID: %u.%u.%u.%u\n",
						(unsigned) b0,
						(unsigned) b1,
						(unsigned) b2,
						(unsigned) b3);
				}

			#endif

			break;

			}

		case tcGPSLatitudeRef:
		case tcGPSLongitudeRef:
		case tcGPSSatellites:
		case tcGPSStatus:
		case tcGPSMeasureMode:
		case tcGPSSpeedRef:
		case tcGPSTrackRef:
		case tcGPSImgDirectionRef:
		case tcGPSMapDatum:
		case tcGPSDestLatitudeRef:
		case tcGPSDestLongitudeRef:
		case tcGPSDestBearingRef:
		case tcGPSDestDistanceRef:
		case tcGPSDateStamp:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttAscii))
				return false;

			dng_string *s;

			switch (tagCode)
				{

				case tcGPSLatitudeRef:
					s = &fGPSLatitudeRef;
					break;

				case tcGPSLongitudeRef:
					s = &fGPSLongitudeRef;
					break;

				case tcGPSSatellites:
					s = &fGPSSatellites;
					break;

				case tcGPSStatus:
					s = &fGPSStatus;
					break;

				case tcGPSMeasureMode:
					s = &fGPSMeasureMode;
					break;

				case tcGPSSpeedRef:
					s = &fGPSSpeedRef;
					break;

				case tcGPSTrackRef:
					s = &fGPSTrackRef;
					break;

				case tcGPSImgDirectionRef:
					s = &fGPSImgDirectionRef;
					break;

				case tcGPSMapDatum:
					s = &fGPSMapDatum;
					break;

				case tcGPSDestLatitudeRef:
					s = &fGPSDestLatitudeRef;
					break;

				case tcGPSDestLongitudeRef:
					s = &fGPSDestLongitudeRef;
					break;

				case tcGPSDestBearingRef:
					s = &fGPSDestBearingRef;
					break;

				case tcGPSDestDistanceRef:
					s = &fGPSDestDistanceRef;
					break;

				case tcGPSDateStamp:
					s = &fGPSDateStamp;
					break;

				default:
					return false;

				}

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							*s);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s: ", LookupTagCode (parentCode, tagCode));

				DumpString (*s);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcGPSLatitude:
		case tcGPSLongitude:
		case tcGPSTimeStamp:
		case tcGPSDestLatitude:
		case tcGPSDestLongitude:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttRational))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 3))
				return false;

			dng_urational *u;

			switch (tagCode)
				{

				case tcGPSLatitude:
					u = fGPSLatitude;
					break;

				case tcGPSLongitude:
					u = fGPSLongitude;
					break;

				case tcGPSTimeStamp:
					u = fGPSTimeStamp;
					break;

				case tcGPSDestLatitude:
					u = fGPSDestLatitude;
					break;

				case tcGPSDestLongitude:
					u = fGPSDestLongitude;
					break;

				default:
					return false;

				}

			u [0] = stream.TagValue_urational (tagType);
			u [1] = stream.TagValue_urational (tagType);
			u [2] = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s:", LookupTagCode (parentCode, tagCode));

				for (uint32 j = 0; j < 3; j++)
					{

					if (u [j].d == 0)
						printf (" -");

					else
						printf (" %0.4f", u [j].As_real64 ());

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcGPSAltitudeRef:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fGPSAltitudeRef = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("GPSAltitudeRef: ");

				switch (fGPSAltitudeRef)
					{

					case 0:
						printf ("Sea level");
						break;

					case 1:
						printf ("Sea level reference (negative value)");
						break;

					default:
						printf ("%u", (unsigned) fGPSAltitudeRef);
						break;

					}

				printf ("\n");

				}

			#endif

			break;

			}

		case tcGPSAltitude:
		case tcGPSDOP:
		case tcGPSSpeed:
		case tcGPSTrack:
		case tcGPSImgDirection:
		case tcGPSDestBearing:
		case tcGPSDestDistance:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttRational))
				return false;

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			dng_urational *u;

			switch (tagCode)
				{

				case tcGPSAltitude:
					u = &fGPSAltitude;
					break;

				case tcGPSDOP:
					u = &fGPSDOP;
					break;

				case tcGPSSpeed:
					u = &fGPSSpeed;
					break;

				case tcGPSTrack:
					u = &fGPSTrack;
					break;

				case tcGPSImgDirection:
					u = &fGPSImgDirection;
					break;

				case tcGPSDestBearing:
					u = &fGPSDestBearing;
					break;

				case tcGPSDestDistance:
					u = &fGPSDestDistance;
					break;

				default:
					return false;

				}

			*u = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s:", LookupTagCode (parentCode, tagCode));

				if (u->d == 0)
					printf (" -");

				else
					printf (" %0.4f", u->As_real64 ());

				printf ("\n");

				}

			#endif

			break;

			}

		case tcGPSProcessingMethod:
		case tcGPSAreaInformation:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttUndefined))
				return false;

			dng_string *s;

			switch (tagCode)
				{

				case tcGPSProcessingMethod:
					s = &fGPSProcessingMethod;
					break;

				case tcGPSAreaInformation:
					s = &fGPSAreaInformation;
					break;

				default:
					return false;

				}

			ParseEncodedStringTag (stream,
								   parentCode,
								   tagCode,
				    			   tagCount,
				    		       *s);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("%s: ", LookupTagCode (parentCode, tagCode));

				DumpString (*s);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcGPSDifferential:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fGPSDifferential = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("GPSDifferential: ");

				switch (fGPSDifferential)
					{

					case 0:
						printf ("Measurement without differential correction");
						break;

					case 1:
						printf ("Differential correction applied");
						break;

					default:
						printf ("%u", (unsigned) fGPSDifferential);

					}

				printf ("\n");

				}

			#endif

			break;

			}

		default:
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

// Parses tags that should only appear in Interoperability IFD

bool dng_exif::Parse_interoperability (dng_stream &stream,
						  			   dng_shared & /* shared */,
									   uint32 parentCode,
									   uint32 tagCode,
									   uint32 tagType,
									   uint32 tagCount,
									   uint64 /* tagOffset */)
	{

	switch (tagCode)
		{

		case tcInteroperabilityIndex:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fInteroperabilityIndex);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("InteroperabilityIndex: ");

				DumpString (fInteroperabilityIndex);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcInteroperabilityVersion:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fInteroperabilityVersion = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{

				real64 x = (b0 - '0') * 10.00 +
						   (b1 - '0') *  1.00 +
						   (b2 - '0') *  0.10 +
						   (b3 - '0') *  0.01;

				printf ("InteroperabilityVersion: %0.2f\n", x);

				}

			#endif

			break;

			}

		case tcRelatedImageFileFormat:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fRelatedImageFileFormat);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("RelatedImageFileFormat: ");

				DumpString (fRelatedImageFileFormat);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcRelatedImageWidth:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fRelatedImageWidth = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("RelatedImageWidth: %u\n", (unsigned) fRelatedImageWidth);
				}

			#endif

			break;

			}

		case tcRelatedImageLength:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fRelatedImageLength = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("RelatedImageLength: %u\n", (unsigned) fRelatedImageLength);
				}

			#endif

			break;

			}

		default:
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

void dng_exif::PostParse (dng_host & /* host */,
						  dng_shared & /* shared */)
	{

	#if qDNGValidate

	const real64 kAPEX_Slop = 0.25;

	// Sanity check on MaxApertureValue.

	if (fMaxApertureValue.d)
		{

		real64 mav = fMaxApertureValue.As_real64 ();

		// Compare against ApertureValue or FNumber.

		real64 av = mav;

		if (fApertureValue.d)
			{

			av = fApertureValue.As_real64 ();

			}

		else if (fFNumber.d)
			{

			real64 fs = fFNumber.As_real64 ();

			if (fs >= 1.0)
				{

				av = 2.0 * log (fs) / log (2.0);

				}

			}

		if (mav > av + kAPEX_Slop)
			{

			ReportWarning ("MaxApertureValue conflicts with ApertureValue and/or FNumber");

			}

		// Compare against LensInfo

		if (fLensInfo [2].d && fLensInfo [3].d)
			{

			real64 fs1 = fLensInfo [2].As_real64 ();
			real64 fs2 = fLensInfo [3].As_real64 ();

			if (fs1 >= 1.0 && fs2 >= 1.0 && fs2 >= fs1)
				{

				real64 av1 = 2.0 * log (fs1) / log (2.0);
				real64 av2 = 2.0 * log (fs2) / log (2.0);

				// Wide angle adapters might create an effective
				// wide FS, and tele-extenders always result
				// in a higher FS.

				if (mav < av1 - kAPEX_Slop - 1.0 ||
					mav > av2 + kAPEX_Slop + 2.0)
					{

					ReportWarning ("Possible MaxApertureValue conflict with LensInfo");

					}

				}

			}

		}

	// Sanity check on FocalLength.

	if (fFocalLength.d)
		{

		real64 fl = fFocalLength.As_real64 ();

		if (fl < 1.0)
			{

			ReportWarning ("FocalLength is less than 1.0 mm (legal but unlikely)");

			}

		else if (fLensInfo [0].d && fLensInfo [1].d)
			{

			real64 minFL = fLensInfo [0].As_real64 ();
			real64 maxFL = fLensInfo [1].As_real64 ();

			// Allow for wide-angle converters and tele-extenders.

			if (fl < minFL * 0.6 ||
			    fl > maxFL * 2.1)
				{

				ReportWarning ("Possible FocalLength conflict with LensInfo");

				}

			}

		}

	#endif

	// Mirror DateTimeOriginal to DateTime.

	if (fDateTime.NotValid () && fDateTimeOriginal.IsValid ())
		{

		fDateTime = fDateTimeOriginal;

		}

	// Mirror ExposureIndex to ISOSpeedRatings.

	if (fISOSpeedRatings [0] == 0 && fExposureIndex.IsValid ())
		{

		fISOSpeedRatings [0] = Round_uint32 (fExposureIndex.As_real64 ());

		}

	// Kodak sets the GPSAltitudeRef without setting the GPSAltitude.

	if (fGPSAltitude.NotValid ())
		{

		fGPSAltitudeRef = 0xFFFFFFFF;

		}

	// If there is no valid GPS data, clear the GPS version number.

	if (fGPSLatitude  [0].NotValid () &&
		fGPSLongitude [0].NotValid () &&
		fGPSAltitude     .NotValid () &&
		fGPSTimeStamp [0].NotValid () &&
		fGPSDateStamp    .IsEmpty  ())
		{

		fGPSVersionID = 0;

		}

	}

/*****************************************************************************/
