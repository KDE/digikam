/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_image_writer.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_image_writer.h"

#include "dng_bottlenecks.h"
#include "dng_camera_profile.h"
#include "dng_color_space.h"
#include "dng_exif.h"
#include "dng_flags.h"
#include "dng_exceptions.h"
#include "dng_host.h"
#include "dng_ifd.h"
#include "dng_image.h"
#include "dng_lossless_jpeg.h"
#include "dng_memory_stream.h"
#include "dng_negative.h"
#include "dng_pixel_buffer.h"
#include "dng_preview.h"
#include "dng_read_image.h"
#include "dng_stream.h"
#include "dng_tag_codes.h"
#include "dng_tag_values.h"
#include "dng_utils.h"
#include "dng_xmp.h"

/*****************************************************************************/

// Defines for testing DNG 1.2 features.

//#define qTestRowInterleave 2

//#define qTestSubTileBlockRows 2
//#define qTestSubTileBlockCols 2

/*****************************************************************************/

dng_resolution::dng_resolution ()

	:	fXResolution ()
	,	fYResolution ()

	,	fResolutionUnit (0)

	{

	}

/******************************************************************************/

static void SpoolAdobeData (dng_stream &stream,
							const dng_negative *negative,
							const dng_jpeg_preview *preview,
							const dng_memory_block *imageResources)
	{

	TempBigEndian tempEndian (stream);

	if (negative && negative->GetXMP ())
		{

		bool marked = false;

		if (negative->GetXMP ()->GetBoolean ("http://ns.adobe.com/xap/1.0/rights/",
											 "Marked",
											 marked))
			{

			stream.Put_uint32 (DNG_CHAR4 ('8','B','I','M'));
			stream.Put_uint16 (1034);
			stream.Put_uint16 (0);

			stream.Put_uint32 (1);

			stream.Put_uint8 (marked ? 1 : 0);

			stream.Put_uint8 (0);

			}

		dng_string webStatement;

		if (negative->GetXMP ()->GetString ("http://ns.adobe.com/xap/1.0/rights/",
											"WebStatement",
											webStatement))
			{

			dng_memory_data buffer;

			uint32 size = webStatement.Get_SystemEncoding (buffer);

			if (size > 0)
				{

				stream.Put_uint32 (DNG_CHAR4 ('8','B','I','M'));
				stream.Put_uint16 (1035);
				stream.Put_uint16 (0);

				stream.Put_uint32 (size);

				stream.Put (buffer.Buffer (), size);

				if (size & 1)
					stream.Put_uint8 (0);

				}

			}

		}

	if (preview)
		{

		preview->SpoolAdobeThumbnail (stream);

		}

	if (negative)
		{

		dng_fingerprint iptcDigest = negative->IPTCDigest ();

		if (iptcDigest.IsValid ())
			{

			stream.Put_uint32 (DNG_CHAR4 ('8','B','I','M'));
			stream.Put_uint16 (1061);
			stream.Put_uint16 (0);

			stream.Put_uint32 (16);

			stream.Put (iptcDigest.data, 16);

			}

		}

	if (imageResources)
		{

		uint32 size = imageResources->LogicalSize ();

		stream.Put (imageResources->Buffer (), size);

		if (size & 1)
			stream.Put_uint8 (0);

		}

	}

/******************************************************************************/

static dng_memory_block * BuildAdobeData (dng_host &host,
										  const dng_negative *negative,
										  const dng_jpeg_preview *preview,
										  const dng_memory_block *imageResources)
	{

	dng_memory_stream stream (host.Allocator ());

	SpoolAdobeData (stream,
					negative,
					preview,
					imageResources);

	return stream.AsMemoryBlock (host.Allocator ());

	}

/*****************************************************************************/

tag_string::tag_string (uint16 code,
				    	const dng_string &s,
				    	bool forceASCII)

	:	tiff_tag (code, ttAscii, 0)

	,	fString (s)

	{

	if (forceASCII)
		{

		fString.ForceASCII ();

		}

	else if (!fString.IsASCII ())
		{

		fType = ttByte;

		}

	fCount = fString.Length () + 1;

	}

/*****************************************************************************/

void tag_string::Put (dng_stream &stream) const
	{

	stream.Put (fString.Get (), Size ());

	}

/*****************************************************************************/

tag_encoded_text::tag_encoded_text (uint16 code,
									const dng_string &text)

	:	tiff_tag (code, ttUndefined, 0)

	,	fText (text)

	,	fUTF16 ()

	{

	if (fText.IsASCII ())
		{

		fCount = 8 + fText.Length ();

		}

	else
		{

		fCount = 8 + fText.Get_UTF16 (fUTF16) * 2;

		}

	}

/*****************************************************************************/

void tag_encoded_text::Put (dng_stream &stream) const
	{

	if (fUTF16.Buffer ())
		{

		stream.Put ("UNICODE\000", 8);

		uint32 chars = (fCount - 8) >> 1;

		const uint16 *buf = fUTF16.Buffer_uint16 ();

		for (uint32 j = 0; j < chars; j++)
			{

			stream.Put_uint16 (buf [j]);

			}

		}

	else
		{

		stream.Put ("ASCII\000\000\000", 8);

		stream.Put (fText.Get (), fCount - 8);

		}

	}

/*****************************************************************************/

void tag_data_ptr::Put (dng_stream &stream) const
	{

	// If we are swapping bytes, we need to swap with the right size
	// entries.

	if (stream.SwapBytes ())
		{

		switch (Type ())
			{

			// Two byte entries.

			case ttShort:
			case ttSShort:
			case ttUnicode:
				{

				const uint16 *p = (const uint16 *) fData;

				uint32 entries = (Size () >> 1);

				for (uint32 j = 0; j < entries; j++)
					{

					stream.Put_uint16 (p [j]);

					}

				return;

				}

			// Four byte entries.

			case ttLong:
			case ttSLong:
			case ttRational:
			case ttSRational:
			case ttIFD:
			case ttFloat:
			case ttComplex:
				{

				const uint32 *p = (const uint32 *) fData;

				uint32 entries = (Size () >> 2);

				for (uint32 j = 0; j < entries; j++)
					{

					stream.Put_uint32 (p [j]);

					}

				return;

				}

			// Eight byte entries.

			case ttDouble:
				{

				const real64 *p = (const real64 *) fData;

				uint32 entries = (Size () >> 3);

				for (uint32 j = 0; j < entries; j++)
					{

					stream.Put_real64 (p [j]);

					}

				return;

				}

			// Entries don't need to be byte swapped.  Fall through
			// to non-byte swapped case.

			default:
				{

				break;

				}

			}

		}

	// Non-byte swapped case.

	stream.Put (fData, Size ());

	}

/******************************************************************************/

tag_matrix::tag_matrix (uint16 code,
		    			const dng_matrix &m)

	:	tag_srational_ptr (code, fEntry, m.Rows () * m.Cols ())

	{

	uint32 index = 0;

	for (uint32 r = 0; r < m.Rows (); r++)
		for (uint32 c = 0; c < m.Cols (); c++)
			{

			fEntry [index].Set_real64 (m [r] [c], 10000);

			index++;

			}

	}

/******************************************************************************/

tag_icc_profile::tag_icc_profile (const void *profileData,
								  uint32 profileSize)

	:	tag_data_ptr (tcICCProfile,
					  ttUndefined,
					  0,
					  NULL)

	{

	if (profileData && profileSize)
		{

		SetCount (profileSize);
		SetData  (profileData);

		}

	}

/******************************************************************************/

void tag_cfa_pattern::Put (dng_stream &stream) const
	{

	stream.Put_uint16 ((uint16) fCols);
	stream.Put_uint16 ((uint16) fRows);

	for (uint32 col = 0; col < fCols; col++)
		for (uint32 row = 0; row < fRows; row++)
			{

			stream.Put_uint8 (fPattern [row * kMaxCFAPattern + col]);

			}

	}

/******************************************************************************/

tag_exif_date_time::tag_exif_date_time (uint16 code,
		            					const dng_date_time &dt)

	:	tag_data_ptr (code, ttAscii, 20, fData)

	{

	if (dt.IsValid ())
		{

		sprintf (fData,
				 "%04d:%02d:%02d %02d:%02d:%02d",
				 (int) dt.fYear,
				 (int) dt.fMonth,
				 (int) dt.fDay,
				 (int) dt.fHour,
				 (int) dt.fMinute,
				 (int) dt.fSecond);

		}

	}

/******************************************************************************/

tag_iptc::tag_iptc (const void *data,
		  			uint32 length)

	:	tiff_tag (tcIPTC_NAA, ttLong, (length + 3) >> 2)

	,	fData   (data  )
	,	fLength (length)

	{

	}

/******************************************************************************/

void tag_iptc::Put (dng_stream &stream) const
	{

	// Note: For historical compatiblity reasons, the standard TIFF data
	// type for IPTC data is ttLong, but without byte swapping.  This really
	// should be ttUndefined, but doing the right thing would break some
	// existing readers.

	stream.Put (fData, fLength);

	// Pad with zeros to get to long word boundary.

	uint32 extra = fCount * 4 - fLength;

	while (extra--)
		{
		stream.Put_uint8 (0);
		}

	}

/******************************************************************************/

tag_xmp::tag_xmp (const dng_xmp *xmp)

	:	tag_uint8_ptr (tcXMP, NULL, 0)

	,	fBuffer ()

	{

	if (xmp)
		{

		fBuffer.Reset (xmp->Serialize (true));

		if (fBuffer.Get ())
			{

			SetData (fBuffer->Buffer_uint8 ());

			SetCount (fBuffer->LogicalSize ());

			}

		}

	}

/******************************************************************************/

void dng_tiff_directory::Add (const tiff_tag *tag)
	{

	if (fEntries >= kMaxEntries)
		{
		ThrowProgramError ();
		}

	// Tags must be sorted in increasing order of tag code.

	uint32 index = fEntries;

	for (uint32 j = 0; j < fEntries; j++)
		{

		if (tag->Code () < fTag [j]->Code ())
			{
			index = j;
			break;
			}

		}

	for (uint32 k = fEntries; k > index; k--)
		{

		fTag [k] = fTag [k - 1];

		}

	fTag [index] = tag;

	fEntries++;

	}

/******************************************************************************/

uint32 dng_tiff_directory::Size () const
	{

	if (!fEntries) return 0;

	uint32 size = fEntries * 12 + 6;

	for (uint32 index = 0; index < fEntries; index++)
		{

		uint32 tagSize = fTag [index]->Size ();

		if (tagSize > 4)
			{

			size += (tagSize + 1) & ~1;

			}

		}

	return size;

	}

/******************************************************************************/

void dng_tiff_directory::Put (dng_stream &stream,
						      OffsetsBase offsetsBase,
						      uint32 explicitBase) const
	{

	if (!fEntries) return;

	uint32 index;

	uint32 bigData = fEntries * 12 + 6;

	if (offsetsBase == offsetsRelativeToStream)
		bigData += (uint32) stream.Position ();

	else if (offsetsBase == offsetsRelativeToExplicitBase)
		bigData += explicitBase;

	stream.Put_uint16 ((uint16) fEntries);

	for (index = 0; index < fEntries; index++)
		{

		const tiff_tag &tag = *fTag [index];

		stream.Put_uint16 (tag.Code  ());
		stream.Put_uint16 (tag.Type  ());
		stream.Put_uint32 (tag.Count ());

		uint32 size = tag.Size ();

		if (size <= 4)
			{

			tag.Put (stream);

			while (size < 4)
				{
				stream.Put_uint8 (0);
				size++;
				}

			}

		else
			{

			stream.Put_uint32 (bigData);

			bigData += (size + 1) & ~1;

			}

		}

	stream.Put_uint32 (fChained);		// Next IFD offset

	for (index = 0; index < fEntries; index++)
		{

		const tiff_tag &tag = *fTag [index];

		uint32 size = tag.Size ();

		if (size > 4)
			{

			tag.Put (stream);

			if (size & 1)
				stream.Put_uint8 (0);

			}

		}

	}

/******************************************************************************/

