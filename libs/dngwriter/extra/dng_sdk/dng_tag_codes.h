/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_tag_codes.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_tag_codes__
#define __dng_tag_codes__

/*****************************************************************************/

// TIFF tags 50706 through 50741 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2003-11-04 & 2003-12-02, purpose "Digital Negative".

// TIFF tags 50778 through 50781 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2004-08-17, purpose "Digital Negative".

// TIFF tags 50827 through 50834 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2004-12-06, purpose "Digital Negative".

// TIFF tag number 50879 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2006-03-23, purpose "Digital Negative".

// TIFF compression numbers 34892 through 34895 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2003-11-04, purpose "Digital Negative".

// TIFF tags numbers 50931 through 50942 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2007-04-30, purpose "Digital Negative".

// TIFF tags numbers 50964 through 50975 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2007-12-17, purpose "Digital Negative".

// TIFF tags numbers 50981 through 50982 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2008-04-01, purpose "Digital Negative".

// TIFF tags numbers 51008 through 51009 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2008-10-15, purpose "Digital Negative".

// TIFF tag number 51022 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2008-12-15, purpose "Digital Negative".

// TIFF tag number 51041 registered at:
// http://partners.adobe.com/asn/tech/tiff/tiffregister.jsp
// on 2009-5-7, purpose "Digital Negative".

/*****************************************************************************/

// TIFF, DNG, TIFF/EP, and Exif tag codes all share the main TIFF tag code
// number space.  In cases where TIFF/EP and Exif have different values for
// tags with the same name, "Exif" is appended to the name of the Exif version
// of the tag.

