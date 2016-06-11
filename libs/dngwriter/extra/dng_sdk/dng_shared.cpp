/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_shared.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_shared.h"

#include "dng_camera_profile.h"
#include "dng_exceptions.h"
#include "dng_globals.h"
#include "dng_parse_utils.h"
#include "dng_tag_codes.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_camera_profile_info::dng_camera_profile_info ()

	:	fBigEndian (false)

	,	fColorPlanes (0)

	,	fCalibrationIlluminant1 (lsUnknown)
	,	fCalibrationIlluminant2 (lsUnknown)

	,	fColorMatrix1 ()
	,	fColorMatrix2 ()

	,	fForwardMatrix1 ()
	,	fForwardMatrix2 ()

	,	fReductionMatrix1 ()
	,	fReductionMatrix2 ()

	,	fProfileCalibrationSignature ()

	,	fProfileName ()

	,	fProfileCopyright ()

	,	fEmbedPolicy (pepAllowCopying)

	,	fProfileHues (0)
	,	fProfileSats (0)
	,	fProfileVals (0)

	,	fHueSatDeltas1Offset (0)
	,	fHueSatDeltas1Count  (0)

	,	fHueSatDeltas2Offset (0)
	,	fHueSatDeltas2Count  (0)

	,	fLookTableHues (0)
	,	fLookTableSats (0)
	,	fLookTableVals (0)

	,	fLookTableOffset (0)
	,	fLookTableCount  (0)

	,	fToneCurveOffset     (0)
	,	fToneCurveCount      (0)

	,	fUniqueCameraModel ()

	{

	}

/*****************************************************************************/

dng_camera_profile_info::~dng_camera_profile_info ()
	{

	}

/*****************************************************************************/

