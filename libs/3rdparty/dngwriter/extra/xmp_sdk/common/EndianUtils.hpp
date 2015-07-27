#ifndef __EndianUtils_hpp__
#define __EndianUtils_hpp__ 1

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include.
#include "XMP_Const.h"

// *** These should be in a more common location. The Unicode conversions of XMPCore have similar utils. 
// *** May want to improve with PowerPC swapping load/store, or SSE instructions.

// =================================================================================================
namespace DngXmpSdk { 
#define kLittleEndianHost (! kBigEndianHost)
#if XMP_WinBuild
	#pragma warning ( disable : 4127 )	// conditional expression is constant
	#define kBigEndianHost 0
#elif XMP_MacBuild
	#if __BIG_ENDIAN__
		#define kBigEndianHost 1
	#elif __LITTLE_ENDIAN__
		#define kBigEndianHost 0
	#else
		#error "Neither __BIG_ENDIAN__ nor __LITTLE_ENDIAN__ is set"
	#endif
#elif XMP_UNIXBuild
	#ifndef kBigEndianHost	// Typically in the makefile for generic UNIX.
		#if __GNUC__ && (__i386__ || __x86_64__)
			#define kBigEndianHost 0
		#else
			#error "Must define kBigEndianHost as 0 or 1 in the makefile."
		#endif
	#endif
#else
	#error "Unknown build environment"
#endif

// =================================================================================================

typedef XMP_Uns16 (*GetUns16_Proc)  ( const void* addr );
typedef XMP_Uns32 (*GetUns32_Proc)  ( const void* addr );
typedef XMP_Uns64 (*GetUns64_Proc)  ( const void* addr );
typedef float     (*GetFloat_Proc)  ( const void* addr );
typedef double    (*GetDouble_Proc) ( const void* addr );

typedef XMP_Uns16 (*MakeUns16_Proc)  ( XMP_Uns16 value );
typedef XMP_Uns32 (*MakeUns32_Proc)  ( XMP_Uns32 value );
typedef XMP_Uns64 (*MakeUns64_Proc)  ( XMP_Uns64 value );
typedef float     (*MakeFloat_Proc)  ( float value );
typedef double    (*MakeDouble_Proc) ( double value );

typedef void (*PutUns16_Proc)  ( XMP_Uns16 value, void* addr );
typedef void (*PutUns32_Proc)  ( XMP_Uns32 value, void* addr );
typedef void (*PutUns64_Proc)  ( XMP_Uns64 value, void* addr );
typedef void (*PutFloat_Proc)  ( float value, void* addr );
typedef void (*PutDouble_Proc) ( double value, void* addr );

// =================================================================================================

static inline XMP_Uns16 Flip2 ( XMP_Uns16 value )
{
	value = ((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8);
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline void Flip2 ( void * addr )
{
	XMP_Uns16 * u16Ptr = (XMP_Uns16*) addr;
	*u16Ptr = Flip2 ( *u16Ptr );
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns32 Flip4 ( XMP_Uns32 value )
{
	value = ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
			((value >> 8) & 0x0000FF00)  | ((value >> 24) & 0x000000FF);
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline void Flip4 ( void * addr )
{
	XMP_Uns32 * u32Ptr = (XMP_Uns32*) addr;
	*u32Ptr = Flip4 ( *u32Ptr );
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns64 Flip8 ( XMP_Uns64 value )
{
	XMP_Uns32 oldLow  = (XMP_Uns32)(value);
	XMP_Uns32 oldHigh = (XMP_Uns32)(value >> 32);
	return (((XMP_Uns64)Flip4(oldLow)) << 32) | ((XMP_Uns64)Flip4(oldHigh));
}

// -------------------------------------------------------------------------------------------------

static inline void Flip8 ( void * addr )
{
	XMP_Uns64 * u64Ptr = (XMP_Uns64*) addr;
	*u64Ptr = Flip8 ( *u64Ptr );
}

// =================================================================================================

static inline XMP_Uns16
GetUns16BE ( const void * addr )
{
	XMP_Uns16 value = *((XMP_Uns16*)addr);
	if ( kLittleEndianHost ) value = Flip2 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns16
GetUns16LE ( const void * addr )
{
	XMP_Uns16 value = *((XMP_Uns16*)addr);
	if ( kBigEndianHost ) value = Flip2 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns32
GetUns32BE ( const void * addr )
{
	XMP_Uns32 value = *((XMP_Uns32*)addr);
	if ( kLittleEndianHost ) value = Flip4 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns32
GetUns32LE ( const void * addr )
{
	XMP_Uns32 value = *((XMP_Uns32*)addr);
	if ( kBigEndianHost ) value = Flip4 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns64
GetUns64BE ( const void * addr )
{
	XMP_Uns64 value = *((XMP_Uns64*)addr);
	if ( kLittleEndianHost ) value = Flip8 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns64
GetUns64LE ( const void * addr )
{
	XMP_Uns64 value = *((XMP_Uns64*)addr);
	if ( kBigEndianHost ) value = Flip8 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline float
GetFloatBE ( const void * addr )
{
	XMP_Uns32 value = *((XMP_Uns32*)addr);
	if ( kLittleEndianHost ) value = Flip4 ( value );
	return *((float*)&value);
}

// -------------------------------------------------------------------------------------------------

static inline float
GetFloatLE ( const void * addr )
{
	XMP_Uns32 value = *((XMP_Uns32*)addr);
	if ( kBigEndianHost ) value = Flip4 ( value );
	return *((float*)&value);
}

// -------------------------------------------------------------------------------------------------

static inline double
GetDoubleBE ( const void * addr )
{
	XMP_Uns64 value = *((XMP_Uns64*)addr);
	if ( kLittleEndianHost ) value = Flip8 ( value );
	return *((double*)&value);
}

// -------------------------------------------------------------------------------------------------

static inline double
GetDoubleLE ( const void * addr )
{
	XMP_Uns64 value = *((XMP_Uns64*)addr);
	if ( kBigEndianHost ) value = Flip8 ( value );
	return *((double*)&value);
}

// =================================================================================================

static inline XMP_Uns16
MakeUns16BE ( XMP_Uns16 value )
{
	if ( kLittleEndianHost ) value = Flip2 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns16
MakeUns16LE ( XMP_Uns16 value )
{
	if ( kBigEndianHost ) value = Flip2 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns32
MakeUns32BE ( XMP_Uns32 value )
{
	if ( kLittleEndianHost ) value = Flip4 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns32
MakeUns32LE ( XMP_Uns32 value )
{
	if ( kBigEndianHost ) value = Flip4 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns64
MakeUns64BE ( XMP_Uns64 value )
{
	if ( kLittleEndianHost ) value = Flip8 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline XMP_Uns64
MakeUns64LE ( XMP_Uns64 value )
{
	if ( kBigEndianHost ) value = Flip8 ( value );
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline float
MakeFloatBE ( float value )
{
	if ( kLittleEndianHost ) {
		XMP_Uns32* intPtr = (XMP_Uns32*) &value;
		*intPtr = Flip4 ( *intPtr );
	}
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline float
MakeFloatLE ( float value )
{
	if ( kBigEndianHost ) {
		XMP_Uns32* intPtr = (XMP_Uns32*) &value;
		*intPtr = Flip4 ( *intPtr );
	}
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline double
MakeDoubleBE ( double value )
{
	if ( kLittleEndianHost ) {
		XMP_Uns64* intPtr = (XMP_Uns64*) &value;
		*intPtr = Flip8 ( *intPtr );
	}
	return value;
}

// -------------------------------------------------------------------------------------------------

static inline double
MakeDoubleLE ( double value )
{
	if ( kBigEndianHost ) {
		XMP_Uns64* intPtr = (XMP_Uns64*) &value;
		*intPtr = Flip8 ( *intPtr );
	}
	return value;
}

// =================================================================================================

static inline void
PutUns16BE ( XMP_Uns16 value, void * addr )
{
	if ( kLittleEndianHost ) value = Flip2 ( value );
	*((XMP_Uns16*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutUns16LE ( XMP_Uns16 value, void * addr )
{
	if ( kBigEndianHost ) value = Flip2 ( value );
	*((XMP_Uns16*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutUns32BE ( XMP_Uns32 value, void * addr )
{
	if ( kLittleEndianHost ) value = Flip4 ( value );
	*((XMP_Uns32*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutUns32LE ( XMP_Uns32 value, void * addr )
{
	if ( kBigEndianHost ) value = Flip4 ( value );
	*((XMP_Uns32*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutUns64BE ( XMP_Uns64 value, void * addr )
{
	if ( kLittleEndianHost ) value = Flip8 ( value );
	*((XMP_Uns64*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutUns64LE ( XMP_Uns64 value, void * addr )
{
	if ( kBigEndianHost ) value = Flip8 ( value );
	*((XMP_Uns64*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutFloatBE ( float value, void * addr )
{
	if ( kLittleEndianHost ) {
		XMP_Uns32* intPtr = (XMP_Uns32*) &value;
		*intPtr = Flip4 ( *intPtr );
	}
	*((float*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutFloatLE ( float value, void * addr )
{
	if ( kBigEndianHost ) {
		XMP_Uns32* intPtr = (XMP_Uns32*) &value;
		*intPtr = Flip4 ( *intPtr );
	}
	*((float*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutDoubleBE ( double value, void * addr )
{
	if ( kLittleEndianHost ) {
		XMP_Uns64* intPtr = (XMP_Uns64*) &value;
		*intPtr = Flip8 ( *intPtr );
	}
	*((double*)addr) = value;
}

// -------------------------------------------------------------------------------------------------

static inline void
PutDoubleLE ( double value, void * addr )
{
	if ( kBigEndianHost ) {
		XMP_Uns64* intPtr = (XMP_Uns64*) &value;
		*intPtr = Flip8 ( *intPtr );
	}
	*((double*)addr) = value;
}

// =================================================================================================
} //namespace

#endif	// __EndianUtils_hpp__