dng_basic_tag_set::dng_basic_tag_set (dng_tiff_directory &directory,
									  const dng_ifd &info)

	:	fNewSubFileType (tcNewSubFileType, info.fNewSubFileType)

	,	fImageWidth  (tcImageWidth , info.fImageWidth )
	,	fImageLength (tcImageLength, info.fImageLength)

	,	fPhotoInterpretation (tcPhotometricInterpretation,
							  (uint16) info.fPhotometricInterpretation)

	,	fFillOrder (tcFillOrder, 1)

	,	fSamplesPerPixel (tcSamplesPerPixel, (uint16) info.fSamplesPerPixel)

	,	fBitsPerSample (tcBitsPerSample,
						fBitsPerSampleData,
						info.fSamplesPerPixel)

	,	fStrips (info.fUsesStrips)

	,	fTileWidth (tcTileWidth, info.fTileWidth)

	,	fTileLength (fStrips ? tcRowsPerStrip : tcTileLength,
					 info.fTileLength)

	,	fTileInfoBuffer (info.TilesPerImage () * 8)

	,	fTileOffsetData (fTileInfoBuffer.Buffer_uint32 ())

	,	fTileOffsets (fStrips ? tcStripOffsets : tcTileOffsets,
					  fTileOffsetData,
					  info.TilesPerImage ())

	,	fTileByteCountData (fTileOffsetData + info.TilesPerImage ())

	,	fTileByteCounts (fStrips ? tcStripByteCounts : tcTileByteCounts,
						 fTileByteCountData,
						 info.TilesPerImage ())

	,	fPlanarConfiguration (tcPlanarConfiguration, pcInterleaved)

	,	fCompression (tcCompression, (uint16) info.fCompression)
	,	fPredictor   (tcPredictor  , (uint16) info.fPredictor  )

	,	fExtraSamples (tcExtraSamples,
					   fExtraSamplesData,
					   info.fExtraSamplesCount)

	,	fSampleFormat (tcSampleFormat,
					   fSampleFormatData,
					   info.fSamplesPerPixel)

	,	fRowInterleaveFactor (tcRowInterleaveFactor,
							  (uint16) info.fRowInterleaveFactor)

	,	fSubTileBlockSize (tcSubTileBlockSize,
						   fSubTileBlockSizeData,
						   2)

	{

	uint32 j;

	for (j = 0; j < info.fSamplesPerPixel; j++)
		{

		fBitsPerSampleData [j] = (uint16) info.fBitsPerSample [0];

		}

	directory.Add (&fNewSubFileType);

	directory.Add (&fImageWidth);
	directory.Add (&fImageLength);

	directory.Add (&fPhotoInterpretation);

	directory.Add (&fSamplesPerPixel);

	directory.Add (&fBitsPerSample);

	if (info.fBitsPerSample [0] !=  8 &&
	    info.fBitsPerSample [0] != 16 &&
	    info.fBitsPerSample [0] != 32)
		{

		directory.Add (&fFillOrder);

		}

	if (!fStrips)
		{

		directory.Add (&fTileWidth);

		}

	directory.Add (&fTileLength);

	directory.Add (&fTileOffsets);
	directory.Add (&fTileByteCounts);

	directory.Add (&fPlanarConfiguration);

	directory.Add (&fCompression);

	if (info.fPredictor != cpNullPredictor)
		{

		directory.Add (&fPredictor);

		}

	if (info.fExtraSamplesCount != 0)
		{

		for (j = 0; j < info.fExtraSamplesCount; j++)
			{
			fExtraSamplesData [j] = (uint16) info.fExtraSamples [j];
			}

		directory.Add (&fExtraSamples);

		}

	if (info.fSampleFormat [0] != sfUnsignedInteger)
		{

		for (j = 0; j < info.fSamplesPerPixel; j++)
			{
			fSampleFormatData [j] = (uint16) info.fSampleFormat [j];
			}

		directory.Add (&fSampleFormat);

		}

	if (info.fRowInterleaveFactor != 1)
		{

		directory.Add (&fRowInterleaveFactor);

		}

	if (info.fSubTileBlockRows != 1 ||
		info.fSubTileBlockCols != 1)
		{

		fSubTileBlockSizeData [0] = (uint16) info.fSubTileBlockRows;
		fSubTileBlockSizeData [1] = (uint16) info.fSubTileBlockCols;

		directory.Add (&fSubTileBlockSize);

		}

	}

/******************************************************************************/

