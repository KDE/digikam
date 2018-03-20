/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_negative.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 *
 */

/*****************************************************************************/

#ifndef __dng_negative__
#define __dng_negative__

/*****************************************************************************/

#include "dng_1d_function.h"
#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_fingerprint.h"
#include "dng_linearization_info.h"
#include "dng_matrix.h"
#include "dng_mosaic_info.h"
#include "dng_opcode_list.h"
#include "dng_orientation.h"
#include "dng_rational.h"
#include "dng_sdk_limits.h"
#include "dng_string.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_types.h"
#include "dng_utils.h"
#include "dng_xy_coord.h"

#include <vector>

/*****************************************************************************/

/// \brief Noise model for photon and sensor read noise, assuming that they are
/// independent random variables and spatially invariant.
///
/// The noise model is N (x) = sqrt (scale*x + offset), where x represents a linear
/// signal value in the range [0,1], and N (x) is the standard deviation (i.e.,
/// noise). The parameters scale and offset are both sensor-dependent and
/// ISO-dependent. scale must be positive, and offset must be non-negative.

class dng_noise_function: public dng_1d_function
	{

	protected:

		real64 fScale;
		real64 fOffset;

	public:

		dng_noise_function ()

			:	fScale	(0.0)
			,	fOffset (0.0)

			{

			}

		dng_noise_function (real64 scale,
							real64 offset)

			:	fScale	(scale)
			,	fOffset (offset)

			{

			}

		virtual real64 Evaluate (real64 x) const
			{
			return sqrt (fScale * x + fOffset);
			}

		real64 Scale () const
			{
			return fScale;
			}

		real64 Offset () const
			{
			return fOffset;
			}

		void SetScale (real64 scale)
			{
			fScale = scale;
			}

		void SetOffset (real64 offset)
			{
			fOffset = offset;
			}

		bool IsValid () const
			{
			return (fScale > 0.0 && fOffset >= 0.0);
			}

	};

/*****************************************************************************/

/// \brief Noise profile for a negative.
///
/// For mosaiced negatives, the noise profile describes the approximate noise
/// characteristics of a mosaic negative after linearization, but prior to
/// demosaicing. For demosaiced negatives (i.e., linear DNGs), the noise profile
/// describes the approximate noise characteristics of the image data immediately
/// following the demosaic step, prior to the processing of opcode list 3.
///
/// A noise profile may contain 1 or N noise functions, where N is the number of
/// color planes for the negative. Otherwise the noise profile is considered to be
/// invalid for that negative. If the noise profile contains 1 noise function, then
/// it is assumed that this single noise function applies to all color planes of the
/// negative. Otherwise, the N noise functions map to the N planes of the negative in
/// order specified in the CFAPlaneColor tag.

class dng_noise_profile
	{

	protected:

		std::vector<dng_noise_function> fNoiseFunctions;

	public:

		dng_noise_profile ();

		explicit dng_noise_profile (const std::vector<dng_noise_function> &functions);

		bool IsValid () const;

		bool IsValidForNegative (const dng_negative &negative) const;

		const dng_noise_function & NoiseFunction (uint32 plane) const;

		uint32 NumFunctions () const;

	};

/*****************************************************************************/

/// \brief Main class for holding DNG image data and associated metadata.

