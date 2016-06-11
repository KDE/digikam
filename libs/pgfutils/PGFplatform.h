/*
 * The Progressive Graphics File; http://www.libpgf.org
 *
 * $Date: 2007-06-12 19:27:47 +0200 (Di, 12 Jun 2007) $
 * $Revision: 307 $
 *
 * This file Copyright (C) 2006 xeraina GmbH, Switzerland
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//////////////////////////////////////////////////////////////////////
/// @file PGFplatform.h
/// @brief PGF platform specific definitions
/// @author C. Stamm

#ifndef PGF_PGFPLATFORM_H
#define PGF_PGFPLATFORM_H

#include <cassert>
#include <cmath>
#include <cstdlib>

//-------------------------------------------------------------------------------
// Endianess detection taken from lcms2 header.
// This list can be endless, so only some checks are performed over here.
//-------------------------------------------------------------------------------
#if defined(_HOST_BIG_ENDIAN) || defined(__BIG_ENDIAN__) || defined(WORDS_BIGENDIAN)
#define PGF_USE_BIG_ENDIAN 1
#endif

#if defined(__sgi__) || defined(__sgi) || defined(__powerpc__) || defined(__sparc) || defined(__sparc__)
#define PGF_USE_BIG_ENDIAN 1
#endif

#if defined(__ppc__) || defined(__s390__) || defined(__s390x__)
#define PGF_USE_BIG_ENDIAN 1
#endif

#ifdef TARGET_CPU_PPC
#define PGF_USE_BIG_ENDIAN 1
#endif

//-------------------------------------------------------------------------------
// ROI support
//-------------------------------------------------------------------------------
#ifndef NPGFROI
#define __PGFROISUPPORT__ // without ROI support the program code gets simpler and smaller
#endif

//-------------------------------------------------------------------------------
// 32 bit per channel support
//-------------------------------------------------------------------------------
#ifndef NPGF32
#define __PGF32SUPPORT__ // without 32 bit the memory consumption during encoding and decoding is much lesser
#endif

//-------------------------------------------------------------------------------
//	32 Bit platform constants
//-------------------------------------------------------------------------------
#define WordWidth			32					///< WordBytes*8
#define WordWidthLog		5					///< ld of WordWidth
#define WordMask			0xFFFFFFE0			///< least WordWidthLog bits are zero
#define WordBytes			4					///< sizeof(UINT32)
#define WordBytesMask		0xFFFFFFFC			///< least WordBytesLog bits are zero
#define WordBytesLog		2					///< ld of WordBytes

//-------------------------------------------------------------------------------
// Alignment macros (used in PGF based libraries)
//-------------------------------------------------------------------------------
#define DWWIDTHBITS(bits)	(((bits) + WordWidth - 1) & WordMask)		///< aligns scanline width in bits to DWORD value
#define DWWIDTH(bytes)		(((bytes) + WordBytes - 1) & WordBytesMask)	///< aligns scanline width in bytes to DWORD value
#define DWWIDTHREST(bytes)	((WordBytes - (bytes)%WordBytes)%WordBytes)	///< DWWIDTH(bytes) - bytes

//-------------------------------------------------------------------------------
// Min-Max macros
//-------------------------------------------------------------------------------
#ifndef __min
	#define __min(x, y)		((x) <= (y) ? (x) : (y))
	#define __max(x, y)		((x) >= (y) ? (x) : (y))
#endif // __min

//-------------------------------------------------------------------------------
//	Defines -- Adobe image modes.
//-------------------------------------------------------------------------------
#define ImageModeBitmap				0
#define ImageModeGrayScale			1
#define ImageModeIndexedColor		2
#define ImageModeRGBColor			3
#define ImageModeCMYKColor			4
#define ImageModeHSLColor			5
#define ImageModeHSBColor			6
#define ImageModeMultichannel		7
#define ImageModeDuotone			8
#define ImageModeLabColor			9
#define ImageModeGray16				10		// 565
#define ImageModeRGB48				11
#define ImageModeLab48				12
#define ImageModeCMYK64				13
#define ImageModeDeepMultichannel	14
#define ImageModeDuotone16			15
// pgf extension
#define ImageModeRGBA				17
#define ImageModeGray32				18		// MSB is 0 (can be interpreted as signed 15.16 fixed point format)
#define ImageModeRGB12				19
#define ImageModeRGB16				20
#define ImageModeUnknown			255


//-------------------------------------------------------------------------------
// WINDOWS
//-------------------------------------------------------------------------------
#if defined(WIN32) || defined(WINCE) || defined(WIN64) || defined(__MINGW32__)
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//-------------------------------------------------------------------------------
// MFC
//-------------------------------------------------------------------------------
#ifdef _MFC_VER
#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif
#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#else

#include <windows.h>
#include <ole2.h>

#endif // _MFC_VER
//-------------------------------------------------------------------------------

#define DllExport   __declspec( dllexport )

//-------------------------------------------------------------------------------
// unsigned number type definitions
//-------------------------------------------------------------------------------
typedef unsigned char		UINT8;
typedef unsigned char		BYTE;
typedef unsigned short		UINT16;
typedef unsigned short      WORD;
typedef	unsigned int		UINT32;
typedef unsigned long       DWORD;
typedef unsigned long       ULONG;
typedef unsigned __int64	UINT64;
typedef unsigned __int64	ULONGLONG;

//-------------------------------------------------------------------------------
// signed number type definitions
//-------------------------------------------------------------------------------
typedef signed char			INT8;
typedef signed short		INT16;
typedef signed int			INT32;
typedef signed int			BOOL;
typedef signed long			LONG;
typedef signed __int64		INT64;
typedef signed __int64		LONGLONG;

//-------------------------------------------------------------------------------
// other types
//-------------------------------------------------------------------------------
typedef int OSError;
typedef bool (__cdecl *CallbackPtr)(double percent, bool escapeAllowed, void *data);

//-------------------------------------------------------------------------------
// struct type definitions
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// DEBUG macros
//-------------------------------------------------------------------------------
#ifndef ASSERT
	#ifdef _DEBUG
		#define ASSERT(x)	assert(x)
	#else
		#if defined(__GNUC__)
			#define ASSERT(ignore)((void) 0)
		#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
			#define ASSERT		__noop
		#else
			#define ASSERT ((void)0)
		#endif
	#endif //_DEBUG
#endif //ASSERT

//-------------------------------------------------------------------------------
// Exception handling macros
//-------------------------------------------------------------------------------
#ifdef NEXCEPTIONS
	extern OSError _PGF_Error_;
	extern OSError GetLastPGFError();

	#define ReturnWithError(err) { _PGF_Error_ = err; return; }
	#define ReturnWithError2(err, ret) { _PGF_Error_ = err; return ret; }
#else
	#define ReturnWithError(err) throw IOException(err)
	#define ReturnWithError2(err, ret) throw IOException(err)
#endif //NEXCEPTIONS

//-------------------------------------------------------------------------------
// constants
//-------------------------------------------------------------------------------
#define FSFromStart		FILE_BEGIN				// 0
#define FSFromCurrent	FILE_CURRENT			// 1
#define FSFromEnd		FILE_END				// 2

#define INVALID_SET_FILE_POINTER ((DWORD)-1)

//-------------------------------------------------------------------------------
// IO Error constants
//-------------------------------------------------------------------------------
#define NoError				ERROR_SUCCESS		///< no error
#define AppError			0x20000000			///< all application error messages must be larger than this value
#define InsufficientMemory	0x20000001			///< memory allocation was not successfull
#define InvalidStreamPos	0x20000002			///< invalid memory stream position
#define EscapePressed		0x20000003			///< user break by ESC
#define WrongVersion		0x20000004			///< wrong PGF version
#define FormatCannotRead	0x20000005			///< wrong data file format
#define ImageTooSmall		0x20000006			///< image is too small
#define ZlibError			0x20000007			///< error in zlib functions
#define ColorTableError		0x20000008			///< errors related to color table size
#define PNGError			0x20000009			///< errors in png functions
#define MissingData			0x2000000A			///< expected data cannot be read

//-------------------------------------------------------------------------------
// methods
//-------------------------------------------------------------------------------
inline OSError FileRead(HANDLE hFile, int *count, void *buffPtr) {
	if (ReadFile(hFile, buffPtr, *count, (ULONG *)count, nullptr)) {
		return NoError;
	} else {
		return GetLastError();
	}
}

inline OSError FileWrite(HANDLE hFile, int *count, void *buffPtr) {
	if (WriteFile(hFile, buffPtr, *count, (ULONG *)count, nullptr)) {
		return NoError;
	} else {
		return GetLastError();
	}
}

inline OSError GetFPos(HANDLE hFile, UINT64 *pos) {
#ifdef WINCE
	LARGE_INTEGER li;
	li.QuadPart = 0;

	li.LowPart = SetFilePointer (hFile, li.LowPart, &li.HighPart, FILE_CURRENT);
	if (li.LowPart == INVALID_SET_FILE_POINTER) {
		OSError err = GetLastError();
		if (err != NoError) {
			return err;
		}
	}
	*pos = li.QuadPart;
	return NoError;
#else
	LARGE_INTEGER li;
	li.QuadPart = 0;
	if (SetFilePointerEx(hFile, li, (PLARGE_INTEGER)pos, FILE_CURRENT)) {
		return NoError;
	} else {
		return GetLastError();
	}
#endif
}

inline OSError SetFPos(HANDLE hFile, int posMode, INT64 posOff) {
#ifdef WINCE
	LARGE_INTEGER li;
	li.QuadPart = posOff;

	if (SetFilePointer (hFile, li.LowPart, &li.HighPart, posMode) == INVALID_SET_FILE_POINTER) {
		OSError err = GetLastError();
		if (err != NoError) {
			return err;
		}
	}
	return NoError;
#else
	LARGE_INTEGER li;
	li.QuadPart = posOff;
	if (SetFilePointerEx(hFile, li, nullptr, posMode)) {
		return NoError;
	} else {
		return GetLastError();
	}
#endif
}
#endif //WIN32


//-------------------------------------------------------------------------------
// Apple OSX
//-------------------------------------------------------------------------------
#ifdef __APPLE__
#define __POSIX__
#endif // __APPLE__


//-------------------------------------------------------------------------------
// LINUX
//-------------------------------------------------------------------------------
#if defined(__linux__) || defined(__GLIBC__)
#define __POSIX__
#endif // __linux__ or __GLIBC__


//-------------------------------------------------------------------------------
// SOLARIS
//-------------------------------------------------------------------------------
#ifdef __sun
#define __POSIX__
#endif // __sun


//-------------------------------------------------------------------------------
// *BSD
//-------------------------------------------------------------------------------
#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)
#ifndef __POSIX__
#define __POSIX__
#endif

#ifndef off64_t
#define off64_t off_t
#endif

#ifndef lseek64
#define lseek64 lseek
#endif

#endif // __NetBSD__ or __OpenBSD__ or __FreeBSD__


//-------------------------------------------------------------------------------
// POSIX *NIXes
//-------------------------------------------------------------------------------

#ifdef __POSIX__
#include <unistd.h>
#include <errno.h>
#include <stdint.h>		// for int64_t and uint64_t
#include <string.h>		// memcpy()

#undef major

//-------------------------------------------------------------------------------
// unsigned number type definitions
//-------------------------------------------------------------------------------

typedef unsigned char		UINT8;
typedef unsigned char		BYTE;
typedef unsigned short		UINT16;
typedef unsigned short		WORD;
typedef unsigned int		UINT32;
typedef unsigned int		DWORD;
typedef unsigned long		ULONG;
typedef unsigned long long  __Uint64;
typedef __Uint64			UINT64;
typedef __Uint64			ULONGLONG;

//-------------------------------------------------------------------------------
// signed number type definitions
//-------------------------------------------------------------------------------
typedef signed char			INT8;
typedef signed short		INT16;
typedef signed int			INT32;
typedef signed int			BOOL;
typedef signed long			LONG;
typedef int64_t				INT64;
typedef int64_t				LONGLONG;

//-------------------------------------------------------------------------------
// other types
//-------------------------------------------------------------------------------
typedef int					OSError;
typedef int					HANDLE;
typedef unsigned long		ULONG_PTR;
typedef void*				PVOID;
typedef char*				LPTSTR;
typedef bool (*CallbackPtr)(double percent, bool escapeAllowed, void *data);

//-------------------------------------------------------------------------------
// struct type definitions
//-------------------------------------------------------------------------------
typedef struct tagRGBTRIPLE {
	BYTE rgbtBlue;
	BYTE rgbtGreen;
	BYTE rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
#endif // __POSIX__


#if defined(__POSIX__) || defined(WINCE)
// CMYK macros
#define GetKValue(cmyk)      ((BYTE)(cmyk))
#define GetYValue(cmyk)      ((BYTE)((cmyk)>> 8))
#define GetMValue(cmyk)      ((BYTE)((cmyk)>>16))
#define GetCValue(cmyk)      ((BYTE)((cmyk)>>24))
#define CMYK(c,m,y,k)		 ((COLORREF)((((BYTE)(k)|((WORD)((BYTE)(y))<<8))|(((DWORD)(BYTE)(m))<<16))|(((DWORD)(BYTE)(c))<<24)))

//-------------------------------------------------------------------------------
// methods
//-------------------------------------------------------------------------------
/* The MulDiv function multiplies two 32-bit values and then divides the 64-bit
 * result by a third 32-bit value. The return value is rounded up or down to
 * the nearest integer.
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winprog/winprog/muldiv.asp
 * */
