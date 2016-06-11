/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_parse_utils.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_parse_utils__
#define __dng_parse_utils__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_flags.h"
#include "dng_types.h"
#include "dng_stream.h"
#include "dng_string.h"
#include "dng_matrix.h"

/*****************************************************************************/

#if qDNGValidate

/*****************************************************************************/

const char * LookupParentCode (uint32 parentCode);

/*****************************************************************************/

const char * LookupTagCode (uint32 parentCode,
							uint32 tagCode);

/*****************************************************************************/

const char * LookupTagType (uint32 tagType);

/*****************************************************************************/

const char * LookupNewSubFileType (uint32 key);

const char * LookupCompression (uint32 key);

const char * LookupPhotometricInterpretation (uint32 key);

const char * LookupOrientation (uint32 key);

const char * LookupResolutionUnit (uint32 key);

const char * LookupCFAColor (uint32 key);

const char * LookupSensingMethod (uint32 key);

const char * LookupExposureProgram (uint32 key);

const char * LookupMeteringMode (uint32 key);

const char * LookupLightSource (uint32 key);

const char * LookupColorSpace (uint32 key);

const char * LookupFileSource (uint32 key);

const char * LookupSceneType (uint32 key);

const char * LookupCustomRendered (uint32 key);

const char * LookupExposureMode (uint32 key);

const char * LookupWhiteBalance (uint32 key);

const char * LookupSceneCaptureType (uint32 key);

const char * LookupGainControl (uint32 key);

const char * LookupContrast (uint32 key);

const char * LookupSaturation (uint32 key);

const char * LookupSharpness (uint32 key);

const char * LookupSubjectDistanceRange (uint32 key);

const char * LookupComponent (uint32 key);

const char * LookupCFALayout (uint32 key);

const char * LookupMakerNoteSafety (uint32 key);

const char * LookupColorimetricReference (uint32 key);

const char * LookupPreviewColorSpace (uint32 key);

const char * LookupJPEGMarker (uint32 key);

/*****************************************************************************/

void DumpHexAscii (dng_stream &stream,
				   uint32 count);

void DumpHexAscii (const uint8 *buf,
				   uint32 count);

void DumpXMP (dng_stream &stream,
			  uint32 count);

void DumpString (const dng_string &s);

void DumpTagValues (dng_stream &stream,
					const char *entry_name,
					uint32 parentCode,
					uint32 tagCode,
					uint32 tagType,
					uint32 tagCount,
					const char *tag_name = NULL);

void DumpMatrix (const dng_matrix &m);

void DumpVector (const dng_vector &v);

void DumpDateTime (const dng_date_time &dt);

void DumpExposureTime (real64 x);

void DumpFingerprint (const dng_fingerprint &p);

void DumpHueSatMap (dng_stream &stream,
				    uint32 hues,
					uint32 sats,
					uint32 vals,
					bool skipSat0);

/*****************************************************************************/

#endif

/*****************************************************************************/

bool CheckTagType (uint32 parentCode,
				   uint32 tagCode,
				   uint32 tagType,
				   uint16 validType0,
				   uint16 validType1 = 0,
				   uint16 validType2 = 0,
				   uint16 validType3 = 0);

bool CheckTagCount (uint32 parentCode,
					uint32 tagCode,
				    uint32 tagCount,
				    uint32 minCount,
				    uint32 maxCount = 0);

bool CheckColorImage (uint32 parentCode,
					  uint32 tagCode,
				      uint32 colorPlanes);

bool CheckMainIFD (uint32 parentCode,
				   uint32 tagCode,
				   uint32 newSubFileType);

bool CheckRawIFD (uint32 parentCode,
				  uint32 tagCode,
				  uint32 photometricInterpretation);

bool CheckCFA (uint32 parentCode,
			   uint32 tagCode,
		       uint32 photometricInterpretation);

/*****************************************************************************/

void ParseStringTag (dng_stream &stream,
					 uint32 parentCode,
					 uint32 tagCode,
				     uint32 tagCount,
				     dng_string &s,
				     bool trimBlanks = true,
				     bool isASCII = true);

void ParseDualStringTag (dng_stream &stream,
					 	 uint32 parentCode,
					 	 uint32 tagCode,
				     	 uint32 tagCount,
				     	 dng_string &s1,
				     	 dng_string &s2);

void ParseEncodedStringTag (dng_stream &stream,
							uint32 parentCode,
							uint32 tagCode,
				    		uint32 tagCount,
				    		dng_string &s);

bool ParseMatrixTag (dng_stream &stream,
					 uint32 parentCode,
					 uint32 tagCode,
					 uint32 tagType,
					 uint32 tagCount,
					 uint32 rows,
					 uint32 cols,
					 dng_matrix &m);

bool ParseVectorTag (dng_stream &stream,
					 uint32 parentCode,
					 uint32 tagCode,
					 uint32 tagType,
					 uint32 tagCount,
					 uint32 count,
					 dng_vector &v);

bool ParseDateTimeTag (dng_stream &stream,
					   uint32 parentCode,
					   uint32 tagCode,
					   uint32 tagType,
					   uint32 tagCount,
					   dng_date_time &dt);

/*****************************************************************************/

#endif

/*****************************************************************************/