exif_tag_set::exif_tag_set (dng_tiff_directory &directory,
					  		const dng_exif &exif,
							bool makerNoteSafe,
							const void *makerNoteData,
							uint32 makerNoteLength,
					 	    bool insideDNG)

	:	fExifIFD ()
	,	fGPSIFD  ()

	,	fExifLink (tcExifIFD, 0)
	,	fGPSLink  (tcGPSInfo, 0)

	,	fAddedExifLink (false)
	,	fAddedGPSLink  (false)

	,	fExifVersion (tcExifVersion, ttUndefined, 4, fExifVersionData)

	,	fExposureTime      (tcExposureTime     , exif.fExposureTime     )
	,	fShutterSpeedValue (tcShutterSpeedValue, exif.fShutterSpeedValue)

	,	fFNumber 	   (tcFNumber      , exif.fFNumber      )
	,	fApertureValue (tcApertureValue, exif.fApertureValue)

	,	fBrightnessValue (tcBrightnessValue, exif.fBrightnessValue)

	,	fExposureBiasValue (tcExposureBiasValue, exif.fExposureBiasValue)

	,	fMaxApertureValue (tcMaxApertureValue , exif.fMaxApertureValue)

	,	fSubjectDistance (tcSubjectDistance, exif.fSubjectDistance)

	,	fFocalLength (tcFocalLength, exif.fFocalLength)

	,	fISOSpeedRatings (tcISOSpeedRatings, (uint16) exif.fISOSpeedRatings [0])

	,	fFlash (tcFlash, (uint16) exif.fFlash)

	,	fExposureProgram (tcExposureProgram, (uint16) exif.fExposureProgram)

	,	fMeteringMode (tcMeteringMode, (uint16) exif.fMeteringMode)

	,	fLightSource (tcLightSource, (uint16) exif.fLightSource)

	,	fSensingMethod (tcSensingMethodExif, (uint16) exif.fSensingMethod)

	,	fFocalLength35mm (tcFocalLengthIn35mmFilm, (uint16) exif.fFocalLengthIn35mmFilm)

	,	fFileSourceData ((uint8) exif.fFileSource)
	,	fFileSource     (tcFileSource, ttUndefined, 1, &fFileSourceData)

	,	fSceneTypeData ((uint8) exif.fSceneType)
	,	fSceneType     (tcSceneType, ttUndefined, 1, &fSceneTypeData)

	,	fCFAPattern (tcCFAPatternExif,
					 exif.fCFARepeatPatternRows,
					 exif.fCFARepeatPatternCols,
					 &exif.fCFAPattern [0] [0])

	,	fCustomRendered 	  (tcCustomRendered		 , (uint16) exif.fCustomRendered	  )
	,	fExposureMode 		  (tcExposureMode		 , (uint16) exif.fExposureMode		  )
	,	fWhiteBalance 		  (tcWhiteBalance		 , (uint16) exif.fWhiteBalance		  )
	,	fSceneCaptureType 	  (tcSceneCaptureType	 , (uint16) exif.fSceneCaptureType	  )
	,	fGainControl 		  (tcGainControl		 , (uint16) exif.fGainControl		  )
	,	fContrast 			  (tcContrast			 , (uint16) exif.fContrast			  )
	,	fSaturation 		  (tcSaturation			 , (uint16) exif.fSaturation		  )
	,	fSharpness 			  (tcSharpness			 , (uint16) exif.fSharpness			  )
	,	fSubjectDistanceRange (tcSubjectDistanceRange, (uint16) exif.fSubjectDistanceRange)

	,	fDigitalZoomRatio (tcDigitalZoomRatio, exif.fDigitalZoomRatio)

	,	fExposureIndex (tcExposureIndexExif, exif.fExposureIndex)

	,	fImageNumber (tcImageNumber, exif.fImageNumber)

	,	fSelfTimerMode (tcSelfTimerMode, (uint16) exif.fSelfTimerMode)

	,	fBatteryLevelA (tcBatteryLevel, exif.fBatteryLevelA)
	,	fBatteryLevelR (tcBatteryLevel, exif.fBatteryLevelR)

	,	fFocalPlaneXResolution (tcFocalPlaneXResolutionExif, exif.fFocalPlaneXResolution)
	,	fFocalPlaneYResolution (tcFocalPlaneYResolutionExif, exif.fFocalPlaneYResolution)

	,	fFocalPlaneResolutionUnit (tcFocalPlaneResolutionUnitExif, (uint16) exif.fFocalPlaneResolutionUnit)

	,	fSubjectArea (tcSubjectArea, fSubjectAreaData, exif.fSubjectAreaCount)

	,	fLensInfo (tcLensInfo, fLensInfoData, 4)

	,	fDateTime		   (tcDateTime		   , exif.fDateTime         .DateTime ())
	,	fDateTimeOriginal  (tcDateTimeOriginal , exif.fDateTimeOriginal .DateTime ())
	,	fDateTimeDigitized (tcDateTimeDigitized, exif.fDateTimeDigitized.DateTime ())

	,	fSubsecTime			 (tcSubsecTime, 		 exif.fDateTime         .Subseconds ())
	,	fSubsecTimeOriginal  (tcSubsecTimeOriginal,  exif.fDateTimeOriginal .Subseconds ())
	,	fSubsecTimeDigitized (tcSubsecTimeDigitized, exif.fDateTimeDigitized.Subseconds ())

	,	fTimeZoneOffset (tcTimeZoneOffset, fTimeZoneOffsetData, 2)

	,	fMake (tcMake, exif.fMake)

	,	fModel (tcModel, exif.fModel)

	,	fArtist (tcArtist, exif.fArtist)

	,	fSoftware (tcSoftware, exif.fSoftware)

	,	fCopyright (tcCopyright, exif.fCopyright)

	,	fMakerNoteSafety (tcMakerNoteSafety, makerNoteSafe ? 1 : 0)

	,	fMakerNote (tcMakerNote, ttUndefined, makerNoteLength, makerNoteData)

	,	fImageDescription (tcImageDescription, exif.fImageDescription)

	,	fSerialNumber (tcCameraSerialNumber, exif.fCameraSerialNumber)

	,	fUserComment (tcUserComment, exif.fUserComment)

	,	fImageUniqueID (tcImageUniqueID, ttAscii, 33, fImageUniqueIDData)

	,	fGPSVersionID (tcGPSVersionID, fGPSVersionData, 4)

	,	fGPSLatitudeRef (tcGPSLatitudeRef, exif.fGPSLatitudeRef)
	,	fGPSLatitude    (tcGPSLatitude,    exif.fGPSLatitude, 3)

	,	fGPSLongitudeRef (tcGPSLongitudeRef, exif.fGPSLongitudeRef)
	,	fGPSLongitude    (tcGPSLongitude,    exif.fGPSLongitude, 3)

	,	fGPSAltitudeRef (tcGPSAltitudeRef, (uint8) exif.fGPSAltitudeRef)
	,	fGPSAltitude    (tcGPSAltitude,            exif.fGPSAltitude   )

	,	fGPSTimeStamp (tcGPSTimeStamp, exif.fGPSTimeStamp, 3)

	,	fGPSSatellites  (tcGPSSatellites , exif.fGPSSatellites )
	,	fGPSStatus      (tcGPSStatus     , exif.fGPSStatus     )
	,	fGPSMeasureMode (tcGPSMeasureMode, exif.fGPSMeasureMode)

	,	fGPSDOP (tcGPSDOP, exif.fGPSDOP)

	,	fGPSSpeedRef (tcGPSSpeedRef, exif.fGPSSpeedRef)
	,	fGPSSpeed    (tcGPSSpeed   , exif.fGPSSpeed   )

	,	fGPSTrackRef (tcGPSTrackRef, exif.fGPSTrackRef)
	,	fGPSTrack    (tcGPSTrack   , exif.fGPSTrack   )

	,	fGPSImgDirectionRef (tcGPSImgDirectionRef, exif.fGPSImgDirectionRef)
	,	fGPSImgDirection    (tcGPSImgDirection   , exif.fGPSImgDirection   )

	,	fGPSMapDatum (tcGPSMapDatum, exif.fGPSMapDatum)

	,	fGPSDestLatitudeRef (tcGPSDestLatitudeRef, exif.fGPSDestLatitudeRef)
	,	fGPSDestLatitude    (tcGPSDestLatitude,    exif.fGPSDestLatitude, 3)

	,	fGPSDestLongitudeRef (tcGPSDestLongitudeRef, exif.fGPSDestLongitudeRef)
	,	fGPSDestLongitude    (tcGPSDestLongitude,    exif.fGPSDestLongitude, 3)

	,	fGPSDestBearingRef (tcGPSDestBearingRef, exif.fGPSDestBearingRef)
	,	fGPSDestBearing    (tcGPSDestBearing   , exif.fGPSDestBearing   )

	,	fGPSDestDistanceRef (tcGPSDestDistanceRef, exif.fGPSDestDistanceRef)
	,	fGPSDestDistance    (tcGPSDestDistance   , exif.fGPSDestDistance   )

	,	fGPSProcessingMethod (tcGPSProcessingMethod, exif.fGPSProcessingMethod)
	,	fGPSAreaInformation  (tcGPSAreaInformation , exif.fGPSAreaInformation )

	,	fGPSDateStamp (tcGPSDateStamp, exif.fGPSDateStamp)

	,	fGPSDifferential (tcGPSDifferential, (uint16) exif.fGPSDifferential)

	{

	if (exif.fExifVersion)
		{

		fExifVersionData [0] = (uint8) (exif.fExifVersion >> 24);
		fExifVersionData [1] = (uint8) (exif.fExifVersion >> 16);
		fExifVersionData [2] = (uint8) (exif.fExifVersion >>  8);
		fExifVersionData [3] = (uint8) (exif.fExifVersion      );

		fExifIFD.Add (&fExifVersion);

		}

	if (exif.fExposureTime.IsValid ())
		{
		fExifIFD.Add (&fExposureTime);
		}

	if (exif.fShutterSpeedValue.IsValid ())
		{
		fExifIFD.Add (&fShutterSpeedValue);
		}

	if (exif.fFNumber.IsValid ())
		{
		fExifIFD.Add (&fFNumber);
		}

	if (exif.fApertureValue.IsValid ())
		{
		fExifIFD.Add (&fApertureValue);
		}

	if (exif.fBrightnessValue.IsValid ())
		{
		fExifIFD.Add (&fBrightnessValue);
		}

	if (exif.fExposureBiasValue.IsValid ())
		{
		fExifIFD.Add (&fExposureBiasValue);
		}

	if (exif.fMaxApertureValue.IsValid ())
		{
		fExifIFD.Add (&fMaxApertureValue);
		}

	if (exif.fSubjectDistance.IsValid ())
		{
		fExifIFD.Add (&fSubjectDistance);
		}

	if (exif.fFocalLength.IsValid ())
		{
		fExifIFD.Add (&fFocalLength);
		}

	if (exif.fISOSpeedRatings [0] != 0)
		{
		fExifIFD.Add (&fISOSpeedRatings);
		}

	if (exif.fFlash <= 0x0FFFF)
		{
		fExifIFD.Add (&fFlash);
		}

	if (exif.fExposureProgram <= 0x0FFFF)
		{
		fExifIFD.Add (&fExposureProgram);
		}

	if (exif.fMeteringMode <= 0x0FFFF)
		{
		fExifIFD.Add (&fMeteringMode);
		}

	if (exif.fLightSource <= 0x0FFFF)
		{
		fExifIFD.Add (&fLightSource);
		}

	if (exif.fSensingMethod <= 0x0FFFF)
		{
		fExifIFD.Add (&fSensingMethod);
		}

	if (exif.fFocalLengthIn35mmFilm != 0)
		{
		fExifIFD.Add (&fFocalLength35mm);
		}

	if (exif.fFileSource <= 0x0FF)
		{
		fExifIFD.Add (&fFileSource);
		}

	if (exif.fSceneType <= 0x0FF)
		{
		fExifIFD.Add (&fSceneType);
		}

	if (exif.fCFARepeatPatternRows &&
	    exif.fCFARepeatPatternCols)
		{
		fExifIFD.Add (&fCFAPattern);
		}

	if (exif.fCustomRendered <= 0x0FFFF)
		{
		fExifIFD.Add (&fCustomRendered);
		}

	if (exif.fExposureMode <= 0x0FFFF)
		{
		fExifIFD.Add (&fExposureMode);
		}

	if (exif.fWhiteBalance <= 0x0FFFF)
		{
		fExifIFD.Add (&fWhiteBalance);
		}

	if (exif.fSceneCaptureType <= 0x0FFFF)
		{
		fExifIFD.Add (&fSceneCaptureType);
		}

	if (exif.fGainControl <= 0x0FFFF)
		{
		fExifIFD.Add (&fGainControl);
		}

	if (exif.fContrast <= 0x0FFFF)
		{
		fExifIFD.Add (&fContrast);
		}

	if (exif.fSaturation <= 0x0FFFF)
		{
		fExifIFD.Add (&fSaturation);
		}

	if (exif.fSharpness <= 0x0FFFF)
		{
		fExifIFD.Add (&fSharpness);
		}

	if (exif.fSubjectDistanceRange <= 0x0FFFF)
		{
		fExifIFD.Add (&fSubjectDistanceRange);
		}

	if (exif.fDigitalZoomRatio.IsValid ())
		{
		fExifIFD.Add (&fDigitalZoomRatio);
		}

	if (exif.fExposureIndex.IsValid ())
		{
		fExifIFD.Add (&fExposureIndex);
		}

	if (insideDNG)	// TIFF-EP only tags
		{

		if (exif.fImageNumber != 0xFFFFFFFF)
			{
			directory.Add (&fImageNumber);
			}

		if (exif.fSelfTimerMode <= 0x0FFFF)
			{
			directory.Add (&fSelfTimerMode);
			}

		if (exif.fBatteryLevelA.NotEmpty ())
			{
			directory.Add (&fBatteryLevelA);
			}

		else if (exif.fBatteryLevelR.IsValid ())
			{
			directory.Add (&fBatteryLevelR);
			}

		}

	if (exif.fFocalPlaneXResolution.IsValid ())
		{
		fExifIFD.Add (&fFocalPlaneXResolution);
		}

	if (exif.fFocalPlaneYResolution.IsValid ())
		{
		fExifIFD.Add (&fFocalPlaneYResolution);
		}

	if (exif.fFocalPlaneResolutionUnit <= 0x0FFFF)
		{
		fExifIFD.Add (&fFocalPlaneResolutionUnit);
		}

	if (exif.fSubjectAreaCount)
		{

		fSubjectAreaData [0] = (uint16) exif.fSubjectArea [0];
		fSubjectAreaData [1] = (uint16) exif.fSubjectArea [1];
		fSubjectAreaData [2] = (uint16) exif.fSubjectArea [2];
		fSubjectAreaData [3] = (uint16) exif.fSubjectArea [3];

		fExifIFD.Add (&fSubjectArea);

		}

	if (exif.fLensInfo [0].IsValid () &&
		exif.fLensInfo [1].IsValid () && insideDNG)
		{

		fLensInfoData [0] = exif.fLensInfo [0];
		fLensInfoData [1] = exif.fLensInfo [1];
		fLensInfoData [2] = exif.fLensInfo [2];
		fLensInfoData [3] = exif.fLensInfo [3];

		directory.Add (&fLensInfo);

		}

	if (exif.fDateTime.IsValid ())
		{

		directory.Add (&fDateTime);

		if (exif.fDateTime.Subseconds ().NotEmpty ())
			{
			fExifIFD.Add (&fSubsecTime);
			}

		}

	if (exif.fDateTimeOriginal.IsValid ())
		{

		fExifIFD.Add (&fDateTimeOriginal);

		if (exif.fDateTimeOriginal.Subseconds ().NotEmpty ())
			{
			fExifIFD.Add (&fSubsecTimeOriginal);
			}

		}

	if (exif.fDateTimeDigitized.IsValid ())
		{

		fExifIFD.Add (&fDateTimeDigitized);

		if (exif.fDateTimeDigitized.Subseconds ().NotEmpty ())
			{
			fExifIFD.Add (&fSubsecTimeDigitized);
			}

		}

	if (insideDNG)	// TIFF-EP only tags
		{

		if (exif.fDateTimeOriginal.IsValid  () &&
			exif.fDateTimeOriginal.TimeZone ().IsExactHourOffset ())
			{

			fTimeZoneOffsetData [0] = (int16) exif.fDateTimeOriginal.TimeZone ().ExactHourOffset ();
			fTimeZoneOffsetData [1] = (int16) exif.fDateTime        .TimeZone ().ExactHourOffset ();

			if (!exif.fDateTime.IsValid  () ||
				!exif.fDateTime.TimeZone ().IsExactHourOffset ())
				{
				fTimeZoneOffset.SetCount (1);
				}

			directory.Add (&fTimeZoneOffset);

			}

		}

	if (exif.fMake.NotEmpty ())
		{
		directory.Add (&fMake);
		}

	if (exif.fModel.NotEmpty ())
		{
		directory.Add (&fModel);
		}

	if (exif.fArtist.NotEmpty ())
		{
		directory.Add (&fArtist);
		}

	if (exif.fSoftware.NotEmpty ())
		{
		directory.Add (&fSoftware);
		}

	if (exif.fCopyright.NotEmpty ())
		{
		directory.Add (&fCopyright);
		}

	if (exif.fImageDescription.NotEmpty ())
		{
		directory.Add (&fImageDescription);
		}

	if (exif.fCameraSerialNumber.NotEmpty () && insideDNG)
		{
		directory.Add (&fSerialNumber);
		}

	if (makerNoteSafe && makerNoteData)
		{

		directory.Add (&fMakerNoteSafety);

		fExifIFD.Add (&fMakerNote);

		}

	if (exif.fUserComment.NotEmpty ())
		{
		fExifIFD.Add (&fUserComment);
		}

	if (exif.fImageUniqueID.IsValid ())
		{

		for (uint32 j = 0; j < 16; j++)
			{

			sprintf (fImageUniqueIDData + j * 2,
					 "%02X",
					 (unsigned) exif.fImageUniqueID.data [j]);

			}

		fExifIFD.Add (&fImageUniqueID);

		}

	if (exif.fGPSVersionID)
		{

		fGPSVersionData [0] = (uint8) (exif.fGPSVersionID >> 24);
		fGPSVersionData [1] = (uint8) (exif.fGPSVersionID >> 16);
		fGPSVersionData [2] = (uint8) (exif.fGPSVersionID >>  8);
		fGPSVersionData [3] = (uint8) (exif.fGPSVersionID      );

		fGPSIFD.Add (&fGPSVersionID);

		}

	if (exif.fGPSLatitudeRef.NotEmpty () &&
		exif.fGPSLatitude [0].IsValid ())
		{
		fGPSIFD.Add (&fGPSLatitudeRef);
		fGPSIFD.Add (&fGPSLatitude   );
		}

	if (exif.fGPSLongitudeRef.NotEmpty () &&
		exif.fGPSLongitude [0].IsValid ())
		{
		fGPSIFD.Add (&fGPSLongitudeRef);
		fGPSIFD.Add (&fGPSLongitude   );
		}

	if (exif.fGPSAltitudeRef <= 0x0FF)
		{
		fGPSIFD.Add (&fGPSAltitudeRef);
		}

	if (exif.fGPSAltitude.IsValid ())
		{
		fGPSIFD.Add (&fGPSAltitude);
		}

	if (exif.fGPSTimeStamp [0].IsValid ())
		{
		fGPSIFD.Add (&fGPSTimeStamp);
		}

	if (exif.fGPSSatellites.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSSatellites);
		}

	if (exif.fGPSStatus.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSStatus);
		}

	if (exif.fGPSMeasureMode.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSMeasureMode);
		}

	if (exif.fGPSDOP.IsValid ())
		{
		fGPSIFD.Add (&fGPSDOP);
		}

	if (exif.fGPSSpeedRef.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSSpeedRef);
		}

	if (exif.fGPSSpeed.IsValid ())
		{
		fGPSIFD.Add (&fGPSSpeed);
		}

	if (exif.fGPSTrackRef.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSTrackRef);
		}

	if (exif.fGPSTrack.IsValid ())
		{
		fGPSIFD.Add (&fGPSTrack);
		}

	if (exif.fGPSImgDirectionRef.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSImgDirectionRef);
		}

	if (exif.fGPSImgDirection.IsValid ())
		{
		fGPSIFD.Add (&fGPSImgDirection);
		}

	if (exif.fGPSMapDatum.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSMapDatum);
		}

	if (exif.fGPSDestLatitudeRef.NotEmpty () &&
		exif.fGPSDestLatitude [0].IsValid ())
		{
		fGPSIFD.Add (&fGPSDestLatitudeRef);
		fGPSIFD.Add (&fGPSDestLatitude   );
		}

	if (exif.fGPSDestLongitudeRef.NotEmpty () &&
		exif.fGPSDestLongitude [0].IsValid ())
		{
		fGPSIFD.Add (&fGPSDestLongitudeRef);
		fGPSIFD.Add (&fGPSDestLongitude   );
		}

	if (exif.fGPSDestBearingRef.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSDestBearingRef);
		}

	if (exif.fGPSDestBearing.IsValid ())
		{
		fGPSIFD.Add (&fGPSDestBearing);
		}

	if (exif.fGPSDestDistanceRef.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSDestDistanceRef);
		}

	if (exif.fGPSDestDistance.IsValid ())
		{
		fGPSIFD.Add (&fGPSDestDistance);
		}

	if (exif.fGPSProcessingMethod.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSProcessingMethod);
		}

	if (exif.fGPSAreaInformation.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSAreaInformation);
		}

	if (exif.fGPSDateStamp.NotEmpty ())
		{
		fGPSIFD.Add (&fGPSDateStamp);
		}

	if (exif.fGPSDifferential <= 0x0FFFF)
		{
		fGPSIFD.Add (&fGPSDifferential);
		}

	AddLinks (directory);

	}

