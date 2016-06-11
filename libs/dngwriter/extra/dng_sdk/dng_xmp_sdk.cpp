/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_xmp_sdk.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_xmp_sdk.h"

#include "dng_auto_ptr.h"
#include "dng_assertions.h"
#include "dng_exceptions.h"
#include "dng_host.h"
#include "dng_memory.h"
#include "dng_string.h"
#include "dng_string_list.h"
#include "dng_utils.h"

/*****************************************************************************/

#if qMacOS
#ifndef MAC_ENV
#define MAC_ENV 1
#endif
#endif

#if qWinOS
#ifndef WIN_ENV
#define WIN_ENV 1
#endif
#endif

#include <new>
#include <string>

#ifdef _MSC_VER
#pragma warning ( disable : 4267 ) // possible loss of data
#endif

#define TXMP_STRING_TYPE std::string

#include "XMP.incl_cpp"

using namespace DngXmpSdk;

/*****************************************************************************/

const char *XMP_NS_TIFF	      = "http://ns.adobe.com/tiff/1.0/";
const char *XMP_NS_EXIF	      = "http://ns.adobe.com/exif/1.0/";
const char *XMP_NS_PHOTOSHOP  = "http://ns.adobe.com/photoshop/1.0/";
const char *XMP_NS_XAP        = "http://ns.adobe.com/xap/1.0/";
const char *XMP_NS_DC		  = "http://purl.org/dc/elements/1.1/";
const char *XMP_NS_XMP_NOTE   = "http://ns.adobe.com/xmp/note/";

const char *XMP_NS_CRS		  = "http://ns.adobe.com/camera-raw-settings/1.0/";
const char *XMP_NS_CRSS		  = "http://ns.adobe.com/camera-raw-saved-settings/1.0/";
const char *XMP_NS_AUX		  = "http://ns.adobe.com/exif/1.0/aux/";

const char *XMP_NS_IPTC		  = "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/";

const char *XMP_NS_CRX 		  = "http://ns.adobe.com/lightroom-settings-experimental/1.0/";

/******************************************************************************/

#define CATCH_XMP(routine, fatal)\
	\
	catch (std::bad_alloc &)\
		{\
		DNG_REPORT ("Info: XMP " routine " threw memory exception");\
		ThrowMemoryFull ();\
		}\
	\
	catch (XMP_Error &error)\
		{\
		const char *errMessage = error.GetErrMsg ();\
		if (errMessage && strlen (errMessage) <= 128)\
			{\
			char errBuffer [256];\
			sprintf (errBuffer, "Info: XMP " routine " threw '%s' exception", errMessage);\
			DNG_REPORT ( errBuffer);\
			}\
		else\
			{\
			DNG_REPORT ("Info: XMP " routine " threw unnamed exception");\
			}\
		if (fatal) ThrowProgramError ();\
		}\
	\
	catch (...)\
		{\
		DNG_REPORT ("Info: XMP " routine " threw unknown exception");\
		if (fatal) ThrowProgramError ();\
		}

/*****************************************************************************/

class dng_xmp_private
	{

	public:

		SXMPMeta *fMeta;

		dng_xmp_private ()
			:	fMeta (NULL)
			{
			}

		dng_xmp_private (const dng_xmp_private &xmp);

		~dng_xmp_private ()
			{
			if (fMeta)
				{
				delete fMeta;
				}
			}

	private:

		// Hidden assignment operator.

		dng_xmp_private & operator= (const dng_xmp_private &xmp);

	};

/*****************************************************************************/

dng_xmp_private::dng_xmp_private (const dng_xmp_private &xmp)

	:	fMeta (NULL)

	{

	if (xmp.fMeta)
		{

		fMeta = new SXMPMeta (*xmp.fMeta);

		if (!fMeta)
			{
			ThrowMemoryFull ();
			}

		}

	}

/*****************************************************************************/

dng_xmp_sdk::dng_xmp_sdk ()

	:	fPrivate (NULL)

	{

	fPrivate = new dng_xmp_private;

	if (!fPrivate)
		{
		ThrowMemoryFull ();
		}

	}

/*****************************************************************************/

dng_xmp_sdk::dng_xmp_sdk (const dng_xmp_sdk &sdk)

	:	fPrivate (NULL)

	{

	fPrivate = new dng_xmp_private (*sdk.fPrivate);

	if (!fPrivate)
		{
		ThrowMemoryFull ();
		}

	}

/*****************************************************************************/

dng_xmp_sdk::~dng_xmp_sdk ()
	{

	if (fPrivate)
		{
		delete fPrivate;
		}

	}

