/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_xmp_sdk.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_xmp_sdk__
#define __dng_xmp_sdk__

/*****************************************************************************/

#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_flags.h"
#include "dng_types.h"

/*****************************************************************************/

extern const char *XMP_NS_TIFF;
extern const char *XMP_NS_EXIF;
extern const char *XMP_NS_PHOTOSHOP;
extern const char *XMP_NS_XAP;
extern const char *XMP_NS_DC;
extern const char *XMP_NS_XMP_NOTE;

extern const char *XMP_NS_CRS;
extern const char *XMP_NS_CRSS;

extern const char *XMP_NS_AUX;

extern const char *XMP_NS_IPTC;

extern const char *XMP_NS_CRX;

/*****************************************************************************/

class dng_xmp_private;

/*****************************************************************************/

typedef bool (IteratePathsCallback) (const char *ns,
			  				   	     const char *path,
			  				         void *callbackData);

/*****************************************************************************/

struct dng_xmp_namespace
	{
	const char * fullName;
	const char * shortName;
	};

/*****************************************************************************/

class dng_xmp_sdk
	{

	private:

		dng_xmp_private *fPrivate;

	public:

		dng_xmp_sdk ();

		dng_xmp_sdk (const dng_xmp_sdk &sdk);

		virtual ~dng_xmp_sdk ();

		static void InitializeSDK (dng_xmp_namespace * extraNamespaces = NULL);

		static void TerminateSDK ();

		bool HasMeta () const;

		void Parse (dng_host &host,
					const char *buffer,
				    uint32 count);

		bool Exists (const char *ns,
					 const char *path) const;

		void AppendArrayItem (const char *ns,
					  const char *arrayName,
					  const char *itemValue,
					  bool isBag = true,
					  bool propIsStruct = false);

		int32 CountArrayItems (const char *ns,
		                       const char *path) const;

		bool HasNameSpace (const char *ns) const;

		void Remove (const char *ns,
				     const char *path);

		void RemoveProperties (const char *ns);

		void ComposeArrayItemPath (const char *ns,
								   const char *arrayName,
								   int32 itemNumber,
								   dng_string &s) const;

		void ComposeStructFieldPath (const char *ns,
								     const char *structName,
								     const char *fieldNS,
									 const char *fieldName,
								     dng_string &s) const;

		bool GetNamespacePrefix (const char *uri,
								 dng_string &s) const;

		bool GetString (const char *ns,
				   		const char *path,
				   		dng_string &s) const;

		void ValidateStringList (const char *ns,
								 const char *path);

		bool GetStringList (const char *ns,
							const char *path,
							dng_string_list &list) const;

		bool GetAltLangDefault (const char *ns,
								const char *path,
								dng_string &s) const;

		bool GetStructField (const char *ns,
							 const char *path,
							 const char *fieldNS,
							 const char *fieldName,
							 dng_string &s) const;

		void Set (const char *ns,
				  const char *path,
				  const char *text);

		void SetString (const char *ns,
				  		const char *path,
				  		const dng_string &s);

		void SetStringList (const char *ns,
				  		    const char *path,
				  		    const dng_string_list &list,
				  		    bool isBag);

		void SetAltLangDefault (const char *ns,
								const char *path,
								const dng_string &s);

		void SetStructField (const char *ns,
							 const char *path,
							 const char *fieldNS,
							 const char *fieldName,
							 const char *text);

		void DeleteStructField (const char *ns,
								const char *structName,
								const char *fieldNS,
								const char *fieldName);

		dng_memory_block * Serialize (dng_memory_allocator &allocator,
									  bool asPacket,
									  uint32 targetBytes,
									  uint32 padBytes,
									  bool forJPEG) const;

		void PackageForJPEG (dng_memory_allocator &allocator,
							 AutoPtr<dng_memory_block> &stdBlock,
							 AutoPtr<dng_memory_block> &extBlock,
							 dng_string &extDigest) const;

		void MergeFromJPEG (const dng_xmp_sdk *xmp);

		void AppendXMP (const dng_xmp_sdk *xmp);

		void ReplaceXMP (dng_xmp_sdk *xmp);

		bool IteratePaths (IteratePathsCallback *callback,
						   void *callbackData = NULL,
						   const char *startNS = 0,
						   const char *startingPath = 0);

	private:

		void ClearMeta ();

		void MakeMeta ();

		void NeedMeta ();

		// Hidden assignment operator.

		dng_xmp_sdk & operator= (const dng_xmp_sdk &sdk);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
