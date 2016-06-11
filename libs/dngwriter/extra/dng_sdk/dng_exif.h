/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_exif.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * EXIF read access support. See the \ref spec_exif "EXIF specification" for full description of tags.
 */

/*****************************************************************************/

#ifndef __dng_exif__
#define __dng_exif__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_date_time.h"
#include "dng_fingerprint.h"
#include "dng_types.h"
#include "dng_matrix.h"
#include "dng_rational.h"
#include "dng_string.h"
#include "dng_stream.h"
#include "dng_sdk_limits.h"

/*****************************************************************************/

/// \brief Container class for parsing and holding EXIF tags.
///
/// Public member fields are documented in \ref spec_exif "EXIF specification."

class dng_exif
	{

	public:

		dng_string fImageDescription;
		dng_string fMake;
		dng_string fModel;
		dng_string fSoftware;
		dng_string fArtist;
		dng_string fCopyright;
		dng_string fCopyright2;
		dng_string fUserComment;

		dng_date_time_info         fDateTime;
		dng_date_time_storage_info fDateTimeStorageInfo;

		dng_date_time_info		   fDateTimeOriginal;
		dng_date_time_storage_info fDateTimeOriginalStorageInfo;

		dng_date_time_info 		   fDateTimeDigitized;
		dng_date_time_storage_info fDateTimeDigitizedStorageInfo;

		uint32 fTIFF_EP_StandardID;
		uint32 fExifVersion;
		uint32 fFlashPixVersion;

		dng_urational fExposureTime;
		dng_urational fFNumber;
		dng_srational fShutterSpeedValue;
		dng_urational fApertureValue;
		dng_srational fBrightnessValue;
		dng_srational fExposureBiasValue;
		dng_urational fMaxApertureValue;
		dng_urational fFocalLength;
		dng_urational fDigitalZoomRatio;
		dng_urational fExposureIndex;
		dng_urational fSubjectDistance;
		dng_urational fGamma;

		dng_urational fBatteryLevelR;
		dng_string    fBatteryLevelA;

		uint32 fExposureProgram;
		uint32 fMeteringMode;
		uint32 fLightSource;
		uint32 fFlash;
		uint32 fFlashMask;
		uint32 fSensingMethod;
		uint32 fColorSpace;
		uint32 fFileSource;
		uint32 fSceneType;
		uint32 fCustomRendered;
		uint32 fExposureMode;
		uint32 fWhiteBalance;
		uint32 fSceneCaptureType;
		uint32 fGainControl;
		uint32 fContrast;
		uint32 fSaturation;
		uint32 fSharpness;
		uint32 fSubjectDistanceRange;
		uint32 fSelfTimerMode;
		uint32 fImageNumber;

		uint32 fFocalLengthIn35mmFilm;

		uint32 fISOSpeedRatings [3];

		uint32 fSubjectAreaCount;
		uint32 fSubjectArea [4];

		uint32 fComponentsConfiguration;

		dng_urational fCompresssedBitsPerPixel;

		uint32 fPixelXDimension;
		uint32 fPixelYDimension;

		dng_urational fFocalPlaneXResolution;
		dng_urational fFocalPlaneYResolution;

		uint32 fFocalPlaneResolutionUnit;

		uint32 fCFARepeatPatternRows;
		uint32 fCFARepeatPatternCols;

		uint8 fCFAPattern [kMaxCFAPattern] [kMaxCFAPattern];

		dng_fingerprint fImageUniqueID;

		uint32 	      fGPSVersionID;
		dng_string    fGPSLatitudeRef;
		dng_urational fGPSLatitude [3];
		dng_string    fGPSLongitudeRef;
		dng_urational fGPSLongitude [3];
		uint32	      fGPSAltitudeRef;
		dng_urational fGPSAltitude;
		dng_urational fGPSTimeStamp [3];
		dng_string    fGPSSatellites;
		dng_string    fGPSStatus;
		dng_string    fGPSMeasureMode;
		dng_urational fGPSDOP;
		dng_string    fGPSSpeedRef;
		dng_urational fGPSSpeed;
		dng_string    fGPSTrackRef;
		dng_urational fGPSTrack;
		dng_string    fGPSImgDirectionRef;
		dng_urational fGPSImgDirection;
		dng_string    fGPSMapDatum;
		dng_string    fGPSDestLatitudeRef;
		dng_urational fGPSDestLatitude [3];
		dng_string    fGPSDestLongitudeRef;
		dng_urational fGPSDestLongitude [3];
		dng_string    fGPSDestBearingRef;
		dng_urational fGPSDestBearing;
		dng_string    fGPSDestDistanceRef;
		dng_urational fGPSDestDistance;
		dng_string    fGPSProcessingMethod;
		dng_string    fGPSAreaInformation;
		dng_string    fGPSDateStamp;
		uint32 	      fGPSDifferential;

		dng_string fInteroperabilityIndex;

		uint32 fInteroperabilityVersion;

		dng_string fRelatedImageFileFormat;

		uint32 fRelatedImageWidth;
		uint32 fRelatedImageLength;

		dng_string fCameraSerialNumber;

		dng_urational fLensInfo [4];

		dng_string fLensID;
		dng_string fLensName;
		dng_string fLensSerialNumber;

		dng_srational fFlashCompensation;

		dng_string fOwnerName;
		dng_string fFirmware;

	public:

		dng_exif ();

		virtual ~dng_exif ();

		virtual dng_exif * Clone () const;

		static real64 SnapExposureTime (real64 et);

		void SetExposureTime (real64 et,
							  bool snap = true);

		void SetShutterSpeedValue (real64 ss);

		static dng_urational EncodeFNumber (real64 fs);

		void SetFNumber (real64 fs);

		void SetApertureValue (real64 av);

		void UpdateDateTime (const dng_date_time_info &dt);

		virtual bool ParseTag (dng_stream &stream,
							   dng_shared &shared,
							   uint32 parentCode,
							   bool isMainIFD,
							   uint32 tagCode,
							   uint32 tagType,
							   uint32 tagCount,
							   uint64 tagOffset);

		virtual void PostParse (dng_host &host,
								dng_shared &shared);

	protected:

		virtual bool Parse_ifd0 (dng_stream &stream,
							     dng_shared &shared,
							 	 uint32 parentCode,
							 	 uint32 tagCode,
							 	 uint32 tagType,
							 	 uint32 tagCount,
							 	 uint64 tagOffset);

		virtual bool Parse_ifd0_main (dng_stream &stream,
							          dng_shared &shared,
						 		 	  uint32 parentCode,
						 		 	  uint32 tagCode,
						 		 	  uint32 tagType,
						 		 	  uint32 tagCount,
						 		 	  uint64 tagOffset);

		virtual bool Parse_ifd0_exif (dng_stream &stream,
							          dng_shared &shared,
						 		 	  uint32 parentCode,
						 		 	  uint32 tagCode,
						 		 	  uint32 tagType,
						 		 	  uint32 tagCount,
						 		 	  uint64 tagOffset);

		virtual bool Parse_gps (dng_stream &stream,
							    dng_shared &shared,
						 		uint32 parentCode,
						 		uint32 tagCode,
						 		uint32 tagType,
						 		uint32 tagCount,
						 		uint64 tagOffset);

		virtual bool Parse_interoperability (dng_stream &stream,
							    			 dng_shared &shared,
						 					 uint32 parentCode,
											 uint32 tagCode,
											 uint32 tagType,
											 uint32 tagCount,
											 uint64 tagOffset);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
