/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_xmp.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_xmp__
#define __dng_xmp__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"
#include "dng_xmp_sdk.h"

/*****************************************************************************/

class dng_xmp
	{

	protected:

		// Sync option bits.

		enum
			{
			ignoreXMP		= 1,		// Force XMP values to match non-XMP
			preferXMP 		= 2,		// Prefer XMP values if conflict
			preferNonXMP	= 4,		// Prefer non-XMP values if conflict
			requireASCII	= 8			// Non-XMP values must be ASCII
			};

		dng_memory_allocator &fAllocator;

		dng_xmp_sdk *fSDK;

	public:

		dng_xmp (dng_memory_allocator &allocator);

		dng_xmp (const dng_xmp &xmp);

		virtual ~dng_xmp ();

		void Parse (dng_host &host,
					const void *buffer,
				    uint32 count);

		dng_memory_block * Serialize (bool asPacket = false,
									  uint32 targetBytes = 0,
									  uint32 padBytes = 4096,
									  bool forJPEG = false) const;

		void PackageForJPEG (AutoPtr<dng_memory_block> &stdBlock,
							 AutoPtr<dng_memory_block> &extBlock,
							 dng_string &extDigest) const;

		void MergeFromJPEG (const dng_xmp &xmp);

		bool HasMeta () const;

		bool Exists (const char *ns,
					 const char *path) const;

		bool HasNameSpace (const char *ns) const;

		bool IteratePaths (IteratePathsCallback *callback,
						   void *callbackData,
						   const char *ns = 0,
						   const char *path = 0);

		void Remove (const char *ns,
				     const char *path);

		void RemoveProperties (const char *ns);

		void Set (const char *ns,
				  const char *path,
				  const char *text);

		bool GetString (const char *ns,
						const char *path,
						dng_string &s) const;

		void SetString (const char *ns,
						const char *path,
						const dng_string &s);

		bool GetStringList (const char *ns,
						    const char *path,
						    dng_string_list &list) const;

		void SetStringList (const char *ns,
						    const char *path,
						    const dng_string_list &list,
						    bool isBag = false);

		void SetStructField (const char *ns,
							 const char *path,
						     const char *fieldNS,
							 const char *fieldName,
							 const dng_string &s);

		void SetStructField (const char *ns,
							 const char *path,
						     const char *fieldNS,
							 const char *fieldName,
							 const char *s);

		void DeleteStructField (const char *ns,
								const char *path,
								const char *fieldNS,
								const char *fieldName);

		bool GetStructField (const char *ns,
							 const char *path,
							 const char *fieldNS,
							 const char *fieldName,
							 dng_string &s) const;

		void SetAltLangDefault (const char *ns,
								const char *path,
								const dng_string &s);

		bool GetAltLangDefault (const char *ns,
								const char *path,
								dng_string &s) const;

		bool GetBoolean (const char *ns,
						 const char *path,
						 bool &x) const;

		void SetBoolean (const char *ns,
						 const char *path,
						 bool x);

		bool Get_int32 (const char *ns,
						const char *path,
						int32 &x) const;

		void Set_int32 (const char *ns,
						const char *path,
						int32 x,
						bool usePlus = false);

		bool Get_uint32 (const char *ns,
						 const char *path,
						 uint32 &x) const;

		void Set_uint32 (const char *ns,
						 const char *path,
						 uint32 x);

		bool Get_real64 (const char *ns,
					     const char *path,
					     real64 &x) const;

		void Set_real64 (const char *ns,
					     const char *path,
					     real64 x,
				         uint32 places = 6,
					     bool trim = true,
					     bool usePlus = false);

		bool Get_urational (const char *ns,
							const char *path,
							dng_urational &r) const;

		void Set_urational (const char *ns,
							const char *path,
							const dng_urational &r);

		bool Get_srational (const char *ns,
							const char *path,
							dng_srational &r) const;

		void Set_srational (const char *ns,
							const char *path,
							const dng_srational &r);

		bool GetFingerprint (const char *ns,
							 const char *path,
							 dng_fingerprint &print) const;

		void SetFingerprint (const char *ns,
							 const char *path,
							 const dng_fingerprint &print);

		dng_fingerprint GetIPTCDigest () const;

		void SetIPTCDigest (dng_fingerprint &digest);

		void IngestIPTC (dng_negative &negative,
					     bool xmpIsNewer = false);

		void RebuildIPTC (dng_negative &negative,
						  bool padForTIFF,
						  bool forceUTF8);

		virtual void SyncExif (dng_exif &exif,
							   const dng_exif *originalExif = NULL,
							   bool doingUpdateFromXMP = false);

		void ValidateStringList (const char *ns,
							     const char *path);

		void ValidateMetadata ();

		void UpdateDateTime (const dng_date_time_info &dt);

		void UpdateExifDates (dng_exif &exif);

		bool HasOrientation () const;

		dng_orientation GetOrientation () const;

		void ClearOrientation ();

		void SetOrientation (const dng_orientation &orientation);

		void SyncOrientation (dng_negative &negative,
					   		  bool xmpIsMaster);

		void ClearImageInfo ();

		void SetImageSize (const dng_point &size);

		void SetSampleInfo (uint32 samplesPerPixel,
							uint32 bitsPerSample);

		void SetPhotometricInterpretation (uint32 pi);

		void SetResolution (const dng_resolution &res);

		void ComposeArrayItemPath (const char *ns,
								   const char *arrayName,
								   int32 itemNumber,
								   dng_string &s) const;

		void ComposeStructFieldPath (const char *ns,
								     const char *structName,
								     const char *fieldNS,
									 const char *fieldName,
								     dng_string &s) const;

		int32 CountArrayItems (const char *ns,
		                       const char *path) const;

		void AppendArrayItem (const char *ns,
							  const char *arrayName,
							  const char *itemValue,
							  bool isBag = true,
							  bool propIsStruct = false);

		static dng_string EncodeFingerprint (const dng_fingerprint &f);

		static dng_fingerprint DecodeFingerprint (const dng_string &s);

	protected:

		static void TrimDecimal (char *s);

		static dng_string EncodeGPSVersion (uint32 version);

		static uint32 DecodeGPSVersion (const dng_string &s);

		static dng_string EncodeGPSCoordinate (const dng_string &ref,
							   				   const dng_urational *coord);

		static void DecodeGPSCoordinate (const dng_string &s,
										 dng_string &ref,
										 dng_urational *coord);

		static dng_string EncodeGPSDateTime (const dng_string &dateStamp,
											 const dng_urational *timeStamp);

		static void DecodeGPSDateTime (const dng_string &s,
									   dng_string &dateStamp,
									   dng_urational *timeStamp);

		bool SyncString (const char *ns,
						 const char *path,
						 dng_string &s,
						 uint32 options = 0);

		void SyncStringList (const char *ns,
						 	 const char *path,
						 	 dng_string_list &list,
						 	 bool isBag = false,
						 	 uint32 options = 0);

		bool SyncAltLangDefault (const char *ns,
								 const char *path,
								 dng_string &s,
								 uint32 options = 0);

		void Sync_uint32 (const char *ns,
						  const char *path,
						  uint32 &x,
						  bool isDefault = false,
						  uint32 options = 0);

		void Sync_uint32_array (const char *ns,
						   		const char *path,
						   		uint32 *data,
						   		uint32 &count,
						   		uint32 maxCount,
						   		uint32 options = 0);

		void Sync_urational (const char *ns,
							 const char *path,
							 dng_urational &r,
							 uint32 options = 0);

		void Sync_srational (const char *ns,
							 const char *path,
							 dng_srational &r,
							 uint32 options = 0);

		void SyncIPTC (dng_iptc &iptc,
					   uint32 options);

		void SyncFlash (uint32 &flashState,
						uint32 &flashMask,
						uint32 options);

	private:

		// Hidden assignment operator.

		dng_xmp & operator= (const dng_xmp &xmp);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