/*****************************************************************************/

static bool gInitializedXMP = false;

/*****************************************************************************/

void dng_xmp_sdk::InitializeSDK (dng_xmp_namespace * extraNamespaces)
	{

	if (!gInitializedXMP)
		{

		try
			{

			if (!SXMPMeta::Initialize ())
				{
				ThrowProgramError ();
				}

			// Register Lightroom beta settings namespace.
			// We no longer read this but I don't want to cut it out this close
			// to a release. [bruzenak]

				{

				TXMP_STRING_TYPE ss;

				SXMPMeta::RegisterNamespace (XMP_NS_CRX,
											 "crx",
											 &ss);

				}

			// Register CRSS snapshots namespace

				{

				TXMP_STRING_TYPE ss;

				SXMPMeta::RegisterNamespace (XMP_NS_CRSS,
											 "crss",
											 &ss);

				}

			// Register extra namespaces.

			if (extraNamespaces != NULL)
				{

				for (; extraNamespaces->fullName != NULL; ++extraNamespaces)
					{

					TXMP_STRING_TYPE ss;

					SXMPMeta::RegisterNamespace (extraNamespaces->fullName,
												 extraNamespaces->shortName,
												 &ss);

					}

				}

			}

		CATCH_XMP ("Initialization", true)

	    gInitializedXMP = true;

		}

	}

/******************************************************************************/

void dng_xmp_sdk::TerminateSDK ()
	{

	if (gInitializedXMP)
		{

		try
			{

			SXMPMeta::Terminate ();

			}

		catch (...)
			{

			}

		gInitializedXMP = false;

		}

	}

/******************************************************************************/

bool dng_xmp_sdk::HasMeta () const
	{

	if (fPrivate->fMeta)
		{

		return true;

		}

	return false;

	}

/******************************************************************************/

void dng_xmp_sdk::ClearMeta ()
	{

	if (HasMeta ())
		{

		delete fPrivate->fMeta;

		fPrivate->fMeta = NULL;

		}

	}

/******************************************************************************/

void dng_xmp_sdk::MakeMeta ()
	{

	ClearMeta ();

	InitializeSDK ();

	try
		{

		fPrivate->fMeta = new SXMPMeta;

		if (!fPrivate->fMeta)
			{

			ThrowMemoryFull ();

			}

		}

	CATCH_XMP ("MakeMeta", true)

	}

/******************************************************************************/

void dng_xmp_sdk::NeedMeta ()
	{

	if (!HasMeta ())
		{

		MakeMeta ();

		}

	}

/******************************************************************************/