__inline int MulDiv(int nNumber, int nNumerator, int nDenominator) {
	INT64 multRes = nNumber*nNumerator;
	INT32 divRes = INT32(multRes/nDenominator);
	return divRes;
}
#endif // __POSIX__ or WINCE


#ifdef __POSIX__
//-------------------------------------------------------------------------------
// DEBUG macros
//-------------------------------------------------------------------------------
#ifndef ASSERT
	#ifdef _DEBUG
		#define ASSERT(x)	assert(x)
	#else
		#define ASSERT(x)
	#endif //_DEBUG
#endif //ASSERT

//-------------------------------------------------------------------------------
// Exception handling macros
//-------------------------------------------------------------------------------
#ifdef NEXCEPTIONS
	extern OSError _PGF_Error_;
	extern OSError GetLastPGFError();

	#define ReturnWithError(err) { _PGF_Error_ = err; return; }
	#define ReturnWithError2(err, ret) { _PGF_Error_ = err; return ret; }
#else
	#define ReturnWithError(err) throw IOException(err)
	#define ReturnWithError2(err, ret) throw IOException(err)
#endif //NEXCEPTIONS

#define THROW_ throw(IOException)
#define CONST const

//-------------------------------------------------------------------------------
// constants
//-------------------------------------------------------------------------------
#define FSFromStart			SEEK_SET
#define FSFromCurrent		SEEK_CUR
#define FSFromEnd			SEEK_END
#define nullptr				NULL