/******************************************************************************/

void exif_tag_set::AddLinks (dng_tiff_directory &directory)
	{

	if (fExifIFD.Size () != 0 && !fAddedExifLink)
		{

		directory.Add (&fExifLink);

		fAddedExifLink = true;

		}

	if (fGPSIFD.Size () != 0 && !fAddedGPSLink)
		{

		directory.Add (&fGPSLink);

		fAddedGPSLink = true;

		}

	}

/******************************************************************************/

class range_tag_set
	{

	private:

		uint32 fActiveAreaData [4];

		tag_uint32_ptr fActiveArea;

		uint32 fMaskedAreaData [kMaxMaskedAreas * 4];

		tag_uint32_ptr fMaskedAreas;

		tag_uint16_ptr fLinearizationTable;

		uint16 fBlackLevelRepeatDimData [2];

		tag_uint16_ptr fBlackLevelRepeatDim;

		dng_urational fBlackLevelData [kMaxBlackPattern *
								       kMaxBlackPattern *
								       kMaxSamplesPerPixel];

		tag_urational_ptr fBlackLevel;

		dng_memory_data fBlackLevelDeltaHData;
		dng_memory_data fBlackLevelDeltaVData;

		tag_srational_ptr fBlackLevelDeltaH;
		tag_srational_ptr fBlackLevelDeltaV;

		uint16 fWhiteLevelData16 [kMaxSamplesPerPixel];
		uint32 fWhiteLevelData32 [kMaxSamplesPerPixel];

		tag_uint16_ptr fWhiteLevel16;
		tag_uint32_ptr fWhiteLevel32;

	public:

		range_tag_set (dng_tiff_directory &directory,
				       const dng_negative &negative);

	};

/******************************************************************************/

range_tag_set::range_tag_set (dng_tiff_directory &directory,
				     	      const dng_negative &negative)

	:	fActiveArea (tcActiveArea,
					 fActiveAreaData,
					 4)

	,	fMaskedAreas (tcMaskedAreas,
					  fMaskedAreaData,
					  0)

	,	fLinearizationTable (tcLinearizationTable,
							 NULL,
							 0)

	,	fBlackLevelRepeatDim (tcBlackLevelRepeatDim,
							  fBlackLevelRepeatDimData,
							  2)

	,	fBlackLevel (tcBlackLevel,
					 fBlackLevelData)

	,	fBlackLevelDeltaHData ()
	,	fBlackLevelDeltaVData ()

	,	fBlackLevelDeltaH (tcBlackLevelDeltaH)
	,	fBlackLevelDeltaV (tcBlackLevelDeltaV)

	,	fWhiteLevel16 (tcWhiteLevel,
					   fWhiteLevelData16)

	,	fWhiteLevel32 (tcWhiteLevel,
					   fWhiteLevelData32)

	{

	const dng_image &rawImage (negative.RawImage ());

	const dng_linearization_info *rangeInfo = negative.GetLinearizationInfo ();

	if (rangeInfo)
		{

		// ActiveArea:

			{

			const dng_rect &r = rangeInfo->fActiveArea;

			if (r.NotEmpty ())
				{

				fActiveAreaData [0] = r.t;
				fActiveAreaData [1] = r.l;
				fActiveAreaData [2] = r.b;
				fActiveAreaData [3] = r.r;

				directory.Add (&fActiveArea);

				}

			}

		// MaskedAreas:

		if (rangeInfo->fMaskedAreaCount)
			{

			fMaskedAreas.SetCount (rangeInfo->fMaskedAreaCount * 4);

			for (uint32 index = 0; index < rangeInfo->fMaskedAreaCount; index++)
				{

				const dng_rect &r = rangeInfo->fMaskedArea [index];

				fMaskedAreaData [index * 4 + 0] = r.t;
				fMaskedAreaData [index * 4 + 1] = r.l;
				fMaskedAreaData [index * 4 + 2] = r.b;
				fMaskedAreaData [index * 4 + 3] = r.r;

				}

			directory.Add (&fMaskedAreas);

			}

		// LinearizationTable:

		if (rangeInfo->fLinearizationTable.Get ())
			{

			fLinearizationTable.SetData  (rangeInfo->fLinearizationTable->Buffer_uint16 ()     );
			fLinearizationTable.SetCount (rangeInfo->fLinearizationTable->LogicalSize   () >> 1);

			directory.Add (&fLinearizationTable);

			}

		// BlackLevelRepeatDim:

			{

			fBlackLevelRepeatDimData [0] = (uint16) rangeInfo->fBlackLevelRepeatRows;
			fBlackLevelRepeatDimData [1] = (uint16) rangeInfo->fBlackLevelRepeatCols;

			directory.Add (&fBlackLevelRepeatDim);

			}

		// BlackLevel:

			{

			uint32 index = 0;

			for (uint16 v = 0; v < rangeInfo->fBlackLevelRepeatRows; v++)
				{

				for (uint32 h = 0; h < rangeInfo->fBlackLevelRepeatCols; h++)
					{

					for (uint32 c = 0; c < rawImage.Planes (); c++)
						{

						fBlackLevelData [index++] = rangeInfo->BlackLevel (v, h, c);

						}

					}

				}

			fBlackLevel.SetCount (rangeInfo->fBlackLevelRepeatRows *
								  rangeInfo->fBlackLevelRepeatCols * rawImage.Planes ());

			directory.Add (&fBlackLevel);

			}

		// BlackLevelDeltaH:

		if (rangeInfo->ColumnBlackCount ())
			{

			uint32 count = rangeInfo->ColumnBlackCount ();

			fBlackLevelDeltaHData.Allocate (count * sizeof (dng_srational));

			dng_srational *blacks = (dng_srational *) fBlackLevelDeltaHData.Buffer ();

			for (uint32 col = 0; col < count; col++)
				{

				blacks [col] = rangeInfo->ColumnBlack (col);

				}

			fBlackLevelDeltaH.SetData  (blacks);
			fBlackLevelDeltaH.SetCount (count );

			directory.Add (&fBlackLevelDeltaH);

			}

		// BlackLevelDeltaV:

		if (rangeInfo->RowBlackCount ())
			{

			uint32 count = rangeInfo->RowBlackCount ();

			fBlackLevelDeltaVData.Allocate (count * sizeof (dng_srational));

			dng_srational *blacks = (dng_srational *) fBlackLevelDeltaVData.Buffer ();

			for (uint32 row = 0; row < count; row++)
				{

				blacks [row] = rangeInfo->RowBlack (row);

				}

			fBlackLevelDeltaV.SetData  (blacks);
			fBlackLevelDeltaV.SetCount (count );

			directory.Add (&fBlackLevelDeltaV);

			}

		}

	// WhiteLevel:

	// Only use the 32-bit data type if we must use it since there
	// are some lazy (non-Adobe) DNG readers out there.

	bool needs32 = false;

	fWhiteLevel16.SetCount (rawImage.Planes ());
	fWhiteLevel32.SetCount (rawImage.Planes ());

	for (uint32 c = 0; c < fWhiteLevel16.Count (); c++)
		{

		fWhiteLevelData32 [c] = negative.WhiteLevel (c);

		if (fWhiteLevelData32 [c] > 0x0FFFF)
			{
			needs32 = true;
			}

		fWhiteLevelData16 [c] = (uint16) fWhiteLevelData32 [c];

		}

	if (needs32)
		{
		directory.Add (&fWhiteLevel32);
		}

	else
		{
		directory.Add (&fWhiteLevel16);
		}

	}

/******************************************************************************/

class mosaic_tag_set
	{

	private:

		uint16 fCFARepeatPatternDimData [2];

		tag_uint16_ptr fCFARepeatPatternDim;

		uint8 fCFAPatternData [kMaxCFAPattern *
							   kMaxCFAPattern];

		tag_uint8_ptr fCFAPattern;

		uint8 fCFAPlaneColorData [kMaxColorPlanes];

		tag_uint8_ptr fCFAPlaneColor;

		tag_uint16 fCFALayout;

		tag_uint32 fGreenSplit;

	public:

		mosaic_tag_set (dng_tiff_directory &directory,
				        const dng_mosaic_info &info);

	};

/******************************************************************************/

mosaic_tag_set::mosaic_tag_set (dng_tiff_directory &directory,
					            const dng_mosaic_info &info)

	:	fCFARepeatPatternDim (tcCFARepeatPatternDim,
						  	  fCFARepeatPatternDimData,
						  	  2)

	,	fCFAPattern (tcCFAPattern,
					 fCFAPatternData)

	,	fCFAPlaneColor (tcCFAPlaneColor,
						fCFAPlaneColorData)

	,	fCFALayout (tcCFALayout,
					(uint16) info.fCFALayout)

	,	fGreenSplit (tcBayerGreenSplit,
					 info.fBayerGreenSplit)

	{

	if (info.IsColorFilterArray ())
		{

		// CFARepeatPatternDim:

		fCFARepeatPatternDimData [0] = (uint16) info.fCFAPatternSize.v;
		fCFARepeatPatternDimData [1] = (uint16) info.fCFAPatternSize.h;

		directory.Add (&fCFARepeatPatternDim);

		// CFAPattern:

		fCFAPattern.SetCount (info.fCFAPatternSize.v *
							  info.fCFAPatternSize.h);

		for (int32 r = 0; r < info.fCFAPatternSize.v; r++)
			{

			for (int32 c = 0; c < info.fCFAPatternSize.h; c++)
				{

				fCFAPatternData [r * info.fCFAPatternSize.h + c] = info.fCFAPattern [r] [c];

				}

			}

		directory.Add (&fCFAPattern);

		// CFAPlaneColor:

		fCFAPlaneColor.SetCount (info.fColorPlanes);

		for (uint32 j = 0; j < info.fColorPlanes; j++)
			{

			fCFAPlaneColorData [j] = info.fCFAPlaneColor [j];

			}

		directory.Add (&fCFAPlaneColor);

		// CFALayout:

		fCFALayout.Set ((uint16) info.fCFALayout);

		directory.Add (&fCFALayout);

		// BayerGreenSplit:  (only include if the pattern is a Bayer pattern)

		if (info.fCFAPatternSize == dng_point (2, 2) &&
			info.fColorPlanes    == 3)
			{

			directory.Add (&fGreenSplit);

			}

		}

	}