bool dng_camera_profile_info::ParseTag (dng_stream &stream,
										uint32 parentCode,
										uint32 tagCode,
										uint32 tagType,
										uint32 tagCount,
										uint64 tagOffset)
	{

	switch (tagCode)
		{

		case tcCalibrationIlluminant1:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fCalibrationIlluminant1 = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CalibrationIlluminant1: %s\n",
						LookupLightSource (fCalibrationIlluminant1));

				}

			#endif

			break;

			}

		case tcCalibrationIlluminant2:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fCalibrationIlluminant2 = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CalibrationIlluminant2: %s\n",
						LookupLightSource (fCalibrationIlluminant2));

				}

			#endif

			break;

			}

		case tcColorMatrix1:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (fColorPlanes == 0)
				{

				fColorPlanes = Pin_uint32 (0, tagCount / 3, kMaxColorPlanes);

				}

			if (!CheckColorImage (parentCode, tagCode, fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 fColorPlanes,
								 3,
								 fColorMatrix1))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ColorMatrix1:\n");

				DumpMatrix (fColorMatrix1);

				}

			#endif

			break;

			}

		case tcColorMatrix2:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 fColorPlanes,
								 3,
								 fColorMatrix2))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ColorMatrix2:\n");

				DumpMatrix (fColorMatrix2);

				}

			#endif

			break;

			}

		case tcForwardMatrix1:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 3,
								 fColorPlanes,
								 fForwardMatrix1))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ForwardMatrix1:\n");

				DumpMatrix (fForwardMatrix1);

				}

			#endif

			break;

			}

		case tcForwardMatrix2:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 3,
								 fColorPlanes,
								 fForwardMatrix2))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ForwardMatrix2:\n");

				DumpMatrix (fForwardMatrix2);

				}

			#endif

			break;

			}

		case tcReductionMatrix1:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 3,
								 fColorPlanes,
								 fReductionMatrix1))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ReductionMatrix1:\n");

				DumpMatrix (fReductionMatrix1);

				}

			#endif

			break;

			}

		case tcReductionMatrix2:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 3,
								 fColorPlanes,
								 fReductionMatrix2))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ReductionMatrix2:\n");

				DumpMatrix (fReductionMatrix2);

				}

			#endif

			break;

			}

		case tcProfileCalibrationSignature:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fProfileCalibrationSignature,
							false,
							false);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileCalibrationSignature: ");

				DumpString (fProfileCalibrationSignature);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcProfileName:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fProfileName,
							false,
							false);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileName: ");

				DumpString (fProfileName);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcProfileCopyright:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fProfileCopyright,
							false,
							false);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileCopyright: ");

				DumpString (fProfileCopyright);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcProfileEmbedPolicy:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fEmbedPolicy = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				const char *policy;

				switch (fEmbedPolicy)
					{

					case pepAllowCopying:
						policy = "Allow copying";
						break;

					case pepEmbedIfUsed:
						policy = "Embed if used";
						break;

					case pepEmbedNever:
						policy = "Embed never";
						break;

					case pepNoRestrictions:
						policy = "No restrictions";
						break;

					default:
						policy = "INVALID VALUE";

					}

				printf ("ProfileEmbedPolicy: %s\n", policy);

				}

			#endif

			break;

			}

		case tcProfileHueSatMapDims:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 2, 3);

			fProfileHues = stream.TagValue_uint32 (tagType);
			fProfileSats = stream.TagValue_uint32 (tagType);

			if (tagCount > 2)
				fProfileVals = stream.TagValue_uint32 (tagType);
			else
				fProfileVals = 1;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileHueSatMapDims: Hues = %u, Sats = %u, Vals = %u\n",
						(unsigned) fProfileHues,
						(unsigned) fProfileSats,
						(unsigned) fProfileVals);

				}

			#endif

			break;

			}

		case tcProfileHueSatMapData1:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttFloat))
				return false;

			bool skipSat0 = (tagCount == fProfileHues *
										(fProfileSats - 1) *
										 fProfileVals * 3);

			if (!skipSat0)
				{

				if (!CheckTagCount (parentCode, tagCode, tagCount, fProfileHues *
																   fProfileSats *
																   fProfileVals * 3))
					return false;

				}

			fBigEndian = stream.BigEndian ();

			fHueSatDeltas1Offset = tagOffset;
			fHueSatDeltas1Count  = tagCount;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileHueSatMapData1:\n");

				DumpHueSatMap (stream,
							   fProfileHues,
							   fProfileSats,
							   fProfileVals,
							   skipSat0);

				}

			#endif

			break;

			}

		case tcProfileHueSatMapData2:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttFloat))
				return false;

			bool skipSat0 = (tagCount == fProfileHues *
										(fProfileSats - 1) *
										 fProfileVals * 3);

			if (!skipSat0)
				{

				if (!CheckTagCount (parentCode, tagCode, tagCount, fProfileHues *
																   fProfileSats *
																   fProfileVals * 3))
					return false;

				}

			fBigEndian = stream.BigEndian ();

			fHueSatDeltas2Offset = tagOffset;
			fHueSatDeltas2Count  = tagCount;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileHueSatMapData2:\n");

				DumpHueSatMap (stream,
							   fProfileHues,
							   fProfileSats,
							   fProfileVals,
							   skipSat0);

				}

			#endif

			break;

			}

		case tcProfileLookTableDims:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 2, 3);

			fLookTableHues = stream.TagValue_uint32 (tagType);
			fLookTableSats = stream.TagValue_uint32 (tagType);

			if (tagCount > 2)
				fLookTableVals = stream.TagValue_uint32 (tagType);
			else
				fLookTableVals = 1;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileLookTableDims: Hues = %u, Sats = %u, Vals = %u\n",
						(unsigned) fLookTableHues,
						(unsigned) fLookTableSats,
						(unsigned) fLookTableVals);

				}

			#endif

			break;

			}

		case tcProfileLookTableData:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttFloat))
				return false;

			bool skipSat0 = (tagCount == fLookTableHues *
										(fLookTableSats - 1) *
										 fLookTableVals * 3);

			if (!skipSat0)
				{

				if (!CheckTagCount (parentCode, tagCode, tagCount, fLookTableHues *
																   fLookTableSats *
																   fLookTableVals * 3))
					return false;

				}

			fBigEndian = stream.BigEndian ();

			fLookTableOffset = tagOffset;
			fLookTableCount  = tagCount;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ProfileLookTableData:\n");

				DumpHueSatMap (stream,
							   fLookTableHues,
							   fLookTableSats,
							   fLookTableVals,
							   skipSat0);

				}

			#endif

			break;

			}

		case tcProfileToneCurve:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttFloat))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 4, tagCount))
				return false;

			if ((tagCount & 1) != 0)
				{

				#if qDNGValidate

					{

					char message [256];

					sprintf (message,
							 "%s %s has odd count (%u)",
							 LookupParentCode (parentCode),
							 LookupTagCode (parentCode, tagCode),
							 (unsigned) tagCount);

					ReportWarning (message);

					}

				#endif

				return false;

				}

			fBigEndian = stream.BigEndian ();

			fToneCurveOffset = tagOffset;
			fToneCurveCount  = tagCount;

			#if qDNGValidate

			if (gVerbose)
				{

				DumpTagValues (stream,
							   "Coord",
							   parentCode,
							   tagCode,
							   tagType,
							   tagCount);


				}

			#endif

			break;

			}

		case tcUniqueCameraModel:
			{

			// Note: This code is only used when parsing stand-alone
			// profiles.  The embedded profiles are assumed to be restricted
			// to the model they are embedded in.

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fUniqueCameraModel,
							false);

			bool didTrim = fUniqueCameraModel.TrimTrailingBlanks ();

			#if qDNGValidate

			if (didTrim)
				{

				ReportWarning ("UniqueCameraModel string has trailing blanks");

				}

			if (gVerbose)
				{

				printf ("UniqueCameraModel: ");

				DumpString (fUniqueCameraModel);

				printf ("\n");

				}

			#else

			(void) didTrim;		// Unused

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