class dng_negative
	{

	public:

		enum RawImageStageEnum
			{
			rawImageStagePreOpcode1,
			rawImageStagePostOpcode1,
			rawImageStagePostOpcode2,
			rawImageStagePreOpcode3,
			rawImageStagePostOpcode3,
			rawImageStageNone
			};

	protected:

		// The object stores an associated allocator. It does not do
		// anything to keep it alive or to release it when the object destructs.
		// Hence, clients will need to make sure that the allocator's lifespan
		// encompasses that of the dng_negative object.

		dng_memory_allocator &fAllocator;

		// Non-localized ASCII model name.

		dng_string fModelName;

		// Localized UTF-8 model name.

		dng_string fLocalName;

		// Base orientation of both the thumbnail and raw data. This is
		// generally based on the EXIF values.

		bool fHasBaseOrientation;

		dng_orientation fBaseOrientation;

		// The area of raw image that should be included in the final converted
		// image. This stems from extra pixels around the edges of the sensor
		// including both the black mask and some additional padding.

		// The default crop can be smaller than the "active" area which includes
		// the padding but not the black masked pixels.

		dng_urational fDefaultCropSizeH;
		dng_urational fDefaultCropSizeV;

		dng_urational fDefaultCropOriginH;
		dng_urational fDefaultCropOriginV;

		// Default scale factors. Generally, 1.0 for square pixel cameras. They
		// can compensate for non-square pixels. The choice of exact values will
		// generally depend on what the camera does. These are particularly
		// interesting for the Nikon D1X and the Fuji diamond mosaic.

		dng_urational fDefaultScaleH;
		dng_urational fDefaultScaleV;

		// Best quality scale factor. Used for the Nikon D1X and Fuji cameras
		// to force everything to be a scale up rather than scale down. So,
		// generally this is 1.0 / min (fDefaultScaleH, fDefaultScaleV) but
		// this isn't used if the scale factors are only slightly different
		// from 1.0.

		dng_urational fBestQualityScale;

		// Scale factors used in demosaic algorithm (calculated).
		// Maps raw image coordinates to full image coordinates -- i.e.,
		// original image coordinates on raw sensor data to coordinates
		// in fStage3Image which is the output of the interpolation step.
		// So, if we downsample when interpolating, these numbers get
		// smaller.

		real64 fRawToFullScaleH;
		real64 fRawToFullScaleV;

		// Relative amount of noise at ISO 100. This is measured per camera model
		// based on looking at flat areas of color.

		dng_urational fBaselineNoise;

		// How much noise reduction has already been applied (0.0 to 1.0) to the
		// the raw image data?  0.0 = none, 1.0 = "ideal" amount--i.e. don't apply any
		// more by default.  0/0 for unknown.

		dng_urational fNoiseReductionApplied;

		// Amount of noise for this negative (see dng_noise_profile for details).

		dng_noise_profile fNoiseProfile;

		// Zero point for the exposure compensation slider. This reflects how
		// the manufacturer sets up the camera and its conversions.

		dng_srational fBaselineExposure;

		// Relative amount of sharpening required. This is chosen per camera
		// model based on how strong the anti-alias filter is on the camera
		// and the quality of the lenses. This scales the sharpness slider
		// value.

		dng_urational fBaselineSharpness;

		// Chroma blur radius (or 0/0 for auto). Set to 0/1 to disable
		// chroma blurring.

		dng_urational fChromaBlurRadius;

		// Anti-alias filter strength (0.0 to 1.0).  Used as a hint
		// to the demosaic algorithms.

		dng_urational fAntiAliasStrength;

		// Linear response limit. The point at which the sensor goes
		// non-linear and color information becomes unreliable. Used in
		// the highlight-recovery logic.

		dng_urational fLinearResponseLimit;

		// Scale factor for shadows slider. The Fuji HDR cameras, for example,
		// need a more sensitive shadow slider.

		dng_urational fShadowScale;

		// Colormetric reference.

		uint32 fColorimetricReference;

		// Number of color channels for this image (e.g. 1, 3, or 4).

		uint32 fColorChannels;

		// Amount by which each channel has already been scaled. Some cameras
		// have analog amplifiers on the color channels and these can result
		// in different scalings per channel. This provides some level of
		// analog white balancing. The Nikon D1 also did digital scaling but
		// this caused problems with highlight recovery.

		dng_vector fAnalogBalance;

		// The "As Shot" neutral color coordinates in native camera space.
		// This overrides fCameraWhiteXY if both are specified. This
		// specifies the values per channel that would result in a neutral
		// color for the "As Shot" case. This is generally supplied by
		// the camera.

		dng_vector fCameraNeutral;

		// The "As Shot" white balance xy coordinates. Sometimes this is
		// supplied by the camera. Sometimes the camera just supplies a name
		// for the white balance.

		dng_xy_coord fCameraWhiteXY;

		// Individual camera calibrations.

		// Camera data --> camera calibration --> "inverse" of color matrix

		// This will be a 4x4 matrix for a 4-color camera. The defaults are
		// almost always the identity matrix and for the cases where they
		// aren't, they are diagonal matrices.

		dng_matrix fCameraCalibration1;
		dng_matrix fCameraCalibration2;

		// Signature which allows a profile to announce that it is compatible
		// with these calibration matrices.

		dng_string fCameraCalibrationSignature;

		// List of camera profiles.

		std::vector<dng_camera_profile *> fCameraProfile;

		// "As shot" camera profile name.

		dng_string fAsShotProfileName;

		// Raw image data digest. This is a MD5 fingerprint of the raw image data
		// in the file, computed using a specific algorithm.  It can be used
		// verify the raw data has not been corrupted.

		mutable dng_fingerprint fRawImageDigest;

		// Raw data unique ID.  This is an unique identifer for the actual
		// raw image data in the file.  It can be used to index into caches
		// for this data.

		mutable dng_fingerprint fRawDataUniqueID;

		// Original raw file name.  Just the file name, not the full path.

		dng_string fOriginalRawFileName;

		// Is the original raw file data availaible?

		bool fHasOriginalRawFileData;

		// The compressed original raw file data.

		AutoPtr<dng_memory_block> fOriginalRawFileData;

		// MD5 digest of original raw file data block.

		mutable dng_fingerprint fOriginalRawFileDigest;

		// DNG private data block.

		AutoPtr<dng_memory_block> fDNGPrivateData;

		// Is the maker note safe to copy from file to file? Defaults to false
		// because many maker notes are not safe.

		bool fIsMakerNoteSafe;

		// MakerNote binary data block.

		AutoPtr<dng_memory_block> fMakerNote;

		// EXIF data.

		AutoPtr<dng_exif> fExif;

		// A copy of the EXIF data before is was synchronized with other metadata sources.

		AutoPtr<dng_exif> fOriginalExif;

		// IPTC binary data block and offset in original file.

		AutoPtr<dng_memory_block> fIPTCBlock;

		uint64 fIPTCOffset;

		// Did the legacy ITPC block use UTF8?

		bool fUsedUTF8forIPTC;

		// XMP data.

		AutoPtr<dng_xmp> fXMP;

		// Was there a valid embedded XMP block?

		bool fValidEmbeddedXMP;

		// Is the XMP data from a sidecar file?

		bool fXMPinSidecar;

		// If the XMP data is from a sidecar file, is the sidecar file newer
		// than the raw file?

		bool fXMPisNewer;

		// Information required to linearize and range map the raw data.

		AutoPtr<dng_linearization_info> fLinearizationInfo;

		// Information required to demoasic the raw data.

		AutoPtr<dng_mosaic_info> fMosaicInfo;

		// Opcode list 1. (Applied to stored data)

		dng_opcode_list fOpcodeList1;

		// Opcode list 2. (Applied to range mapped data)

		dng_opcode_list fOpcodeList2;

		// Opcode list 3. (Post demosaic)

		dng_opcode_list fOpcodeList3;

		// Stage 1 image, which is image data stored in a DNG file.

		AutoPtr<dng_image> fStage1Image;

		// Stage 2 image, which is the stage 1 image after it has been
		// linearized and range mapped.

		AutoPtr<dng_image> fStage2Image;

		// Stage 3 image, which is the stage 2 image after it has been
		// demosaiced.

		AutoPtr<dng_image> fStage3Image;

		// Additiona gain applied when building the stage 3 image.

		real64 fStage3Gain;

		// Were any approximations (e.g. downsampling, etc.) applied
		// file reading this image?

		bool fIsPreview;

		// Does the file appear to be damaged?

		bool fIsDamaged;

		// At what processing stage did we grab a copy of raw image data?

		RawImageStageEnum fRawImageStage;

		// The raw image data that we grabbed, if any.

		AutoPtr<dng_image> fRawImage;

	public:

		virtual ~dng_negative ();

		static dng_negative * Make (dng_memory_allocator &allocator);

		/// Provide access to the memory allocator used for this object.

		dng_memory_allocator & Allocator () const
			{
			return fAllocator;
			}

		/// Getter for ModelName.

		void SetModelName (const char *name)
			{
			fModelName.Set_ASCII (name);
			}

		/// Setter for ModelName.

		const dng_string & ModelName () const
			{
			return fModelName;
			}

		/// Setter for LocalName.

		void SetLocalName (const char *name)
			{
			fLocalName.Set (name);
			}

		/// Getter for LocalName.

		const dng_string & LocalName () const
			{
			return fLocalName;
			}

		/// Setter for BaseOrientation.

		void SetBaseOrientation (const dng_orientation &orientation);

		/// Has BaseOrientation been set?

		bool HasBaseOrientation () const
			{
			return fHasBaseOrientation;
			}

		/// Getter for BaseOrientation.

		const dng_orientation & BaseOrientation () const
			{
			return fBaseOrientation;
			}

		/// Hook to allow SDK host code to add additional rotations.

		virtual dng_orientation Orientation () const;

		/// Logically rotates the image by changing the orientation values.
		/// This will also update the XMP data.

		void ApplyOrientation (const dng_orientation &orientation);

		/// Setter for DefaultCropSize.

		void SetDefaultCropSize (const dng_urational &sizeH,
						         const dng_urational &sizeV)
			{
			fDefaultCropSizeH = sizeH;
			fDefaultCropSizeV = sizeV;
			}

		/// Setter for DefaultCropSize.

		void SetDefaultCropSize (uint32 sizeH,
						         uint32 sizeV)
			{
			SetDefaultCropSize (dng_urational (sizeH, 1),
						        dng_urational (sizeV, 1));
			}

		/// Getter for DefaultCropSize horizontal.

		const dng_urational & DefaultCropSizeH () const
			{
			return fDefaultCropSizeH;
			}

		/// Getter for DefaultCropSize vertical.

		const dng_urational & DefaultCropSizeV () const
			{
			return fDefaultCropSizeV;
			}

		/// Setter for DefaultCropOrigin.

		void SetDefaultCropOrigin (const dng_urational &originH,
							       const dng_urational &originV)
			{
			fDefaultCropOriginH = originH;
			fDefaultCropOriginV = originV;
			}

		/// Setter for DefaultCropOrigin.

		void SetDefaultCropOrigin (uint32 originH,
							       uint32 originV)
			{
			SetDefaultCropOrigin (dng_urational (originH, 1),
						   		  dng_urational (originV, 1));
			}

		/// Set default crop around center of image.

		void SetDefaultCropCentered (const dng_point &rawSize)
			{

			uint32 sizeH = Round_uint32 (fDefaultCropSizeH.As_real64 ());
			uint32 sizeV = Round_uint32 (fDefaultCropSizeV.As_real64 ());

			SetDefaultCropOrigin ((rawSize.h - sizeH) >> 1,
								  (rawSize.v - sizeV) >> 1);

			}

		/// Get default crop origin horizontal value.

		const dng_urational & DefaultCropOriginH () const
			{
			return fDefaultCropOriginH;
			}

		/// Get default crop origin vertical value.

		const dng_urational & DefaultCropOriginV () const
			{
			return fDefaultCropOriginV;
			}

		/// Setter for DefaultScale.

		void SetDefaultScale (const dng_urational &scaleH,
							  const dng_urational &scaleV)
			{
			fDefaultScaleH = scaleH;
			fDefaultScaleV = scaleV;
			}

		/// Get default scale horizontal value.

		const dng_urational & DefaultScaleH () const
			{
			return fDefaultScaleH;
			}

		/// Get default scale vertical value.

		const dng_urational & DefaultScaleV () const
			{
			return fDefaultScaleV;
			}

		/// Setter for BestQualityScale.

		void SetBestQualityScale (const dng_urational &scale)
			{
			fBestQualityScale = scale;
			}

		/// Getter for BestQualityScale.

		const dng_urational & BestQualityScale () const
			{
			return fBestQualityScale;
			}

		/// API for raw to full image scaling factors horizontal.

		real64 RawToFullScaleH () const
			{
			return fRawToFullScaleH;
			}

		/// API for raw to full image scaling factors vertical.

		real64 RawToFullScaleV () const
			{
			return fRawToFullScaleV;
			}

		/// Get default scale factor.
		/// When specifing a single scale factor, we use the horizontal
		/// scale factor,  and let the vertical scale factor be calculated
		/// based on the pixel aspect ratio.

		real64 DefaultScale () const
			{
			return DefaultScaleH ().As_real64 ();
			}

		/// Default cropped image size (at scale == 1.0) width.

		real64 SquareWidth () const
			{
			return DefaultCropSizeH ().As_real64 ();
			}

		/// Default cropped image size (at scale == 1.0) height.

		real64 SquareHeight () const
			{
			return DefaultCropSizeV ().As_real64 () *
				   DefaultScaleV    ().As_real64 () /
				   DefaultScaleH    ().As_real64 ();
			}

		/// Default cropped image aspect ratio.

		real64 AspectRatio () const
			{
			return SquareWidth  () /
				   SquareHeight ();
			}

		/// Pixel aspect ratio of stage 3 image.

		real64 PixelAspectRatio () const
			{
			return (DefaultScaleH ().As_real64 () / RawToFullScaleH ()) /
				   (DefaultScaleV ().As_real64 () / RawToFullScaleV ());
			}

		/// Default cropped image size at given scale factor width.

		uint32 FinalWidth (real64 scale) const
			{
			return Round_uint32 (SquareWidth () * scale);
			}

		/// Default cropped image size at given scale factor height.

		uint32 FinalHeight (real64 scale) const
			{
			return Round_uint32 (SquareHeight () * scale);
			}

		/// Default cropped image size at default scale factor width.

		uint32 DefaultFinalWidth () const
			{
			return FinalWidth (DefaultScale ());
			}

		/// Default cropped image size at default scale factor height.

		uint32 DefaultFinalHeight () const
			{
			return FinalHeight (DefaultScale ());
			}

		/// Get best quality width.
		/// For a naive conversion, one could use either the default size,
		/// or the best quality size.

		uint32 BestQualityFinalWidth () const
			{
			return FinalWidth (DefaultScale () * BestQualityScale ().As_real64 ());
			}

		/// Get best quality height.
		/// For a naive conversion, one could use either the default size,
		/// or the best quality size.

		uint32 BestQualityFinalHeight () const
			{
			return FinalHeight (DefaultScale () * BestQualityScale ().As_real64 ());
			}

		/// The default crop area after applying the specified horizontal and
		/// vertical scale factors to the stage 3 image.

		dng_rect DefaultCropArea (real64 scaleH = 1.0,
						    	  real64 scaleV = 1.0) const;

		/// Setter for BaselineNoise.

		void SetBaselineNoise (real64 noise)
			{
			fBaselineNoise.Set_real64 (noise, 100);
			}

		/// Getter for BaselineNoise as dng_urational.

		const dng_urational & BaselineNoiseR () const
			{
			return fBaselineNoise;
			}

		/// Getter for BaselineNoise as real64.

		real64 BaselineNoise () const
			{
			return fBaselineNoise.As_real64 ();
			}

		/// Setter for NoiseReductionApplied.

		void SetNoiseReductionApplied (const dng_urational &value)
			{
			fNoiseReductionApplied = value;
			}

		/// Getter for NoiseReductionApplied.

		const dng_urational & NoiseReductionApplied () const
			{
			return fNoiseReductionApplied;
			}

		/// Setter for noise profile.

		void SetNoiseProfile (const dng_noise_profile &noiseProfile)
			{
			fNoiseProfile = noiseProfile;
			}

		/// Does this negative have a valid noise profile?

		bool HasNoiseProfile () const
			{
			return fNoiseProfile.IsValidForNegative (*this);
			}

		/// Getter for noise profile.

		const dng_noise_profile & NoiseProfile () const
			{
			return fNoiseProfile;
			}

		/// Setter for BaselineExposure.

		void SetBaselineExposure (real64 exposure)
			{
			fBaselineExposure.Set_real64 (exposure, 100);
			}

		/// Getter for BaselineExposure as dng_urational.

		const dng_srational & BaselineExposureR () const
			{
			return fBaselineExposure;
			}

		/// Getter for BaselineExposure as real64.

		real64 BaselineExposure () const
			{
			return BaselineExposureR ().As_real64 ();
			}

		/// Setter for BaselineSharpness.

		void SetBaselineSharpness (real64 sharpness)
			{
			fBaselineSharpness.Set_real64 (sharpness, 100);
			}

		/// Getter for BaselineSharpness as dng_urational.

		const dng_urational & BaselineSharpnessR () const
			{
			return fBaselineSharpness;
			}

		/// Getter for BaselineSharpness as real64.

		real64 BaselineSharpness () const
			{
			return BaselineSharpnessR ().As_real64 ();
			}

		/// Setter for ChromaBlurRadius.

		void SetChromaBlurRadius (const dng_urational &radius)
			{
			fChromaBlurRadius = radius;
			}

		/// Getter for ChromaBlurRadius as dng_urational.

		const dng_urational & ChromaBlurRadius () const
			{
			return fChromaBlurRadius;
			}

		/// Setter for AntiAliasStrength.

		void SetAntiAliasStrength (const dng_urational &strength)
			{
			fAntiAliasStrength = strength;
			}

		/// Getter for AntiAliasStrength as dng_urational.

		const dng_urational & AntiAliasStrength () const
			{
			return fAntiAliasStrength;
			}

		/// Setter for LinearResponseLimit.

		void SetLinearResponseLimit (real64 limit)
			{
			fLinearResponseLimit.Set_real64 (limit, 100);
			}

		/// Getter for LinearResponseLimit as dng_urational.

		const dng_urational & LinearResponseLimitR () const
			{
			return fLinearResponseLimit;
			}

		/// Getter for LinearResponseLimit as real64.

		real64 LinearResponseLimit () const
			{
			return LinearResponseLimitR ().As_real64 ();
			}

		/// Setter for ShadowScale.

		void SetShadowScale (const dng_urational &scale);

		/// Getter for ShadowScale as dng_urational.

		const dng_urational & ShadowScaleR () const
			{
			return fShadowScale;
			}

		/// Getter for ShadowScale as real64.

		real64 ShadowScale () const
			{
			return ShadowScaleR ().As_real64 ();
			}

		// API for ColorimetricReference.

		void SetColorimetricReference (uint32 ref)
			{
			fColorimetricReference = ref;
			}

		uint32 ColorimetricReference () const
			{
			return fColorimetricReference;
			}

		/// Setter for ColorChannels.

		void SetColorChannels (uint32 channels)
			{
			fColorChannels = channels;
			}

		/// Getter for ColorChannels.

		uint32 ColorChannels () const
			{
			return fColorChannels;
			}

		/// Setter for Monochrome.

		void SetMonochrome ()
			{
			SetColorChannels (1);
			}

		/// Getter for Monochrome.

		bool IsMonochrome () const
			{
			return ColorChannels () == 1;
			}

		/// Setter for AnalogBalance.

		void SetAnalogBalance (const dng_vector &b);

		/// Getter for AnalogBalance as dng_urational.

		dng_urational AnalogBalanceR (uint32 channel) const;

		/// Getter for AnalogBalance as real64.

		real64 AnalogBalance (uint32 channel) const;

		/// Setter for CameraNeutral.

		void SetCameraNeutral (const dng_vector &n);

		/// Clear CameraNeutral.

		void ClearCameraNeutral ()
			{
			fCameraNeutral.Clear ();
			}

		/// Determine if CameraNeutral has been set but not cleared.

		bool HasCameraNeutral () const
			{
			return fCameraNeutral.NotEmpty ();
			}

		/// Getter for CameraNeutral.

		const dng_vector & CameraNeutral () const
			{
			return fCameraNeutral;
			}

		dng_urational CameraNeutralR (uint32 channel) const;

		/// Setter for CameraWhiteXY.

		void SetCameraWhiteXY (const dng_xy_coord &coord);

		bool HasCameraWhiteXY () const
			{
			return fCameraWhiteXY.IsValid ();
			}

		const dng_xy_coord & CameraWhiteXY () const;

		void GetCameraWhiteXY (dng_urational &x,
							   dng_urational &y) const;

		// API for camera calibration:

		/// Setter for first of up to two color matrices used for individual camera calibrations.
		///
		/// The sequence of matrix transforms is:
		/// Camera data --> camera calibration --> "inverse" of color matrix
		///
		/// This will be a 4x4 matrix for a four-color camera. The defaults are
		/// almost always the identity matrix, and for the cases where they
		/// aren't, they are diagonal matrices.

		void SetCameraCalibration1 (const dng_matrix &m);

		/// Setter for second of up to two color matrices used for individual camera calibrations.
		///
		/// The sequence of matrix transforms is:
		/// Camera data --> camera calibration --> "inverse" of color matrix
		///
		/// This will be a 4x4 matrix for a four-color camera. The defaults are
		/// almost always the identity matrix, and for the cases where they
		/// aren't, they are diagonal matrices.

		void SetCameraCalibration2 (const dng_matrix &m);

		/// Getter for first of up to two color matrices used for individual camera calibrations.

		const dng_matrix & CameraCalibration1 () const
			{
			return fCameraCalibration1;
			}

		/// Getter for second of up to two color matrices used for individual camera calibrations.

		const dng_matrix & CameraCalibration2 () const
			{
			return fCameraCalibration2;
			}

		void SetCameraCalibrationSignature (const char *signature)
			{
			fCameraCalibrationSignature.Set (signature);
			}

		const dng_string & CameraCalibrationSignature () const
			{
			return fCameraCalibrationSignature;
			}

		// Camera Profile API:

		void AddProfile (AutoPtr<dng_camera_profile> &profile);

		void ClearProfiles ();

		uint32 ProfileCount () const;

		const dng_camera_profile & ProfileByIndex (uint32 index) const;

		const dng_camera_profile * ProfileByID (const dng_camera_profile_id &id,
												bool useDefaultIfNoMatch = true) const;

		bool HasProfileID (const dng_camera_profile_id &id) const
			{
			return ProfileByID (id, false) != NULL;
			}

		// Returns the camera profile to embed when saving to DNG:

		virtual const dng_camera_profile * CameraProfileToEmbed () const;

		// API for AsShotProfileName.

		void SetAsShotProfileName (const char *name)
			{
			fAsShotProfileName.Set (name);
			}

		const dng_string & AsShotProfileName () const
			{
			return fAsShotProfileName;
			}

		// Makes a dng_color_spec object for this negative.

		virtual dng_color_spec * MakeColorSpec (const dng_camera_profile_id &id) const;

		// API for RawImageDigest:

		void SetRawImageDigest (const dng_fingerprint &digest)
			{
			fRawImageDigest = digest;
			}

		void ClearRawImageDigest ()
			{
			fRawImageDigest.Clear ();
			}

		const dng_fingerprint & RawImageDigest () const
			{
			return fRawImageDigest;
			}

		void FindRawImageDigest (dng_host &host) const;

		void ValidateRawImageDigest (dng_host &host);

		// API for RawDataUniqueID:

		void SetRawDataUniqueID (const dng_fingerprint &id)
			{
			fRawDataUniqueID = id;
			}

		const dng_fingerprint & RawDataUniqueID () const
			{
			return fRawDataUniqueID;
			}

		void FindRawDataUniqueID (dng_host &host) const;

		void RecomputeRawDataUniqueID (dng_host &host);

		// API for original raw file name:

		void SetOriginalRawFileName (const char *name)
			{
			fOriginalRawFileName.Set (name);
			}

		bool HasOriginalRawFileName () const
			{
			return fOriginalRawFileName.NotEmpty ();
			}

		const dng_string & OriginalRawFileName () const
			{
			return fOriginalRawFileName;
			}

		// API for original raw file data:

		void SetHasOriginalRawFileData (bool hasData)
			{
			fHasOriginalRawFileData = hasData;
			}

		bool CanEmbedOriginalRaw () const
			{
			return fHasOriginalRawFileData && HasOriginalRawFileName ();
			}

		void SetOriginalRawFileData (AutoPtr<dng_memory_block> &data)
			{
			fOriginalRawFileData.Reset (data.Release ());
			}

		const void * OriginalRawFileData () const
			{
			return fOriginalRawFileData.Get () ? fOriginalRawFileData->Buffer ()
											   : NULL;
			}

		uint32 OriginalRawFileDataLength () const
			{
			return fOriginalRawFileData.Get () ? fOriginalRawFileData->LogicalSize ()
											   : 0;
			}

		// API for original raw file data digest.

		void SetOriginalRawFileDigest (const dng_fingerprint &digest)
			{
			fOriginalRawFileDigest = digest;
			}

		const dng_fingerprint & OriginalRawFileDigest () const
			{
			return fOriginalRawFileDigest;
			}

		void FindOriginalRawFileDigest () const;

		void ValidateOriginalRawFileDigest ();

		// API for DNG private data:

		void SetPrivateData (AutoPtr<dng_memory_block> &block)
			{
			fDNGPrivateData.Reset (block.Release ());
			}

		void ClearPrivateData ()
			{
			fDNGPrivateData.Reset ();
			}

		const uint8 * PrivateData () const
			{
			return fDNGPrivateData.Get () ? fDNGPrivateData->Buffer_uint8 ()
										  : NULL;
			}

		uint32 PrivateLength () const
			{
			return fDNGPrivateData.Get () ? fDNGPrivateData->LogicalSize ()
										  : 0;
			}

		// API for MakerNote data:

		void SetMakerNoteSafety (bool safe)
			{
			fIsMakerNoteSafe = safe;
			}

		bool IsMakerNoteSafe () const
			{
			return fIsMakerNoteSafe;
			}

		void SetMakerNote (AutoPtr<dng_memory_block> &block)
			{
			fMakerNote.Reset (block.Release ());
			}

		void ClearMakerNote ()
			{
			fMakerNote.Reset ();
			}

		const void * MakerNoteData () const
			{
			return fMakerNote.Get () ? fMakerNote->Buffer ()
									 : NULL;
			}

		uint32 MakerNoteLength () const
			{
			return fMakerNote.Get () ? fMakerNote->LogicalSize ()
									 : 0;
			}

		// API for EXIF metadata:

		dng_exif * GetExif ()
			{
			return fExif.Get ();
			}

		const dng_exif * GetExif () const
			{
			return fExif.Get ();
			}

		virtual dng_memory_block * BuildExifBlock (const dng_resolution *resolution = NULL,
												   bool includeIPTC = false,
												   bool minimalEXIF = false,
												   const dng_jpeg_preview *thumbnail = NULL) const;

		// API for original EXIF metadata.

		dng_exif * GetOriginalExif ()
			{
			return fOriginalExif.Get ();
			}

		const dng_exif * GetOriginalExif () const
			{
			return fOriginalExif.Get ();
			}

		// API for IPTC metadata:

		void SetIPTC (AutoPtr<dng_memory_block> &block,
					  uint64 offset);

		void SetIPTC (AutoPtr<dng_memory_block> &block);

		void ClearIPTC ();

		const void * IPTCData () const;

		uint32 IPTCLength () const;

		uint64 IPTCOffset () const;

		dng_fingerprint IPTCDigest (bool includePadding = true) const;

		void RebuildIPTC (bool padForTIFF,
						  bool forceUTF8);

		bool UsedUTF8forIPTC () const
			{
			return fUsedUTF8forIPTC;
			}

		void SetUsedUTF8forIPTC (bool used)
			{
			fUsedUTF8forIPTC = used;
			}

		// API for XMP metadata:

		bool SetXMP (dng_host &host,
					 const void *buffer,
					 uint32 count,
					 bool xmpInSidecar = false,
					 bool xmpIsNewer = false);

		dng_xmp * GetXMP ()
			{
			return fXMP.Get ();
			}

		const dng_xmp * GetXMP () const
			{
			return fXMP.Get ();
			}

		bool XMPinSidecar () const
			{
			return fXMPinSidecar;
			}

		// API for linearization information:

		const dng_linearization_info * GetLinearizationInfo () const
			{
			return fLinearizationInfo.Get ();
			}

		void ClearLinearizationInfo ()
			{
			fLinearizationInfo.Reset ();
			}

		// Linearization curve.  Usually used to increase compression ratios
		// by storing the compressed data in a more visually uniform space.
		// This is a 16-bit LUT that maps the stored data back to linear.

		void SetLinearization (AutoPtr<dng_memory_block> &curve);

		// Active area (non-black masked pixels).  These pixels are trimmed
		// during linearization step.

		void SetActiveArea (const dng_rect &area);

		// Areas that are known to contain black masked pixels that can
		// be used to estimate black levels.

		void SetMaskedAreas (uint32 count,
							 const dng_rect *area);

		void SetMaskedArea (const dng_rect &area)
			{
			SetMaskedAreas (1, &area);
			}

		// Sensor black level information.

		void SetBlackLevel (real64 black,
							int32 plane = -1);

		void SetQuadBlacks (real64 black0,
						    real64 black1,
						    real64 black2,
						    real64 black3);

		void SetRowBlacks (const real64 *blacks,
						   uint32 count);

		void SetColumnBlacks (const real64 *blacks,
							  uint32 count);

		// Sensor white level information.

		uint32 WhiteLevel (uint32 plane = 0) const;

		void SetWhiteLevel (uint32 white,
							int32 plane = -1);

		// API for mosaic information:

		const dng_mosaic_info * GetMosaicInfo () const
			{
			return fMosaicInfo.Get ();
			}

		void ClearMosaicInfo ()
			{
			fMosaicInfo.Reset ();
			}

		// ColorKeys APIs:

		void SetColorKeys (ColorKeyCode color0,
						   ColorKeyCode color1,
						   ColorKeyCode color2,
						   ColorKeyCode color3 = colorKeyMaxEnum);

		void SetRGB ()
			{

			SetColorChannels (3);

			SetColorKeys (colorKeyRed,
						  colorKeyGreen,
						  colorKeyBlue);

			}

		void SetCMY ()
			{

			SetColorChannels (3);

			SetColorKeys (colorKeyCyan,
						  colorKeyMagenta,
						  colorKeyYellow);

			}

		void SetGMCY ()
			{

			SetColorChannels (4);

			SetColorKeys (colorKeyGreen,
						  colorKeyMagenta,
					      colorKeyCyan,
						  colorKeyYellow);

			}

		// APIs to set mosaic patterns.

		void SetBayerMosaic (uint32 phase);

		void SetFujiMosaic (uint32 phase);

		void SetQuadMosaic (uint32 pattern);

		// BayerGreenSplit.

		void SetGreenSplit (uint32 split);

		// APIs for opcode lists.

		const dng_opcode_list & OpcodeList1 () const
			{
			return fOpcodeList1;
			}

		dng_opcode_list & OpcodeList1 ()
			{
			return fOpcodeList1;
			}

		const dng_opcode_list & OpcodeList2 () const
			{
			return fOpcodeList2;
			}

		dng_opcode_list & OpcodeList2 ()
			{
			return fOpcodeList2;
			}

		const dng_opcode_list & OpcodeList3 () const
			{
			return fOpcodeList3;
			}

		dng_opcode_list & OpcodeList3 ()
			{
			return fOpcodeList3;
			}

		// First part of parsing logic.

		virtual void Parse (dng_host &host,
							dng_stream &stream,
							dng_info &info);

		// Second part of parsing logic.  This is split off from the
		// first part because these operations are useful when extending
		// this sdk to support non-DNG raw formats.

		virtual void PostParse (dng_host &host,
								dng_stream &stream,
								dng_info &info);

		// Synchronize metadata sources.

		virtual void SynchronizeMetadata ();

		// Routines to update the date/time field in the EXIF and XMP
		// metadata.

		void UpdateDateTime (const dng_date_time_info &dt);

		void UpdateDateTimeToNow ();

		// Developer's utility function to switch to four color Bayer
		// interpolation.  This is useful for evaluating how much green
		// split a Bayer pattern sensor has.

		virtual bool SetFourColorBayer ();

		// Access routines for the image stages.

		const dng_image * Stage1Image () const
			{
			return fStage1Image.Get ();
			}

		const dng_image * Stage2Image () const
			{
			return fStage2Image.Get ();
			}

		const dng_image * Stage3Image () const
			{
			return fStage3Image.Get ();
			}

		// Returns the processing stage of the raw image data.

		RawImageStageEnum RawImageStage () const
			{
			return fRawImageStage;
			}

		// Returns the raw image data.

		const dng_image & RawImage () const;

		// Read the stage 1 image.

		virtual void ReadStage1Image (dng_host &host,
									  dng_stream &stream,
									  dng_info &info);

		// Assign the stage 1 image.

		void SetStage1Image (AutoPtr<dng_image> &image);

		// Assign the stage 2 image.

		void SetStage2Image (AutoPtr<dng_image> &image);

		// Assign the stage 3 image.

		void SetStage3Image (AutoPtr<dng_image> &image);

		// Build the stage 2 (linearized and range mapped) image.

		void BuildStage2Image (dng_host &host,
							   uint32 pixelType = ttShort);

		// Build the stage 3 (demosaiced) image.

		void BuildStage3Image (dng_host &host,
							   int32 srcPlane = -1);

		// Additional gain applied when building the stage 3 image.

		void SetStage3Gain (real64 gain)
			{
			fStage3Gain = gain;
			}

		real64 Stage3Gain () const
			{
			return fStage3Gain;
			}

		// IsPreview API:

		void SetIsPreview (bool preview)
			{
			fIsPreview = preview;
			}

		bool IsPreview () const
			{
			return fIsPreview;
			}

		// IsDamaged API:

		void SetIsDamaged (bool damaged)
			{
			fIsDamaged = damaged;
			}

		bool IsDamaged () const
			{
			return fIsDamaged;
			}

	protected:

		dng_negative (dng_memory_allocator &allocator);

		virtual void Initialize ();

		virtual dng_exif * MakeExif ();

		virtual dng_xmp * MakeXMP ();

		virtual dng_linearization_info * MakeLinearizationInfo ();

		void NeedLinearizationInfo ();

		virtual dng_mosaic_info * MakeMosaicInfo ();

		void NeedMosaicInfo ();

		virtual void DoBuildStage2 (dng_host &host,
									uint32 pixelType);

		virtual void DoInterpolateStage3 (dng_host &host,
									      int32 srcPlane);

		virtual void DoMergeStage3 (dng_host &host);

		virtual void DoBuildStage3 (dng_host &host,
									int32 srcPlane);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
