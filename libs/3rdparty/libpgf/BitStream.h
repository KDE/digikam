/*
 * The Progressive Graphics File; http://www.libpgf.org
 * 
 * $Date: 2006-06-04 22:05:59 +0200 (So, 04 Jun 2006) $
 * $Revision: 229 $
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

#ifndef PGF_BITSTREAM_H
#define PGF_BITSTREAM_H

#include "PGFtypes.h"

// constants
//static const WordWidth = 32;
//static const WordWidthLog = 5;
static const UINT32 Filled = 0xFFFFFFFF;

#define MAKEU64(a, b) ((UINT64) (((UINT32) (a)) | ((UINT64) ((UINT32) (b))) << 32)) 
 
// these procedures have to be inlined because of performance reasons

//////////////////////////////////////////////////////////////////////
/// Set one bit of a bit stream to 1
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
inline void SetBit(UINT32* stream, UINT32 pos) {
	stream[pos >> WordWidthLog] |= (1 << (pos%WordWidth));
}

//////////////////////////////////////////////////////////////////////
/// Set one bit of a bit stream to 0
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
inline void ClearBit(UINT32* stream, UINT32 pos) {
	stream[pos >> WordWidthLog] &= ~(1 << (pos%WordWidth)); 
}

//////////////////////////////////////////////////////////////////////
/// Return one bit of a bit stream
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @return bit at position pos of bit stream stream
inline bool GetBit(UINT32* stream, UINT32 pos)  {
	return (stream[pos >> WordWidthLog] & (1 << (pos%WordWidth))) > 0;

}

//////////////////////////////////////////////////////////////////////
/// Compare k-bit binary representation of stream at postition pos with val
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param k Number of bits to compare
/// @return true if equal
inline bool CompareBitBlock(UINT32* stream, UINT32 pos, UINT32 k, UINT32 val) {
	const UINT32 iLoInt = pos >> WordWidthLog;
	const UINT32 iHiInt = (pos + k - 1) >> WordWidthLog;
	ASSERT(iLoInt <= iHiInt);
	const UINT32 mask = (Filled >> (WordWidth - k));

	if (iLoInt == iHiInt) {
		// fits into one integer
		val &= mask;
		val <<= (pos%WordWidth);
		return (stream[iLoInt] & val) == val;
	} else {
		// must be splitted over integer boundary
		UINT64 v1 = MAKEU64(stream[iLoInt], stream[iHiInt]);
		UINT64 v2 = UINT64(val & mask) << (pos%WordWidth);
		return (v1 & v2) == v2;
	}
}

//////////////////////////////////////////////////////////////////////
/// Store k-bit binary representation of val in stream at postition pos
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param k Number of bits of integer representation of val
inline void SetValueBlock(UINT32* stream, UINT32 pos, UINT32 val, UINT32 k) {
	const UINT32 iLoInt = pos >> WordWidthLog;
	const UINT32 iHiInt = (pos + k - 1) >> WordWidthLog;
	ASSERT(iLoInt <= iHiInt);
	const UINT32 loMask = Filled << (pos%WordWidth);
	const UINT32 hiMask = Filled >> (WordWidth - 1 - ((pos + k - 1)%WordWidth));

	if (iLoInt == iHiInt) {
		// fits into one integer
		stream[iLoInt] &= ~(loMask & hiMask);
		stream[iLoInt] |= val << (pos%WordWidth);
	} else {
		// must be splitted over integer boundary
		stream[iLoInt] &= ~loMask;
		stream[iLoInt] |= val << (pos%WordWidth);
		stream[iHiInt] &= ~hiMask;
		stream[iHiInt] |= val >> (WordWidth - (pos%WordWidth));
	}
}

//////////////////////////////////////////////////////////////////////
/// Read k-bit number from stream at position pos
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param k Number of bits to read: 1 <= k <= 32
inline UINT32 GetValueBlock(UINT32* stream, UINT32 pos, UINT32 k) {
	UINT32 count, hiCount;
	const UINT32 iLoInt = pos >> WordWidthLog;				// integer of first bit
	const UINT32 iHiInt = (pos + k - 1) >> WordWidthLog;		// integer of last bit
	const UINT32 loMask = Filled << (pos%WordWidth);
	const UINT32 hiMask = Filled >> (WordWidth - 1 - ((pos + k - 1)%WordWidth));
	
	if (iLoInt == iHiInt) {
		// inside integer boundary
		count = stream[iLoInt] & (loMask & hiMask);
		count >>= pos%WordWidth;
	} else {
		// overlapping integer boundary
		count = stream[iLoInt] & loMask;
		count >>= pos%WordWidth;
		hiCount = stream[iHiInt] & hiMask;
		hiCount <<= WordWidth - (pos%WordWidth);
		count |= hiCount;
	}
	return count;
}

//////////////////////////////////////////////////////////////////////
/// Clear block of size at least len at position pos in stream
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param len Number of bits set to 0
inline void ClearBitBlock(UINT32* stream, UINT32 pos, UINT32 len) {
	ASSERT(len > 0);
	const UINT32 iFirstInt = pos >> WordWidthLog;
	const UINT32 iLastInt = (pos + len - 1) >> WordWidthLog;

	const UINT32 startMask = Filled << (pos%WordWidth);
//	const UINT32 endMask=Filled>>(WordWidth-1-((pos+len-1)%WordWidth));

	if (iFirstInt == iLastInt) {
		stream[iFirstInt] &= ~(startMask /*& endMask*/);
	} else {
		stream[iFirstInt] &= ~startMask;
		for (UINT32 i = iFirstInt + 1; i <= iLastInt; i++) { // changed <=
			stream[i] = 0;
		}
		//stream[iLastInt] &= ~endMask;
	}
}

