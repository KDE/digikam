/*
 * The Progressive Graphics File; http://www.libpgf.org
 *
 * $Date: 2007-06-11 10:56:17 +0200 (Mo, 11 Jun 2007) $
 * $Revision: 299 $
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
/// @file PGFtypes.h
/// @brief PGF definitions
/// @author C. Stamm

#ifndef PGF_PGFTYPES_H
#define PGF_PGFTYPES_H

#include "PGFplatform.h"

//-------------------------------------------------------------------------------
//	Codec versions
//
// Version 2:	modified data structure PGFHeader (backward compatibility assured)
// Version 4:	DataT: INT32 instead of INT16, allows 31 bit per pixel and channel (backward compatibility assured)
// Version 5:	ROI, new block-reordering scheme (backward compatibility assured)
// Version 6:	modified data structure PGFPreHeader: hSize (header size) is now a UINT32 instead of a UINT16 (backward compatibility assured)
// Version 7:	last two bytes in header are now used for extended version numbers; new data representation for bitmaps (backward compatibility assured)
//
//-------------------------------------------------------------------------------
#define PGFMajorNumber		7
#define PGFYear				15
#define	PGFWeek				32

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

//#define PGFCodecVersionID		0x071532
#define PGFCodecVersionID PPCAT(PPCAT(PPCAT(0x0, PGFMajorNumber), PGFYear), PGFWeek)
//#define PGFCodecVersion		"7.15.32"			///< Major number, Minor number: Year (2) Week (2)
#define PGFCodecVersion STRINGIZE(PPCAT(PPCAT(PPCAT(PPCAT(PGFMajorNumber, .), PGFYear), .), PGFWeek))

//-------------------------------------------------------------------------------
//	Image constants
//-------------------------------------------------------------------------------
#define PGFMagic			"PGF"				///< PGF identification
#define MaxLevel			30					///< maximum number of transform levels
#define NSubbands			4					///< number of subbands per level
#define MaxChannels			8					///< maximum number of (color) channels
#define DownsampleThreshold 3					///< if quality is larger than this threshold than downsampling is used
#define ColorTableLen		256					///< size of color lookup table (clut)
// version flags
#define Version2			2					///< data structure PGFHeader of major version 2
#define PGF32				4					///< 32 bit values are used -> allows at maximum 31 bits, otherwise 16 bit values are used -> allows at maximum 15 bits
#define PGFROI				8					///< supports Regions Of Interest
#define Version5			16					///< new coding scheme since major version 5
#define Version6			32					///< hSize in PGFPreHeader uses 32 bits instead of 16 bits
#define Version7			64					///< Codec major and minor version number stored in PGFHeader
// version numbers
#ifdef __PGF32SUPPORT__
#define PGFVersion			(Version2 | PGF32 | Version5 | Version6 | Version7)	///< current standard version
#else
#define PGFVersion			(Version2 |         Version5 | Version6 | Version7)	///< current standard version
#endif

//-------------------------------------------------------------------------------
//	Coder constants
//-------------------------------------------------------------------------------
#define BufferSize			16384				///< must be a multiple of WordWidth, BufferSize <= UINT16_MAX
#define RLblockSizeLen		15					///< block size length (< 16): ld(BufferSize) < RLblockSizeLen <= 2*ld(BufferSize)
#define LinBlockSize		8					///< side length of a coefficient block in a HH or LL subband
#define InterBlockSize		4					///< side length of a coefficient block in a HL or LH subband
#ifdef __PGF32SUPPORT__
	#define MaxBitPlanes	31					///< maximum number of bit planes of m_value: 32 minus sign bit
#else
	#define MaxBitPlanes	15					///< maximum number of bit planes of m_value: 16 minus sign bit
#endif
#define MaxBitPlanesLog		5					///< number of bits to code the maximum number of bit planes (in 32 or 16 bit mode)
#define MaxQuality			MaxBitPlanes		///< maximum quality

//-------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------
enum Orientation		{ LL = 0, HL = 1, LH = 2, HH = 3 };
enum ProgressMode		{ PM_Relative, PM_Absolute };
enum UserdataPolicy		{ UP_Skip = 0, UP_CachePrefix = 1, UP_CacheAll = 2 };

/// general PGF file structure
/// PGFPreHeader PGFHeader [PGFPostHeader] LevelLengths Level_n-1 Level_n-2 ... Level_0
/// PGFPostHeader ::= [ColorTable] [UserData]
/// LevelLengths  ::= UINT32[nLevels]

#pragma pack(1)
/////////////////////////////////////////////////////////////////////
/// PGF magic and version (part of PGF pre-header)
/// @author C. Stamm
/// @brief PGF identification and version
struct PGFMagicVersion {
	char magic[3];				///< PGF identification = "PGF"
	UINT8 version;				///< PGF version
	// total: 4 Bytes
};

/////////////////////////////////////////////////////////////////////
/// PGF pre-header defined header length and PGF identification and version
/// @author C. Stamm
/// @brief PGF pre-header
struct PGFPreHeader : PGFMagicVersion {
	UINT32 hSize;				///< total size of PGFHeader, [ColorTable], and [UserData] in bytes (since Version 6: 4 Bytes)
	// total: 8 Bytes
};

/////////////////////////////////////////////////////////////////////
/// Version number since major version 7
/// @author C. Stamm
/// @brief version number stored in header since major version 7
struct PGFVersionNumber {
	PGFVersionNumber(UINT8 _major, UINT8 _year, UINT8 _week)
#ifdef PGF_USE_BIG_ENDIAN
	 : week(_week),
	   year(_year),
	   major(_major)
#else
	 : major(_major),
	   year(_year),
	   week(_week)
#endif // PGF_USE_BIG_ENDIAN
	{
	}

#ifdef PGF_USE_BIG_ENDIAN
	UINT16 week  : 6;	///< week number in a year
	UINT16 year  : 6;	///< year since 2000 (year 2001 = 1)
	UINT16 major : 4;	///< major version number
#else
	UINT16 major : 4;	///< major version number
	UINT16 year  : 6;	///< year since 2000 (year 2001 = 1)
	UINT16 week  : 6;	///< week number in a year
#endif // PGF_USE_BIG_ENDIAN
};

/////////////////////////////////////////////////////////////////////
/// PGF header contains image information
/// @author C. Stamm
/// @brief PGF header
struct PGFHeader {
	PGFHeader() : width(0), height(0), nLevels(0), quality(0), bpp(0), channels(0), mode(ImageModeUnknown), usedBitsPerChannel(0), version(0, 0, 0) {}
	UINT32 width;				///< image width in pixels
	UINT32 height;				///< image height in pixels
	UINT8 nLevels;				///< number of FWT transforms
	UINT8 quality;				///< quantization parameter: 0=lossless, 4=standard, 6=poor quality
	UINT8 bpp;					///< bits per pixel
	UINT8 channels;				///< number of channels
	UINT8 mode;					///< image mode according to Adobe's image modes
	UINT8 usedBitsPerChannel;	///< number of used bits per channel in 16- and 32-bit per channel modes
	PGFVersionNumber version;	///< codec version number: (since Version 7)
	// total: 16 Bytes
};

/////////////////////////////////////////////////////////////////////
/// PGF post-header is optional. It contains color table and user data
/// @author C. Stamm
/// @brief Optional PGF post-header
struct PGFPostHeader {
	RGBQUAD clut[ColorTableLen];///< color table for indexed color images (optional part of file header)
	UINT8 *userData;			///< user data of size userDataLen (optional part of file header)
	UINT32 userDataLen;			///< user data size in bytes (not part of file header)
	UINT32 cachedUserDataLen;	///< cached user data size in bytes (not part of file header)
};

/////////////////////////////////////////////////////////////////////
/// ROI block header is used with ROI coding scheme. It contains block size and tile end flag
/// @author C. Stamm
/// @brief Block header used with ROI coding scheme
union ROIBlockHeader {
	/// Constructor
	/// @param v Buffer size
	ROIBlockHeader(UINT16 v) { val = v; }
	/// Constructor
	/// @param size Buffer size
	/// @param end 0/1 Flag; 1: last part of a tile
	ROIBlockHeader(UINT32 size, bool end)	{ ASSERT(size < (1 << RLblockSizeLen)); rbh.bufferSize = size; rbh.tileEnd = end; }

	UINT16 val; ///< unstructured union value
	/// @brief Named ROI block header (part of the union)
	struct RBH {
#ifdef PGF_USE_BIG_ENDIAN
		UINT16 tileEnd   :				1;	///< 1: last part of a tile
		UINT16 bufferSize: RLblockSizeLen;	///< number of uncoded UINT32 values in a block
#else
		UINT16 bufferSize: RLblockSizeLen;	///< number of uncoded UINT32 values in a block
		UINT16 tileEnd   :				1;	///< 1: last part of a tile
#endif // PGF_USE_BIG_ENDIAN
	} rbh;	///< ROI block header
	// total: 2 Bytes
};

#pragma pack()

/////////////////////////////////////////////////////////////////////
/// PGF I/O exception
/// @author C. Stamm
/// @brief PGF exception
struct IOException {
	/// Standard constructor
	IOException() : error(NoError) {}
	/// Constructor
	/// @param err Run-time error
	IOException(OSError err) : error(err) {}

	OSError error;				///< operating system error code
};

/////////////////////////////////////////////////////////////////////
/// Rectangle
/// @author C. Stamm
/// @brief Rectangle
struct PGFRect {
	/// Standard constructor
	PGFRect() : left(0), top(0), right(0), bottom(0) {}
	/// Constructor
	/// @param x Left offset
	/// @param y Top offset
	/// @param width Rectangle width
	/// @param height Rectangle height
	PGFRect(UINT32 x, UINT32 y, UINT32 width, UINT32 height) : left(x), top(y), right(x + width), bottom(y + height) {}

#ifdef WIN32
	PGFRect(const RECT& rect) : left(rect.left), top(rect.top), right(rect.right), bottom(rect.bottom) {
		ASSERT(rect.left >= 0 && rect.right >= 0 && rect.left <= rect.right);
		ASSERT(rect.top >= 0 && rect.bottom >= 0 && rect.top <= rect.bottom);
	}
	PGFRect& operator=(const RECT& rect) {
		left = rect.left; top = rect.top; right = rect.right; bottom = rect.bottom;
		return *this;
	}
	operator RECT() {
		RECT rect = { (LONG)left, (LONG)top, (LONG)right, (LONG)bottom };
		return rect;
	}
#endif

	/// @return Rectangle width
	UINT32 Width() const					{ return right - left; }
	/// @return Rectangle height
	UINT32 Height() const					{ return bottom - top; }

	/// Test if point (x,y) is inside this rectangle (inclusive top-left edges, exclusive bottom-right edges)
	/// @param x Point coordinate x
	/// @param y Point coordinate y
	/// @return True if point (x,y) is inside this rectangle (inclusive top-left edges, exclusive bottom-right edges)
	bool IsInside(UINT32 x, UINT32 y) const { return (x >= left && x < right && y >= top && y < bottom); }

	UINT32 left, top, right, bottom;
};

#ifdef __PGF32SUPPORT__
typedef INT32 DataT;
#else
typedef INT16 DataT;
#endif

typedef void (*RefreshCB)(void *p);

//-------------------------------------------------------------------------------
// Image constants
//-------------------------------------------------------------------------------
#define MagicVersionSize	sizeof(PGFMagicVersion)
#define PreHeaderSize		sizeof(PGFPreHeader)
#define HeaderSize			sizeof(PGFHeader)
#define ColorTableSize		(ColorTableLen*sizeof(RGBQUAD))
#define DataTSize			sizeof(DataT)
#define MaxUserDataSize		0x7FFFFFFF

#endif //PGF_PGFTYPES_H