//-------------------------------------------------------------------------------
// IO Error constants
//-------------------------------------------------------------------------------
#define NoError					0x0000			///< no error
#define AppError				0x2000			///< all application error messages must be larger than this value
#define InsufficientMemory		0x2001			///< memory allocation wasn't successfull
#define InvalidStreamPos		0x2002			///< invalid memory stream position
#define EscapePressed			0x2003			///< user break by ESC
#define WrongVersion			0x2004			///< wrong pgf version
#define FormatCannotRead		0x2005			///< wrong data file format
#define ImageTooSmall			0x2006			///< image is too small
#define ZlibError				0x2007			///< error in zlib functions
#define ColorTableError			0x2008			///< errors related to color table size
#define PNGError				0x2009			///< errors in png functions
#define MissingData				0x200A			///< expected data cannot be read

//-------------------------------------------------------------------------------
// methods
//-------------------------------------------------------------------------------
__inline OSError FileRead(HANDLE hFile, int *count, void *buffPtr) {
	*count = (int)read(hFile, buffPtr, *count);
	if (*count != -1) {
		return NoError;
	} else {
		return errno;
	}
}

__inline OSError FileWrite(HANDLE hFile, int *count, void *buffPtr) {
	*count = (int)write(hFile, buffPtr, (size_t)*count);
	if (*count != -1) {
		return NoError;
	} else {
		return errno;
	}
}

