/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_parse_utils.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_parse_utils.h"

#include "dng_date_time.h"
#include "dng_globals.h"
#include "dng_ifd.h"
#include "dng_tag_codes.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_types.h"
#include "dng_stream.h"
#include "dng_exceptions.h"
#include "dng_utils.h"

/*****************************************************************************/

#if qDNGValidate

/*****************************************************************************/

struct dng_name_table
	{
	uint32 key;
	const char *name;
	};

/*****************************************************************************/

static const char * LookupName (uint32 key,
						 		const dng_name_table *table,
						 		uint32 table_entries)
	{

	for (uint32 index = 0; index < table_entries; index++)
		{

		if (key == table [index] . key)
			{

			return table [index] . name;

			}

		}

	return NULL;

	}

/*****************************************************************************/

const char * LookupParentCode (uint32 parentCode)
	{

	const dng_name_table kParentCodeNames [] =
		{
		{	0,							"IFD 0"							},
		{	tcExifIFD,					"Exif IFD"						},
		{	tcGPSInfo,					"GPS IFD"						},
		{	tcInteroperabilityIFD,		"Interoperability IFD"			},
		{	tcKodakDCRPrivateIFD,		"Kodak DCR Private IFD"			},
		{	tcKodakKDCPrivateIFD,		"Kodak KDC Private IFD"			},
		{	tcCanonMakerNote,			"Canon MakerNote"				},
		{	tcEpsonMakerNote,			"Epson MakerNote"				},
		{	tcFujiMakerNote,			"Fuji MakerNote"				},
		{	tcHasselbladMakerNote,		"Hasselblad MakerNote"			},
		{	tcKodakMakerNote,			"Kodak MakerNote"				},
		{	tcKodakMakerNote65280,		"Kodak MakerNote 65280"			},
		{	tcLeicaMakerNote,			"Leica MakerNote"				},
		{	tcMamiyaMakerNote,			"Mamiya MakerNote"				},
		{	tcMinoltaMakerNote,			"Minolta MakerNote"				},
		{	tcNikonMakerNote,			"Nikon MakerNote"				},
		{	tcOlympusMakerNote,			"Olympus MakerNote"				},
		{	tcOlympusMakerNote8208,		"Olympus MakerNote 8208"		},
		{	tcOlympusMakerNote8224,		"Olympus MakerNote 8224"		},
		{	tcOlympusMakerNote8240,		"Olympus MakerNote 8240"		},
		{	tcOlympusMakerNote8256,		"Olympus MakerNote 8256"		},
		{	tcOlympusMakerNote8272,		"Olympus MakerNote 8272"		},
		{	tcOlympusMakerNote12288,	"Olympus MakerNote 12288"		},
		{	tcPanasonicMakerNote,		"Panasonic MakerNote"			},
		{	tcPentaxMakerNote,			"Pentax MakerNote"				},
		{	tcPhaseOneMakerNote,		"Phase One MakerNote"			},
		{	tcRicohMakerNote,			"Ricoh MakerNote"				},
		{	tcRicohMakerNoteCameraInfo,	"Ricoh MakerNote Camera Info"	},
		{	tcSonyMakerNote,			"Sony MakerNote"				},
		{	tcSonyMakerNoteSubInfo,		"Sony MakerNote SubInfo"		},
		{	tcSonyPrivateIFD1,			"Sony Private IFD 1"			},
		{	tcSonyPrivateIFD2,			"Sony Private IFD 2"			},
		{	tcSonyPrivateIFD3A,			"Sony Private IFD 3A"			},
		{	tcSonyPrivateIFD3B,			"Sony Private IFD 3B"			},
		{	tcSonyPrivateIFD3C,			"Sony Private IFD 3C"			},
		{	tcCanonCRW,					"Canon CRW"						},
		{	tcContaxRAW,				"Contax RAW"					},
		{	tcFujiRAF,					"Fuji RAF"						},
		{	tcLeafMOS,					"Leaf MOS"						},
		{	tcMinoltaMRW,				"Minolta MRW"					},
		{	tcPanasonicRAW,				"Panasonic RAW"					},
		{	tcFoveonX3F,				"Foveon X3F"					},
		{	tcJPEG,						"JPEG"							},
		{	tcAdobePSD,					"Adobe PSD"						}
		};

	const char *name = LookupName (parentCode,
								   kParentCodeNames,
								   sizeof (kParentCodeNames    ) /
								   sizeof (kParentCodeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	if (parentCode >= tcFirstSubIFD &&
		parentCode <= tcLastSubIFD)
		{

		sprintf (s, "SubIFD %u", (unsigned) (parentCode - tcFirstSubIFD + 1));

		}

	else if (parentCode >= tcFirstChainedIFD &&
			 parentCode <= tcLastChainedIFD)
		{

		sprintf (s, "Chained IFD %u", (unsigned) (parentCode - tcFirstChainedIFD + 1));

		}

	else
		{

		sprintf (s, "ParentIFD %u", (unsigned) parentCode);

		}

	return s;

	}

/*****************************************************************************/

const char * LookupTagCode (uint32 parentCode,
							uint32 tagCode)
	{

	const dng_name_table kTagNames [] =
		{
		{	tcNewSubFileType,					"NewSubFileType"				},
		{	tcSubFileType,						"SubFileType"					},
		{	tcImageWidth,						"ImageWidth"					},
		{	tcImageLength,						"ImageLength"					},
		{	tcBitsPerSample,					"BitsPerSample"					},
		{	tcCompression,						"Compression"					},
		{	tcPhotometricInterpretation,		"PhotometricInterpretation"		},
		{	tcThresholding,						"Thresholding"					},
		{	tcCellWidth,						"CellWidth"						},
		{	tcCellLength,						"CellLength"					},
		{	tcFillOrder,						"FillOrder"						},
		{	tcImageDescription,					"ImageDescription"				},
		{	tcMake,								"Make"							},
		{	tcModel,							"Model"							},
		{	tcStripOffsets,						"StripOffsets"					},
		{	tcOrientation,						"Orientation"					},
		{	tcSamplesPerPixel,					"SamplesPerPixel"				},
		{	tcRowsPerStrip,						"RowsPerStrip"					},
		{	tcStripByteCounts,					"StripByteCounts"				},
		{	tcMinSampleValue,					"MinSampleValue"				},
		{	tcMaxSampleValue,					"MaxSampleValue"				},
		{	tcXResolution,						"XResolution"					},
		{	tcYResolution,						"YResolution"					},
		{	tcPlanarConfiguration,				"PlanarConfiguration"			},
		{	tcFreeOffsets,						"FreeOffsets"					},
		{	tcFreeByteCounts,					"FreeByteCounts"				},
		{	tcGrayResponseUnit,					"GrayResponseUnit"				},
		{	tcGrayResponseCurve,				"GrayResponseCurve"				},
		{	tcResolutionUnit,					"ResolutionUnit"				},
		{	tcTransferFunction,					"TransferFunction"				},
		{	tcSoftware,							"Software"						},
		{	tcDateTime,							"DateTime"						},
		{	tcArtist,							"Artist"						},
		{	tcHostComputer,						"HostComputer"					},
		{	tcWhitePoint,						"WhitePoint"					},
		{	tcPrimaryChromaticities,			"PrimaryChromaticities"			},
		{	tcColorMap,							"ColorMap"						},
		{	tcTileWidth,						"TileWidth"						},
		{	tcTileLength,						"TileLength"					},
		{	tcTileOffsets,						"TileOffsets"					},
		{	tcTileByteCounts,					"TileByteCounts"				},
		{	tcSubIFDs,							"SubIFDs"						},
		{	tcExtraSamples,						"ExtraSamples"					},
		{	tcSampleFormat,						"SampleFormat"					},
		{	tcJPEGTables,						"JPEGTables"					},
		{	tcJPEGProc,							"JPEGProc"						},
		{	tcJPEGInterchangeFormat,			"JPEGInterchangeFormat"			},
		{	tcJPEGInterchangeFormatLength,		"JPEGInterchangeFormatLength"	},
		{	tcYCbCrCoefficients,				"YCbCrCoefficients"				},
		{	tcYCbCrSubSampling,					"YCbCrSubSampling"				},
		{	tcYCbCrPositioning,					"YCbCrPositioning"				},
		{	tcReferenceBlackWhite,				"ReferenceBlackWhite"			},
		{	tcXMP,								"XMP"							},
		{	tcKodakCameraSerialNumber,			"KodakCameraSerialNumber"		},
		{	tcCFARepeatPatternDim,				"CFARepeatPatternDim"			},
		{	tcCFAPattern,						"CFAPattern"					},
		{	tcBatteryLevel,						"BatteryLevel"					},
		{	tcKodakDCRPrivateIFD,				"KodakDCRPrivateIFD"			},
		{	tcCopyright,						"Copyright"						},
		{	tcExposureTime,						"ExposureTime"					},
		{	tcFNumber,							"FNumber"						},
		{	tcIPTC_NAA,							"IPTC/NAA"						},
		{	tcLeafPKTS,							"LeafPKTS"						},
		{	tcAdobeData,						"AdobeData"						},
		{	tcExifIFD,							"ExifIFD"						},
		{	tcICCProfile,						"ICCProfile"					},
		{	tcExposureProgram,					"ExposureProgram"				},
		{	tcSpectralSensitivity,				"SpectralSensitivity"			},
		{	tcGPSInfo,							"GPSInfo"						},
		{	tcISOSpeedRatings,					"ISOSpeedRatings"				},
		{	tcOECF,								"OECF"							},
		{	tcInterlace,						"Interlace"						},
		{	tcTimeZoneOffset,					"TimeZoneOffset"				},
		{	tcSelfTimerMode,					"SelfTimerMode"					},
		{	tcExifVersion,						"ExifVersion"					},
		{	tcDateTimeOriginal,					"DateTimeOriginal"				},
		{	tcDateTimeDigitized,				"DateTimeDigitized"				},
		{	tcComponentsConfiguration,			"ComponentsConfiguration"		},
		{	tcCompressedBitsPerPixel,			"CompressedBitsPerPixel"		},
		{	tcShutterSpeedValue,				"ShutterSpeedValue"				},
		{	tcApertureValue,					"ApertureValue"					},
		{	tcBrightnessValue,					"BrightnessValue"				},
		{	tcExposureBiasValue,				"ExposureBiasValue"				},
		{	tcMaxApertureValue,					"MaxApertureValue"				},
		{	tcSubjectDistance,					"SubjectDistance"				},
		{	tcMeteringMode,						"MeteringMode"					},
		{	tcLightSource,						"LightSource"					},
		{	tcFlash,							"Flash"							},
		{	tcFocalLength,						"FocalLength"					},
		{	tcFlashEnergy,						"FlashEnergy"					},
		{	tcSpatialFrequencyResponse,			"SpatialFrequencyResponse"		},
		{	tcNoise,							"Noise"							},
		{	tcFocalPlaneXResolution,			"FocalPlaneXResolution"			},
		{	tcFocalPlaneYResolution,			"FocalPlaneYResolution"			},
		{	tcFocalPlaneResolutionUnit,			"FocalPlaneResolutionUnit"		},
		{	tcImageNumber,						"ImageNumber"					},
		{	tcSecurityClassification,			"SecurityClassification"		},
		{	tcImageHistory,						"ImageHistory"					},
		{	tcSubjectArea,						"SubjectArea"					},
		{	tcExposureIndex,					"ExposureIndex"					},
		{	tcTIFF_EP_StandardID,				"TIFF/EPStandardID"				},
		{	tcSensingMethod,					"SensingMethod"					},
		{	tcMakerNote,						"MakerNote"						},
		{	tcUserComment,						"UserComment"					},
		{	tcSubsecTime,						"SubsecTime"					},
		{	tcSubsecTimeOriginal,				"SubsecTimeOriginal"			},
		{	tcSubsecTimeDigitized,				"SubsecTimeDigitized"			},
		{	tcAdobeLayerData,					"AdobeLayerData"				},
		{	tcFlashPixVersion,					"FlashPixVersion"				},
		{	tcColorSpace,						"ColorSpace"					},
		{	tcPixelXDimension,					"PixelXDimension"				},
		{	tcPixelYDimension,					"PixelYDimension"				},
		{	tcRelatedSoundFile,					"RelatedSoundFile"				},
		{	tcInteroperabilityIFD,				"InteroperabilityIFD"			},
		{	tcFlashEnergyExif,					"FlashEnergyExif"				},
		{	tcSpatialFrequencyResponseExif,		"SpatialFrequencyResponseExif"	},
		{	tcFocalPlaneXResolutionExif,		"FocalPlaneXResolutionExif"		},
		{	tcFocalPlaneYResolutionExif,		"FocalPlaneYResolutionExif"		},
		{	tcFocalPlaneResolutionUnitExif,		"FocalPlaneResolutionUnitExif"	},
		{	tcSubjectLocation,					"SubjectLocation"				},
		{	tcExposureIndexExif,				"ExposureIndexExif"				},
		{	tcSensingMethodExif,				"SensingMethodExif"				},
		{	tcFileSource,						"FileSource"					},
		{	tcSceneType,						"SceneType"						},
		{	tcCFAPatternExif,					"CFAPatternExif"				},
		{	tcCustomRendered,					"CustomRendered"				},
		{	tcExposureMode,						"ExposureMode"					},
		{	tcWhiteBalance,						"WhiteBalance"					},
		{	tcDigitalZoomRatio,					"DigitalZoomRatio"				},
		{	tcFocalLengthIn35mmFilm,			"FocalLengthIn35mmFilm"			},
		{	tcSceneCaptureType,					"SceneCaptureType"				},
		{	tcGainControl,						"GainControl"					},
		{	tcContrast,							"Contrast"						},
		{	tcSaturation,						"Saturation"					},
		{	tcSharpness,						"Sharpness"						},
		{	tcDeviceSettingDescription,			"DeviceSettingDescription"		},
		{	tcSubjectDistanceRange,				"SubjectDistanceRange"			},
		{	tcImageUniqueID,					"ImageUniqueID"					},
		{	tcGamma,							"Gamma"							},
		{	tcPrintImageMatchingInfo,			"PrintImageMatchingInfo"		},
		{	tcDNGVersion,						"DNGVersion"					},
		{	tcDNGBackwardVersion,				"DNGBackwardVersion"			},
		{	tcUniqueCameraModel,				"UniqueCameraModel"				},
		{	tcLocalizedCameraModel,				"LocalizedCameraModel"			},
		{	tcCFAPlaneColor,					"CFAPlaneColor"					},
		{	tcCFALayout,						"CFALayout"						},
		{	tcLinearizationTable,				"LinearizationTable"			},
		{	tcBlackLevelRepeatDim,				"BlackLevelRepeatDim"			},
		{	tcBlackLevel,						"BlackLevel"					},
		{	tcBlackLevelDeltaH,					"BlackLevelDeltaH"				},
		{	tcBlackLevelDeltaV,					"BlackLevelDeltaV"				},
		{	tcWhiteLevel,						"WhiteLevel"					},
		{	tcDefaultScale,						"DefaultScale"					},
		{	tcDefaultCropOrigin,				"DefaultCropOrigin"				},
		{	tcDefaultCropSize,					"DefaultCropSize"				},
		{	tcColorMatrix1,						"ColorMatrix1"					},
		{	tcColorMatrix2,						"ColorMatrix2"					},
		{	tcCameraCalibration1,				"CameraCalibration1"			},
		{	tcCameraCalibration2,				"CameraCalibration2"			},
		{	tcReductionMatrix1,					"ReductionMatrix1"				},
		{	tcReductionMatrix2,					"ReductionMatrix2"				},
		{	tcAnalogBalance,					"AnalogBalance"					},
		{	tcAsShotNeutral,					"AsShotNeutral"					},
		{	tcAsShotWhiteXY,					"AsShotWhiteXY"					},
		{	tcBaselineExposure,					"BaselineExposure"				},
		{	tcBaselineNoise,					"BaselineNoise"					},
		{	tcBaselineSharpness,				"BaselineSharpness"				},
		{	tcBayerGreenSplit,					"BayerGreenSplit"				},
		{	tcLinearResponseLimit,				"LinearResponseLimit"			},
		{	tcCameraSerialNumber,				"CameraSerialNumber"			},
		{	tcLensInfo,							"LensInfo"						},
		{	tcChromaBlurRadius,					"ChromaBlurRadius"				},
		{	tcAntiAliasStrength,				"AntiAliasStrength"				},
		{	tcShadowScale,						"ShadowScale"					},
		{	tcDNGPrivateData,					"DNGPrivateData"				},
		{	tcMakerNoteSafety,					"MakerNoteSafety"				},
		{	tcCalibrationIlluminant1,			"CalibrationIlluminant1"		},
		{	tcCalibrationIlluminant2,			"CalibrationIlluminant2"		},
		{	tcBestQualityScale,					"BestQualityScale"				},
		{	tcRawDataUniqueID,					"RawDataUniqueID"				},
		{	tcOriginalRawFileName,				"OriginalRawFileName"			},
		{	tcOriginalRawFileData,				"OriginalRawFileData"			},
		{	tcActiveArea,						"ActiveArea"					},
		{	tcMaskedAreas,						"MaskedAreas"					},
		{	tcAsShotICCProfile,					"AsShotICCProfile"				},
		{	tcAsShotPreProfileMatrix,			"AsShotPreProfileMatrix"		},
		{	tcCurrentICCProfile,				"CurrentICCProfile"				},
		{	tcCurrentPreProfileMatrix,			"CurrentPreProfileMatrix"		},
		{	tcColorimetricReference,			"ColorimetricReference"			},
		{	tcCameraCalibrationSignature,		"CameraCalibrationSignature"	},
		{	tcProfileCalibrationSignature,		"ProfileCalibrationSignature"	},
		{	tcExtraCameraProfiles,				"ExtraCameraProfiles"			},
		{	tcAsShotProfileName,				"AsShotProfileName"				},
		{	tcNoiseReductionApplied,			"NoiseReductionApplied"			},
		{	tcProfileName,						"ProfileName"					},
		{	tcProfileHueSatMapDims,				"ProfileHueSatMapDims"			},
		{	tcProfileHueSatMapData1,			"ProfileHueSatMapData1"			},
		{	tcProfileHueSatMapData2,			"ProfileHueSatMapData2"			},
		{	tcProfileToneCurve,					"ProfileToneCurve"				},
		{	tcProfileEmbedPolicy,				"ProfileEmbedPolicy"			},
		{	tcProfileCopyright,					"ProfileCopyright"				},
		{	tcForwardMatrix1,					"ForwardMatrix1"				},
		{	tcForwardMatrix2,					"ForwardMatrix2"				},
		{	tcPreviewApplicationName,			"PreviewApplicationName"		},
		{	tcPreviewApplicationVersion,		"PreviewApplicationVersion"		},
		{	tcPreviewSettingsName,				"PreviewSettingsName"		    },
		{	tcPreviewSettingsDigest,			"PreviewSettingsDigest"		    },
		{	tcPreviewColorSpace,				"PreviewColorSpace"				},
		{	tcPreviewDateTime,					"PreviewDateTime"				},
		{	tcRawImageDigest,					"RawImageDigest"				},
		{	tcOriginalRawFileDigest,			"OriginalRawFileDigest"			},
		{	tcSubTileBlockSize,					"SubTileBlockSize"				},
		{	tcRowInterleaveFactor,				"RowInterleaveFactor"			},
		{	tcProfileLookTableDims,				"ProfileLookTableDims"			},
		{	tcProfileLookTableData,				"ProfileLookTableData"			},
		{	tcOpcodeList1,						"OpcodeList1"					},
		{	tcOpcodeList2,						"OpcodeList2"					},
		{	tcOpcodeList3,						"OpcodeList3"					},
		{	tcNoiseProfile,						"NoiseProfile"					},
		{	tcKodakKDCPrivateIFD,				"KodakKDCPrivateIFD"			}
		};

	const dng_name_table kGPSTagNames [] =
		{
		{	tcGPSVersionID,					"GPSVersionID"			},
		{	tcGPSLatitudeRef,				"GPSLatitudeRef"		},
		{	tcGPSLatitude,					"GPSLatitude"			},
		{	tcGPSLongitudeRef,				"GPSLongitudeRef"		},
		{	tcGPSLongitude,					"GPSLongitude"			},
		{	tcGPSAltitudeRef,				"GPSAltitudeRef"		},
		{	tcGPSAltitude,					"GPSAltitude"			},
		{	tcGPSTimeStamp,					"GPSTimeStamp"			},
		{	tcGPSSatellites,				"GPSSatellites"			},
		{	tcGPSStatus,					"GPSStatus"				},
		{	tcGPSMeasureMode,				"GPSMeasureMode"		},
		{	tcGPSDOP,						"GPSDOP"				},
		{	tcGPSSpeedRef,					"GPSSpeedRef"			},
		{	tcGPSSpeed,						"GPSSpeed"				},
		{	tcGPSTrackRef,					"GPSTrackRef"			},
		{	tcGPSTrack,						"GPSTrack"				},
		{	tcGPSImgDirectionRef,			"GPSImgDirectionRef"	},
		{	tcGPSImgDirection,				"GPSImgDirection"		},
		{	tcGPSMapDatum,					"GPSMapDatum"			},
		{	tcGPSDestLatitudeRef,			"GPSDestLatitudeRef"	},
		{	tcGPSDestLatitude,				"GPSDestLatitude"		},
		{	tcGPSDestLongitudeRef,			"GPSDestLongitudeRef"	},
		{	tcGPSDestLongitude,				"GPSDestLongitude"		},
		{	tcGPSDestBearingRef,			"GPSDestBearingRef"		},
		{	tcGPSDestBearing,				"GPSDestBearing"		},
		{	tcGPSDestDistanceRef,			"GPSDestDistanceRef"	},
		{	tcGPSDestDistance,				"GPSDestDistance"		},
		{	tcGPSProcessingMethod,			"GPSProcessingMethod"	},
		{	tcGPSAreaInformation,			"GPSAreaInformation"	},
		{	tcGPSDateStamp,					"GPSDateStamp"			},
		{	tcGPSDifferential,				"GPSDifferential"		}
		};

	const dng_name_table kInteroperabilityTagNames [] =
		{
		{	tcInteroperabilityIndex,		"InteroperabilityIndex"		},
		{	tcInteroperabilityVersion,		"InteroperabilityVersion"	},
		{	tcRelatedImageFileFormat,		"RelatedImageFileFormat"	},
		{	tcRelatedImageWidth,			"RelatedImageWidth"			},
		{	tcRelatedImageLength,			"RelatedImageLength"		}
		};

	const dng_name_table kFujiTagNames [] =
		{
		{	tcFujiHeader,					"FujiHeader"	},
		{	tcFujiRawInfo1,					"FujiRawInfo1"	},
		{	tcFujiRawInfo2,					"FujiRawInfo2"	}
		};

	const dng_name_table kContaxTagNames [] =
		{
		{	tcContaxHeader,					"ContaxHeader"	}
		};

	const char *name = NULL;

	if (parentCode == 0         										 ||
		parentCode == tcExifIFD 										 ||
		parentCode == tcLeafMOS 										 ||
		parentCode >= tcFirstSubIFD     && parentCode <= tcLastSubIFD    ||
		parentCode >= tcFirstChainedIFD && parentCode <= tcLastChainedIFD)
		{

		name = LookupName (tagCode,
						   kTagNames,
						   sizeof (kTagNames    ) /
						   sizeof (kTagNames [0]));

		}

	else if (parentCode == tcGPSInfo)
		{

		name = LookupName (tagCode,
						   kGPSTagNames,
						   sizeof (kGPSTagNames    ) /
						   sizeof (kGPSTagNames [0]));

		}

	else if (parentCode == tcInteroperabilityIFD)
		{

		name = LookupName (tagCode,
						   kInteroperabilityTagNames,
						   sizeof (kInteroperabilityTagNames    ) /
						   sizeof (kInteroperabilityTagNames [0]));

		}

	else if (parentCode == tcFujiRAF)
		{

		name = LookupName (tagCode,
						   kFujiTagNames,
						   sizeof (kFujiTagNames    ) /
						   sizeof (kFujiTagNames [0]));

		}

	else if (parentCode == tcContaxRAW)
		{

		name = LookupName (tagCode,
						   kContaxTagNames,
						   sizeof (kContaxTagNames    ) /
						   sizeof (kContaxTagNames [0]));

		}

	if (name)
		{
		return name;
		}

	static char s [32];

	if (parentCode == tcCanonCRW)
		{
		sprintf (s, "CRW_%04X", (unsigned) tagCode);
		}

	else if (parentCode == tcMinoltaMRW)
		{

		char c1 = (char) ((tagCode >> 24) & 0xFF);
		char c2 = (char) ((tagCode >> 16) & 0xFF);
		char c3 = (char) ((tagCode >>  8) & 0xFF);
		char c4 = (char) ((tagCode      ) & 0xFF);

		if (c1 < ' ') c1 = '_';
		if (c2 < ' ') c2 = '_';
		if (c3 < ' ') c3 = '_';
		if (c4 < ' ') c4 = '_';

		sprintf (s, "MRW%c%c%c%c", c1, c2, c3, c4);

		}

	else if (parentCode == tcFujiRawInfo1)
		{
		sprintf (s, "RAF1_%04X", (unsigned) tagCode);
		}

	else if (parentCode == tcFujiRawInfo2)
		{
		sprintf (s, "RAF2_%04X", (unsigned) tagCode);
		}

	else
		{
		sprintf (s, "Tag%u", (unsigned) tagCode);
		}

	return s;

	}

/*****************************************************************************/

const char * LookupTagType (uint32 tagType)
	{

	const dng_name_table kTagTypeNames [] =
		{
		{	ttByte,			"Byte"		},
		{	ttAscii,		"ASCII"		},
		{	ttShort,		"Short"		},
		{	ttLong,			"Long"		},
		{	ttRational,		"Rational"	},
		{	ttSByte,		"SByte"		},
		{	ttUndefined,	"Undefined"	},
		{	ttSShort,		"SShort"	},
		{	ttSLong,		"SLong"		},
		{	ttSRational,	"SRational"	},
		{	ttFloat,		"Float"		},
		{	ttDouble,		"Double"	},
		{	ttIFD,			"IFD"		},
		{	ttUnicode,		"Unicode"	},
		{	ttComplex,		"Complex"	}
		};

	const char *name = LookupName (tagType,
								   kTagTypeNames,
								   sizeof (kTagTypeNames    ) /
								   sizeof (kTagTypeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "Type%u", (unsigned) tagType);

	return s;

	}

/*****************************************************************************/

const char * LookupNewSubFileType (uint32 key)
	{

	const dng_name_table kNewSubFileTypeNames [] =
		{
		{	sfMainImage		 , "Main Image"			},
		{	sfPreviewImage	 , "Preview Image"		},
		{	sfAltPreviewImage, "Alt Preview Image"	}
		};

	const char *name = LookupName (key,
								   kNewSubFileTypeNames,
								   sizeof (kNewSubFileTypeNames    ) /
								   sizeof (kNewSubFileTypeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupCompression (uint32 key)
	{

	const dng_name_table kCompressionNames [] =
		{
		{	ccUncompressed,		"Uncompressed"	},
		{	ccLZW,				"LZW"			},
		{	ccOldJPEG,			"Old JPEG"		},
		{	ccJPEG,				"JPEG"			},
		{	ccDeflate,			"Deflate"		},
		{	ccPackBits,			"PackBits"		},
		{	ccOldDeflate,		"OldDeflate"	}
		};

	const char *name = LookupName (key,
								   kCompressionNames,
								   sizeof (kCompressionNames    ) /
								   sizeof (kCompressionNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupPhotometricInterpretation (uint32 key)
	{

	const dng_name_table kPhotometricInterpretationNames [] =
		{
		{	piWhiteIsZero, 			"WhiteIsZero"		},
		{	piBlackIsZero,			"BlackIsZero"		},
		{	piRGB,					"RGB"				},
		{	piRGBPalette,			"RGBPalette"		},
		{	piTransparencyMask,		"TransparencyMask"	},
		{	piCMYK,					"CMYK"				},
		{	piYCbCr,				"YCbCr"				},
		{	piCIELab,				"CIELab"			},
		{	piICCLab,				"ICCLab"			},
		{	piCFA,					"CFA"				},
		{	piLinearRaw,			"LinearRaw"			}
		};

	const char *name = LookupName (key,
								   kPhotometricInterpretationNames,
								   sizeof (kPhotometricInterpretationNames    ) /
								   sizeof (kPhotometricInterpretationNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupOrientation (uint32 key)
	{

	const dng_name_table kOrientationNames [] =
		{
		{	1, 	"1 - 0th row is top, 0th column is left"		},
		{	2,	"2 - 0th row is top, 0th column is right"		},
		{	3,	"3 - 0th row is bottom, 0th column is right"	},
		{	4,	"4 - 0th row is bottom, 0th column is left"		},
		{	5,	"5 - 0th row is left, 0th column is top"		},
		{	6,	"6 - 0th row is right, 0th column is top"		},
		{	7,	"7 - 0th row is right, 0th column is bottom"	},
		{	8,	"8 - 0th row is left, 0th column is bottom"		},
		{	9,	"9 - unknown"									}
		};

	const char *name = LookupName (key,
								   kOrientationNames,
								   sizeof (kOrientationNames    ) /
								   sizeof (kOrientationNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupResolutionUnit (uint32 key)
	{

	const dng_name_table kResolutionUnitNames [] =
		{
		{	ruNone, 	"None"			},
		{	ruInch,		"Inch"			},
		{	ruCM,		"cm"			},
		{	ruMM,		"mm"			},
		{	ruMicroM,	"Micrometer"	}
		};

	const char *name = LookupName (key,
								   kResolutionUnitNames,
								   sizeof (kResolutionUnitNames    ) /
								   sizeof (kResolutionUnitNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupCFAColor (uint32 key)
	{

	const dng_name_table kCFAColorNames [] =
		{
		{	0, "Red"		},
		{	1, "Green"		},
		{	2, "Blue"		},
		{	3, "Cyan"		},
		{	4, "Magenta"	},
		{	5, "Yellow"		},
		{	6, "White"		}
		};

	const char *name = LookupName (key,
								   kCFAColorNames,
								   sizeof (kCFAColorNames    ) /
								   sizeof (kCFAColorNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "Color%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupSensingMethod (uint32 key)
	{

	const dng_name_table kSensingMethodNames [] =
		{
		{	0, "Undefined"				},
		{	1, "MonochromeArea"			},
		{	2, "OneChipColorArea"		},
		{	3, "TwoChipColorArea"		},
		{	4, "ThreeChipColorArea"		},
		{	5, "ColorSequentialArea"	},
		{	6, "MonochromeLinear"		},
		{	7, "TriLinear"				},
		{	8, "ColorSequentialLinear"	}
		};

	const char *name = LookupName (key,
								   kSensingMethodNames,
								   sizeof (kSensingMethodNames    ) /
								   sizeof (kSensingMethodNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupExposureProgram (uint32 key)
	{

	const dng_name_table kExposureProgramNames [] =
		{
		{	epUnidentified,		"Unidentified"		},
		{	epManual,			"Manual"			},
		{	epProgramNormal,	"Program Normal"	},
		{	epAperturePriority,	"Aperture Priority"	},
		{	epShutterPriority, 	"Shutter Priority"	},
		{	epProgramCreative,	"Program Creative"	},
		{	epProgramAction,	"Program Action"	},
		{	epPortraitMode,		"Portrait Mode"		},
		{	epLandscapeMode,	"Landscape Mode"	}
		};

	const char *name = LookupName (key,
								   kExposureProgramNames,
								   sizeof (kExposureProgramNames    ) /
								   sizeof (kExposureProgramNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupMeteringMode (uint32 key)
	{

	const dng_name_table kMeteringModeNames [] =
		{
		{	mmUnidentified,   			"Unknown"				},
		{	mmAverage,   				"Average"				},
		{	mmCenterWeightedAverage,	"CenterWeightedAverage"	},
		{	mmSpot,   					"Spot"					},
		{	mmMultiSpot,   				"MultiSpot"				},
		{	mmPattern,   				"Pattern"				},
		{	mmPartial,   				"Partial"				},
		{	mmOther, 					"Other"					}
		};

	const char *name = LookupName (key,
								   kMeteringModeNames,
								   sizeof (kMeteringModeNames    ) /
								   sizeof (kMeteringModeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupLightSource (uint32 key)
	{

	const dng_name_table kLightSourceNames [] =
		{
		{	lsUnknown,				"Unknown"									},
		{	lsDaylight,				"Daylight"									},
		{	lsFluorescent,			"Fluorescent"								},
		{	lsTungsten,				"Tungsten (incandescent light)"				},
		{	lsFlash,				"Flash"										},
		{	lsFineWeather,			"Fine weather"								},
		{	lsCloudyWeather,		"Cloudy weather"							},
		{	lsShade,				"Shade"										},
		{	lsDaylightFluorescent,	"Daylight fluorescent (D 5700 - 7100K)"		},
		{	lsDayWhiteFluorescent,	"Day white fluorescent (N 4600 - 5400K)"	},
		{	lsCoolWhiteFluorescent,	"Cool white fluorescent (W 3900 - 4500K)"	},
		{	lsWhiteFluorescent,		"White fluorescent (WW 3200 - 3700K)"		},
		{	lsStandardLightA,		"Standard light A"							},
		{	lsStandardLightB,		"Standard light B"							},
		{	lsStandardLightC,		"Standard light C"							},
		{	lsD55,					"D55"										},
		{	lsD65,					"D65"										},
		{	lsD75,					"D75"										},
		{	lsD50,					"D50"										},
		{	lsISOStudioTungsten,	"ISO studio tungsten"						},
		{	lsOther,				"Other"										}
		};

	const char *name = LookupName (key,
								   kLightSourceNames,
								   sizeof (kLightSourceNames    ) /
								   sizeof (kLightSourceNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	if (key & 0x08000)
		{

		sprintf (s, "%uK", (unsigned) (key & 0x7FFF));

		}

	else
		{

		sprintf (s, "%u", (unsigned) key);

		}

	return s;

	}

/*****************************************************************************/

const char * LookupColorSpace (uint32 key)
	{

	const dng_name_table kColorSpaceNames [] =
		{
		{	1,		"sRGB"			},
		{	0xFFFF,	"Uncalibrated"	}
		};

	const char *name = LookupName (key,
								   kColorSpaceNames,
								   sizeof (kColorSpaceNames    ) /
								   sizeof (kColorSpaceNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupFileSource (uint32 key)
	{

	const dng_name_table kFileSourceNames [] =
		{
		{	3,		"DSC"	}
		};

	const char *name = LookupName (key,
								   kFileSourceNames,
								   sizeof (kFileSourceNames    ) /
								   sizeof (kFileSourceNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupSceneType (uint32 key)
	{

	const dng_name_table kSceneTypeNames [] =
		{
		{	1,	"A directly photographed image"		}
		};

	const char *name = LookupName (key,
								   kSceneTypeNames,
								   sizeof (kSceneTypeNames    ) /
								   sizeof (kSceneTypeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupCustomRendered (uint32 key)
	{

	const dng_name_table kCustomRenderedNames [] =
		{
		{	0,		"Normal process"	},
		{	1,		"Custom process"	}
		};

	const char *name = LookupName (key,
								   kCustomRenderedNames,
								   sizeof (kCustomRenderedNames    ) /
								   sizeof (kCustomRenderedNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupExposureMode (uint32 key)
	{

	const dng_name_table kExposureModeNames [] =
		{
		{	0,	"Auto exposure"		},
		{	1,	"Manual exposure"	},
		{	2,	"Auto bracket"		}
		};

	const char *name = LookupName (key,
								   kExposureModeNames,
								   sizeof (kExposureModeNames    ) /
								   sizeof (kExposureModeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupWhiteBalance (uint32 key)
	{

	const dng_name_table kWhiteBalanceNames [] =
		{
		{	0,	"Auto white balance"	},
		{	1,	"Manual white balance"	}
		};

	const char *name = LookupName (key,
								   kWhiteBalanceNames,
								   sizeof (kWhiteBalanceNames    ) /
								   sizeof (kWhiteBalanceNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupSceneCaptureType (uint32 key)
	{

	const dng_name_table kSceneCaptureTypeNames [] =
		{
		{	0,	"Standard"		},
		{	1,	"Landscape"		},
		{	2,	"Portrait"		},
		{	3,	"Night scene"	}
		};

	const char *name = LookupName (key,
								   kSceneCaptureTypeNames,
								   sizeof (kSceneCaptureTypeNames    ) /
								   sizeof (kSceneCaptureTypeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupGainControl (uint32 key)
	{

	const dng_name_table kGainControlNames [] =
		{
		{	0,	"None"				},
		{	1,	"Low gain up"		},
		{	2,	"High gain up"		},
		{	3,	"Low gain down"		},
		{	4,	"High gain down"	}
		};

	const char *name = LookupName (key,
								   kGainControlNames,
								   sizeof (kGainControlNames    ) /
								   sizeof (kGainControlNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupContrast (uint32 key)
	{

	const dng_name_table kContrastNames [] =
		{
		{	0,	"Normal"	},
		{	1,	"Soft"		},
		{	2,	"Hard"		}
		};

	const char *name = LookupName (key,
								   kContrastNames,
								   sizeof (kContrastNames    ) /
								   sizeof (kContrastNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupSaturation (uint32 key)
	{

	const dng_name_table kSaturationNames [] =
		{
		{	0,	"Normal"			},
		{	1,	"Low saturation"	},
		{	2,	"High saturation"	}
		};

	const char *name = LookupName (key,
								   kSaturationNames,
								   sizeof (kSaturationNames    ) /
								   sizeof (kSaturationNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupSharpness (uint32 key)
	{

	const dng_name_table kSharpnessNames [] =
		{
		{	0,	"Normal"	},
		{	1,	"Soft"		},
		{	2,	"Hard"		}
		};

	const char *name = LookupName (key,
								   kSharpnessNames,
								   sizeof (kSharpnessNames    ) /
								   sizeof (kSharpnessNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupSubjectDistanceRange (uint32 key)
	{

	const dng_name_table kSubjectDistanceRangeNames [] =
		{
		{	0,	"Unknown"		},
		{	1,	"Macro"			},
		{	2,	"Close view"	},
		{	3,	"Distant view"	}
		};

	const char *name = LookupName (key,
								   kSubjectDistanceRangeNames,
								   sizeof (kSubjectDistanceRangeNames    ) /
								   sizeof (kSubjectDistanceRangeNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupComponent (uint32 key)
	{

	const dng_name_table kComponentNames [] =
		{
		{	0, "-"	},
		{	1, "Y"	},
		{	2, "Cb"	},
		{	3, "Cr"	},
		{	4, "R"	},
		{	5, "G"	},
		{	6, "B"	}
		};

	const char *name = LookupName (key,
								   kComponentNames,
								   sizeof (kComponentNames    ) /
								   sizeof (kComponentNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupCFALayout (uint32 key)
	{

	const dng_name_table kCFALayoutNames [] =
		{
		{	1,	"Rectangular (or square) layout"																		},
		{	2,	"Staggered layout A: even columns are offset down by 1/2 row"											},
		{	3,	"Staggered layout B: even columns are offset up by 1/2 row"												},
		{	4,	"Staggered layout C: even rows are offset right by 1/2 column"											},
		{	5,	"Staggered layout D: even rows are offset left by 1/2 column"											},
		{	6,	"Staggered layout E: even rows are offset up by 1/2 row, even columns are offset left by 1/2 column"	},
		{	7,	"Staggered layout F: even rows are offset up by 1/2 row, even columns are offset right by 1/2 column"	},
		{	8,	"Staggered layout G: even rows are offset down by 1/2 row, even columns are offset left by 1/2 column"	},
		{	9,	"Staggered layout H: even rows are offset down by 1/2 row, even columns are offset right by 1/2 column"	}
		};

	const char *name = LookupName (key,
								   kCFALayoutNames,
								   sizeof (kCFALayoutNames    ) /
								   sizeof (kCFALayoutNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupMakerNoteSafety (uint32 key)
	{

	const dng_name_table kMakerNoteSafetyNames [] =
		{
		{	0,	"Unsafe"	},
		{	1,	"Safe"		}
		};

	const char *name = LookupName (key,
								   kMakerNoteSafetyNames,
								   sizeof (kMakerNoteSafetyNames    ) /
								   sizeof (kMakerNoteSafetyNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupColorimetricReference (uint32 key)
	{

	const dng_name_table kColorimetricReferenceNames [] =
		{
		{	crSceneReferred,	"Scene Referred"	},
		{	crICCProfilePCS,	"ICC Profile PCS"	}
		};

	const char *name = LookupName (key,
								   kColorimetricReferenceNames,
								   sizeof (kColorimetricReferenceNames    ) /
								   sizeof (kColorimetricReferenceNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupPreviewColorSpace (uint32 key)
	{

	const dng_name_table kPreviewColorSpaceNames [] =
		{
		{	previewColorSpace_Unknown    ,	"Unknown"			},
		{	previewColorSpace_GrayGamma22,	"Gray Gamma 2.2"	},
		{	previewColorSpace_sRGB       ,	"sRGB"				},
		{	previewColorSpace_AdobeRGB   ,	"Adobe RGB (1998)"	},
		{	previewColorSpace_ProPhotoRGB,	"Pro Photo RGB"	    }
		};

	const char *name = LookupName (key,
								   kPreviewColorSpaceNames,
								   sizeof (kPreviewColorSpaceNames    ) /
								   sizeof (kPreviewColorSpaceNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "%u", (unsigned) key);

	return s;

	}

/*****************************************************************************/

const char * LookupJPEGMarker (uint32 key)
	{

	const dng_name_table kJPEGMarkerNames [] =
		{
		{	M_TEM,		"TEM"	},
		{	M_SOF0,		"SOF0"	},
		{	M_SOF1,		"SOF1"	},
		{	M_SOF2,		"SOF2"	},
		{	M_SOF3,		"SOF3"	},
		{	M_DHT,		"DHT"	},
		{	M_SOF5,		"SOF5"	},
		{	M_SOF6,		"SOF6"	},
		{	M_SOF7,		"SOF7"	},
		{	M_JPG,		"JPG"	},
		{	M_SOF9,		"SOF9"	},
		{	M_SOF10,	"SOF10"	},
		{	M_SOF11,	"SOF11"	},
		{	M_DAC,		"DAC"	},
		{	M_SOF13,	"SOF13"	},
		{	M_SOF14,	"SOF14"	},
		{	M_SOF15,	"SOF15"	},
		{	M_RST0,		"RST0"	},
		{	M_RST1,		"RST1"	},
		{	M_RST2,		"RST2"	},
		{	M_RST3,		"RST3"	},
		{	M_RST4,		"RST4"	},
		{	M_RST5,		"RST5"	},
		{	M_RST6,		"RST6"	},
		{	M_RST7,		"RST7"	},
		{	M_SOI,		"SOI"	},
		{	M_EOI,		"EOI"	},
		{	M_SOS,		"SOS"	},
		{	M_DQT,		"DQT"	},
		{	M_DNL,		"DNL"	},
		{	M_DRI,		"DRI"	},
		{	M_DHP,		"DHP"	},
		{	M_EXP,		"EXP"	},
		{	M_APP0,		"APP0"	},
		{	M_APP1,		"APP1"	},
		{	M_APP2,		"APP2"	},
		{	M_APP3,		"APP3"	},
		{	M_APP4,		"APP4"	},
		{	M_APP5,		"APP5"	},
		{	M_APP6,		"APP6"	},
		{	M_APP7,		"APP7"	},
		{	M_APP8,		"APP8"	},
		{	M_APP9,		"APP9"	},
		{	M_APP10,	"APP10"	},
		{	M_APP11,	"APP11"	},
		{	M_APP12,	"APP12"	},
		{	M_APP13,	"APP13"	},
		{	M_APP14,	"APP14"	},
		{	M_APP15,	"APP15"	},
		{	M_JPG0,		"JPG0"	},
		{	M_JPG1,		"JPG1"	},
		{	M_JPG2,		"JPG2"	},
		{	M_JPG3,		"JPG3"	},
		{	M_JPG4,		"JPG4"	},
		{	M_JPG5,		"JPG5"	},
		{	M_JPG6,		"JPG6"	},
		{	M_JPG7,		"JPG7"	},
		{	M_JPG8,		"JPG8"	},
		{	M_JPG9,		"JPG9"	},
		{	M_JPG10,	"JPG10"	},
		{	M_JPG11,	"JPG11"	},
		{	M_JPG12,	"JPG12"	},
		{	M_JPG13,	"JPG13"	},
		{	M_COM,		"COM"	},
		{	M_ERROR,	"ERROR"	}
		};

	const char *name = LookupName (key,
								   kJPEGMarkerNames,
								   sizeof (kJPEGMarkerNames    ) /
								   sizeof (kJPEGMarkerNames [0]));

	if (name)
		{
		return name;
		}

	static char s [32];

	sprintf (s, "0x%02X", (unsigned) key);

	return s;

	}

/*****************************************************************************/

void DumpHexAscii (dng_stream &stream,
				   uint32 count)
	{

	uint32 rows = (count + 15) >> 4;

	if (rows > gDumpLineLimit)
		rows = gDumpLineLimit;

	for (uint32 row = 0; row < rows; row++)
		{

		printf ("    ");

		uint32 col;

		uint32 cols = count - (row << 4);

		if (cols > 16)
			cols = 16;

		uint8 x [16];

		for (col = 0; col < 16; col++)
			{

			x [col] = ' ';

			if (col < cols)
				{

				x [col] = stream.Get_uint8 ();

				printf ("%02x ", x [col]);

				}

			else
				{
				printf ("   ");
				}

			}

		printf ("   ");

		for (col = 0; col < 16; col++)
			{

			if (x [col] >= (uint8) ' ' && x [col] <= (uint8) '~')
				{
				printf ("%c", x [col]);
				}

			else
				{
				printf (".");
				}

			}

		printf ("\n");

		}

	if (count > rows * 16)
		{
		printf ("    ... %u more bytes\n", (unsigned) (count - rows * 16));
		}

	}

/*****************************************************************************/

void DumpHexAscii (const uint8 *buf,
				   uint32 count)
	{

	uint32 rows = (count + 15) >> 4;

	if (rows > gDumpLineLimit)
		rows = gDumpLineLimit;

	for (uint32 row = 0; row < rows; row++)
		{

		printf ("    ");

		uint32 col;

		uint32 cols = count - (row << 4);

		if (cols > 16)
			cols = 16;

		uint8 x [16];

		for (col = 0; col < 16; col++)
			{

			x [col] = ' ';

			if (col < cols)
				{

				x [col] = *(buf++);

				printf ("%02x ", x [col]);

				}

			else
				{
				printf ("   ");
				}

			}

		printf ("   ");

		for (col = 0; col < 16; col++)
			{

			if (x [col] >= (uint8) ' ' && x [col] <= (uint8) '~')
				{
				printf ("%c", x [col]);
				}

			else
				{
				printf (".");
				}

			}

		printf ("\n");

		}

	if (count > rows * 16)
		{
		printf ("    ... %u more bytes\n", (unsigned) (count - rows * 16));
		}

	}

/*****************************************************************************/

void DumpXMP (dng_stream &stream,
			  uint32 count)
	{

	uint32 lineLength = 0;

	while (count > 0)
		{

		uint32 x = stream.Get_uint8 ();

		if (x == 0) break;

		count--;

		if (lineLength == 0)
			{

			printf ("XMP: ");

			lineLength = 5;

			}

		if (x == '\n' ||
			x == '\r')
			{

			printf ("\n");

			lineLength = 0;

			}

		else
			{

			if (lineLength >= 128)
				{

				printf ("\nXMP: ");

				lineLength = 5;

				}

			if (x >= ' ' && x <= '~')
				{

				printf ("%c", (char) x);

				lineLength += 1;

				}

			else
				{

				printf ("\\%03o", (unsigned) x);

				lineLength += 4;

				}

			}

		}

	if (lineLength != 0)
		{

		printf ("\n");

		}

	}

/*****************************************************************************/

void DumpString (const dng_string &s)
	{

	const uint32 kMaxDumpString = gDumpLineLimit * 64;

	printf ("\"");

	const char *ss = s.Get ();

	uint32 total = 0;

	while (*ss != 0 && total++ < kMaxDumpString)
		{

		uint32 c = dng_string::DecodeUTF8 (ss);

		if (c >= ' ' && c <= '~')
			{
			printf ("%c", (char) c);
			}

		else switch (c)
			{

			case '\t':
				{
				printf ("\\t");
				break;
				}

			case '\n':
				{
				printf ("\\n");
				break;
				}

			case '\r':
				{
				printf ("\\r");
				break;
				}

			default:
				{
				printf ("[%X]", (unsigned) c);
				break;
				}

			}

		}

	uint32 extra = (uint32) strlen (ss);

	if (extra > 0)
		{
		printf ("...\" (%u more bytes)", (unsigned) extra);
		}

	else
		{
		printf ("\"");
		}

	}

/*****************************************************************************/

void DumpTagValues (dng_stream &stream,
					const char *entry_name,
					uint32 parentCode,
					uint32 tagCode,
					uint32 tagType,
					uint32 tagCount,
					const char *tag_name)
	{

	const uint32 kMaxDumpSingleLine = 4;

	const uint32 kMaxDumpArray = Max_uint32 (gDumpLineLimit, kMaxDumpSingleLine);

	printf ("%s:", tag_name ? tag_name
							: LookupTagCode (parentCode, tagCode));

	switch (tagType)
		{

		case ttShort:
		case ttLong:
		case ttIFD:
		case ttSByte:
		case ttSShort:
		case ttSLong:
		case ttRational:
		case ttSRational:
		case ttFloat:
		case ttDouble:
			{

			if (tagCount > kMaxDumpSingleLine)
				{

				printf (" %u entries", (unsigned) tagCount);

				}

			for (uint32 j = 0; j < tagCount && j < kMaxDumpArray; j++)
				{

				if (tagCount <= kMaxDumpSingleLine)
					{

					if (j == 0)
						{

						printf (" %s =", entry_name);

						}

					printf (" ");

					}

				else
					{

					printf ("\n    %s [%u] = ", entry_name, (unsigned) j);

					}

				switch (tagType)
					{

					case ttByte:
					case ttShort:
					case ttLong:
					case ttIFD:
						{

						uint32 x = stream.TagValue_uint32 (tagType);

						printf ("%u", (unsigned) x);

						break;

						}

					case ttSByte:
					case ttSShort:
					case ttSLong:
						{

						int32 x = stream.TagValue_int32 (tagType);

						printf ("%d", (int) x);

						break;

						}

					case ttRational:
						{

						dng_urational x = stream.TagValue_urational (tagType);

						printf ("%u/%u", (unsigned) x.n, (unsigned) x.d);

						break;

						}

					case ttSRational:
						{

						dng_srational x = stream.TagValue_srational (tagType);

						printf ("%d/%d", (int) x.n, (int) x.d);

						break;

						}

					default:
						{

						real64 x = stream.TagValue_real64 (tagType);

						printf ("%f", x);

						}

					}

				}

			printf ("\n");

			if (tagCount > kMaxDumpArray)
				{

				printf ("    ... %u more entries\n", (unsigned) (tagCount - kMaxDumpArray));

				}

			break;

			}

		case ttAscii:
			{

			dng_string s;

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							s,
							false);

			printf (" ");

			DumpString (s);

			printf ("\n");

			break;

			}

		default:
			{

			uint32 tagSize = tagCount * TagTypeSize (tagType);

			if (tagCount == 1 && (tagType == ttByte ||
								  tagType == ttUndefined))
				{

				uint8 x = stream.Get_uint8 ();

				printf (" %s = %u\n", LookupTagType (tagType), x);

				}

			else
				{

				printf (" %s, size = %u\n", LookupTagType (tagType), (unsigned) tagSize);

				DumpHexAscii (stream, tagSize);

				}

			break;
			}

		}

	}

/*****************************************************************************/

void DumpMatrix (const dng_matrix &m)
	{

	for (uint32 row = 0; row < m.Rows (); row++)
		{

		for (uint32 col = 0; col < m.Cols (); col++)
			{

			if (col == 0)
				printf ("    ");
			else
				printf (" ");

			printf ("%8.4f", m [row] [col]);

			}

		printf ("\n");

		}

	}

/*****************************************************************************/

void DumpVector (const dng_vector &v)
	{

	for (uint32 index = 0; index < v.Count (); index++)
		{

		printf (" %0.4f", v [index]);

		}

	printf ("\n");

	}

/*****************************************************************************/

void DumpDateTime (const dng_date_time &dt)
	{

	printf ("%04d:%02d:%02d %02d:%02d:%02d",
			(int) dt.fYear,
			(int) dt.fMonth,
			(int) dt.fDay,
			(int) dt.fHour,
			(int) dt.fMinute,
			(int) dt.fSecond);

	}

/*****************************************************************************/

void DumpExposureTime (real64 x)
	{

	if (x > 0.0)
		{

		if (x >= 0.25)
			{
			printf ("%0.2f sec", x);
			}

		else if (x >= 0.01)
			{
			printf ("1/%0.1f sec", 1.0 / x);
			}

		else
			{
			printf ("1/%0.0f sec", 1.0 / x);
			}

		}

	else
		{

		printf ("<invalid>");

		}

	}

/*****************************************************************************/

void DumpFingerprint (const dng_fingerprint &p)
	{

	printf ("<");

	for (uint32 j = 0; j < 16; j++)
		{
		printf ("%02x", p.data [j]);
		}

	printf (">");

	}

/*****************************************************************************/

void DumpHueSatMap (dng_stream &stream,
				    uint32 hues,
					uint32 sats,
					uint32 vals,
					bool skipSat0)
	{

	uint32 doneLines = 0;
	uint32 skipLines = 0;

	for (uint32 v = 0; v < vals; v++)
		{

		for (uint32 h = 0; h < hues; h++)
			{

			for (uint32 s = skipSat0 ? 1 : 0; s < sats; s++)
				{

				real32 dh = stream.Get_real32 ();
				real32 ds = stream.Get_real32 ();
				real32 dv = stream.Get_real32 ();

				if (gDumpLineLimit == 0 ||
					gDumpLineLimit > doneLines)
					{

					doneLines++;

					if (vals == 1)
						{

						printf ("    h [%2u] s [%2u]:  h=%8.4f s=%6.4f v=%6.4f\n",
								(unsigned) h,
								(unsigned) s,
								(double) dh,
								(double) ds,
								(double) dv);

						}

					else
						{

						printf ("    v [%2u] h [%2u] s [%2u]:  h=%8.4f s=%6.4f v=%6.4f\n",
								(unsigned) v,
								(unsigned) h,
								(unsigned) s,
								(double) dh,
								(double) ds,
								(double) dv);

						}

					}

				else
					{

					skipLines++;

					}

				}

			}

		}

	if (skipLines > 0)
		{

		printf ("    ... %u more entries\n", (unsigned) skipLines);

		}

	}

/*****************************************************************************/

#endif

/*****************************************************************************/

bool CheckTagType (uint32 parentCode,
				   uint32 tagCode,
				   uint32 tagType,
				   uint16 validType0,
				   uint16 validType1,
				   uint16 validType2,
				   uint16 validType3)
	{

	if (tagType != validType0 &&
		tagType != validType1 &&
		tagType != validType2 &&
		tagType != validType3)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s has unexpected type (%s)",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode),
					 LookupTagType (tagType));

			ReportWarning (message);

			}

		#else

		parentCode;		// Unused
		tagCode;		// Unused

		#endif

		return false;

		}

	return true;

	}

/*****************************************************************************/

bool CheckTagCount (uint32 parentCode,
					uint32 tagCode,
				    uint32 tagCount,
				    uint32 minCount,
				    uint32 maxCount)
	{

	if (maxCount < minCount)
		maxCount = minCount;

	if (tagCount < minCount ||
		tagCount > maxCount)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s has unexpected count (%u)",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode),
					 (unsigned) tagCount);

			ReportWarning (message);

			}

		#else

		parentCode;		// Unused
		tagCode;		// Unused

		#endif

		return false;

		}

	return true;

	}

/*****************************************************************************/

bool CheckColorImage (uint32 parentCode,
					  uint32 tagCode,
				      uint32 colorPlanes)
	{

	if (colorPlanes == 0)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s is not allowed with unknown color plane count "
					 " (missing ColorMatrix1 tag?)",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		#else

		parentCode;		// Unused
		tagCode;		// Unused

		#endif

		return false;

		}

	if (colorPlanes == 1)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s is not allowed with monochrome images",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		#endif

		return false;

		}

	return true;

	}

/*****************************************************************************/

bool CheckMainIFD (uint32 parentCode,
				   uint32 tagCode,
				   uint32 newSubFileType)
	{

	if (newSubFileType != sfMainImage)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s is not allowed IFDs with NewSubFileType != 0",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		#else

		parentCode;			// Unused
		tagCode;			// Unused

		#endif

		return false;

		}

	return true;

	}

/*****************************************************************************/

bool CheckRawIFD (uint32 parentCode,
				  uint32 tagCode,
				  uint32 photometricInterpretation)
	{

	if (photometricInterpretation != piCFA &&
		photometricInterpretation != piLinearRaw)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s is not allowed in IFDs with a non-raw PhotometricInterpretation",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		#else

		parentCode;			// Unused
		tagCode;			// Unused

		#endif

		return false;

		}

	return true;

	}

/*****************************************************************************/

bool CheckCFA (uint32 parentCode,
			   uint32 tagCode,
		       uint32 photometricInterpretation)
	{

	if (photometricInterpretation != piCFA)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s is not allowed in IFDs with a non-CFA PhotometricInterpretation",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		#else

		parentCode;			// Unused
		tagCode;			// Unused

		#endif

		return false;

		}

	return true;

	}

/*****************************************************************************/

void ParseStringTag (dng_stream &stream,
					 uint32 parentCode,
					 uint32 tagCode,
				     uint32 tagCount,
				     dng_string &s,
				     bool trimBlanks,
				     bool isASCII)
	{

	if (tagCount == 0 ||
		tagCount == 0xFFFFFFFF)
		{

		s.Clear ();

		return;

		}

	dng_memory_data temp_buffer (tagCount + 1);

	char *buffer = temp_buffer.Buffer_char ();

	stream.Get (buffer, tagCount);

	// Make sure the string is null terminated.

	if (buffer [tagCount - 1] != 0)
		{

		buffer [tagCount] = 0;

		#if qDNGValidate

			{

			bool hasNull = false;

			for (uint32 j = 0; j < tagCount; j++)
				{

				if (buffer [j] == 0)
					{

					hasNull = true;

					break;

					}

				}

			if (!hasNull && parentCode < tcFirstMakerNoteIFD)
				{

				char message [256];

				sprintf (message,
						 "%s %s is not NULL terminated",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode));

				ReportWarning (message);

				}

			}

		#else

		parentCode;			// Unused
		tagCode;			// Unused

		#endif

		}

	if (isASCII)
		{
		s.Set_ASCII (buffer);
		}
	else
		{
		s.Set (buffer);
		}

	#if qDNGValidate

	if (parentCode < tcFirstMakerNoteIFD)
		{

		if (isASCII && !s.IsASCII ())
			{

			char message [256];

			sprintf (message,
					 "%s %s has non-ASCII characters",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		}

	#endif

	if (trimBlanks)
		{

		s.TrimTrailingBlanks ();

		}

	}

/*****************************************************************************/

void ParseDualStringTag (dng_stream &stream,
					 	 uint32 parentCode,
					 	 uint32 tagCode,
				     	 uint32 tagCount,
				     	 dng_string &s1,
				     	 dng_string &s2)
	{

	if (tagCount == 0 ||
		tagCount == 0xFFFFFFFF)
		{

		s1.Clear ();
		s2.Clear ();

		return;

		}

	dng_memory_data temp_buffer (tagCount + 1);

	char *buffer = temp_buffer.Buffer_char ();

	stream.Get (buffer, tagCount);

	// Make sure the string is null terminated.

	if (buffer [tagCount - 1] != 0)
		{

		buffer [tagCount] = 0;

		#if qDNGValidate

			{

			uint32 nullCount = 0;

			for (uint32 j = 0; j < tagCount; j++)
				{

				if (buffer [j] == 0)
					{

					nullCount++;

					}

				}

			if (nullCount < 2 && parentCode < tcFirstMakerNoteIFD)
				{

				char message [256];

				sprintf (message,
						 "%s %s is not NULL terminated",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode));

				ReportWarning (message);

				}

			}

		#else

		parentCode;			// Unused
		tagCode;			// Unused

		#endif

		}

	s1.Set_ASCII (buffer);
	s2.Set_ASCII (NULL  );

	for (uint32 j = 1; j < tagCount - 1; j++)
		{

		if (buffer [j - 1] != 0 &&
			buffer [j    ] == 0)
			{

			s2.Set_ASCII (buffer + j + 1);

			break;

			}

		}

	#if qDNGValidate

		{

		if (!s1.IsASCII () ||
			!s2.IsASCII ())
			{

			char message [256];

			sprintf (message,
					 "%s %s has non-ASCII characters",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		}

	#endif

	s1.TrimTrailingBlanks ();
	s2.TrimTrailingBlanks ();

	}

/*****************************************************************************/

void ParseEncodedStringTag (dng_stream &stream,
							uint32 parentCode,
							uint32 tagCode,
				    		uint32 tagCount,
				    		dng_string &s)
	{

	if (tagCount < 8)
		{

		#if qDNGValidate

			{

			char message [256];

			sprintf (message,
					 "%s %s has unexpected count (%u)",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode),
					 (unsigned) tagCount);

			ReportWarning (message);

			}

		#else

		parentCode;			// Unused
		tagCode;			// Unused

		#endif

		s.Clear ();

		return;

		}

	char label [8];

	stream.Get (label, 8);

	// Sometimes lowercase is used by mistake.  Accept this, but issue
	// warning.

		{

		bool hadLower = false;

		for (uint32 j = 0; j < 8; j++)
			{

			if (label [j] >= 'a' && label [j] <= 'z')
				{

				label [j] = 'A' + (label [j] - 'a');

				hadLower = true;

				}

			}

		#if qDNGValidate

		if (hadLower)
			{

			char message [256];

			sprintf (message,
					 "%s %s text encoding label not all uppercase",
					 LookupParentCode (parentCode),
					 LookupTagCode (parentCode, tagCode));

			ReportWarning (message);

			}

		#endif

		}

	if (memcmp (label, "UNICODE\000", 8) == 0)
		{

		uint32 uChars = (tagCount - 8) >> 1;

		dng_memory_data temp_buffer ((uChars + 1) * 2);

		uint16 *buffer = temp_buffer.Buffer_uint16 ();

		for (uint32 j = 0; j < uChars; j++)
			{

			buffer [j] = stream.Get_uint16 ();

			}

		buffer [uChars] = 0;

		#if qDNGValidate

			{

			// If the writer used UTF-8 rather than UTF-16, and padded
			// the string with blanks, then there will be lots of 0x2020
			// (unicode dagger symbol) characters in the string.

			uint32 count2020 = 0;

			for (uint32 k = 0; buffer [k] != 0; k++)
				{

				if (buffer [k] == 0x2020)
					{

					count2020++;

					}

				}

			if (count2020 > 1)
				{

				char message [256];

				sprintf (message,
						 "%s %s text appears to be UTF-8 rather than UTF-16",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode));

				ReportWarning (message);

				}

			}

		#endif

		s.Set_UTF16 (buffer);

		}

	else
		{

		uint32 aChars = tagCount - 8;

		dng_memory_data temp_buffer (aChars + 1);

		char *buffer = temp_buffer.Buffer_char ();

		stream.Get (buffer, aChars);

		buffer [aChars] = 0;

		enum dng_encoding
			{
			dng_encoding_ascii,
			dng_encoding_jis_x208_1990,
			dng_encoding_unknown
			};

		dng_encoding encoding = dng_encoding_unknown;

		if (memcmp (label, "ASCII\000\000\000", 8) == 0)
			{

			encoding = dng_encoding_ascii;

			}

		else if (memcmp (label, "JIS\000\000\000\000\000\000", 8) == 0)
			{

			encoding = dng_encoding_jis_x208_1990;

			}

		else
			{

			// Some Nikon D1 files have UserComment tags with zero encoding bits and
			// garbage text values.  So don't try to parse tags with unknown text
			// encoding unless all the characters are printing ASCII.

			#if qDNGValidate

			if (memcmp (label, "\000\000\000\000\000\000\000\000\000", 8) == 0)
				{

				// Many camera makes store null tags with all zero encoding, so
				// don't report a warning message for null strings.

				if (buffer [0] != 0)
					{

					char message [256];

					sprintf (message,
							 "%s %s has unknown encoding",
							 LookupParentCode (parentCode),
							 LookupTagCode (parentCode, tagCode));

					ReportWarning (message);

					}

				}

			else
				{

				char message [256];

				sprintf (message,
						 "%s %s has unexpected text encoding",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode));

				ReportWarning (message);

				}

			#endif

			}

		// If text encoding was unknown, and the text is anything
		// other than pure ASCII, then ignore it.

		if (encoding == dng_encoding_unknown)
			{

			encoding = dng_encoding_ascii;

			for (uint32 i = 0; i < aChars && buffer [i] != 0; i++)
				{

				if (buffer [i] < ' ' ||
					buffer [i] > '~')
					{

					buffer [0] = 0;

					break;

					}

				}

			}

		switch (encoding)
			{

			case dng_encoding_ascii:
				{
				s.Set_ASCII (buffer);
				break;
				}

			case dng_encoding_jis_x208_1990:
				{
				s.Set_JIS_X208_1990 (buffer);
				break;
				}

			case dng_encoding_unknown:
				{
				s.Set_SystemEncoding (buffer);
				break;
				}

			default:
				break;

			}

		#if qDNGValidate

			{

			if (encoding == dng_encoding_ascii && !s.IsASCII ())
				{

				char message [256];

				sprintf (message,
						 "%s %s has non-ASCII characters",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode));

				ReportWarning (message);

				}

			}

		#endif

		}

	s.TrimTrailingBlanks ();

	}

/*****************************************************************************/

bool ParseMatrixTag (dng_stream &stream,
					 uint32 parentCode,
					 uint32 tagCode,
					 uint32 tagType,
					 uint32 tagCount,
					 uint32 rows,
					 uint32 cols,
					 dng_matrix &m)
	{

	if (CheckTagCount (parentCode, tagCode, tagCount, rows * cols))
		{

		dng_matrix temp (rows, cols);

		for (uint32 row = 0; row < rows; row++)
			for (uint32 col = 0; col < cols; col++)
				{

				temp [row] [col] = stream.TagValue_real64 (tagType);

				}

		m = temp;

		return true;

		}

	return false;

	}

/*****************************************************************************/

bool ParseVectorTag (dng_stream &stream,
					 uint32 parentCode,
					 uint32 tagCode,
					 uint32 tagType,
					 uint32 tagCount,
					 uint32 count,
					 dng_vector &v)
	{

	if (CheckTagCount (parentCode, tagCode, tagCount, count))
		{

		dng_vector temp (count);

		for (uint32 index = 0; index < count; index++)
			{

			temp [index] = stream.TagValue_real64 (tagType);

			}

		v = temp;

		return true;

		}

	return false;

	}

/*****************************************************************************/

bool ParseDateTimeTag (dng_stream &stream,
					   uint32 parentCode,
					   uint32 tagCode,
					   uint32 tagType,
					   uint32 tagCount,
					   dng_date_time &dt)
	{

	if (!CheckTagType (parentCode, tagCode, tagType, ttAscii))
		{
		return false;
		}

	// Kludge: Some versions of PaintShop Pro write these fields
	// with a length of 21 rather than 20.  Otherwise they are
	// correctly formated.  So relax this test and allow these
	// these longer than standard tags to be parsed.

	(void) CheckTagCount (parentCode, tagCode, tagCount, 20);

	if (tagCount < 20)
		{
		return false;
		}

	char s [21];

	stream.Get (s, 20);

	s [20] = 0;

	// See if this is a valid date/time string.

	if (dt.Parse (s))
		{
		return true;
		}

	// Accept strings that contain only blanks, colons, and zeros as
	// valid "null" dates.

	dt = dng_date_time ();

	for (uint32 index = 0; index < 21; index++)
		{

		char c = s [index];

		if (c == 0)
			{
			return true;
			}

		if (c != ' ' && c != ':' && c != '0')
			{

			#if qDNGValidate

				{

				char message [256];

				sprintf (message,
						 "%s %s is not a valid date/time",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode));

				ReportWarning (message);

				}

			#endif

			return false;

			}

		}

	return false;

	}

/*****************************************************************************/