enum
	{
	tcNewSubFileType				= 254,
	tcSubFileType					= 255,
	tcImageWidth					= 256,
	tcImageLength					= 257,
	tcBitsPerSample					= 258,
	tcCompression					= 259,
	tcPhotometricInterpretation		= 262,
	tcThresholding					= 263,
	tcCellWidth						= 264,
	tcCellLength					= 265,
	tcFillOrder						= 266,
	tcImageDescription				= 270,
	tcMake							= 271,
	tcModel							= 272,
	tcStripOffsets					= 273,
	tcOrientation					= 274,
	tcSamplesPerPixel				= 277,
	tcRowsPerStrip					= 278,
	tcStripByteCounts				= 279,
	tcMinSampleValue				= 280,
	tcMaxSampleValue				= 281,
	tcXResolution					= 282,
	tcYResolution					= 283,
	tcPlanarConfiguration			= 284,
	tcFreeOffsets					= 285,
	tcFreeByteCounts				= 286,
	tcGrayResponseUnit				= 290,
	tcGrayResponseCurve				= 291,
	tcResolutionUnit				= 296,
	tcTransferFunction				= 301,
	tcSoftware						= 305,
	tcDateTime						= 306,
	tcArtist						= 315,
	tcHostComputer					= 316,
	tcPredictor						= 317,
	tcWhitePoint					= 318,
	tcPrimaryChromaticities			= 319,
	tcColorMap						= 320,
	tcTileWidth						= 322,
	tcTileLength					= 323,
	tcTileOffsets					= 324,
	tcTileByteCounts				= 325,
	tcSubIFDs						= 330,
	tcExtraSamples					= 338,
	tcSampleFormat					= 339,
	tcJPEGTables					= 347,
	tcJPEGProc						= 512,
	tcJPEGInterchangeFormat			= 513,
	tcJPEGInterchangeFormatLength	= 514,
	tcYCbCrCoefficients				= 529,
	tcYCbCrSubSampling				= 530,
	tcYCbCrPositioning				= 531,
	tcReferenceBlackWhite			= 532,
	tcXMP							= 700,
	tcKodakCameraSerialNumber		= 33405,
	tcCFARepeatPatternDim			= 33421,
	tcCFAPattern					= 33422,
	tcBatteryLevel					= 33423,
	tcKodakDCRPrivateIFD			= 33424,
	tcCopyright						= 33432,
	tcExposureTime					= 33434,
	tcFNumber						= 33437,
	tcIPTC_NAA						= 33723,
	tcLeafPKTS						= 34310,
	tcAdobeData						= 34377,
	tcExifIFD						= 34665,
	tcICCProfile					= 34675,
	tcExposureProgram				= 34850,
	tcSpectralSensitivity			= 34852,
	tcGPSInfo						= 34853,
	tcISOSpeedRatings				= 34855,
	tcOECF							= 34856,
	tcInterlace						= 34857,
	tcTimeZoneOffset				= 34858,
	tcSelfTimerMode					= 34859,
	tcExifVersion					= 36864,
	tcDateTimeOriginal				= 36867,
	tcDateTimeDigitized				= 36868,
	tcComponentsConfiguration		= 37121,
	tcCompressedBitsPerPixel		= 37122,
	tcShutterSpeedValue				= 37377,
	tcApertureValue					= 37378,
	tcBrightnessValue				= 37379,
	tcExposureBiasValue				= 37380,
	tcMaxApertureValue				= 37381,
	tcSubjectDistance				= 37382,
	tcMeteringMode					= 37383,
	tcLightSource					= 37384,
	tcFlash							= 37385,
	tcFocalLength					= 37386,
	tcFlashEnergy					= 37387,
	tcSpatialFrequencyResponse		= 37388,
	tcNoise							= 37389,
	tcFocalPlaneXResolution 		= 37390,
	tcFocalPlaneYResolution			= 37391,
	tcFocalPlaneResolutionUnit		= 37392,
	tcImageNumber					= 37393,
	tcSecurityClassification		= 37394,
	tcImageHistory					= 37395,
	tcSubjectArea					= 37396,
	tcExposureIndex					= 37397,
	tcTIFF_EP_StandardID			= 37398,
	tcSensingMethod					= 37399,
	tcMakerNote						= 37500,
	tcUserComment					= 37510,
	tcSubsecTime					= 37520,
	tcSubsecTimeOriginal			= 37521,
	tcSubsecTimeDigitized			= 37522,
	tcAdobeLayerData				= 37724,
	tcFlashPixVersion				= 40960,
	tcColorSpace					= 40961,
	tcPixelXDimension				= 40962,
	tcPixelYDimension				= 40963,
	tcRelatedSoundFile				= 40964,
	tcInteroperabilityIFD			= 40965,
	tcFlashEnergyExif				= 41483,
	tcSpatialFrequencyResponseExif 	= 41484,
	tcFocalPlaneXResolutionExif		= 41486,
	tcFocalPlaneYResolutionExif		= 41487,
	tcFocalPlaneResolutionUnitExif	= 41488,
	tcSubjectLocation				= 41492,
	tcExposureIndexExif			    = 41493,
	tcSensingMethodExif				= 41495,
	tcFileSource					= 41728,
	tcSceneType						= 41729,
	tcCFAPatternExif				= 41730,
	tcCustomRendered				= 41985,
	tcExposureMode					= 41986,
	tcWhiteBalance					= 41987,
	tcDigitalZoomRatio				= 41988,
	tcFocalLengthIn35mmFilm			= 41989,
	tcSceneCaptureType				= 41990,
	tcGainControl					= 41991,
	tcContrast						= 41992,
	tcSaturation					= 41993,
	tcSharpness						= 41994,
	tcDeviceSettingDescription		= 41995,
	tcSubjectDistanceRange			= 41996,
	tcImageUniqueID					= 42016,
	tcGamma							= 42240,
	tcPrintImageMatchingInfo		= 50341,
	tcDNGVersion					= 50706,
	tcDNGBackwardVersion			= 50707,
	tcUniqueCameraModel				= 50708,
	tcLocalizedCameraModel			= 50709,
	tcCFAPlaneColor					= 50710,
	tcCFALayout						= 50711,
	tcLinearizationTable			= 50712,
	tcBlackLevelRepeatDim			= 50713,
	tcBlackLevel					= 50714,
	tcBlackLevelDeltaH				= 50715,
	tcBlackLevelDeltaV				= 50716,
	tcWhiteLevel					= 50717,
	tcDefaultScale					= 50718,
	tcDefaultCropOrigin				= 50719,
	tcDefaultCropSize				= 50720,
	tcColorMatrix1					= 50721,
	tcColorMatrix2					= 50722,
	tcCameraCalibration1			= 50723,
	tcCameraCalibration2			= 50724,
	tcReductionMatrix1				= 50725,
	tcReductionMatrix2				= 50726,
	tcAnalogBalance					= 50727,
	tcAsShotNeutral					= 50728,
	tcAsShotWhiteXY					= 50729,
	tcBaselineExposure				= 50730,
	tcBaselineNoise					= 50731,
	tcBaselineSharpness				= 50732,
	tcBayerGreenSplit				= 50733,
	tcLinearResponseLimit			= 50734,
	tcCameraSerialNumber			= 50735,
	tcLensInfo						= 50736,
	tcChromaBlurRadius				= 50737,
	tcAntiAliasStrength				= 50738,
	tcShadowScale					= 50739,
	tcDNGPrivateData				= 50740,
	tcMakerNoteSafety				= 50741,
	tcCalibrationIlluminant1		= 50778,
	tcCalibrationIlluminant2		= 50779,
	tcBestQualityScale				= 50780,
	tcRawDataUniqueID				= 50781,
	tcOriginalRawFileName			= 50827,
	tcOriginalRawFileData			= 50828,
	tcActiveArea					= 50829,
	tcMaskedAreas					= 50830,
	tcAsShotICCProfile				= 50831,
	tcAsShotPreProfileMatrix		= 50832,
	tcCurrentICCProfile				= 50833,
	tcCurrentPreProfileMatrix		= 50834,
	tcColorimetricReference			= 50879,
	tcCameraCalibrationSignature	= 50931,
	tcProfileCalibrationSignature  	= 50932,
	tcExtraCameraProfiles			= 50933,
	tcAsShotProfileName				= 50934,
	tcNoiseReductionApplied			= 50935,
	tcProfileName					= 50936,
	tcProfileHueSatMapDims			= 50937,
	tcProfileHueSatMapData1			= 50938,
	tcProfileHueSatMapData2			= 50939,
	tcProfileToneCurve				= 50940,
	tcProfileEmbedPolicy			= 50941,
	tcProfileCopyright				= 50942,
	tcForwardMatrix1				= 50964,
	tcForwardMatrix2				= 50965,
	tcPreviewApplicationName		= 50966,
	tcPreviewApplicationVersion		= 50967,
	tcPreviewSettingsName			= 50968,
	tcPreviewSettingsDigest			= 50969,
	tcPreviewColorSpace				= 50970,
	tcPreviewDateTime				= 50971,
	tcRawImageDigest				= 50972,
	tcOriginalRawFileDigest			= 50973,
	tcSubTileBlockSize				= 50974,
	tcRowInterleaveFactor			= 50975,
	tcProfileLookTableDims			= 50981,
	tcProfileLookTableData			= 50982,
	tcOpcodeList1					= 51008,
	tcOpcodeList2					= 51009,
	tcOpcodeList3					= 51022,
	tcNoiseProfile					= 51041,
	tcKodakKDCPrivateIFD			= 65024
	};