__inline OSError GetFPos(HANDLE hFile, UINT64 *pos) {
	#ifdef __APPLE__
		off_t ret;
		if ((ret = lseek(hFile, 0, SEEK_CUR)) == -1) {
			return errno;
		} else {
			*pos = (UINT64)ret;
			return NoError;
		}
	#else
		off64_t ret;
		if ((ret = lseek64(hFile, 0, SEEK_CUR)) == -1) {
			return errno;
		} else {
			*pos = (UINT64)ret;
			return NoError;
		}
	#endif
}

__inline OSError SetFPos(HANDLE hFile, int posMode, INT64 posOff) {
	#ifdef __APPLE__
		if ((lseek(hFile, (off_t)posOff, posMode)) == -1) {
			return errno;
		} else {
			return NoError;
		}
	#else
		if ((lseek64(hFile, (off64_t)posOff, posMode)) == -1) {
			return errno;
		} else {
			return NoError;
		}
	#endif
}

#endif /* __POSIX__ */
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//	Big Endian
//-------------------------------------------------------------------------------
#ifdef PGF_USE_BIG_ENDIAN

#ifndef _lrotl
	#define _lrotl(x,n)	(((x) << ((UINT32)(n))) | ((x) >> (32 - (UINT32)(n))))
#endif

__inline UINT16 ByteSwap(UINT16 wX) {
	return ((wX & 0xFF00) >> 8) | ((wX & 0x00FF) << 8);
}

