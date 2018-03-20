/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_memory.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** Support for memory allocation.
 */

/*****************************************************************************/

#ifndef __dng_memory__
#define __dng_memory__

/*****************************************************************************/

#include "dng_types.h"

/*****************************************************************************/

/// \brief Class to provide resource acquisition is instantiation discipline
/// for small memory allocations.
///
/// This class does not use dng_memory_allocator for memory allocation.

class dng_memory_data
	{

	private:

		void *fBuffer;

	public:

		/// Construct an empty memory buffer using malloc.
		/// \exception dng_memory_full with fErrorCode equal to dng_error_memory.

		dng_memory_data ();

		/// Construct memory buffer of size bytes using malloc.
		/// \param size Number of bytes of memory needed.
		/// \exception dng_memory_full with fErrorCode equal to dng_error_memory.

		dng_memory_data (uint32 size);

		/// Release memory buffer using free.

		~dng_memory_data ();

		/// Clear existing memory buffer and allocate new memory of size bytes.
		/// \param size Number of bytes of memory needed.
		/// \exception dng_memory_full with fErrorCode equal to dng_error_memory.

		void Allocate (uint32 size);

		/// Release any allocated memory using free. Object is still valid and
		/// Allocate can be called again.

		void Clear ();

		/// Return pointer to allocated memory as a void *..
		/// \retval void * valid for as many bytes as were allocated.

		void * Buffer ()
			{
			return fBuffer;
			}

		/// Return pointer to allocated memory as a const void *.
		/// \retval const void * valid for as many bytes as were allocated.

		const void * Buffer () const
			{
			return fBuffer;
			}

		/// Return pointer to allocated memory as a char *.
		/// \retval char * valid for as many bytes as were allocated.

		char * Buffer_char ()
			{
			return (char *) Buffer ();
			}

		/// Return pointer to allocated memory as a const char *.
		/// \retval const char * valid for as many bytes as were allocated.

		const char * Buffer_char () const
			{
			return (const char *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint8 *.
		/// \retval uint8 * valid for as many bytes as were allocated.

		uint8 * Buffer_uint8 ()
			{
			return (uint8 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const uint8 *.
		/// \retval const uint8 * valid for as many bytes as were allocated.

		const uint8 * Buffer_uint8 () const
			{
			return (const uint8 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint16 *.
		/// \retval uint16 * valid for as many bytes as were allocated.

		uint16 * Buffer_uint16 ()
			{
			return (uint16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const uint16 *.
		/// \retval const uint16 * valid for as many bytes as were allocated.

		const uint16 * Buffer_uint16 () const
			{
			return (const uint16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a int16 *.
		/// \retval int16 * valid for as many bytes as were allocated.

		int16 * Buffer_int16 ()
			{
			return (int16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int16 *.
		/// \retval const int16 * valid for as many bytes as were allocated.

		const int16 * Buffer_int16 () const
			{
			return (const int16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint32 *.
		/// \retval uint32 * valid for as many bytes as were allocated.

		uint32 * Buffer_uint32 ()
			{
			return (uint32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint32 *.
		/// \retval uint32 * valid for as many bytes as were allocated.

		const uint32 * Buffer_uint32 () const
			{
			return (const uint32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int32 *.
		/// \retval const int32 * valid for as many bytes as were allocated.

		int32 * Buffer_int32 ()
			{
			return (int32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int32 *.
		/// \retval const int32 * valid for as many bytes as were allocated.

		const int32 * Buffer_int32 () const
			{
			return (const int32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint64 *.
		/// \retval uint64 * valid for as many bytes as were allocated.

		uint64 * Buffer_uint64 ()
			{
			return (uint64 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint64 *.
		/// \retval uint64 * valid for as many bytes as were allocated.

		const uint64 * Buffer_uint64 () const
			{
			return (const uint64 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int64 *.
		/// \retval const int64 * valid for as many bytes as were allocated.

		int64 * Buffer_int64 ()
			{
			return (int64 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int64 *.
		/// \retval const int64 * valid for as many bytes as were allocated.

		const int64 * Buffer_int64 () const
			{
			return (const int64 *) Buffer ();
			}

		/// Return pointer to allocated memory as a real32 *.
		/// \retval real32 * valid for as many bytes as were allocated.

		real32 * Buffer_real32 ()
			{
			return (real32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const real32 *.
		/// \retval const real32 * valid for as many bytes as were allocated.

		const real32 * Buffer_real32 () const
			{
			return (const real32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a real64 *.
		/// \retval real64 * valid for as many bytes as were allocated.

		real64 * Buffer_real64 ()
			{
			return (real64 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const real64 *.
		/// \retval const real64 * valid for as many bytes as were allocated.

		const real64 * Buffer_real64 () const
			{
			return (const real64 *) Buffer ();
			}

	private:

		// Hidden copy constructor and assignment operator.

		dng_memory_data (const dng_memory_data &data);

		dng_memory_data & operator= (const dng_memory_data &data);

	};

/*****************************************************************************/

/// \brief Class to provide resource acquisition is instantiation discipline for
/// image buffers and other larger memory allocations.
///
/// This class requires a dng_memory_allocator for allocation.

class dng_memory_block
	{

	private:

		uint32 fLogicalSize;

		void *fBuffer;

	protected:

		dng_memory_block (uint32 logicalSize)
			:	fLogicalSize (logicalSize)
			,	fBuffer (NULL)
			{
			}

		uint32 PhysicalSize ()
			{
			return fLogicalSize + 64;
			}

		void SetBuffer (void *p)
			{
			fBuffer = (void *) ((((uintptr) p) + 15) & ~((uintptr) 15));
			}

	public:

		virtual ~dng_memory_block ()
			{
			}

		/// Getter for available size, in bytes, of memory block.
		/// \retval size in bytes of available memory in memory block.

		uint32 LogicalSize () const
			{
			return fLogicalSize;
			}

		/// Return pointer to allocated memory as a void *..
		/// \retval void * valid for as many bytes as were allocated.

		void * Buffer ()
			{
			return fBuffer;
			}

		/// Return pointer to allocated memory as a const void *.
		/// \retval const void * valid for as many bytes as were allocated.

		const void * Buffer () const
			{
			return fBuffer;
			}

		/// Return pointer to allocated memory as a char *.
		/// \retval char * valid for as many bytes as were allocated.

		char * Buffer_char ()
			{
			return (char *) Buffer ();
			}

		/// Return pointer to allocated memory as a const char *.
		/// \retval const char * valid for as many bytes as were allocated.

		const char * Buffer_char () const
			{
			return (const char *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint8 *.
		/// \retval uint8 * valid for as many bytes as were allocated.

		uint8 * Buffer_uint8 ()
			{
			return (uint8 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const uint8 *.
		/// \retval const uint8 * valid for as many bytes as were allocated.

		const uint8 * Buffer_uint8 () const
			{
			return (const uint8 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint16 *.
		/// \retval uint16 * valid for as many bytes as were allocated.

		uint16 * Buffer_uint16 ()
			{
			return (uint16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const uint16 *.
		/// \retval const uint16 * valid for as many bytes as were allocated.

		const uint16 * Buffer_uint16 () const
			{
			return (const uint16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a int16 *.
		/// \retval int16 * valid for as many bytes as were allocated.

		int16 * Buffer_int16 ()
			{
			return (int16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int16 *.
		/// \retval const int16 * valid for as many bytes as were allocated.

		const int16 * Buffer_int16 () const
			{
			return (const int16 *) Buffer ();
			}

		/// Return pointer to allocated memory as a uint32 *.
		/// \retval uint32 * valid for as many bytes as were allocated.

		uint32 * Buffer_uint32 ()
			{
			return (uint32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const uint32 *.
		/// \retval const uint32 * valid for as many bytes as were allocated.

		const uint32 * Buffer_uint32 () const
			{
			return (const uint32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a int32 *.
		/// \retval int32 * valid for as many bytes as were allocated.

		int32 * Buffer_int32 ()
			{
			return (int32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const int32 *.
		/// \retval const int32 * valid for as many bytes as were allocated.

		const int32 * Buffer_int32 () const
			{
			return (const int32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a real32 *.
		/// \retval real32 * valid for as many bytes as were allocated.

		real32 * Buffer_real32 ()
			{
			return (real32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const real32 *.
		/// \retval const real32 * valid for as many bytes as were allocated.

		const real32 * Buffer_real32 () const
			{
			return (const real32 *) Buffer ();
			}

		/// Return pointer to allocated memory as a real64 *.
		/// \retval real64 * valid for as many bytes as were allocated.

		real64 * Buffer_real64 ()
			{
			return (real64 *) Buffer ();
			}

		/// Return pointer to allocated memory as a const real64 *.
		/// \retval const real64 * valid for as many bytes as were allocated.

		const real64 * Buffer_real64 () const
			{
			return (const real64 *) Buffer ();
			}

	private:

		// Hidden copy constructor and assignment operator.

		dng_memory_block (const dng_memory_block &data);

		dng_memory_block & operator= (const dng_memory_block &data);

	};

/*****************************************************************************/

/// \brief Interface for dng_memory_block allocator.

class dng_memory_allocator
	{

	public:

		virtual ~dng_memory_allocator ()
			{
			}

		/// Allocate a dng_memory block.
		/// \param size Number of bytes in memory block.
		/// \retval A dng_memory_block with at least size bytes of valid storage.
		/// \exception dng_exception with fErrorCode equal to dng_error_memory.

		virtual dng_memory_block * Allocate (uint32 size);

	};

/*****************************************************************************/

/// \brief Default memory allocator used if NULL is passed in for allocator
/// when constructing a dng_host.
///
/// Uses new and delete for memory block object and malloc/free for underlying
/// buffer.

extern dng_memory_allocator gDefaultDNGMemoryAllocator;

/*****************************************************************************/

#endif

/*****************************************************************************/
