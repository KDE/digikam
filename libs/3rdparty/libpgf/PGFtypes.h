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

#ifndef PGF_PGFTYPES_H
#define PGF_PGFTYPES_H

#include "PGFplatform.h"

//-------------------------------------------------------------------------------
//	Constraints
//-------------------------------------------------------------------------------
// BufferSize <= UINT16_MAX

//-------------------------------------------------------------------------------
//	Codec versions
//
// Version 2:	modified data structure PGFHeader
// Version 3:	INT32 instead of INT16, allows 31 bit per pixel and channel
// Version 5:	ROI, new block-reordering scheme
//-------------------------------------------------------------------------------
#define PGFCodecVersion		"5.08.22"			// Major number
												// Minor number: Year (2) Week (2)

//-------------------------------------------------------------------------------
//	Image constants
//-------------------------------------------------------------------------------
#define Magic				"PGF"				// PGF identification
#define MaxLevel			30					// maximum number of transform levels
#define NSubbands			4					// number of subbands per level
#define MaxChannels			8					// maximum number of (color) channels
#define DownsampleThreshold 3					// if quality is larger than this threshold than downsampling is used
#define DefaultBGColor		255					// default background color is white
#define ColorTableLen		256					// size of color lookup table (clut)
// version flags
#define Version2			2					// data structure PGFHeader of major version 2
#ifdef __PGF32SUPPORT__
#define PGF32				4					// 32 bit values are used -> allows at maximum 31 bits
#else
#define PGF32				0					// 16 bit values are used -> allows at maximum 15 bits
#endif
#define PGFROI				8					// supports Regions Of Interest
#define Version5			16					// coding scheme of major version 5
// version numbers
#define PGFVersion			(Version2 | Version5 | PGF32)	// current standard version

//-------------------------------------------------------------------------------
//	Coder constants
//-------------------------------------------------------------------------------
#define BufferSize			16384				// must be a multiple of WordWidth: RLblockSizeLen = 15
#define RLblockSizeLen		15					// block size length (< 16): ld(BufferSize) < RLblockSizeLen <= 2*ld(BufferSize)
#define LinBlockSize		8					// side length of a coefficient block in a HH or LL subband
#define InterBlockSize		4					// side length of a coefficient block in a HL or LH subband
#ifdef __PGF32SUPPORT__
#define MaxBitPlanes		31					// maximum number of bit planes of m_value: 32 minus sign bit
#else
#define MaxBitPlanes		15					// maximum number of bit planes of m_value: 16 minus sign bit
#endif
#define MaxBitPlanesLog		5					// number of bits to code the maximum number of bit planes (in 32 or 16 bit mode)
#define MaxQuality			MaxBitPlanes		// maximum quality

//-------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------
enum Orientation { LL=0, HL=1, LH=2, HH=3 };

// general file structure
// PGFPreHeader PGFHeader PGFPostHeader LevelLengths Level_n-1 Level_n-2 ... Level_0
// PGFPostHeader ::= [ColorTable] [UserData]
// LevelLengths  ::= UINT32[nLevels]

#pragma pack(1)
struct PGFPreHeader {
	char magic[3];				// PGF identification = "PGF"
	UINT8 version;				// PGF version
	UINT16 hSize;				// total size of PGFHeader, [ColorTable], and [UserData] in bytes
	// total: 6 Bytes
};

struct PGFHeader {
	UINT32 width;
	UINT32 height;
	UINT8 nLevels;
	UINT8 quality;
	UINT8 bpp;					// bits per pixel
	UINT8 channels;				// number of channels
	UINT8 mode;					// image mode according to Adobe's image modes
	RGBTRIPLE background;		// background color used in RGBA color mode
	// total: 16 Bytes
};

struct PGFPostHeader {
	RGBQUAD clut[ColorTableLen];// color table for indexed color images
	UINT8 *userData;			// user data of size userDataLen
	UINT16 userDataLen;			// user data size in bytes
	// total: at least 258 Bytes
};

union ROIBlockHeader {
	ROIBlockHeader(UINT16 v) { val = v; }
	ROIBlockHeader(UINT32 size, bool end)
	{ 
	    ASSERT(size < (1 << RLblockSizeLen));
	    bufferSize = size; 
	    tileEnd = end;
	}

	UINT16 val;
	struct {
		UINT16 bufferSize: RLblockSizeLen;	// number of uncoded UINT32 values in a block
		UINT16 tileEnd   :				1;	// 1: last part of a tile
	};
	// total: 2 Bytes
};

#pragma pack()

struct IOException {
	IOException() : error(NoError) {}
	IOException(OSError err) : error(err) {}

	OSError error;				// operating system error code
};

struct PGFRect {
	PGFRect() : left(0), top(0), right(0), bottom(0) {}
	PGFRect(UINT32 x, UINT32 y, UINT32 width, UINT32 height) : left(x), top(y), right(x + width), bottom(y + height) {}

	UINT32 Width() const					{ return right - left; }
	UINT32 Height() const					{ return bottom - top; }
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
#define PreHeaderSize		sizeof(PGFPreHeader)
#define HeaderSize			sizeof(PGFHeader)
#define ColorTableSize		ColorTableLen*sizeof(RGBQUAD)

#endif //PGF_PGFTYPES_H