/******************************************************************************/

class color_tag_set
	{

	private:

		uint32 fColorChannels;

		tag_matrix fCameraCalibration1;
		tag_matrix fCameraCalibration2;

		tag_string fCameraCalibrationSignature;

		tag_string fAsShotProfileName;

		dng_urational fAnalogBalanceData [4];

		tag_urational_ptr fAnalogBalance;

		dng_urational fAsShotNeutralData [4];

		tag_urational_ptr fAsShotNeutral;

		dng_urational fAsShotWhiteXYData [2];

		tag_urational_ptr fAsShotWhiteXY;

		tag_urational fLinearResponseLimit;

	public:

		color_tag_set (dng_tiff_directory &directory,
				       const dng_negative &negative);

	};

/******************************************************************************/

color_tag_set::color_tag_set (dng_tiff_directory &directory,
				     	  	  const dng_negative &negative)

	:	fColorChannels (negative.ColorChannels ())

	,	fCameraCalibration1 (tcCameraCalibration1,
						     negative.CameraCalibration1 ())

	,	fCameraCalibration2 (tcCameraCalibration2,
						     negative.CameraCalibration2 ())

	,	fCameraCalibrationSignature (tcCameraCalibrationSignature,
									 negative.CameraCalibrationSignature ())

	,	fAsShotProfileName (tcAsShotProfileName,
							negative.AsShotProfileName ())

	,	fAnalogBalance (tcAnalogBalance,
						fAnalogBalanceData,
						fColorChannels)

	,	fAsShotNeutral (tcAsShotNeutral,
						fAsShotNeutralData,
						fColorChannels)

	,	fAsShotWhiteXY (tcAsShotWhiteXY,
						fAsShotWhiteXYData,
						2)

	,	fLinearResponseLimit (tcLinearResponseLimit,
						      negative.LinearResponseLimitR ())

	{

	if (fColorChannels > 1)
		{

		uint32 channels2 = fColorChannels * fColorChannels;

		if (fCameraCalibration1.Count () == channels2)
			{

			directory.Add (&fCameraCalibration1);

			}

		if (fCameraCalibration2.Count () == channels2)
			{

			directory.Add (&fCameraCalibration2);

			}

		if (fCameraCalibration1.Count () == channels2 ||
			fCameraCalibration2.Count () == channels2)
			{

			if (negative.CameraCalibrationSignature ().NotEmpty ())
				{

				directory.Add (&fCameraCalibrationSignature);

				}

			}

		if (negative.AsShotProfileName ().NotEmpty ())
			{

			directory.Add (&fAsShotProfileName);

			}

		for (uint32 j = 0; j < fColorChannels; j++)
			{

			fAnalogBalanceData [j] = negative.AnalogBalanceR (j);

			}

		directory.Add (&fAnalogBalance);

		if (negative.HasCameraNeutral ())
			{

			for (uint32 k = 0; k < fColorChannels; k++)
				{

				fAsShotNeutralData [k] = negative.CameraNeutralR (k);

				}

			directory.Add (&fAsShotNeutral);

			}

		else if (negative.HasCameraWhiteXY ())
			{

			negative.GetCameraWhiteXY (fAsShotWhiteXYData [0],
									   fAsShotWhiteXYData [1]);

			directory.Add (&fAsShotWhiteXY);

			}

		directory.Add (&fLinearResponseLimit);

		}

	}

/******************************************************************************/

class profile_tag_set
	{

	private:

		tag_uint16 fCalibrationIlluminant1;
		tag_uint16 fCalibrationIlluminant2;

		tag_matrix fColorMatrix1;
		tag_matrix fColorMatrix2;

		tag_matrix fForwardMatrix1;
		tag_matrix fForwardMatrix2;

		tag_matrix fReductionMatrix1;
		tag_matrix fReductionMatrix2;

		tag_string fProfileName;

		tag_string fProfileCalibrationSignature;

		tag_uint32 fEmbedPolicyTag;

		tag_string fCopyrightTag;

		uint32 fHueSatMapDimData [3];

		tag_uint32_ptr fHueSatMapDims;

		tag_data_ptr fHueSatData1;
		tag_data_ptr fHueSatData2;

		uint32 fLookTableDimData [3];

		tag_uint32_ptr fLookTableDims;

		tag_data_ptr fLookTableData;

		dng_memory_data fToneCurveBuffer;

		tag_data_ptr fToneCurveTag;

	public:

		profile_tag_set (dng_tiff_directory &directory,
						 const dng_camera_profile &profile);

	};

/******************************************************************************/

profile_tag_set::profile_tag_set (dng_tiff_directory &directory,
				     	  	      const dng_camera_profile &profile)

	:	fCalibrationIlluminant1 (tcCalibrationIlluminant1,
								 (uint16) profile.CalibrationIlluminant1 ())

	,	fCalibrationIlluminant2 (tcCalibrationIlluminant2,
								 (uint16) profile.CalibrationIlluminant2 ())

	,	fColorMatrix1 (tcColorMatrix1,
					   profile.ColorMatrix1 ())

	,	fColorMatrix2 (tcColorMatrix2,
					   profile.ColorMatrix2 ())

	,	fForwardMatrix1 (tcForwardMatrix1,
						 profile.ForwardMatrix1 ())

	,	fForwardMatrix2 (tcForwardMatrix2,
						 profile.ForwardMatrix2 ())

	,	fReductionMatrix1 (tcReductionMatrix1,
						   profile.ReductionMatrix1 ())

	,	fReductionMatrix2 (tcReductionMatrix2,
						   profile.ReductionMatrix2 ())

	,	fProfileName (tcProfileName,
					  profile.Name (),
					  false)

	,	fProfileCalibrationSignature (tcProfileCalibrationSignature,
									  profile.ProfileCalibrationSignature (),
									  false)

	,	fEmbedPolicyTag (tcProfileEmbedPolicy,
						 profile.EmbedPolicy ())

	,	fCopyrightTag (tcProfileCopyright,
					   profile.Copyright (),
					   false)

	,	fHueSatMapDims (tcProfileHueSatMapDims,
						fHueSatMapDimData,
						3)

	,	fHueSatData1 (tcProfileHueSatMapData1,
					  ttFloat,
					  profile.HueSatDeltas1 ().DeltasCount () * 3,
					  profile.HueSatDeltas1 ().GetDeltas ())

	,	fHueSatData2 (tcProfileHueSatMapData2,
					  ttFloat,
					  profile.HueSatDeltas2 ().DeltasCount () * 3,
					  profile.HueSatDeltas2 ().GetDeltas ())

	,	fLookTableDims (tcProfileLookTableDims,
						fLookTableDimData,
						3)

	,	fLookTableData (tcProfileLookTableData,
						ttFloat,
						profile.LookTable ().DeltasCount () * 3,
					    profile.LookTable ().GetDeltas ())

	,	fToneCurveBuffer ()

	,	fToneCurveTag (tcProfileToneCurve,
					   ttFloat,
					   0,
					   NULL)

	{

	if (profile.HasColorMatrix1 ())
		{

		uint32 colorChannels = profile.ColorMatrix1 ().Rows ();

		directory.Add (&fCalibrationIlluminant1);

		directory.Add (&fColorMatrix1);

		if (fForwardMatrix1.Count () == colorChannels * 3)
			{

			directory.Add (&fForwardMatrix1);

			}

		if (colorChannels > 3 && fReductionMatrix1.Count () == colorChannels * 3)
			{

			directory.Add (&fReductionMatrix1);

			}

		if (profile.HasColorMatrix2 ())
			{

			directory.Add (&fCalibrationIlluminant2);

			directory.Add (&fColorMatrix2);

			if (fForwardMatrix2.Count () == colorChannels * 3)
				{

				directory.Add (&fForwardMatrix2);

				}

			if (colorChannels > 3 && fReductionMatrix2.Count () == colorChannels * 3)
				{

				directory.Add (&fReductionMatrix2);

				}

			}

		if (profile.Name ().NotEmpty ())
			{

			directory.Add (&fProfileName);

			}

		if (profile.ProfileCalibrationSignature ().NotEmpty ())
			{

			directory.Add (&fProfileCalibrationSignature);

			}

		directory.Add (&fEmbedPolicyTag);

		if (profile.Copyright ().NotEmpty ())
			{

			directory.Add (&fCopyrightTag);

			}

		bool haveHueSat1 = profile.HueSatDeltas1 ().IsValid ();

		bool haveHueSat2 = profile.HueSatDeltas2 ().IsValid () &&
						   profile.HasColorMatrix2 ();

		if (haveHueSat1 || haveHueSat2)
			{

			uint32 hueDivs = 0;
			uint32 satDivs = 0;
			uint32 valDivs = 0;

			if (haveHueSat1)
				{

				profile.HueSatDeltas1 ().GetDivisions (hueDivs,
													   satDivs,
													   valDivs);

				}

			else
				{

				profile.HueSatDeltas2 ().GetDivisions (hueDivs,
													   satDivs,
													   valDivs);

				}

			fHueSatMapDimData [0] = hueDivs;
			fHueSatMapDimData [1] = satDivs;
			fHueSatMapDimData [2] = valDivs;

			directory.Add (&fHueSatMapDims);

			}

		if (haveHueSat1)
			{

			directory.Add (&fHueSatData1);

			}

		if (haveHueSat2)
			{

			directory.Add (&fHueSatData2);

			}

		if (profile.HasLookTable ())
			{

			uint32 hueDivs = 0;
			uint32 satDivs = 0;
			uint32 valDivs = 0;

			profile.LookTable ().GetDivisions (hueDivs,
											   satDivs,
											   valDivs);

			fLookTableDimData [0] = hueDivs;
			fLookTableDimData [1] = satDivs;
			fLookTableDimData [2] = valDivs;

			directory.Add (&fLookTableDims);

			directory.Add (&fLookTableData);

			}

		if (profile.ToneCurve ().IsValid ())
			{

			// Tone curve stored as pairs of 32-bit coordinates.  Probably could do with
			// 16-bits here, but should be small number of points so...

			uint32 toneCurvePoints = (uint32) (profile.ToneCurve ().fCoord.size ());

			fToneCurveBuffer.Allocate (toneCurvePoints * 2 * sizeof (real32));

			real32 *points = fToneCurveBuffer.Buffer_real32 ();

			fToneCurveTag.SetCount (toneCurvePoints * 2);
			fToneCurveTag.SetData  (points);

			for (uint32 i = 0; i < toneCurvePoints; i++)
				{

				// Transpose coordinates so they are in a more expected
				// order (domain -> range).

				points [i * 2    ] = (real32) profile.ToneCurve ().fCoord [i].h;
				points [i * 2 + 1] = (real32) profile.ToneCurve ().fCoord [i].v;

				}

			directory.Add (&fToneCurveTag);

			}

		}

	}

/******************************************************************************/

tiff_dng_extended_color_profile::tiff_dng_extended_color_profile
								 (const dng_camera_profile &profile)

	:	fProfile (profile)

	{

	}

/******************************************************************************/

void tiff_dng_extended_color_profile::Put (dng_stream &stream,
										   bool includeModelRestriction)
	{

	// Profile header.

	stream.Put_uint16 (stream.BigEndian () ? byteOrderMM : byteOrderII);

	stream.Put_uint16 (magicExtendedProfile);

	stream.Put_uint32 (8);

	// Profile tags.

	profile_tag_set tagSet (*this, fProfile);

	// Camera this profile is for.

	tag_string cameraModelTag (tcUniqueCameraModel,
							   fProfile.UniqueCameraModelRestriction ());

	if (includeModelRestriction)
		{

		if (fProfile.UniqueCameraModelRestriction ().NotEmpty ())
			{

			Add (&cameraModelTag);

			}

		}

	// Write it all out.

	dng_tiff_directory::Put (stream, offsetsRelativeToExplicitBase, 8);

	}

/*****************************************************************************/