void dng_xmp_sdk::Parse (dng_host &host,
						 const char *buffer,
						 uint32 count)
	{

	MakeMeta ();

	try
		{

		try
			{

			fPrivate->fMeta->ParseFromBuffer (buffer, count);

			}

	    CATCH_XMP ("ParseFromBuffer", true)

	    }

	catch (dng_exception &except)
		{

		ClearMeta ();

		if (host.IsTransientError (except.ErrorCode ()))
			{

			throw;

			}

		ThrowBadFormat ();

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::AppendArrayItem (const char *ns,
								   const char *arrayName,
								   const char *itemValue,
								   bool isBag,
								   bool propIsStruct)
	{

	NeedMeta();

	try
		{

		fPrivate->fMeta->AppendArrayItem (ns,
										  arrayName,
										  isBag ? kXMP_PropValueIsArray
												: kXMP_PropArrayIsOrdered,
										  itemValue,
										  propIsStruct ? kXMP_PropValueIsStruct
													   : 0);

		}
	CATCH_XMP ("AppendArrayItem", true )

	}

/*****************************************************************************/

int32 dng_xmp_sdk::CountArrayItems (const char *ns,
								    const char *path) const
	{

	if (HasMeta ())
		{

		try
			{

			return fPrivate->fMeta->CountArrayItems (ns, path);

			}

		CATCH_XMP ("CountArrayItems", false)

		}

	return 0;

	}

/*****************************************************************************/

bool dng_xmp_sdk::Exists (const char *ns,
					 	  const char *path) const
	{

	if (HasMeta ())
		{

		try
			{

			return fPrivate->fMeta->DoesPropertyExist (ns, path);

			}

		catch (...)
			{

			// Does not exist...

			}

		}

	return false;

	}

/*****************************************************************************/

bool dng_xmp_sdk::HasNameSpace (const char *ns) const
	{

	bool result = false;

	if (HasMeta ())
		{

		try
			{

			SXMPIterator iter (*fPrivate->fMeta, ns);

			TXMP_STRING_TYPE ns;
			TXMP_STRING_TYPE prop;

			if (iter.Next (&ns,
						   &prop,
						   NULL,
						   NULL))
				{

				result = true;

				}

			}

		CATCH_XMP ("HasNameSpace", true)

		}

	return result;

	}

/*****************************************************************************/

void dng_xmp_sdk::Remove (const char *ns,
				     	  const char *path)
	{

	if (HasMeta ())
		{

		try
			{

			fPrivate->fMeta->DeleteProperty (ns, path);

			}

		CATCH_XMP ("DeleteProperty", false)

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::RemoveProperties (const char *ns)
	{

	if (HasMeta ())
		{

		try
			{

			SXMPUtils::RemoveProperties (fPrivate->fMeta,
										 ns,
										 NULL,
										 kXMPUtil_DoAllProperties);

			}

		catch (...)
			{

			}

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::ComposeArrayItemPath (const char *ns,
										const char *arrayName,
										int32 index,
										dng_string &s) const
	{

	try
		{

		std::string ss;

		SXMPUtils::ComposeArrayItemPath (ns, arrayName, index, &ss);

		s.Set (ss.c_str ());

		return;

		}

	CATCH_XMP ("ComposeArrayItemPath", true)

	}

/*****************************************************************************/

void dng_xmp_sdk::ComposeStructFieldPath (const char *ns,
										  const char *structName,
										  const char *fieldNS,
										  const char *fieldName,
										  dng_string &s) const
	{

	try
		{

		std::string ss;

		SXMPUtils::ComposeStructFieldPath (ns,
										   structName,
										   fieldNS,
										   fieldName,
										   &ss);

		s.Set (ss.c_str ());

		return;

		}

	CATCH_XMP ("ComposeStructFieldPath", true)

	}

/*****************************************************************************/

bool dng_xmp_sdk::GetNamespacePrefix (const char *uri,
									  dng_string &s) const
	{

	bool result = false;

	if (HasMeta ())
		{

		try
			{

			std::string ss;

			fPrivate->fMeta->GetNamespacePrefix (uri, &ss);

			s.Set (ss.c_str ());

			result = true;

			}

		CATCH_XMP ("GetNamespacePrefix", false)

		}

	return result;

	}

/*****************************************************************************/

bool dng_xmp_sdk::GetString (const char *ns,
				   		     const char *path,
				   		     dng_string &s) const
	{

	bool result = false;

	if (HasMeta ())
		{

		try
			{

			TXMP_STRING_TYPE ss;

			if (fPrivate->fMeta->GetProperty (ns, path, &ss, NULL))
				{

				s.Set (ss.c_str ());

				result = true;

				}

			}

		CATCH_XMP ("GetProperty", false)

		}

	return result;

	}

/*****************************************************************************/

void dng_xmp_sdk::ValidateStringList (const char *ns,
								      const char *path)
	{

	if (Exists (ns, path))
		{

		bool bogus = true;

		try
			{

			XMP_Index index = 1;

			TXMP_STRING_TYPE ss;

			while (fPrivate->fMeta->GetArrayItem (ns,
												  path,
												  index++,
												  &ss,
												  NULL))
				{

				}

			bogus = false;

			}

		CATCH_XMP ("GetArrayItem", false)

		if (bogus)
			{

			Remove (ns, path);

			}

		}

	}

/*****************************************************************************/

bool dng_xmp_sdk::GetStringList (const char *ns,
								 const char *path,
								 dng_string_list &list) const
	{

	bool result = false;

	if (HasMeta ())
		{

		try
			{

			XMP_Index index = 1;

			TXMP_STRING_TYPE ss;

			while (fPrivate->fMeta->GetArrayItem (ns,
												  path,
												  index++,
												  &ss,
												  NULL))
				{

				dng_string s;

				s.Set (ss.c_str ());

				list.Append (s);

				result = true;

				}

			}

		CATCH_XMP ("GetArrayItem", false)

		}

	return result;

	}

/*****************************************************************************/

bool dng_xmp_sdk::GetAltLangDefault (const char *ns,
									 const char *path,
									 dng_string &s) const
	{

	bool result = false;

	if (HasMeta ())
		{

		try
			{

			TXMP_STRING_TYPE ss;

			if (fPrivate->fMeta->GetLocalizedText (ns,
												   path,
												   "x-default",
									  	   		   "x-default",
												   NULL,
									  	  		   &ss,
												   NULL))
				{

				s.Set (ss.c_str ());

				result = true;

				}

			}

		CATCH_XMP ("GetLocalizedText", false)

		}

	return result;

	}

/*****************************************************************************/

bool dng_xmp_sdk::GetStructField (const char *ns,
								  const char *path,
								  const char *fieldNS,
								  const char *fieldName,
								  dng_string &s) const
	{

	bool result = false;

	if (HasMeta ())
		{

		try
			{

			TXMP_STRING_TYPE ss;

			if (fPrivate->fMeta->GetStructField (ns,
												 path,
												 fieldNS,
												 fieldName,
												 &ss,
												 NULL))
				{

				s.Set (ss.c_str ());

				result = true;

				}

			}

		CATCH_XMP ("GetStructField", false)

		}

	return result;

	}

/*****************************************************************************/

void dng_xmp_sdk::Set (const char *ns,
				  	   const char *path,
				  	   const char *text)
	{

	NeedMeta ();

	try
		{

		fPrivate->fMeta->SetProperty (ns, path, text);

		return;

		}

	catch (...)
		{

		// Failed for some reason.

		}

	// Remove existing value and try again.

	Remove (ns, path);

	try
		{

		fPrivate->fMeta->SetProperty (ns, path, text);

		}

	CATCH_XMP ("SetProperty", true)

	}

/*****************************************************************************/

void dng_xmp_sdk::SetString (const char *ns,
				  			 const char *path,
				  			 const dng_string &s)
	{

	dng_string ss (s);

	ss.SetLineEndings ('\n');

	ss.StripLowASCII ();

	Set (ns, path, ss.Get ());

	}

/*****************************************************************************/

void dng_xmp_sdk::SetStringList (const char *ns,
				  		    	 const char *path,
				  		    	 const dng_string_list &list,
				  		    	 bool isBag)
	{

	// Remove any existing structure.

	Remove (ns, path);

	// If list is not empty, add the items.

	if (list.Count ())
		{

		NeedMeta ();

		for (uint32 index = 0; index < list.Count (); index++)
			{

			dng_string s (list [index]);

			s.SetLineEndings ('\n');

			s.StripLowASCII ();

			try
				{

				fPrivate->fMeta->AppendArrayItem (ns,
												  path,
												  isBag ? kXMP_PropValueIsArray
														: kXMP_PropArrayIsOrdered,
												  s.Get ());

				}

			CATCH_XMP ("AppendArrayItem", true)

			}

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::SetAltLangDefault (const char *ns,
									 const char *path,
									 const dng_string &s)
	{

	NeedMeta ();

	Remove (ns, path);

	dng_string ss (s);

	ss.SetLineEndings ('\n');

	ss.StripLowASCII ();

	try
		{

		fPrivate->fMeta->SetLocalizedText (ns,
									  	   path,
									  	   "x-default",
									  	   "x-default",
									  	   ss.Get ());

		}

	CATCH_XMP ("SetLocalizedText", true)

	}

/*****************************************************************************/

void dng_xmp_sdk::SetStructField (const char *ns,
								  const char *path,
								  const char *fieldNS,
								  const char *fieldName,
								  const char *text)
	{

	NeedMeta ();

	try
		{

		fPrivate->fMeta->SetStructField (ns,
							  			 path,
							  			 fieldNS,
							  			 fieldName,
							  			 text);

		}

	CATCH_XMP ("SetStructField", true)

	}

/*****************************************************************************/

void dng_xmp_sdk::DeleteStructField (const char *ns,
									 const char *structName,
									 const char *fieldNS,
						             const char *fieldName)
	{

	if (HasMeta ())
		{

		try
			{

			fPrivate->fMeta->DeleteStructField (ns, structName, fieldNS, fieldName);

			}

		catch (...)
			{

			}

		}

	}

/*****************************************************************************/

dng_memory_block * dng_xmp_sdk::Serialize (dng_memory_allocator &allocator,
									       bool asPacket,
									       uint32 targetBytes,
									       uint32 padBytes,
									       bool forJPEG) const
	{

	// The largest XMP packet you can embed in JPEG using normal methods:

	const uint32 kJPEG_XMP_Limit = 65504;

	if (HasMeta ())
		{

		TXMP_STRING_TYPE s;

		bool havePacket = false;

		uint32 formatOption = forJPEG ? kXMP_UseCompactFormat : 0;

	    if (asPacket && targetBytes)
	    	{

	    	try
	    		{

	    		fPrivate->fMeta->SerializeToBuffer (&s,
	    											formatOption | kXMP_ExactPacketLength,
	    											targetBytes,
	    											"",
													" ");

				havePacket = true;

	    		}

	    	catch (...)
	    		{

	    		// Most likely the packet cannot fit in the target
	    		// byte count.  So try again without the limit.

	    		}

	    	}

		if (!havePacket)
			{

			try
				{

				fPrivate->fMeta->SerializeToBuffer (&s,
													formatOption |
													(asPacket ? 0
															  : kXMP_OmitPacketWrapper),
													(asPacket ? padBytes
															  : 0),
													"",
													" ");

				}

			CATCH_XMP ("SerializeToBuffer", true)

			}

		uint32 packetLen = (uint32) s.size ();

		if (forJPEG && asPacket && padBytes > 0 && targetBytes <= kJPEG_XMP_Limit &&
												   packetLen   >  kJPEG_XMP_Limit)
			{

			uint32 overLimitCount = packetLen - kJPEG_XMP_Limit;

			if (overLimitCount > padBytes)
				{
				padBytes = 0;
				}
			else
				{
				padBytes -= overLimitCount;
				}

			try
				{

				fPrivate->fMeta->SerializeToBuffer (&s,
													formatOption,
													padBytes,
													"",
													" ");

				}

			CATCH_XMP ("SerializeToBuffer", true)

			packetLen = (uint32) s.size ();

			}

		if (packetLen)
			{

			AutoPtr<dng_memory_block> buffer (allocator.Allocate (packetLen));

			memcpy (buffer->Buffer (), s.c_str (), packetLen);

			return buffer.Release ();

			}

		}

	return NULL;

	}

/*****************************************************************************/

void dng_xmp_sdk::PackageForJPEG (dng_memory_allocator &allocator,
								  AutoPtr<dng_memory_block> &stdBlock,
								  AutoPtr<dng_memory_block> &extBlock,
								  dng_string &extDigest) const
	{

	if (HasMeta ())
		{

		TXMP_STRING_TYPE stdStr;
		TXMP_STRING_TYPE extStr;
		TXMP_STRING_TYPE digestStr;

		try
			{

			SXMPUtils::PackageForJPEG (*fPrivate->fMeta,
									   &stdStr,
									   &extStr,
									   &digestStr);

			}

		CATCH_XMP ("PackageForJPEG", true)

		uint32 stdLen = (uint32) stdStr.size ();
		uint32 extLen = (uint32) extStr.size ();

		if (stdLen)
			{

			stdBlock.Reset (allocator.Allocate (stdLen));

			memcpy (stdBlock->Buffer (), stdStr.c_str (), stdLen);

			}

		if (extLen)
			{

			extBlock.Reset (allocator.Allocate (extLen));

			memcpy (extBlock->Buffer (), extStr.c_str (), extLen);

			if (digestStr.size () != 32)
				{
				ThrowProgramError ();
				}

			extDigest.Set (digestStr.c_str ());

			}

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::MergeFromJPEG (const dng_xmp_sdk *xmp)
	{

	if (xmp && xmp->HasMeta ())
		{

		NeedMeta ();

		try
			{

			SXMPUtils::MergeFromJPEG (fPrivate->fMeta,
									  *xmp->fPrivate->fMeta);

			}

		CATCH_XMP ("MergeFromJPEG", true)

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::AppendXMP (const dng_xmp_sdk *xmp)
	{

	if (xmp && xmp->HasMeta ())
		{

		NeedMeta ();

		try
			{

			SXMPUtils::AppendProperties (*xmp->fPrivate->fMeta,
									     fPrivate->fMeta,
										 kXMPUtil_DoAllProperties |
										 kXMPUtil_ReplaceOldValues);

			}

		CATCH_XMP ("AppendProperties", true)

		}

	}

/*****************************************************************************/

void dng_xmp_sdk::ReplaceXMP (dng_xmp_sdk *xmp)
	{

	ClearMeta ();

	if (xmp && xmp->HasMeta ())
		{

		fPrivate->fMeta = xmp->fPrivate->fMeta;

		xmp->fPrivate->fMeta = NULL;

		}

	}

/*****************************************************************************/

bool dng_xmp_sdk::IteratePaths (IteratePathsCallback *callback,
						        void *callbackData,
								const char* startingNS,
								const char* startingPath)
	{

	if (HasMeta ())
		{

		try
			{

			SXMPIterator iter (*fPrivate->fMeta, startingNS, startingPath);

			TXMP_STRING_TYPE ns;
			TXMP_STRING_TYPE prop;

			while (iter.Next (&ns,
							  &prop,
							  NULL,
							  NULL))
				{

				if (!callback (ns  .c_str (),
						  	   prop.c_str (),
						  	   callbackData))
					{

					return false;

					}

				}

			}

		CATCH_XMP ("IteratePaths", true)

		}

	return true;

	}

/*****************************************************************************/
