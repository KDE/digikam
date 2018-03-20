/*****************************************************************************/
// Copyright 2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_preview.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_preview.h"

#include "dng_assertions.h"
#include "dng_image.h"
#include "dng_image_writer.h"
#include "dng_memory.h"
#include "dng_stream.h"
#include "dng_tag_codes.h"
#include "dng_tag_values.h"

/*****************************************************************************/

class dng_preview_tag_set: public dng_basic_tag_set
	{

	private:

		tag_string fApplicationNameTag;

		tag_string fApplicationVersionTag;

		tag_string fSettingsNameTag;

		dng_fingerprint fSettingsDigest;

		tag_uint8_ptr fSettingsDigestTag;

		tag_uint32 fColorSpaceTag;

		tag_string fDateTimeTag;

	public:

		dng_preview_tag_set (dng_tiff_directory &directory,
							 const dng_preview &preview,
							 const dng_ifd &ifd);

		virtual ~dng_preview_tag_set ();

	};

/*****************************************************************************/

dng_preview_tag_set::dng_preview_tag_set (dng_tiff_directory &directory,
										  const dng_preview &preview,
										  const dng_ifd &ifd)

	:	dng_basic_tag_set (directory, ifd)

	,	fApplicationNameTag (tcPreviewApplicationName,
							 preview.fInfo.fApplicationName,
							 false)

	,	fApplicationVersionTag (tcPreviewApplicationVersion,
							    preview.fInfo.fApplicationVersion,
								false)

	,	fSettingsNameTag (tcPreviewSettingsName,
						  preview.fInfo.fSettingsName,
						  false)

	,	fSettingsDigest (preview.fInfo.fSettingsDigest)

	,	fSettingsDigestTag (tcPreviewSettingsDigest,
							fSettingsDigest.data,
							16)

	,	fColorSpaceTag (tcPreviewColorSpace,
						preview.fInfo.fColorSpace)

	,	fDateTimeTag (tcPreviewDateTime,
					  preview.fInfo.fDateTime,
					  true)

	{

	if (preview.fInfo.fApplicationName.NotEmpty ())
		{

		directory.Add (&fApplicationNameTag);

		}

	if (preview.fInfo.fApplicationVersion.NotEmpty ())
		{

		directory.Add (&fApplicationVersionTag);

		}

	if (preview.fInfo.fSettingsName.NotEmpty ())
		{

		directory.Add (&fSettingsNameTag);

		}

	if (preview.fInfo.fSettingsDigest.IsValid ())
		{

		directory.Add (&fSettingsDigestTag);

		}

	if (preview.fInfo.fColorSpace != previewColorSpace_MaxEnum)
		{

		directory.Add (&fColorSpaceTag);

		}

	if (preview.fInfo.fDateTime.NotEmpty ())
		{

		directory.Add (&fDateTimeTag);

		}

	}

/*****************************************************************************/

dng_preview_tag_set::~dng_preview_tag_set ()
	{

	}

/*****************************************************************************/

dng_preview::dng_preview ()

	:	fInfo ()

	{

	}

/*****************************************************************************/

dng_preview::~dng_preview ()
	{

	}

/*****************************************************************************/

dng_image_preview::dng_image_preview ()

	:	fImage ()
	,	fIFD   ()

	{

	}

/*****************************************************************************/

dng_image_preview::~dng_image_preview ()
	{

	}

/*****************************************************************************/

dng_basic_tag_set * dng_image_preview::AddTagSet (dng_tiff_directory &directory) const
	{

	fIFD.fNewSubFileType = fInfo.fIsPrimary ? sfPreviewImage
											: sfAltPreviewImage;

	fIFD.fImageWidth  = fImage->Width  ();
	fIFD.fImageLength = fImage->Height ();

	fIFD.fSamplesPerPixel = fImage->Planes ();

	fIFD.fPhotometricInterpretation = fIFD.fSamplesPerPixel == 1 ? piBlackIsZero
																 : piRGB;

	fIFD.fBitsPerSample [0] = TagTypeSize (fImage->PixelType ()) * 8;

	for (uint32 j = 1; j < fIFD.fSamplesPerPixel; j++)
		{
		fIFD.fBitsPerSample [j] = fIFD.fBitsPerSample [0];
		}

	fIFD.SetSingleStrip ();

	return new dng_preview_tag_set (directory, *this, fIFD);

	}

/*****************************************************************************/

void dng_image_preview::WriteData (dng_host &host,
								   dng_image_writer &writer,
								   dng_basic_tag_set &basic,
								   dng_stream &stream) const
	{

	writer.WriteImage (host,
					   fIFD,
					   basic,
					   stream,
				       *fImage.Get ());

	}

/*****************************************************************************/

class dng_jpeg_preview_tag_set: public dng_preview_tag_set
	{

	private:

		dng_urational fCoefficientsData [3];

		tag_urational_ptr fCoefficientsTag;

		uint16 fSubSamplingData [2];

		tag_uint16_ptr fSubSamplingTag;

		tag_uint16 fPositioningTag;

		dng_urational fReferenceData [6];

		tag_urational_ptr fReferenceTag;

	public:

		dng_jpeg_preview_tag_set (dng_tiff_directory &directory,
								  const dng_jpeg_preview &preview,
								  const dng_ifd &ifd);

		virtual ~dng_jpeg_preview_tag_set ();

	};

/******************************************************************************/