tag_dng_noise_profile::tag_dng_noise_profile (const dng_noise_profile &profile)

	:	tag_data_ptr (tcNoiseProfile,
					  ttDouble,
					  2 * profile.NumFunctions (),
					  fValues)

	{

	DNG_REQUIRE (profile.NumFunctions () <= kMaxColorPlanes,
				 "Too many noise functions in tag_dng_noise_profile.");

	for (uint32 i = 0; i < profile.NumFunctions (); i++)
		{

		fValues [(2 * i)	] = profile.NoiseFunction (i).Scale	 ();
		fValues [(2 * i) + 1] = profile.NoiseFunction (i).Offset ();

		}

	}

/*****************************************************************************/

dng_image_writer::dng_image_writer ()

	:	fCompressedBuffer   ()
	,	fUncompressedBuffer ()
	,	fSubTileBlockBuffer ()

	{

	}

/*****************************************************************************/

dng_image_writer::~dng_image_writer ()
	{

	}

/*****************************************************************************/

uint32 dng_image_writer::CompressedBufferSize (const dng_ifd &ifd,
											   uint32 uncompressedSize)
	{

	// If we are saving lossless JPEG from an 8-bit image, reserve
	// space to pad the data out to 16-bits.

	if (ifd.fCompression == ccJPEG && ifd.fBitsPerSample [0] <= 8)
		{

		return uncompressedSize * 2;

		}

	return 0;

	}

/*****************************************************************************/

void dng_image_writer::EncodePredictor (dng_host & /* host */,
									    const dng_ifd &ifd,
						        	    dng_pixel_buffer & /* buffer */)
	{

	if (ifd.fPredictor != cpNullPredictor)
		{

		ThrowProgramError ();

		}

	}

/*****************************************************************************/

void dng_image_writer::ByteSwapBuffer (dng_host & /* host */,
									   dng_pixel_buffer &buffer)
	{

	uint32 pixels = buffer.fRowStep * buffer.fArea.H ();

	switch (buffer.fPixelSize)
		{

		case 2:
			{

			DoSwapBytes16 ((uint16 *) buffer.fData,
						   pixels);

			break;

			}

		case 4:
			{

			DoSwapBytes32 ((uint32 *) buffer.fData,
						   pixels);

			break;

			}

		default:
			break;

		}

	}

/*****************************************************************************/

void dng_image_writer::ReorderSubTileBlocks (const dng_ifd &ifd,
											 dng_pixel_buffer &buffer)
	{

	uint32 blockRows = ifd.fSubTileBlockRows;
	uint32 blockCols = ifd.fSubTileBlockCols;

	uint32 rowBlocks = buffer.fArea.H () / blockRows;
	uint32 colBlocks = buffer.fArea.W () / blockCols;

	int32 rowStep = buffer.fRowStep * buffer.fPixelSize;
	int32 colStep = buffer.fColStep * buffer.fPixelSize;

	int32 rowBlockStep = rowStep * blockRows;
	int32 colBlockStep = colStep * blockCols;

	uint32 blockColBytes = blockCols * buffer.fPlanes * buffer.fPixelSize;

	const uint8 *s0 = fUncompressedBuffer->Buffer_uint8 ();
	      uint8 *d0 = fSubTileBlockBuffer->Buffer_uint8 ();

	for (uint32 rowBlock = 0; rowBlock < rowBlocks; rowBlock++)
		{

		const uint8 *s1 = s0;

		for (uint32 colBlock = 0; colBlock < colBlocks; colBlock++)
			{

			const uint8 *s2 = s1;

			for (uint32 blockRow = 0; blockRow < blockRows; blockRow++)
				{

				for (uint32 j = 0; j < blockColBytes; j++)
					{

					d0 [j] = s2 [j];

					}

				d0 += blockColBytes;

				s2 += rowStep;

				}

			s1 += colBlockStep;

			}

		s0 += rowBlockStep;

		}

	// Copy back reordered pixels.

	DoCopyBytes (fSubTileBlockBuffer->Buffer      (),
				 fUncompressedBuffer->Buffer      (),
				 fUncompressedBuffer->LogicalSize ());

	}

/*****************************************************************************/

void dng_image_writer::WriteData (dng_host &host,
								  const dng_ifd &ifd,
						          dng_stream &stream,
						          dng_pixel_buffer &buffer)
	{

	switch (ifd.fCompression)
		{

		case ccUncompressed:
			{

			// Special case support for when we save to 8-bits from
			// 16-bit data.

			if (ifd.fBitsPerSample [0] == 8 && buffer.fPixelType == ttShort)
				{

				uint32 count = buffer.fRowStep *
							   buffer.fArea.H ();

				const uint16 *sPtr = (const uint16 *) buffer.fData;

				for (uint32 j = 0; j < count; j++)
					{

					stream.Put_uint8 ((uint8) sPtr [j]);

					}

				}

			else
				{

				// Swap bytes if required.

				if (stream.SwapBytes ())
					{

					ByteSwapBuffer (host, buffer);

					}

				// Write the bytes.

				stream.Put (buffer.fData, buffer.fRowStep *
										  buffer.fArea.H () *
										  buffer.fPixelSize);

				}

			break;

			}

		case ccJPEG:
			{

			dng_pixel_buffer temp (buffer);

			if (buffer.fPixelType == ttByte)
				{

				// The lossless JPEG encoder needs 16-bit data, so if we are
				// are saving 8 bit data, we need to pad it out to 16-bits.

				temp.fData = fCompressedBuffer->Buffer ();

				temp.fPixelType = ttShort;
				temp.fPixelSize = 2;

				temp.CopyArea (buffer,
							   buffer.fArea,
							   buffer.fPlane,
							   buffer.fPlanes);

				}

			EncodeLosslessJPEG ((const uint16 *) temp.fData,
								temp.fArea.H (),
								temp.fArea.W (),
								temp.fPlanes,
								ifd.fBitsPerSample [0],
								temp.fRowStep,
								temp.fColStep,
								stream);

			break;

			}

		default:
			{

			ThrowProgramError ();
			break;
			}

		}

	}

/*****************************************************************************/

void dng_image_writer::WriteTile (dng_host &host,
						          const dng_ifd &ifd,
						          dng_stream &stream,
						          const dng_image &image,
						          const dng_rect &tileArea,
						          uint32 fakeChannels)
	{

	// Create pixel buffer to hold uncompressed tile.

	dng_pixel_buffer buffer;

	buffer.fArea = tileArea;

	buffer.fPlane  = 0;
	buffer.fPlanes = ifd.fSamplesPerPixel;

	buffer.fRowStep   = buffer.fPlanes * tileArea.W ();
	buffer.fColStep   = buffer.fPlanes;
	buffer.fPlaneStep = 1;

	buffer.fPixelType = image.PixelType ();
	buffer.fPixelSize = image.PixelSize ();

	buffer.fData = fUncompressedBuffer->Buffer ();

	// Get the uncompressed data.

	image.Get (buffer, dng_image::edge_zero);

	// Deal with sub-tile blocks.

	if (ifd.fSubTileBlockRows > 1)
		{

		ReorderSubTileBlocks (ifd, buffer);

		}

	// Run predictor.

	EncodePredictor (host,
					 ifd,
					 buffer);

	// Adjust pixel buffer for fake channels.

	if (fakeChannels > 1)
		{

		buffer.fPlanes  *= fakeChannels;
		buffer.fColStep *= fakeChannels;

		buffer.fArea.r = buffer.fArea.l + (buffer.fArea.W () / fakeChannels);

		}

	// Compress (if required) and write out the data.

	WriteData (host,
			   ifd,
			   stream,
			   buffer);

	}

/*****************************************************************************/

void dng_image_writer::WriteImage (dng_host &host,
						           const dng_ifd &ifd,
						           dng_basic_tag_set &basic,
						           dng_stream &stream,
						           const dng_image &image,
						           uint32 fakeChannels)
	{

	// Deal with row interleaved images.

	if (ifd.fRowInterleaveFactor > 1 &&
		ifd.fRowInterleaveFactor < ifd.fImageLength)
		{

		dng_ifd tempIFD (ifd);

		tempIFD.fRowInterleaveFactor = 1;

		dng_row_interleaved_image tempImage (*((dng_image *) &image),
											 ifd.fRowInterleaveFactor);

		WriteImage (host,
					tempIFD,
					basic,
					stream,
					tempImage,
					fakeChannels);

		return;

		}

	// Compute basic information.

	uint32 bytesPerSample = TagTypeSize (image.PixelType ());

	uint32 bytesPerPixel = ifd.fSamplesPerPixel * bytesPerSample;

	uint32 tileRowBytes = ifd.fTileWidth * bytesPerPixel;

	// If we can compute the number of bytes needed to store the
	// data, we can split the write for each tile into sub-tiles.

	uint32 subTileLength = ifd.fTileLength;

	if (ifd.TileByteCount (ifd.TileArea (0, 0)) != 0)
		{

		subTileLength = Pin_uint32 (ifd.fSubTileBlockRows,
									kImageBufferSize / tileRowBytes,
									ifd.fTileLength);

		// Don't split sub-tiles across subTileBlocks.

		subTileLength = subTileLength / ifd.fSubTileBlockRows
									  * ifd.fSubTileBlockRows;

		}

	// Allocate buffer to hold one sub-tile of uncompressed data.

	uint32 uncompressedSize = subTileLength * tileRowBytes;

	fUncompressedBuffer.Reset (host.Allocate (uncompressedSize));

	// Buffer to repack tiles order.

	if (ifd.fSubTileBlockRows > 1)
		{

		fSubTileBlockBuffer.Reset (host.Allocate (uncompressedSize));

		}

	// Allocate compressed buffer, if required.

	uint32 compressedSize = CompressedBufferSize (ifd, uncompressedSize);

	if (compressedSize)
		{

		fCompressedBuffer.Reset (host.Allocate (compressedSize));

		}

	// Write out each tile.

	uint32 tileIndex = 0;

	uint32 tilesAcross = ifd.TilesAcross ();
	uint32 tilesDown   = ifd.TilesDown   ();

	for (uint32 rowIndex = 0; rowIndex < tilesDown; rowIndex++)
		{

		for (uint32 colIndex = 0; colIndex < tilesAcross; colIndex++)
			{

			// Remember this offset.

			uint32 tileOffset = (uint32) stream.Position ();

			basic.SetTileOffset (tileIndex, tileOffset);

			// Split tile into sub-tiles if possible.

			dng_rect tileArea = ifd.TileArea (rowIndex, colIndex);

			uint32 subTileCount = (tileArea.H () + subTileLength - 1) /
								  subTileLength;

			for (uint32 subIndex = 0; subIndex < subTileCount; subIndex++)
				{

				host.SniffForAbort ();

				dng_rect subArea (tileArea);

				subArea.t = tileArea.t + subIndex * subTileLength;

				subArea.b = Min_int32 (subArea.t + subTileLength,
									   tileArea.b);

				// Write the sub-tile.

				WriteTile (host,
						   ifd,
						   stream,
						   image,
						   subArea,
						   fakeChannels);

				}

			// Update tile count.

			uint32 tileByteCount = (uint32) stream.Position () - tileOffset;

			basic.SetTileByteCount (tileIndex, tileByteCount);

			tileIndex++;

			// Keep the tiles on even byte offsets.

			if (tileByteCount & 1)
				{
				stream.Put_uint8 (0);
				}

			}

		}

	// We are done with the compression buffers.

	fCompressedBuffer  .Reset ();
	fUncompressedBuffer.Reset ();

	}

/*****************************************************************************/

void dng_image_writer::WriteTIFF (dng_host &host,
								  dng_stream &stream,
								  const dng_image &image,
								  uint32 photometricInterpretation,
								  uint32 compression,
								  dng_negative *negative,
								  const dng_color_space *space,
								  const dng_resolution *resolution,
								  const dng_jpeg_preview *thumbnail,
								  const dng_memory_block *imageResources)
	{

	const void *profileData = NULL;
	uint32 profileSize = 0;

	const uint8 *data = NULL;
	uint32 size = 0;

	if (space && space->ICCProfile (size, data))
		{

		profileData = data;
		profileSize = size;

		}

	WriteTIFFWithProfile (host,
						  stream,
						  image,
						  photometricInterpretation,
						  compression,
						  negative,
						  profileData,
						  profileSize,
						  resolution,
						  thumbnail,
						  imageResources);

	}

/*****************************************************************************/