//////////////////////////////////////////////////////////////////////
/// Set block of size at least len at position pos in stream
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param len Number of bits set to 1
inline void SetBitBlock(UINT32* stream, UINT32 pos, UINT32 len) {
	ASSERT(len > 0);

	const UINT32 iFirstInt = pos >> WordWidthLog;
	const UINT32 iLastInt = (pos + len - 1) >> WordWidthLog;

	const UINT32 startMask = Filled << (pos%WordWidth);
//	const UINT32 endMask=Filled>>(WordWidth-1-((pos+len-1)%WordWidth));

	if (iFirstInt == iLastInt) {
		stream[iFirstInt] |= (startMask /*& endMask*/);
	} else {
		stream[iFirstInt] |= startMask;
		for (UINT32 i = iFirstInt + 1; i <= iLastInt; i++) { // changed <=
			stream[i] = Filled;
		}
		//stream[iLastInt] &= ~endMask;
	}
}

//////////////////////////////////////////////////////////////////////
/// Returns the distance to the next 1 in stream at position pos.
/// If no 1 is found within len bits, then len is returned.
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param len size of search area (in bits)
/// return The distance to the next 1 in stream at position pos
inline UINT32 SeekBitRange(UINT32* stream, UINT32 pos, UINT32 len) {
	UINT32 count = 0;
	UINT32 wordPos = pos >> WordWidthLog;
	UINT32 testMask = 1 << (pos%WordWidth);
	UINT32* word = stream + wordPos;

	while (((*word & testMask) == 0) && (count < len)) {
		count++; 
		testMask <<= 1;
		if (!testMask) {
			word++; testMask = 1;

			// fast steps if all bits in a word are zero
			while ((count + WordWidth <= len) && (*word == 0)) {
				word++; 
				count += WordWidth;
			}
		}
	}

	return count;
}

//////////////////////////////////////////////////////////////////////
/// Returns the distance to the next 0 in stream at position pos.
/// If no 0 is found within len bits, then len is returned.
/// @param stream A bit stream composed of unsigned integer arrays
/// @param pos A valid zero-based position in the bit stream
/// @param len size of search area (in bits)
/// return The distance to the next 0 in stream at position pos
inline UINT32 SeekBit1Range(UINT32* stream, UINT32 pos, UINT32 len) {
	UINT32 count = 0;
	UINT32  wordPos = pos >> WordWidthLog;
	UINT32  bitPos  = pos%WordWidth;
	UINT32 testMask = 1 << bitPos;

	while (((stream[wordPos] & testMask) != 0) && (count < len)) {
		ASSERT(bitPos < WordWidth);
		count++; 
		bitPos++;
		if (bitPos < WordWidth) {
			testMask <<= 1;
		} else {
			wordPos++; bitPos = 0; testMask = 1;

			// fast steps if all bits in a word are zero
			while ((count + WordWidth <= len) && (stream[wordPos] == Filled)) {
				wordPos++; 
				count += WordWidth;
			}
		}
	}
	return count;
}

//////////////////////////////////////////////////////////////////////
/// Compute position to beginning of the next 32-bit word
/// @param pos Current bit stream position
/// @return Position of next 32-bit word
inline UINT32 AlignWordPos(UINT32 pos) {
//	return (pos%WordWidth) ? pos + (WordWidth - pos%WordWidth) : pos;
//	return pos + (WordWidth - pos%WordWidth)%WordWidth;
	return ((pos + WordWidth - 1) >> WordWidthLog) << WordWidthLog;
}

//////////////////////////////////////////////////////////////////////
/// Compute number of the 32-bit words
/// @param pos Current bit stream position
/// @return Number of 32-bit words
inline UINT32 NumberOfWords(UINT32 pos) {
	return (pos + WordWidth - 1) >> WordWidthLog;
}
#endif //PGF_BITSTREAM_H