dng_jpeg_preview_tag_set::dng_jpeg_preview_tag_set (dng_tiff_directory &directory,
													const dng_jpeg_preview &preview,
													const dng_ifd &ifd)

	:	dng_preview_tag_set (directory, preview, ifd)

	,	fCoefficientsTag (tcYCbCrCoefficients, fCoefficientsData, 3)

	,	fSubSamplingTag (tcYCbCrSubSampling, fSubSamplingData, 2)

	,	fPositioningTag (tcYCbCrPositioning, preview.fYCbCrPositioning)

	,	fReferenceTag (tcReferenceBlackWhite, fReferenceData, 6)

	{

	if (preview.fPhotometricInterpretation == piYCbCr)
		{

		fCoefficientsData [0] = dng_urational (299, 1000);
		fCoefficientsData [1] = dng_urational (587, 1000);
		fCoefficientsData [2] = dng_urational (114, 1000);

		directory.Add (&fCoefficientsTag);

		fSubSamplingData [0] = (uint16) preview.fYCbCrSubSampling.h;
		fSubSamplingData [1] = (uint16) preview.fYCbCrSubSampling.v;

		directory.Add (&fSubSamplingTag);

		directory.Add (&fPositioningTag);

		fReferenceData [0] = dng_urational (  0, 1);
		fReferenceData [1] = dng_urational (255, 1);
		fReferenceData [2] = dng_urational (128, 1);
		fReferenceData [3] = dng_urational (255, 1);
		fReferenceData [4] = dng_urational (128, 1);
		fReferenceData [5] = dng_urational (255, 1);

		directory.Add (&fReferenceTag);

		}

	}

/*****************************************************************************/

dng_jpeg_preview_tag_set::~dng_jpeg_preview_tag_set ()
	{

	}

/*****************************************************************************/

dng_jpeg_preview::dng_jpeg_preview ()

	:	fPreviewSize 			   ()
	,	fPhotometricInterpretation (piYCbCr)
	,	fYCbCrSubSampling 		   (1, 1)
	,	fYCbCrPositioning		   (2)
	,	fCompressedData			   ()

	{

	}

/*****************************************************************************/

dng_jpeg_preview::~dng_jpeg_preview ()
	{

	}

/*****************************************************************************/

dng_basic_tag_set * dng_jpeg_preview::AddTagSet (dng_tiff_directory &directory) const
	{

	dng_ifd ifd;

	ifd.fNewSubFileType = fInfo.fIsPrimary ? sfPreviewImage
										   : sfAltPreviewImage;

	ifd.fImageWidth  = fPreviewSize.h;
	ifd.fImageLength = fPreviewSize.v;

	ifd.fPhotometricInterpretation = fPhotometricInterpretation;

	ifd.fBitsPerSample [0] = 8;
	ifd.fBitsPerSample [1] = 8;
	ifd.fBitsPerSample [2] = 8;

	ifd.fSamplesPerPixel = (fPhotometricInterpretation == piBlackIsZero ? 1 : 3);

	ifd.fCompression = ccJPEG;
	ifd.fPredictor   = cpNullPredictor;

	ifd.SetSingleStrip ();

	return new dng_jpeg_preview_tag_set (directory, *this, ifd);

	}

/*****************************************************************************/

void dng_jpeg_preview::WriteData (dng_host & /* host */,
								  dng_image_writer & /* writer */,
								  dng_basic_tag_set &basic,
								  dng_stream &stream) const
	{

	basic.SetTileOffset (0, (uint32) stream.Position ());

	basic.SetTileByteCount (0, fCompressedData->LogicalSize ());

	stream.Put (fCompressedData->Buffer      (),
				fCompressedData->LogicalSize ());

	if (fCompressedData->LogicalSize () & 1)
		{
		stream.Put_uint8 (0);
		}

	}

/*****************************************************************************/

void dng_jpeg_preview::SpoolAdobeThumbnail (dng_stream &stream) const
	{

	DNG_ASSERT (fCompressedData.Get (),
				"SpoolAdobeThumbnail: no data");

	DNG_ASSERT (fPhotometricInterpretation == piYCbCr,
				"SpoolAdobeThumbnail: Non-YCbCr");

	uint32 compressedSize = fCompressedData->LogicalSize ();

	stream.Put_uint32 (DNG_CHAR4 ('8','B','I','M'));
	stream.Put_uint16 (1036);
	stream.Put_uint16 (0);

	stream.Put_uint32 (compressedSize + 28);

	uint32 widthBytes = (fPreviewSize.h * 24 + 31) / 32 * 4;

	stream.Put_uint32 (1);
	stream.Put_uint32 (fPreviewSize.h);
	stream.Put_uint32 (fPreviewSize.v);
	stream.Put_uint32 (widthBytes);
	stream.Put_uint32 (widthBytes * fPreviewSize.v);
	stream.Put_uint32 (compressedSize);
	stream.Put_uint16 (24);
	stream.Put_uint16 (1);

	stream.Put (fCompressedData->Buffer (),
			    compressedSize);

	if (compressedSize & 1)
		{
		stream.Put_uint8 (0);
		}

	}

/*****************************************************************************/

dng_preview_list::dng_preview_list ()

	:	fCount (0)

	{

	}

/*****************************************************************************/

dng_preview_list::~dng_preview_list ()
	{

	}

/*****************************************************************************/

void dng_preview_list::Append (AutoPtr<dng_preview> &preview)
	{

	if (preview.Get ())
		{

		DNG_ASSERT (fCount < kMaxDNGPreviews, "DNG preview list overflow");

		if (fCount < kMaxDNGPreviews)
			{

			fPreview [fCount++] . Reset (preview.Release ());

			}

		}

	}

/*****************************************************************************/