bool dng_camera_profile_info::ParseExtended (dng_stream &stream)
	{

	try
		{

		// Offsets are relative to the start of this structure, not the entire file.

		uint64 startPosition = stream.Position ();

		// Read header. Like a TIFF header, but with different magic number
		// Plus all offsets are relative to the start of the IFD, not to the
		// stream or file.

		uint16 byteOrder = stream.Get_uint16 ();

		if (byteOrder == byteOrderMM)
			fBigEndian = true;

		else if (byteOrder == byteOrderII)
			fBigEndian = false;

		else
			return false;

		TempBigEndian setEndianness (stream, fBigEndian);

		uint16 magicNumber = stream.Get_uint16 ();

		if (magicNumber != magicExtendedProfile)
			{
			return false;
			}

		uint32 offset = stream.Get_uint32 ();

		stream.Skip (offset - 8);

		// Start on IFD entries.

		uint32 ifdEntries = stream.Get_uint16 ();

		if (ifdEntries < 1)
			{
			return false;
			}

		for (uint32 tag_index = 0; tag_index < ifdEntries; tag_index++)
			{

			stream.SetReadPosition (startPosition + 8 + 2 + tag_index * 12);

			uint16 tagCode  = stream.Get_uint16 ();
			uint32 tagType  = stream.Get_uint16 ();
			uint32 tagCount = stream.Get_uint32 ();

			uint64 tagOffset = stream.Position ();

			if (TagTypeSize (tagType) * tagCount > 4)
				{

				tagOffset = startPosition + stream.Get_uint32 ();

				stream.SetReadPosition (tagOffset);

				}

			if (!ParseTag (stream,
						   0,
						   tagCode,
						   tagType,
						   tagCount,
						   tagOffset))
				{

				#if qDNGValidate

				if (gVerbose)
					{

					stream.SetReadPosition (tagOffset);

					printf ("*");

					DumpTagValues (stream,
								   LookupTagType (tagType),
								   0,
								   tagCode,
								   tagType,
								   tagCount);

					}

				#endif

				}

			}

		return true;

		}

	catch (...)
		{

		// Eat parsing errors.

		}

	return false;

	}

/*****************************************************************************/

dng_shared::dng_shared ()

	:	fExifIFD 			 (0)
	,	fGPSInfo 			 (0)
	,	fInteroperabilityIFD (0)
	,	fKodakDCRPrivateIFD  (0)
	,	fKodakKDCPrivateIFD  (0)

	,	fXMPCount  (0)
	,	fXMPOffset (0)

	,	fIPTC_NAA_Count  (0)
	,	fIPTC_NAA_Offset (0)

	,	fMakerNoteCount  (0)
	,	fMakerNoteOffset (0)
	,	fMakerNoteSafety (0)

	,	fDNGVersion         (0)
	,	fDNGBackwardVersion (0)

	,	fUniqueCameraModel    ()
	,	fLocalizedCameraModel ()

	,	fCameraProfile ()

	,	fExtraCameraProfiles ()

	,	fCameraCalibration1 ()
	,	fCameraCalibration2 ()

	,	fCameraCalibrationSignature  ()

	,	fAnalogBalance ()

	,	fAsShotNeutral ()

	,	fAsShotWhiteXY ()

	,	fBaselineExposure      (0, 1)
	,	fBaselineNoise         (1, 1)
	,	fNoiseReductionApplied (0, 0)
	,	fBaselineSharpness     (1, 1)
	,	fLinearResponseLimit   (1, 1)
	,	fShadowScale           (1, 1)

	,	fDNGPrivateDataCount  (0)
	,	fDNGPrivateDataOffset (0)

	,	fRawImageDigest ()

	,	fRawDataUniqueID ()

	,	fOriginalRawFileName ()

	,	fOriginalRawFileDataCount  (0)
	,	fOriginalRawFileDataOffset (0)

	,	fOriginalRawFileDigest ()

	,	fAsShotICCProfileCount  (0)
	,	fAsShotICCProfileOffset (0)

	,	fAsShotPreProfileMatrix ()

	,	fCurrentICCProfileCount  (0)
	,	fCurrentICCProfileOffset (0)

	,	fCurrentPreProfileMatrix ()

	,	fColorimetricReference (crSceneReferred)

	,	fAsShotProfileName ()

	,	fNoiseProfile ()

	{

	}

/*****************************************************************************/

dng_shared::~dng_shared ()
	{

	}

/*****************************************************************************/

