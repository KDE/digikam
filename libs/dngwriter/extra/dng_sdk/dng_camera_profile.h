/******************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/******************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_camera_profile.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Support for DNG camera color profile information.
 *  Per the \ref spec_dng "DNG 1.1.0 specification", a DNG file can store up to
 *  two sets of color profile information for a camera in the DNG file from that
 *  camera. The second set is optional and when there are two sets, they represent
 *  profiles made under different illumination.
 *
 *  Profiling information is optionally separated into two parts. One part represents
 *  a profile for a reference camera. (ColorMatrix1 and ColorMatrix2 here.) The
 *  second is a per-camera calibration that takes into account unit-to-unit variation.
 *  This is designed to allow replacing the reference color matrix with one of one's
 *  own construction while maintaining any unit-specific calibration the camera
 *  manufacturer may have provided.
 *
 * See Appendix 6 of the \ref spec_dng "DNG 1.1.0 specification" for more information.
 */

#ifndef __dng_camera_profile__
#define __dng_camera_profile__

/******************************************************************************/

#include "dng_assertions.h"
#include "dng_classes.h"
#include "dng_fingerprint.h"
#include "dng_hue_sat_map.h"
#include "dng_matrix.h"
#include "dng_string.h"
#include "dng_tag_values.h"
#include "dng_tone_curve.h"

/******************************************************************************/

extern const char * kProfileName_Embedded;

extern const char * kAdobeCalibrationSignature;

/******************************************************************************/

class dng_camera_profile_id
	{

	private:

		dng_string fName;

		dng_fingerprint fFingerprint;

	public:

		dng_camera_profile_id ()

			:	fName        ()
			,	fFingerprint ()

			{
			}

		dng_camera_profile_id (const char *name)

			:	fName		 ()
			,	fFingerprint ()

			{
			fName.Set (name);
			}

		dng_camera_profile_id (const dng_string &name)

			:	fName		 (name)
			,	fFingerprint ()

			{
			}

		dng_camera_profile_id (const char *name,
							   const dng_fingerprint &fingerprint)

			:	fName		 ()
			,	fFingerprint (fingerprint)

			{
			fName.Set (name);
			DNG_ASSERT (!fFingerprint.IsValid () || fName.NotEmpty (),
						"Cannot have profile fingerprint without name");
			}

		dng_camera_profile_id (const dng_string &name,
							   const dng_fingerprint &fingerprint)

			:	fName		 (name)
			,	fFingerprint (fingerprint)

			{
			DNG_ASSERT (!fFingerprint.IsValid () || fName.NotEmpty (),
						"Cannot have profile fingerprint without name");
			}

		const dng_string & Name () const
			{
			return fName;
			}

		const dng_fingerprint & Fingerprint () const
			{
			return fFingerprint;
			}

		bool operator== (const dng_camera_profile_id &id) const
			{
			return fName        == id.fName &&
				   fFingerprint == id.fFingerprint;
			}

		bool operator!= (const dng_camera_profile_id &id) const
			{
			return !(*this == id);
			}

		bool IsValid () const
			{
			return fName.NotEmpty ();		// Fingerprint is optional.
			}

		void Clear ()
			{
			*this = dng_camera_profile_id ();
			}

	};

/******************************************************************************/

/// \brief Container for DNG camera color profile and calibration data.