void dng_image_writer::WriteTIFFWithProfile (dng_host &host,
											 dng_stream &stream,
											 const dng_image &image,
											 uint32 photometricInterpretation,
											 uint32 compression,
											 dng_negative *negative,
											 const void *profileData,
											 uint32 profileSize,
											 const dng_resolution *resolution,
											 const dng_jpeg_preview *thumbnail,
											 const dng_memory_block *imageResources)
	{

	uint32 j;

	dng_ifd ifd;

	ifd.fNewSubFileType = sfMainImage;

	ifd.fImageWidth  = image.Bounds ().W ();
	ifd.fImageLength = image.Bounds ().H ();

	ifd.fSamplesPerPixel = image.Planes ();

	ifd.fBitsPerSample [0] = TagTypeSize (image.PixelType ()) * 8;

	for (j = 1; j < ifd.fSamplesPerPixel; j++)
		{
		ifd.fBitsPerSample [j] = ifd.fBitsPerSample [0];
		}

	ifd.fPhotometricInterpretation = photometricInterpretation;

	ifd.fCompression = compression;

	if (ifd.fCompression == ccUncompressed)
		{

		ifd.SetSingleStrip ();

		}

	else
		{

		ifd.FindStripSize (128 * 1024);

		ifd.fPredictor = cpHorizontalDifference;

		}

	uint32 extraSamples = 0;

	switch (photometricInterpretation)
		{

		case piBlackIsZero:
			{
			extraSamples = image.Planes () - 1;
			break;
			}

		case piRGB:
			{
			extraSamples = image.Planes () - 3;
			break;
			}

		default:
			break;

		}

	ifd.fExtraSamplesCount = extraSamples;

	if (image.PixelType () == ttFloat)
		{

		for (j = 0; j < ifd.fSamplesPerPixel; j++)
			{
			ifd.fSampleFormat [j] = sfFloatingPoint;
			}

		}

	dng_tiff_directory mainIFD;

	dng_basic_tag_set basic (mainIFD, ifd);

	// Resolution.

	dng_resolution res;

	if (resolution)
		{
		res = *resolution;
		}

	tag_urational tagXResolution (tcXResolution, res.fXResolution);
	tag_urational tagYResolution (tcYResolution, res.fYResolution);

	tag_uint16 tagResolutionUnit (tcResolutionUnit, res.fResolutionUnit);

	if (resolution)
		{
		mainIFD.Add (&tagXResolution   );
		mainIFD.Add (&tagYResolution   );
		mainIFD.Add (&tagResolutionUnit);
		}

	// ICC Profile.

	tag_icc_profile iccProfileTag (profileData, profileSize);

	if (iccProfileTag.Count ())
		{
		mainIFD.Add (&iccProfileTag);
		}

	// Rebuild IPTC with TIFF padding bytes.

	if (negative && negative->GetXMP ())
		{

		negative->RebuildIPTC (true, false);

		}

	// XMP metadata.

	AutoPtr<dng_xmp> xmp;

	if (negative && negative->GetXMP ())
		{

		xmp.Reset (new dng_xmp (*negative->GetXMP ()));

		xmp->ClearOrientation ();

		xmp->ClearImageInfo ();

		xmp->SetImageSize (image.Size ());

		xmp->SetSampleInfo (ifd.fSamplesPerPixel,
							ifd.fBitsPerSample [0]);

		xmp->SetPhotometricInterpretation (ifd.fPhotometricInterpretation);

		if (resolution)
			{
			xmp->SetResolution (*resolution);
			}

		}

	tag_xmp tagXMP (xmp.Get ());

	if (tagXMP.Count ())
		{
		mainIFD.Add (&tagXMP);
		}

	xmp.Reset ();

	// IPTC metadata.

	tag_iptc tagIPTC (negative ? negative->IPTCData   () : NULL,
					  negative ? negative->IPTCLength () : 0);

	if (tagIPTC.Count ())
		{
		mainIFD.Add (&tagIPTC);
		}

	// Adobe data (thumbnail and IPTC digest)

	AutoPtr<dng_memory_block> adobeData (BuildAdobeData (host,
														 negative,
														 thumbnail,
														 imageResources));

	tag_uint8_ptr tagAdobe (tcAdobeData,
							adobeData->Buffer_uint8 (),
							adobeData->LogicalSize ());

	if (tagAdobe.Count ())
		{
		mainIFD.Add (&tagAdobe);
		}

	// Exif metadata.

	exif_tag_set exifSet (mainIFD,
						  negative && negative->GetExif () ? *negative->GetExif ()
														   : dng_exif (),
						  negative ? negative->IsMakerNoteSafe () : false,
						  negative ? negative->MakerNoteData   () : NULL,
						  negative ? negative->MakerNoteLength () : 0,
						  false);

	// Find offset to main image data.

	uint32 offsetMainIFD = 8;

	uint32 offsetExifData = offsetMainIFD + mainIFD.Size ();

	exifSet.Locate (offsetExifData);

	uint32 offsetMainData = offsetExifData + exifSet.Size ();

	stream.SetWritePosition (offsetMainData);

	// Write the main image data.

	WriteImage (host,
				ifd,
				basic,
				stream,
				image);

	// Trim the file to this length.

	stream.SetLength (stream.Position ());

	// TIFF has a 4G size limit.

	if (stream.Length () > 0x0FFFFFFFFL)
		{
		ThrowImageTooBigTIFF ();
		}

	// Write TIFF Header.

	stream.SetWritePosition (0);

	stream.Put_uint16 (stream.BigEndian () ? byteOrderMM : byteOrderII);

	stream.Put_uint16 (42);

	stream.Put_uint32 (offsetMainIFD);

	// Write the IFDs.

	mainIFD.Put (stream);

	exifSet.Put (stream);

	stream.Flush ();

	}

/*****************************************************************************/