/*****************************************************************************/

// Additional values that can be passed as IFD parent codes.

enum
	{

	tcFirstSubIFD					= 0x10000,
	tcLastSubIFD					= 0x1FFFF,

	tcFirstChainedIFD				= 0x20000,
	tcLastChainedIFD				= 0x2FFFF,

	tcFirstMakerNoteIFD				= 0x30000,
	tcLastMakerNoteIFD				= 0x3FFFF,

	tcCanonMakerNote				= tcFirstMakerNoteIFD,
	tcEpsonMakerNote,
	tcFujiMakerNote,
	tcHasselbladMakerNote,
	tcKodakMakerNote,
	tcKodakMakerNote65280,
	tcLeicaMakerNote,
	tcMamiyaMakerNote,
	tcMinoltaMakerNote,
	tcNikonMakerNote,
	tcOlympusMakerNote,
	tcOlympusMakerNote8208,
	tcOlympusMakerNote8224,
	tcOlympusMakerNote8240,
	tcOlympusMakerNote8256,
	tcOlympusMakerNote8272,
	tcOlympusMakerNote12288,
	tcPanasonicMakerNote,
	tcPentaxMakerNote,
	tcPhaseOneMakerNote,
	tcRicohMakerNote,
	tcRicohMakerNoteCameraInfo,
	tcSonyMakerNote,
	tcSonyMakerNoteSubInfo,
	tcSonyPrivateIFD1,
	tcSonyPrivateIFD2,
	tcSonyPrivateIFD3A,
	tcSonyPrivateIFD3B,
	tcSonyPrivateIFD3C,

	tcCanonCRW						= 0x40000,
	tcContaxRAW,
	tcContaxHeader,
	tcFujiRAF,
	tcFujiHeader,
	tcFujiRawInfo1,
	tcFujiRawInfo2,
	tcLeafMOS,
	tcMinoltaMRW,
	tcPanasonicRAW,
	tcFoveonX3F,
	tcJPEG,
	tcAdobePSD

	};