class dng_camera_profile
	{

	protected:

		// Name of this camera profile.

		dng_string fName;

		// Light sources for up to two calibrations. These use the EXIF
		// encodings for illuminant and are used to distinguish which
		// matrix to use.

		uint32 fCalibrationIlluminant1;
		uint32 fCalibrationIlluminant2;

		// Color matrices for up to two calibrations.

		// These matrices map XYZ values to non-white balanced camera values.
		// Adobe needs to go that direction in order to determine the clipping
		// points for highlight recovery logic based on the white point.  If
		// cameras were all 3-color, the matrix could be stored as a forward matrix,
		// but we need the backwards matrix to deal with 4-color cameras.

		dng_matrix fColorMatrix1;
		dng_matrix fColorMatrix2;

		// These matrices map white balanced camera values to XYZ chromatically
		// adapted to D50 (the ICC profile PCS white point).  If the matrices
		// exist, then this implies that white balancing should be done by scaling
		// camera values with a diagonal matrix.

		dng_matrix fForwardMatrix1;
		dng_matrix fForwardMatrix2;

		// Dimensionality reduction hints for more than three color cameras.
		// This is an optional matrix that maps the camera's color components
		// to 3 components.  These are only used if the forward matrices don't
		// exist, and are used invert the color matrices.

		dng_matrix fReductionMatrix1;
		dng_matrix fReductionMatrix2;

		// MD5 hash for all data bits of the profile.

		mutable dng_fingerprint fFingerprint;

		// Copyright notice from creator of profile.

		dng_string fCopyright;

		// Rules for how this profile can be embedded and/or copied.

		uint32 fEmbedPolicy;

		// 2-D (or 3-D) hue/sat tables to modify colors.

		dng_hue_sat_map fHueSatDeltas1;
		dng_hue_sat_map fHueSatDeltas2;

		// 3-D hue/sat table to apply a "look".

		dng_hue_sat_map fLookTable;

		// The "as shot" tone curve for this profile.  Check IsValid method
		// to tell if one exists in profile.

		dng_tone_curve fToneCurve;

		// If this string matches the fCameraCalibrationSignature of the
		// negative, then use the calibration matrix values from the negative.

		dng_string fProfileCalibrationSignature;

		// If non-empty, only allow use of this profile with camera having
		// same unique model name.

		dng_string fUniqueCameraModelRestriction;

		// Was this profile read from inside a DNG file? (If so, we wnat
		// to be sure to include it again when writing out an updated
		// DNG file)

		bool fWasReadFromDNG;

		// Was this profile stubbed to save memory (and no longer valid
		// for building color conversion tables)?

		bool fWasStubbed;

	public:

		dng_camera_profile ();

		virtual ~dng_camera_profile ();

		// API for profile name:

		/// Setter for camera profile name.
		/// \param name Name to use for this camera profile.

		void SetName (const char *name)
			{
			fName.Set (name);
			ClearFingerprint ();
			}

		/// Getter for camera profile name.
		/// \retval Name of profile.

		const dng_string & Name () const
			{
			return fName;
			}

		/// Test if this name is embedded.
		/// \retval true if the name matches the name of the embedded camera profile.

		bool NameIsEmbedded () const
			{
			return fName.Matches (kProfileName_Embedded, true);
			}

		// API for calibration illuminants:

		/// Setter for first of up to two light sources used for calibration.
		/// Uses the EXIF encodings for illuminant and is used to distinguish which
		/// matrix to use.
		/// Corresponds to the DNG CalibrationIlluminant1 tag.

		void SetCalibrationIlluminant1 (uint32 light)
			{
			fCalibrationIlluminant1 = light;
			ClearFingerprint ();
			}

		/// Setter for second of up to two light sources used for calibration.
		/// Uses the EXIF encodings for illuminant and is used to distinguish which
		/// matrix to use.
		/// Corresponds to the DNG CalibrationIlluminant2 tag.

		void SetCalibrationIlluminant2 (uint32 light)
			{
			fCalibrationIlluminant2 = light;
			ClearFingerprint ();
			}

		/// Getter for first of up to two light sources used for calibration.
		/// Uses the EXIF encodings for illuminant and is used to distinguish which
		/// matrix to use.
		/// Corresponds to the DNG CalibrationIlluminant1 tag.

		uint32 CalibrationIlluminant1 () const
			{
			return fCalibrationIlluminant1;
			}

		/// Getter for second of up to two light sources used for calibration.
		/// Uses the EXIF encodings for illuminant and is used to distinguish which
		/// matrix to use.
		/// Corresponds to the DNG CalibrationIlluminant2 tag.

		uint32 CalibrationIlluminant2 () const
			{
			return fCalibrationIlluminant2;
			}

		/// Getter for first of up to two light sources used for calibration, returning
		/// result as color temperature.

		real64 CalibrationTemperature1 () const
			{
			return IlluminantToTemperature (CalibrationIlluminant1 ());
			}

		/// Getter for second of up to two light sources used for calibration, returning
		/// result as color temperature.

		real64 CalibrationTemperature2 () const
			{
			return IlluminantToTemperature (CalibrationIlluminant2 ());
			}

		// API for color matrices:

		/// Utility function to normalize the scale of the color matrix.

		static void NormalizeColorMatrix (dng_matrix &m);

		/// Setter for first of up to two color matrices used for reference camera calibrations.
		/// These matrices map XYZ values to camera values.  The DNG SDK needs to map colors
		/// that direction in order to determine the clipping points for
		/// highlight recovery logic based on the white point.  If cameras
		/// were all three-color, the matrix could be stored as a forward matrix.
		/// The inverse matrix is requried to support four-color cameras.

		void SetColorMatrix1 (const dng_matrix &m);

		/// Setter for second of up to two color matrices used for reference camera calibrations.
		/// These matrices map XYZ values to camera values.  The DNG SDK needs to map colors
		/// that direction in order to determine the clipping points for
		/// highlight recovery logic based on the white point.  If cameras
		/// were all three-color, the matrix could be stored as a forward matrix.
		/// The inverse matrix is requried to support four-color cameras.

		void SetColorMatrix2 (const dng_matrix &m);

		/// Predicate to test if first camera matrix is set

		bool HasColorMatrix1 () const;

		/// Predicate to test if second camera matrix is set

		bool HasColorMatrix2 () const;

		/// Getter for first of up to two color matrices used for calibrations.

		const dng_matrix & ColorMatrix1 () const
			{
			return fColorMatrix1;
			}

		/// Getter for second of up to two color matrices used for calibrations.

		const dng_matrix & ColorMatrix2 () const
			{
			return fColorMatrix2;
			}

		// API for forward matrices:

		/// Utility function to normalize the scale of the forward matrix.

		static void NormalizeForwardMatrix (dng_matrix &m);

		/// Setter for first of up to two forward matrices used for calibrations.

		void SetForwardMatrix1 (const dng_matrix &m);

		/// Setter for second of up to two forward matrices used for calibrations.

		void SetForwardMatrix2 (const dng_matrix &m);

		/// Getter for first of up to two forward matrices used for calibrations.

		const dng_matrix & ForwardMatrix1 () const
			{
			return fForwardMatrix1;
			}

		/// Getter for second of up to two forward matrices used for calibrations.

		const dng_matrix & ForwardMatrix2 () const
			{
			return fForwardMatrix2;
			}

		// API for reduction matrices:

		/// Setter for first of up to two dimensionality reduction hints for four-color cameras.
		/// This is an optional matrix that maps four components to three.
		/// See Appendix 6 of the \ref spec_dng "DNG 1.1.0 specification."

		void SetReductionMatrix1 (const dng_matrix &m);

		/// Setter for second of up to two dimensionality reduction hints for four-color cameras.
		/// This is an optional matrix that maps four components to three.
		/// See Appendix 6 of the \ref spec_dng "DNG 1.1.0 specification."

		void SetReductionMatrix2 (const dng_matrix &m);

		/// Getter for first of up to two dimensionality reduction hints for four color cameras.

		const dng_matrix & ReductionMatrix1 () const
			{
			return fReductionMatrix1;
			}

		/// Getter for second of up to two dimensionality reduction hints for four color cameras.

		const dng_matrix & ReductionMatrix2 () const
			{
			return fReductionMatrix2;
			}

		/// Getter function from profile fingerprint.

		const dng_fingerprint &Fingerprint () const
			{

			if (!fFingerprint.IsValid ())
				CalculateFingerprint ();

			return fFingerprint;

			}

		/// Getter for camera profile id.
		/// \retval ID of profile.

		dng_camera_profile_id ProfileID () const
			{
			return dng_camera_profile_id (Name (), Fingerprint ());
			}

		/// Setter for camera profile copyright.
		/// \param copyright Copyright string to use for this camera profile.

		void SetCopyright (const char *copyright)
			{
			fCopyright.Set (copyright);
			ClearFingerprint ();
			}

		/// Getter for camera profile copyright.
		/// \retval Copyright string for profile.

		const dng_string & Copyright () const
			{
			return fCopyright;
			}

		// Accessors for embed policy.

		void SetEmbedPolicy (uint32 policy)
			{
			fEmbedPolicy = policy;
			ClearFingerprint ();
			}

		uint32 EmbedPolicy () const
			{
			return fEmbedPolicy;
			}

		bool IsLegalToEmbed () const
			{
			return WasReadFromDNG () ||
				   EmbedPolicy () == pepAllowCopying ||
				   EmbedPolicy () == pepEmbedIfUsed  ||
				   EmbedPolicy () == pepNoRestrictions;
			}

		// Accessors for hue sat maps.

		bool HasHueSatDeltas () const
			{
			return fHueSatDeltas1.IsValid ();
			}

		const dng_hue_sat_map & HueSatDeltas1 () const
			{
			return fHueSatDeltas1;
			}

		void SetHueSatDeltas1 (const dng_hue_sat_map &deltas1);

		const dng_hue_sat_map & HueSatDeltas2 () const
			{
			return fHueSatDeltas2;
			}

		void SetHueSatDeltas2 (const dng_hue_sat_map &deltas2);

		// Accessors for look table.

		bool HasLookTable () const
			{
			return fLookTable.IsValid ();
			}

		const dng_hue_sat_map & LookTable () const
			{
			return fLookTable;
			}

		void SetLookTable (const dng_hue_sat_map &table);

		// Accessors for tone curve.

		const dng_tone_curve & ToneCurve () const
			{
			return fToneCurve;
			}

		void SetToneCurve (const dng_tone_curve &curve)
			{
			fToneCurve = curve;
			ClearFingerprint ();
			}

		// Accessors for profile calibration signature.

		void SetProfileCalibrationSignature (const char *signature)
			{
			fProfileCalibrationSignature.Set (signature);
			}

		const dng_string & ProfileCalibrationSignature () const
			{
			return fProfileCalibrationSignature;
			}

		/// Setter for camera unique model name to restrict use of this profile.
		/// \param camera Camera unique model name designating only camera this
		/// profile can be used with. (Empty string for no restriction.)

		void SetUniqueCameraModelRestriction (const char *camera)
			{
			fUniqueCameraModelRestriction.Set (camera);
			// Not included in fingerprint, so don't need ClearFingerprint ().
			}

		/// Getter for camera unique model name to restrict use of this profile.
		/// \retval Unique model name of only camera this profile can be used with
		/// or empty if no restriction.

		const dng_string & UniqueCameraModelRestriction () const
			{
			return fUniqueCameraModelRestriction;
			}

		// Accessors for was read from DNG flag.

		void SetWasReadFromDNG (bool state = true)
			{
			fWasReadFromDNG = state;
			}

		bool WasReadFromDNG () const
			{
			return fWasReadFromDNG;
			}

		/// Determines if this a valid profile for this number of color channels?
		/// \retval true if the profile is valid.

		bool IsValid (uint32 channels) const;

		/// Predicate to check if two camera profiles are colorwise equal, thus ignores
		/// the profile name.
		/// \param profile Camera profile to compare to.

		bool EqualData (const dng_camera_profile &profile) const;

		/// Parse profile from dng_camera_profile_info data.

		void Parse (dng_stream &stream,
					dng_camera_profile_info &profileInfo);

		/// Parse from an extended profile stream, which is similar to stand alone
		/// TIFF file.

		bool ParseExtended (dng_stream &stream);

		/// Convert from a three-color to a four-color Bayer profile.

		virtual void SetFourColorBayer ();

		/// Find the hue/sat table to use for a given white point, if any.
		/// The calling routine owns the resulting table.

		dng_hue_sat_map * HueSatMapForWhite (const dng_xy_coord &white) const;

		/// Stub out the profile (free memory used by large tables).

		void Stub ();

		/// Was this profile stubbed?

		bool WasStubbed () const
			{
			return fWasStubbed;
			}

	protected:

		static real64 IlluminantToTemperature (uint32 light);

		void ClearFingerprint ()
			{
			fFingerprint.Clear ();
			}

		void CalculateFingerprint () const;

		static bool ValidForwardMatrix (const dng_matrix &m);

		static void ReadHueSatMap (dng_stream &stream,
								   dng_hue_sat_map &hueSatMap,
								   uint32 hues,
								   uint32 sats,
								   uint32 vals,
								   bool skipSat0);

	};

/******************************************************************************/

void SplitCameraProfileName (const dng_string &name,
							 dng_string &baseName,
							 int32 &version);

/******************************************************************************/

#endif

/******************************************************************************/