bool dng_shared::ParseTag (dng_stream &stream,
						   dng_exif &exif,
						   uint32 parentCode,
						   bool /* isMainIFD */,
						   uint32 tagCode,
						   uint32 tagType,
						   uint32 tagCount,
						   uint64 tagOffset,
						   int64 /* offsetDelta */)
	{

	if (parentCode == 0)
		{

		if (Parse_ifd0 (stream,
						exif,
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
							 exif,
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

bool dng_shared::Parse_ifd0 (dng_stream &stream,
							 dng_exif & /* exif */,
							 uint32 parentCode,
							 uint32 tagCode,
							 uint32 tagType,
							 uint32 tagCount,
							 uint64 tagOffset)
	{

	switch (tagCode)
		{

		case tcXMP:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			fXMPCount  = tagCount;
			fXMPOffset = fXMPCount ? tagOffset : 0;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("XMP: Count = %u, Offset = %u\n",
						(unsigned) fXMPCount,
						(unsigned) fXMPOffset);

				if (fXMPCount)
					{

					DumpXMP (stream, fXMPCount);

					}

				}

			#endif

			break;

			}

		case tcIPTC_NAA:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong, ttAscii);

			fIPTC_NAA_Count  = tagCount * TagTypeSize (tagType);
			fIPTC_NAA_Offset = fIPTC_NAA_Count ? tagOffset : 0;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("IPTC/NAA: Count = %u, Offset = %u\n",
						(unsigned) fIPTC_NAA_Count,
						(unsigned) fIPTC_NAA_Offset);

				if (fIPTC_NAA_Count)
					{

					DumpHexAscii (stream, fIPTC_NAA_Count);

					}

				// Compute and output the digest.

				dng_memory_data buffer (fIPTC_NAA_Count);

				stream.SetReadPosition (fIPTC_NAA_Offset);

				stream.Get (buffer.Buffer (), fIPTC_NAA_Count);

				const uint8 *data = buffer.Buffer_uint8 ();

				uint32 count = fIPTC_NAA_Count;

				// Method 1: Counting all bytes (this is correct).

					{

					dng_md5_printer printer;

					printer.Process (data, count);

					printf ("IPTCDigest: ");

					DumpFingerprint (printer.Result ());

					printf ("\n");

					}

				// Method 2: Ignoring zero padding.

					{

					uint32 removed = 0;

					while ((removed < 3) && (count > 0) && (data [count - 1] == 0))
						{
						removed++;
						count--;
						}

					if (removed != 0)
						{

						dng_md5_printer printer;

						printer.Process (data, count);

						printf ("IPTCDigest (ignoring zero padding): ");

						DumpFingerprint (printer.Result ());

						printf ("\n");

						}

					}

				}

			#endif

			break;

			}

		case tcExifIFD:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong, ttIFD);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fExifIFD = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("ExifIFD: %u\n", (unsigned) fExifIFD);
				}

			#endif

			break;

			}

		case tcGPSInfo:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong, ttIFD);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fGPSInfo = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("GPSInfo: %u\n", (unsigned) fGPSInfo);
				}

			#endif

			break;

			}

		case tcKodakDCRPrivateIFD:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong, ttIFD);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fKodakDCRPrivateIFD = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("KodakDCRPrivateIFD: %u\n", (unsigned) fKodakDCRPrivateIFD);
				}

			#endif

			break;

			}

		case tcKodakKDCPrivateIFD:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong, ttIFD);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fKodakKDCPrivateIFD = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("KodakKDCPrivateIFD: %u\n", (unsigned) fKodakKDCPrivateIFD);
				}

			#endif

			break;

			}

		case tcDNGVersion:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fDNGVersion = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("DNGVersion: %u.%u.%u.%u\n",
						(unsigned) b0,
						(unsigned) b1,
						(unsigned) b2,
						(unsigned) b3);
				}

			#endif

			break;

			}

		case tcDNGBackwardVersion:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			CheckTagCount (parentCode, tagCode, tagCount, 4);

			uint32 b0 = stream.Get_uint8 ();
			uint32 b1 = stream.Get_uint8 ();
			uint32 b2 = stream.Get_uint8 ();
			uint32 b3 = stream.Get_uint8 ();

			fDNGBackwardVersion = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("DNGBackwardVersion: %u.%u.%u.%u\n",
						(unsigned) b0,
						(unsigned) b1,
						(unsigned) b2,
						(unsigned) b3);
				}

			#endif

			break;

			}

		case tcUniqueCameraModel:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fUniqueCameraModel,
							false);

			bool didTrim = fUniqueCameraModel.TrimTrailingBlanks ();

			#if qDNGValidate

			if (didTrim)
				{

				ReportWarning ("UniqueCameraModel string has trailing blanks");

				}

			if (gVerbose)
				{

				printf ("UniqueCameraModel: ");

				DumpString (fUniqueCameraModel);

				printf ("\n");

				}

			#else

			(void) didTrim;		// Unused

			#endif

			break;

			}

		case tcLocalizedCameraModel:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fLocalizedCameraModel,
							false,
							false);

			bool didTrim = fLocalizedCameraModel.TrimTrailingBlanks ();

			#if qDNGValidate

			if (didTrim)
				{

				ReportWarning ("LocalizedCameraModel string has trailing blanks");

				}

			if (gVerbose)
				{

				printf ("LocalizedCameraModel: ");

				DumpString (fLocalizedCameraModel);

				printf ("\n");

				}

			#else

			(void) didTrim;		// Unused

			#endif

			break;

			}

		case tcCameraCalibration1:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 fCameraProfile.fColorPlanes,
								 fCameraProfile.fColorPlanes,
								 fCameraCalibration1))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CameraCalibration1:\n");

				DumpMatrix (fCameraCalibration1);

				}

			#endif

			break;

			}

		case tcCameraCalibration2:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 fCameraProfile.fColorPlanes,
								 fCameraProfile.fColorPlanes,
								 fCameraCalibration2))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CameraCalibration2:\n");

				DumpMatrix (fCameraCalibration2);

				}

			#endif

			break;

			}

		case tcCameraCalibrationSignature:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fCameraCalibrationSignature,
							false,
							false);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CameraCalibrationSignature: ");

				DumpString (fCameraCalibrationSignature);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcAnalogBalance:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			if (!ParseVectorTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 fCameraProfile.fColorPlanes,
								 fAnalogBalance))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("AnalogBalance:");

				DumpVector (fAnalogBalance);

				}

			#endif

			break;

			}

		case tcAsShotNeutral:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			if (!ParseVectorTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 fCameraProfile.fColorPlanes,
								 fAsShotNeutral))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("AsShotNeutral:");

				DumpVector (fAsShotNeutral);

				}

			#endif

			break;

			}

		case tcAsShotWhiteXY:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 2))
				return false;

			fAsShotWhiteXY.x = stream.TagValue_real64 (tagType);
			fAsShotWhiteXY.y = stream.TagValue_real64 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("AsShotWhiteXY: %0.4f %0.4f\n",
						fAsShotWhiteXY.x,
						fAsShotWhiteXY.y);

				}

			#endif

			break;

			}

		case tcBaselineExposure:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fBaselineExposure = stream.TagValue_srational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("BaselineExposure: %+0.2f\n",
					    fBaselineExposure.As_real64 ());

				}

			#endif

			break;

			}

		case tcBaselineNoise:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fBaselineNoise = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("BaselineNoise: %0.2f\n",
						fBaselineNoise.As_real64 ());

				}

			#endif

			break;

			}

		case tcNoiseReductionApplied:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttRational))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 1))
				return false;

			fNoiseReductionApplied = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("NoiseReductionApplied: %u/%u\n",
						(unsigned) fNoiseReductionApplied.n,
						(unsigned) fNoiseReductionApplied.d);

				}

			#endif

			break;

			}

		case tcNoiseProfile:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttDouble))
				return false;

			// Must be an even, positive number of doubles in a noise profile.

			if (!tagCount || (tagCount & 1))
				return false;

			// Determine number of planes (i.e., half the number of doubles).

			const uint32 numPlanes = Pin_uint32 (0,
												 tagCount >> 1,
												 kMaxColorPlanes);

			// Parse the noise function parameters.

			std::vector<dng_noise_function> noiseFunctions;

			for (uint32 i = 0; i < numPlanes; i++)
				{

				const real64 scale	= stream.TagValue_real64 (tagType);
				const real64 offset = stream.TagValue_real64 (tagType);

				noiseFunctions.push_back (dng_noise_function (scale, offset));

				}

			// Store the noise profile.

			fNoiseProfile = dng_noise_profile (noiseFunctions);

			// Debug.

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("NoiseProfile:\n");

				printf ("  Planes: %u\n", numPlanes);

				for (uint32 plane = 0; plane < numPlanes; plane++)
					{

					printf ("  Noise function for plane %u: scale = %.8lf, offset = %.8lf\n",
							plane,
							noiseFunctions [plane].Scale  (),
							noiseFunctions [plane].Offset ());

					}

				}

			#endif

			break;

			}

		case tcBaselineSharpness:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fBaselineSharpness = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("BaselineSharpness: %0.2f\n",
					    fBaselineSharpness.As_real64 ());

				}

			#endif

			break;

			}

		case tcLinearResponseLimit:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fLinearResponseLimit = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("LinearResponseLimit: %0.2f\n",
						fLinearResponseLimit.As_real64 ());

				}

			#endif

			break;

			}

		case tcShadowScale:
			{

			CheckTagType (parentCode, tagCode, tagType, ttRational);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fShadowScale = stream.TagValue_urational (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ShadowScale: %0.4f\n",
						fShadowScale.As_real64 ());

				}

			#endif

			break;

			}

		case tcDNGPrivateData:
			{

			CheckTagType (parentCode, tagCode, tagType, ttByte);

			fDNGPrivateDataCount  = tagCount;
			fDNGPrivateDataOffset = tagOffset;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("DNGPrivateData: Count = %u, Offset = %u\n",
						(unsigned) fDNGPrivateDataCount,
						(unsigned) fDNGPrivateDataOffset);

				DumpHexAscii (stream, tagCount);

				}

			#endif

			break;

			}

		case tcMakerNoteSafety:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fMakerNoteSafety = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("MakerNoteSafety: %s\n",
						LookupMakerNoteSafety (fMakerNoteSafety));

				}

			#endif

			break;

			}

		case tcRawImageDigest:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttByte))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 16))
				return false;

			stream.Get (fRawImageDigest.data, 16);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("RawImageDigest: ");

				DumpFingerprint (fRawImageDigest);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcRawDataUniqueID:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttByte))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 16))
				return false;

			stream.Get (fRawDataUniqueID.data, 16);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("RawDataUniqueID: ");

				DumpFingerprint (fRawDataUniqueID);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcOriginalRawFileName:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fOriginalRawFileName,
							false,
							false);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("OriginalRawFileName: ");

				DumpString (fOriginalRawFileName);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcOriginalRawFileData:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			fOriginalRawFileDataCount  = tagCount;
			fOriginalRawFileDataOffset = tagOffset;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("OriginalRawFileData: Count = %u, Offset = %u\n",
						(unsigned) fOriginalRawFileDataCount,
						(unsigned) fOriginalRawFileDataOffset);

				DumpHexAscii (stream, tagCount);

				}

			#endif

			break;

			}

		case tcOriginalRawFileDigest:
			{

			if (!CheckTagType (parentCode, tagCode, tagType, ttByte))
				return false;

			if (!CheckTagCount (parentCode, tagCode, tagCount, 16))
				return false;

			stream.Get (fOriginalRawFileDigest.data, 16);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("OriginalRawFileDigest: ");

				DumpFingerprint (fOriginalRawFileDigest);

				printf ("\n");

				}

			#endif

			break;

			}

		case tcAsShotICCProfile:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			fAsShotICCProfileCount  = tagCount;
			fAsShotICCProfileOffset = tagOffset;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("AsShotICCProfile: Count = %u, Offset = %u\n",
						(unsigned) fAsShotICCProfileCount,
						(unsigned) fAsShotICCProfileOffset);

				DumpHexAscii (stream, tagCount);

				}

			#endif

			break;

			}

		case tcAsShotPreProfileMatrix:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			uint32 rows = fCameraProfile.fColorPlanes;

			if (tagCount == fCameraProfile.fColorPlanes * 3)
				{
				rows = 3;
				}

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 rows,
								 fCameraProfile.fColorPlanes,
								 fAsShotPreProfileMatrix))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("AsShotPreProfileMatrix:\n");

				DumpMatrix (fAsShotPreProfileMatrix);

				}

			#endif

			break;

			}

		case tcCurrentICCProfile:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			fCurrentICCProfileCount  = tagCount;
			fCurrentICCProfileOffset = tagOffset;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CurrentICCProfile: Count = %u, Offset = %u\n",
						(unsigned) fCurrentICCProfileCount,
						(unsigned) fCurrentICCProfileOffset);

				DumpHexAscii (stream, tagCount);

				}

			#endif

			break;

			}

		case tcCurrentPreProfileMatrix:
			{

			CheckTagType (parentCode, tagCode, tagType, ttSRational);

			if (!CheckColorImage (parentCode, tagCode, fCameraProfile.fColorPlanes))
				return false;

			uint32 rows = fCameraProfile.fColorPlanes;

			if (tagCount == fCameraProfile.fColorPlanes * 3)
				{
				rows = 3;
				}

			if (!ParseMatrixTag (stream,
								 parentCode,
								 tagCode,
								 tagType,
								 tagCount,
								 rows,
								 fCameraProfile.fColorPlanes,
								 fCurrentPreProfileMatrix))
				return false;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("CurrentPreProfileMatrix:\n");

				DumpMatrix (fCurrentPreProfileMatrix);

				}

			#endif

			break;

			}

		case tcColorimetricReference:
			{

			CheckTagType (parentCode, tagCode, tagType, ttShort);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fColorimetricReference = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ColorimetricReference: %s\n",
						LookupColorimetricReference (fColorimetricReference));

				}

			#endif

			break;

			}

		case tcExtraCameraProfiles:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong);

			CheckTagCount (parentCode, tagCode, tagCount, 1, tagCount);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("ExtraCameraProfiles: %u\n", (unsigned) tagCount);

				}

			#endif

			fExtraCameraProfiles.reserve (tagCount);

			for (uint32 index = 0; index < tagCount; index++)
				{

				#if qDNGValidate

				if (gVerbose)
					{

					printf ("\nExtraCameraProfile [%u]:\n\n", (unsigned) index);

					}

				#endif

				stream.SetReadPosition (tagOffset + index * 4);

				uint32 profileOffset = stream.TagValue_uint32 (tagType);

				dng_camera_profile_info profileInfo;

				stream.SetReadPosition (profileOffset);

				if (profileInfo.ParseExtended (stream))
					{

					fExtraCameraProfiles.push_back (profileInfo);

					}

				else
					{

					#if qDNGValidate

					ReportWarning ("Unable to parse extra camera profile");

					#endif

					}

				}

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("\nDone with ExtraCameraProfiles\n\n");

				}

			#endif

			break;

			}

		case tcAsShotProfileName:
			{

			CheckTagType (parentCode, tagCode, tagType, ttAscii, ttByte);

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							fAsShotProfileName,
							false,
							false);

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("AsShotProfileName: ");

				DumpString (fAsShotProfileName);

				printf ("\n");

				}

			#endif

			break;

			}

		default:
			{

			// The main camera profile tags also appear in IFD 0

			return fCameraProfile.ParseTag (stream,
											parentCode,
											tagCode,
											tagType,
											tagCount,
											tagOffset);

			}

		}

	return true;

	}