void dng_image_writer::WriteDNG (dng_host &host,
							     dng_stream &stream,
							     const dng_negative &negative,
							     const dng_image_preview &thumbnail,
							     uint32 compression,
							     const dng_preview_list *previewList)
	{

	uint32 j;

	// Figure out what main version to use.

	uint32 dngVersion = dngVersion_Current;

	// Figure out what backward version to use.

	uint32 dngBackwardVersion = dngVersion_1_1_0_0;

	#if defined(qTestRowInterleave) || defined(qTestSubTileBlockRows) || defined(qTestSubTileBlockCols)
	dngBackwardVersion = Max_uint32 (dngBackwardVersion, dngVersion_1_2_0_0);
	#endif

	dngBackwardVersion = Max_uint32 (dngBackwardVersion,
									 negative.OpcodeList1 ().MinVersion (false));

	dngBackwardVersion = Max_uint32 (dngBackwardVersion,
									 negative.OpcodeList2 ().MinVersion (false));

	dngBackwardVersion = Max_uint32 (dngBackwardVersion,
									 negative.OpcodeList3 ().MinVersion (false));

	if (negative.GetMosaicInfo () &&
		negative.GetMosaicInfo ()->fCFALayout >= 6)
		{
		dngBackwardVersion = Max_uint32 (dngBackwardVersion, dngVersion_1_3_0_0);
		}

	if (dngBackwardVersion > dngVersion)
		{
		ThrowProgramError ();
		}

	// Create the main IFD

	dng_tiff_directory mainIFD;

	// Include DNG version tags.

	uint8 dngVersionData [4];

	dngVersionData [0] = (uint8) (dngVersion >> 24);
	dngVersionData [1] = (uint8) (dngVersion >> 16);
	dngVersionData [2] = (uint8) (dngVersion >>  8);
	dngVersionData [3] = (uint8) (dngVersion      );

	tag_uint8_ptr tagDNGVersion (tcDNGVersion, dngVersionData, 4);

	mainIFD.Add (&tagDNGVersion);

	uint8 dngBackwardVersionData [4];

	dngBackwardVersionData [0] = (uint8) (dngBackwardVersion >> 24);
	dngBackwardVersionData [1] = (uint8) (dngBackwardVersion >> 16);
	dngBackwardVersionData [2] = (uint8) (dngBackwardVersion >>  8);
	dngBackwardVersionData [3] = (uint8) (dngBackwardVersion      );

	tag_uint8_ptr tagDNGBackwardVersion (tcDNGBackwardVersion, dngBackwardVersionData, 4);

	mainIFD.Add (&tagDNGBackwardVersion);

	// The main IFD contains the thumbnail.

	AutoPtr<dng_basic_tag_set> thmBasic (thumbnail.AddTagSet (mainIFD));

	// Get the raw image we are writing.

	const dng_image &rawImage (negative.RawImage ());

	// We currently don't support compression for deeper
	// than 16-bit images.

	if (rawImage.PixelType () == ttLong)
		{
		compression = ccUncompressed;
		}

	// Get a copy of the mosaic info.

	dng_mosaic_info mosaicInfo;

	if (negative.GetMosaicInfo ())
		{
		mosaicInfo = *(negative.GetMosaicInfo ());
		}

	// Create the IFD for the raw data.

	dng_tiff_directory rawIFD;

	// Create a dng_ifd record for the raw image.

	dng_ifd info;

	info.fImageWidth  = rawImage.Width  ();
	info.fImageLength = rawImage.Height ();

	info.fSamplesPerPixel = rawImage.Planes ();

	info.fPhotometricInterpretation = mosaicInfo.IsColorFilterArray () ? piCFA
																	   : piLinearRaw;

	info.fCompression = compression;

	uint32 rawPixelType = rawImage.PixelType ();

	if (rawPixelType == ttShort)
		{

		// See if we are using a linearization table with <= 256 entries, in which
		// case the useful data will all fit within 8-bits.

		const dng_linearization_info *rangeInfo = negative.GetLinearizationInfo ();

		if (rangeInfo)
			{

			if (rangeInfo->fLinearizationTable.Get ())
				{

				uint32 entries = rangeInfo->fLinearizationTable->LogicalSize () >> 1;

				if (entries <= 256)
					{

					rawPixelType = ttByte;

					}

				}

			}

		}

	switch (rawPixelType)
		{

		case ttByte:
			{
			info.fBitsPerSample [0] = 8;
			break;
			}

		case ttShort:
			{
			info.fBitsPerSample [0] = 16;
			break;
			}

		case ttLong:
			{
			info.fBitsPerSample [0] = 32;
			break;
			}

		default:
			{
			ThrowProgramError ();
			break;
			}

		}

	// For lossless JPEG compression, we often lie about the
	// actual channel count to get the predictors to work across
	// same color mosaic pixels.

	uint32 fakeChannels = 1;

	if (info.fCompression == ccJPEG)
		{

		if (mosaicInfo.IsColorFilterArray ())
			{

			if (mosaicInfo.fCFAPatternSize.h == 4)
				{
				fakeChannels = 4;
				}

			else if (mosaicInfo.fCFAPatternSize.h == 2)
				{
				fakeChannels = 2;
				}

			// However, lossless JEPG is limited to four channels,
			// so compromise might be required.

			while (fakeChannels * info.fSamplesPerPixel > 4 &&
				   fakeChannels > 1)
				{

				fakeChannels >>= 1;

				}

			}

		}

	// Figure out tile sizes.

	if (info.fCompression == ccJPEG)
		{

		info.FindTileSize (128 * 1024);

		}

	// Don't use tiles for uncompressed images.

	else
		{

		info.SetSingleStrip ();

		}

	#ifdef qTestRowInterleave

	info.fRowInterleaveFactor = qTestRowInterleave;

	#endif

	#if defined(qTestSubTileBlockRows) && defined(qTestSubTileBlockCols)

	info.fSubTileBlockRows = qTestSubTileBlockRows;
	info.fSubTileBlockCols = qTestSubTileBlockCols;

	if (fakeChannels == 2)
		fakeChannels = 4;

	#endif

	// Basic information.

	dng_basic_tag_set rawBasic (rawIFD, info);

	// DefaultScale tag.

	dng_urational defaultScaleData [2];

	defaultScaleData [0] = negative.DefaultScaleH ();
	defaultScaleData [1] = negative.DefaultScaleV ();

	tag_urational_ptr tagDefaultScale (tcDefaultScale,
								       defaultScaleData,
								       2);

	rawIFD.Add (&tagDefaultScale);

	// Best quality scale tag.

	tag_urational tagBestQualityScale (tcBestQualityScale,
									   negative.BestQualityScale ());

	rawIFD.Add (&tagBestQualityScale);

	// DefaultCropOrigin tag.

	dng_urational defaultCropOriginData [2];

	defaultCropOriginData [0] = negative.DefaultCropOriginH ();
	defaultCropOriginData [1] = negative.DefaultCropOriginV ();

	tag_urational_ptr tagDefaultCropOrigin (tcDefaultCropOrigin,
								            defaultCropOriginData,
								            2);

	rawIFD.Add (&tagDefaultCropOrigin);

	// DefaultCropSize tag.

	dng_urational defaultCropSizeData [2];

	defaultCropSizeData [0] = negative.DefaultCropSizeH ();
	defaultCropSizeData [1] = negative.DefaultCropSizeV ();

	tag_urational_ptr tagDefaultCropSize (tcDefaultCropSize,
								          defaultCropSizeData,
								          2);

	rawIFD.Add (&tagDefaultCropSize);

	// Range mapping tag set.

	range_tag_set rangeSet (rawIFD, negative);

	// Mosaic pattern information.

	mosaic_tag_set mosaicSet (rawIFD, mosaicInfo);

	// Chroma blur radius.

	tag_urational tagChromaBlurRadius (tcChromaBlurRadius,
									   negative.ChromaBlurRadius ());

	if (negative.ChromaBlurRadius ().IsValid ())
		{

		rawIFD.Add (&tagChromaBlurRadius);

		}

	// Anti-alias filter strength.

	tag_urational tagAntiAliasStrength (tcAntiAliasStrength,
									    negative.AntiAliasStrength ());

	if (negative.AntiAliasStrength ().IsValid ())
		{

		rawIFD.Add (&tagAntiAliasStrength);

		}

	// Profile and other color related tags.

	AutoPtr<profile_tag_set> profileSet;

	AutoPtr<color_tag_set> colorSet;

	std::vector<uint32> extraProfileIndex;

	if (!negative.IsMonochrome ())
		{

		const dng_camera_profile &mainProfile (*negative.CameraProfileToEmbed ());

		profileSet.Reset (new profile_tag_set (mainIFD,
											   mainProfile));

		colorSet.Reset (new color_tag_set (mainIFD,
										   negative));

		// Build list of profile indices to include in extra profiles tag.

		uint32 profileCount = negative.ProfileCount ();

		for (uint32 index = 0; index < profileCount; index++)
			{

			const dng_camera_profile &profile (negative.ProfileByIndex (index));

			if (&profile != &mainProfile)
				{

				if (profile.WasReadFromDNG ())
					{

					extraProfileIndex.push_back (index);

					}

				}

			}

		}

	// Extra camera profiles tag.

	uint32 extraProfileCount = (uint32) extraProfileIndex.size ();

	dng_memory_data extraProfileOffsets (extraProfileCount * sizeof (uint32));

	tag_uint32_ptr extraProfileTag (tcExtraCameraProfiles,
									extraProfileOffsets.Buffer_uint32 (),
									extraProfileCount);

	if (extraProfileCount)
		{

		mainIFD.Add (&extraProfileTag);

		}

	// Other tags.

	tag_uint16 tagOrientation (tcOrientation,
						       (uint16) negative.Orientation ().GetTIFF ());

	mainIFD.Add (&tagOrientation);

	tag_srational tagBaselineExposure (tcBaselineExposure,
								       negative.BaselineExposureR ());

	mainIFD.Add (&tagBaselineExposure);

	tag_urational tagBaselineNoise (tcBaselineNoise,
							        negative.BaselineNoiseR ());

	mainIFD.Add (&tagBaselineNoise);

	tag_urational tagNoiseReductionApplied (tcNoiseReductionApplied,
											negative.NoiseReductionApplied ());

	if (negative.NoiseReductionApplied ().IsValid ())
		{

		mainIFD.Add (&tagNoiseReductionApplied);

		}

	tag_dng_noise_profile tagNoiseProfile (negative.NoiseProfile ());

	if (negative.NoiseProfile ().IsValidForNegative (negative))
		{

		mainIFD.Add (&tagNoiseProfile);

		}

	tag_urational tagBaselineSharpness (tcBaselineSharpness,
								        negative.BaselineSharpnessR ());

	mainIFD.Add (&tagBaselineSharpness);

	tag_string tagUniqueName (tcUniqueCameraModel,
						      negative.ModelName (),
						      true);

	mainIFD.Add (&tagUniqueName);

	tag_string tagLocalName (tcLocalizedCameraModel,
						     negative.LocalName (),
						     false);

	if (negative.LocalName ().NotEmpty ())
		{

		mainIFD.Add (&tagLocalName);

		}

	tag_urational tagShadowScale (tcShadowScale,
							      negative.ShadowScaleR ());

	mainIFD.Add (&tagShadowScale);

	tag_uint16 tagColorimetricReference (tcColorimetricReference,
										 (uint16) negative.ColorimetricReference ());

	if (negative.ColorimetricReference () != crSceneReferred)
		{

		mainIFD.Add (&tagColorimetricReference);

		}

	negative.FindRawImageDigest (host);

	tag_uint8_ptr tagRawImageDigest (tcRawImageDigest,
									 negative.RawImageDigest ().data,
							   		 16);

	if (negative.RawImageDigest ().IsValid ())
		{

		mainIFD.Add (&tagRawImageDigest);

		}

	negative.FindRawDataUniqueID (host);

	tag_uint8_ptr tagRawDataUniqueID (tcRawDataUniqueID,
							   		  negative.RawDataUniqueID ().data,
							   		  16);

	if (negative.RawDataUniqueID ().IsValid ())
		{

		mainIFD.Add (&tagRawDataUniqueID);

		}

	tag_string tagOriginalRawFileName (tcOriginalRawFileName,
						   			   negative.OriginalRawFileName (),
						   			   false);

	if (negative.HasOriginalRawFileName ())
		{

		mainIFD.Add (&tagOriginalRawFileName);

		}

	negative.FindOriginalRawFileDigest ();

	tag_data_ptr tagOriginalRawFileData (tcOriginalRawFileData,
										 ttUndefined,
										 negative.OriginalRawFileDataLength (),
										 negative.OriginalRawFileData       ());

	tag_uint8_ptr tagOriginalRawFileDigest (tcOriginalRawFileDigest,
											negative.OriginalRawFileDigest ().data,
											16);

	if (negative.OriginalRawFileData ())
		{

		mainIFD.Add (&tagOriginalRawFileData);

		mainIFD.Add (&tagOriginalRawFileDigest);

		}

	// XMP metadata.

	AutoPtr<dng_xmp> xmp;

	if (negative.GetXMP ())
		{

		xmp.Reset (new dng_xmp (*negative.GetXMP ()));

		// Make sure the XMP orientation always matches the
		// tag orientation.

		xmp->SetOrientation (negative.Orientation ());

		}

	tag_xmp tagXMP (xmp.Get ());

	if (tagXMP.Count ())
		{

		mainIFD.Add (&tagXMP);

		}

	xmp.Reset ();

	// Exif tags.

	exif_tag_set exifSet (mainIFD,
						  *negative.GetExif (),
						  negative.IsMakerNoteSafe (),
						  negative.MakerNoteData   (),
						  negative.MakerNoteLength (),
						  true);

	// Private data.

	tag_uint8_ptr tagPrivateData (tcDNGPrivateData,
						   		  negative.PrivateData (),
						   		  negative.PrivateLength ());

	if (negative.PrivateLength ())
		{

		mainIFD.Add (&tagPrivateData);

		}

	// Opcode list 1.

	AutoPtr<dng_memory_block> opcodeList1Data (negative.OpcodeList1 ().Spool (host));

	tag_data_ptr tagOpcodeList1 (tcOpcodeList1,
								 ttUndefined,
								 opcodeList1Data.Get () ? opcodeList1Data->LogicalSize () : 0,
								 opcodeList1Data.Get () ? opcodeList1Data->Buffer      () : NULL);

	if (opcodeList1Data.Get ())
		{

		rawIFD.Add (&tagOpcodeList1);

		}

	// Opcode list 2.

	AutoPtr<dng_memory_block> opcodeList2Data (negative.OpcodeList2 ().Spool (host));

	tag_data_ptr tagOpcodeList2 (tcOpcodeList2,
								 ttUndefined,
								 opcodeList2Data.Get () ? opcodeList2Data->LogicalSize () : 0,
								 opcodeList2Data.Get () ? opcodeList2Data->Buffer      () : NULL);

	if (opcodeList2Data.Get ())
		{

		rawIFD.Add (&tagOpcodeList2);

		}

	// Opcode list 3.

	AutoPtr<dng_memory_block> opcodeList3Data (negative.OpcodeList3 ().Spool (host));

	tag_data_ptr tagOpcodeList3 (tcOpcodeList3,
								 ttUndefined,
								 opcodeList3Data.Get () ? opcodeList3Data->LogicalSize () : 0,
								 opcodeList3Data.Get () ? opcodeList3Data->Buffer      () : NULL);

	if (opcodeList3Data.Get ())
		{

		rawIFD.Add (&tagOpcodeList3);

		}

	// Add other subfiles.

	uint32 subFileCount = 1;

	// Add previews.

	uint32 previewCount = previewList ? previewList->Count () : 0;

	AutoPtr<dng_tiff_directory> previewIFD [kMaxDNGPreviews];

	AutoPtr<dng_basic_tag_set> previewBasic [kMaxDNGPreviews];

	for (j = 0; j < previewCount; j++)
		{

		previewIFD [j] . Reset (new dng_tiff_directory);

		previewBasic [j] . Reset (previewList->Preview (j).AddTagSet (*previewIFD [j]));

		subFileCount++;

		}

	// And a link to the raw and JPEG image IFDs.

	uint32 subFileData [kMaxDNGPreviews + 1];

	tag_uint32_ptr tagSubFile (tcSubIFDs,
							   subFileData,
							   subFileCount);

	mainIFD.Add (&tagSubFile);

	// Skip past the header and IFDs for now.

	uint32 currentOffset = 8;

	currentOffset += mainIFD.Size ();

	subFileData [0] = currentOffset;

	currentOffset += rawIFD.Size ();

	for (j = 0; j < previewCount; j++)
		{

		subFileData [j + 1] = currentOffset;

		currentOffset += previewIFD [j]->Size ();

		}

	exifSet.Locate (currentOffset);

	currentOffset += exifSet.Size ();

	stream.SetWritePosition (currentOffset);

	// Write the extra profiles.

	if (extraProfileCount)
		{

		for (j = 0; j < extraProfileCount; j++)
			{

			extraProfileOffsets.Buffer_uint32 () [j] = (uint32) stream.Position ();

			uint32 index = extraProfileIndex [j];

			const dng_camera_profile &profile (negative.ProfileByIndex (index));

			tiff_dng_extended_color_profile extraWriter (profile);

			extraWriter.Put (stream, false);

			}

		}

	// Write the thumbnail data.

	thumbnail.WriteData (host,
						 *this,
						 *thmBasic,
						 stream);

	// Write the preview data.

	for (j = 0; j < previewCount; j++)
		{

		previewList->Preview (j).WriteData (host,
							                *this,
							                *previewBasic [j],
							                stream);

		}

	// Write the raw data.

	WriteImage (host,
				info,
				rawBasic,
				stream,
				rawImage,
				fakeChannels);

	// Trim the file to this length.

	stream.SetLength (stream.Position ());

	// DNG has a 4G size limit.

	if (stream.Length () > 0x0FFFFFFFFL)
		{
		ThrowImageTooBigDNG ();
		}

	// Write TIFF Header.

	stream.SetWritePosition (0);

	stream.Put_uint16 (stream.BigEndian () ? byteOrderMM : byteOrderII);

	stream.Put_uint16 (42);

	stream.Put_uint32 (8);

	// Write the IFDs.

	mainIFD.Put (stream);

	rawIFD.Put (stream);

	for (j = 0; j < previewCount; j++)
		{

		previewIFD [j]->Put (stream);

		}

	exifSet.Put (stream);

	stream.Flush ();

	}

/*****************************************************************************/