/*****************************************************************************/

// GPS tag codes are only valid in the GPS IFD.

enum
	{
	tcGPSVersionID					= 0,
	tcGPSLatitudeRef				= 1,
	tcGPSLatitude					= 2,
	tcGPSLongitudeRef				= 3,
	tcGPSLongitude					= 4,
	tcGPSAltitudeRef				= 5,
	tcGPSAltitude					= 6,
	tcGPSTimeStamp					= 7,
	tcGPSSatellites					= 8,
	tcGPSStatus						= 9,
	tcGPSMeasureMode				= 10,
	tcGPSDOP						= 11,
	tcGPSSpeedRef					= 12,
	tcGPSSpeed						= 13,
	tcGPSTrackRef					= 14,
	tcGPSTrack						= 15,
	tcGPSImgDirectionRef			= 16,
	tcGPSImgDirection				= 17,
	tcGPSMapDatum					= 18,
	tcGPSDestLatitudeRef			= 19,
	tcGPSDestLatitude				= 20,
	tcGPSDestLongitudeRef			= 21,
	tcGPSDestLongitude				= 22,
	tcGPSDestBearingRef				= 23,
	tcGPSDestBearing				= 24,
	tcGPSDestDistanceRef			= 25,
	tcGPSDestDistance				= 26,
	tcGPSProcessingMethod			= 27,
	tcGPSAreaInformation			= 28,
	tcGPSDateStamp					= 29,
	tcGPSDifferential				= 30
	};

/*****************************************************************************/

// Tag codes used in the Interoperability IFD.

enum
	{
	tcInteroperabilityIndex			= 0x0001,
	tcInteroperabilityVersion		= 0x0002,
	tcRelatedImageFileFormat		= 0x1000,
	tcRelatedImageWidth				= 0x1001,
	tcRelatedImageLength			= 0x1002
	};

/*****************************************************************************/

// JPEG marker codes.

enum JpegMarker
	{

	M_TEM = 0x01,

	M_SOF0  = 0xc0,
	M_SOF1  = 0xc1,
	M_SOF2  = 0xc2,
	M_SOF3  = 0xc3,
	M_DHT   = 0xc4,
	M_SOF5  = 0xc5,
	M_SOF6  = 0xc6,
	M_SOF7  = 0xc7,
	M_JPG   = 0xc8,
	M_SOF9  = 0xc9,
	M_SOF10 = 0xca,
	M_SOF11 = 0xcb,
	M_DAC   = 0xcc,
	M_SOF13 = 0xcd,
	M_SOF14 = 0xce,
	M_SOF15 = 0xcf,

	M_RST0 = 0xd0,
	M_RST1 = 0xd1,
	M_RST2 = 0xd2,
	M_RST3 = 0xd3,
	M_RST4 = 0xd4,
	M_RST5 = 0xd5,
	M_RST6 = 0xd6,
	M_RST7 = 0xd7,

	M_SOI = 0xd8,
	M_EOI = 0xd9,
	M_SOS = 0xda,
	M_DQT = 0xdb,
	M_DNL = 0xdc,
	M_DRI = 0xdd,
	M_DHP = 0xde,
	M_EXP = 0xdf,

	M_APP0  = 0xe0,
	M_APP1  = 0xe1,
	M_APP2  = 0xe2,
	M_APP3  = 0xe3,
	M_APP4  = 0xe4,
	M_APP5  = 0xe5,
	M_APP6  = 0xe6,
	M_APP7  = 0xe7,
	M_APP8  = 0xe8,
	M_APP9  = 0xe9,
	M_APP10 = 0xea,
	M_APP11 = 0xeb,
	M_APP12 = 0xec,
	M_APP13 = 0xed,
	M_APP14 = 0xee,
	M_APP15 = 0xef,

	M_JPG0  = 0xf0,
	M_JPG1  = 0xf1,
	M_JPG2  = 0xf2,
	M_JPG3  = 0xf3,
	M_JPG4  = 0xf4,
	M_JPG5  = 0xf5,
	M_JPG6  = 0xf6,
	M_JPG7  = 0xf7,
	M_JPG8  = 0xf8,
	M_JPG9  = 0xf9,
	M_JPG10 = 0xfa,
	M_JPG11 = 0xfb,
	M_JPG12 = 0xfc,
	M_JPG13 = 0xfd,
	M_COM   = 0xfe,

	M_ERROR = 0x100

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