__inline UINT32 ByteSwap(UINT32 dwX) {
#ifdef _X86_
	_asm mov eax, dwX
	_asm bswap eax
	_asm mov dwX, eax
	return dwX;
#else
	return _lrotl(((dwX & 0xFF00FF00) >> 8) | ((dwX & 0x00FF00FF) << 8), 16);
#endif
}

#if defined(WIN32) || defined(WIN64)
__inline UINT64 ByteSwap(UINT64 ui64) {
	return _byteswap_uint64(ui64);
}
#endif

#define __VAL(x) ByteSwap(x)

#else //PGF_USE_BIG_ENDIAN

	#define __VAL(x) (x)

#endif //PGF_USE_BIG_ENDIAN

// OpenMP rules (inspired from libraw project)
// NOTE: Use LIBPGF_DISABLE_OPENMP to disable OpenMP support in whole libpgf
#ifndef LIBPGF_DISABLE_OPENMP
# if defined (_OPENMP)
#  if defined (WIN32) || defined(WIN64)
#   if defined (_MSC_VER) && (_MSC_VER >= 1500)
//   VS2008 SP1 and VS2010+ : OpenMP works OK
#    define LIBPGF_USE_OPENMP
#   elif defined (__INTEL_COMPILER) && (__INTEL_COMPILER >=910)
//   untested on 9.x and 10.x, Intel documentation claims OpenMP 2.5 support in 9.1
#    define LIBPGF_USE_OPENMP
#   else
#    undef LIBPGF_USE_OPENMP
#   endif
//  Not Win32
#  elif (defined(__APPLE__) || defined(__MACOSX__)) && defined(_REENTRANT)
#   undef LIBPGF_USE_OPENMP
#  else
#   define LIBPGF_USE_OPENMP
#  endif
# endif // defined (_OPENMP)
#endif // ifndef LIBPGF_DISABLE_OPENMP
#ifdef LIBPGF_USE_OPENMP
#include <omp.h>
#endif

#endif //PGF_PGFPLATFORM_H