/*****************************************************************************/

// Parses tags that should only appear in IFD 0 or EXIF IFD.

bool dng_shared::Parse_ifd0_exif (dng_stream &stream,
								  dng_exif & /* exif */,
						  	   	  uint32 parentCode,
						  	      uint32 tagCode,
						  	      uint32 tagType,
						  	      uint32 tagCount,
						  	      uint64 tagOffset)
	{

	switch (tagCode)
		{

		case tcMakerNote:
			{

			CheckTagType (parentCode, tagCode, tagType, ttUndefined);

			fMakerNoteCount  = tagCount;
			fMakerNoteOffset = tagOffset;

			#if qDNGValidate

			if (gVerbose)
				{

				printf ("MakerNote: Count = %u, Offset = %u\n",
						(unsigned) fMakerNoteCount,
						(unsigned) fMakerNoteOffset);

				DumpHexAscii (stream, tagCount);

				}

			#endif

			break;

			}

		case tcInteroperabilityIFD:
			{

			CheckTagType (parentCode, tagCode, tagType, ttLong, ttIFD);

			CheckTagCount (parentCode, tagCode, tagCount, 1);

			fInteroperabilityIFD = stream.TagValue_uint32 (tagType);

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("InteroperabilityIFD: %u\n", (unsigned) fInteroperabilityIFD);
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

void dng_shared::PostParse (dng_host & /* host */,
							dng_exif & /* exif */)
	{

	// Fill in default values for DNG images.

	if (fDNGVersion != 0)
		{

		// Support for DNG versions before 1.0.0.0.

		if (fDNGVersion < dngVersion_1_0_0_0)
			{

			#if qDNGValidate

			ReportWarning ("DNGVersion less than 1.0.0.0");

			#endif

			// The CalibrationIlluminant tags were added just before
			// DNG version 1.0.0.0, and were hardcoded before that.

			fCameraProfile.fCalibrationIlluminant1 = lsStandardLightA;
			fCameraProfile.fCalibrationIlluminant2 = lsD65;

			fDNGVersion = dngVersion_1_0_0_0;

			}

		// Default value for DNGBackwardVersion tag.

		if (fDNGBackwardVersion == 0)
			{

			fDNGBackwardVersion = fDNGVersion & 0xFFFF0000;

			}

		// Check DNGBackwardVersion value.

		if (fDNGBackwardVersion < dngVersion_1_0_0_0)
			{

			#if qDNGValidate

			ReportWarning ("DNGBackwardVersion less than 1.0.0.0");

			#endif

			fDNGBackwardVersion = dngVersion_1_0_0_0;

			}

		if (fDNGBackwardVersion > fDNGVersion)
			{

			#if qDNGValidate

			ReportWarning ("DNGBackwardVersion > DNGVersion");

			#endif

			fDNGBackwardVersion = fDNGVersion;

			}

		// Check UniqueCameraModel.

		if (fUniqueCameraModel.IsEmpty ())
			{

			#if qDNGValidate

			ReportWarning ("Missing or invalid UniqueCameraModel");

			#endif

			fUniqueCameraModel.Set ("Digital Negative");

			}

		// If we don't know the color depth yet, it must be a monochrome DNG.

		if (fCameraProfile.fColorPlanes == 0)
			{

			fCameraProfile.fColorPlanes = 1;

			}

		// Check color info.

		if (fCameraProfile.fColorPlanes > 1)
			{

			// Check illuminant pair.

			if (fCameraProfile.fColorMatrix2.NotEmpty ())
				{

				if (fCameraProfile.fCalibrationIlluminant1 == lsUnknown ||
					(fCameraProfile.fCalibrationIlluminant2 == lsUnknown ||
					(fCameraProfile.fCalibrationIlluminant1 == fCameraProfile.fCalibrationIlluminant2)))
					{

					#if qDNGValidate

					ReportWarning ("Invalid CalibrationIlluminant pair");

					#endif

					fCameraProfile.fColorMatrix2 = dng_matrix ();

					}

				}

			// If the colorimetric reference is the ICC profile PCS, then the
			// data must already be white balanced.  The "AsShotWhiteXY" is required
			// to be the ICC Profile PCS white point.

			if (fColorimetricReference == crICCProfilePCS)
				{

				if (fAsShotNeutral.NotEmpty ())
					{

					#if qDNGValidate

					ReportWarning ("AsShotNeutral not allowed for this "
								   "ColorimetricReference value");

					#endif

					fAsShotNeutral.Clear ();

					}

				dng_xy_coord pcs = PCStoXY ();

				#if qDNGValidate

				if (fAsShotWhiteXY.IsValid ())
					{

					if (Abs_real64 (fAsShotWhiteXY.x - pcs.x) > 0.01 ||
						Abs_real64 (fAsShotWhiteXY.y - pcs.y) > 0.01)
						{

						ReportWarning ("AsShotWhiteXY does not match the ICC Profile PCS");

						}

					}

				#endif

				fAsShotWhiteXY = pcs;

				}

			else
				{

				// Warn if both AsShotNeutral and AsShotWhiteXY are specified.

				if (fAsShotNeutral.NotEmpty () && fAsShotWhiteXY.IsValid ())
					{

					#if qDNGValidate

					ReportWarning ("Both AsShotNeutral and AsShotWhiteXY included");

					#endif

					fAsShotWhiteXY = dng_xy_coord ();

					}

				// Warn if neither AsShotNeutral nor AsShotWhiteXY are specified.

				#if qDNGValidate

				if (fAsShotNeutral.IsEmpty () && !fAsShotWhiteXY.IsValid ())
					{

					ReportWarning ("Neither AsShotNeutral nor AsShotWhiteXY included",
								   "legal but not recommended");

					}

				#endif

				}

			// Default values of calibration signatures are required for legacy
			// compatiblity.

			if (fCameraProfile.fCalibrationIlluminant1 == lsStandardLightA &&
				fCameraProfile.fCalibrationIlluminant2 == lsD65            &&
				fCameraCalibration1.Rows () == fCameraProfile.fColorPlanes &&
				fCameraCalibration1.Cols () == fCameraProfile.fColorPlanes &&
				fCameraCalibration2.Rows () == fCameraProfile.fColorPlanes &&
				fCameraCalibration2.Cols () == fCameraProfile.fColorPlanes &&
				fCameraCalibrationSignature.IsEmpty ()                     &&
				fCameraProfile.fProfileCalibrationSignature.IsEmpty ()     )
				{

				fCameraCalibrationSignature.Set (kAdobeCalibrationSignature);

				fCameraProfile.fProfileCalibrationSignature.Set (kAdobeCalibrationSignature);

				}

			}

		// Check BaselineNoise.

		if (fBaselineNoise.As_real64 () <= 0.0)
			{

			#if qDNGValidate

			ReportWarning ("Invalid BaselineNoise");

			#endif

			fBaselineNoise = dng_urational (1, 1);

			}

		// Check BaselineSharpness.

		if (fBaselineSharpness.As_real64 () <= 0.0)
			{

			#if qDNGValidate

			ReportWarning ("Invalid BaselineSharpness");

			#endif

			fBaselineSharpness = dng_urational (1, 1);

			}

		// Check NoiseProfile.

		if (!fNoiseProfile.IsValid () && fNoiseProfile.NumFunctions () != 0)
			{

			#if qDNGValidate

			ReportWarning ("Invalid NoiseProfile");

			#endif

			fNoiseProfile = dng_noise_profile ();

			}

		// Check LinearResponseLimit.

		if (fLinearResponseLimit.As_real64 () < 0.5 ||
			fLinearResponseLimit.As_real64 () > 1.0)
			{

			#if qDNGValidate

			ReportWarning ("Invalid LinearResponseLimit");

			#endif

			fLinearResponseLimit = dng_urational (1, 1);

			}

		// Check ShadowScale.

		if (fShadowScale.As_real64 () <= 0.0)
			{

			#if qDNGValidate

			ReportWarning ("Invalid ShadowScale");

			#endif

			fShadowScale = dng_urational (1, 1);

			}

		}

	}

/*****************************************************************************/

bool dng_shared::IsValidDNG ()
	{

	// Check DNGVersion value.

	if (fDNGVersion < dngVersion_1_0_0_0)
		{

		#if qDNGValidate

		ReportError ("Missing or invalid DNGVersion");

		#endif

		return false;

		}

	// Check DNGBackwardVersion value.

	if (fDNGBackwardVersion > dngVersion_Current)
		{

		#if qDNGValidate

		ReportError ("DNGBackwardVersion (or DNGVersion) is too high");

		#endif

		return false;

		}

	// Check color transform info.

	if (fCameraProfile.fColorPlanes > 1)
		{

		// CameraCalibration1 is optional, but it must be valid if present.

		if (fCameraCalibration1.Cols () != 0 ||
			fCameraCalibration1.Rows () != 0)
			{

			if (fCameraCalibration1.Cols () != fCameraProfile.fColorPlanes ||
				fCameraCalibration1.Rows () != fCameraProfile.fColorPlanes)
				{

				#if qDNGValidate

				ReportError ("CameraCalibration1 is wrong size");

				#endif

				return false;

				}

			// Make sure it is invertable.

			try
				{

				(void) Invert (fCameraCalibration1);

				}

			catch (...)
				{

				#if qDNGValidate

				ReportError ("CameraCalibration1 is not invertable");

				#endif

				return false;

				}

			}

		// CameraCalibration2 is optional, but it must be valid if present.

		if (fCameraCalibration2.Cols () != 0 ||
			fCameraCalibration2.Rows () != 0)
			{

			if (fCameraCalibration2.Cols () != fCameraProfile.fColorPlanes ||
				fCameraCalibration2.Rows () != fCameraProfile.fColorPlanes)
				{

				#if qDNGValidate

				ReportError ("CameraCalibration2 is wrong size");

				#endif

				return false;

				}

			// Make sure it is invertable.

			try
				{

				(void) Invert (fCameraCalibration2);

				}

			catch (...)
				{

				#if qDNGValidate

				ReportError ("CameraCalibration2 is not invertable");

				#endif

				return false;

				}

			}

		// Check analog balance

		dng_matrix analogBalance;

		if (fAnalogBalance.NotEmpty ())
			{

			analogBalance = fAnalogBalance.AsDiagonal ();

			try
				{

				(void) Invert (analogBalance);

				}

			catch (...)
				{

				#if qDNGValidate

				ReportError ("AnalogBalance is not invertable");

				#endif

				return false;

				}

			}

		}

	return true;

	}

/*****************************************************************************/
