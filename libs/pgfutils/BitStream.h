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

//////////////////////////////////////////////////////////////////////
/// @file Bitstream.h
/// @brief PGF bit-stream operations
/// @author C. Stamm

#ifndef PGF_BITSTREAM_H
#define PGF_BITSTREAM_H

#include "PGFtypes.h"

//////////////////////////////////////////////////////////////////////
// constants
//static const WordWidth = 32;
//static const WordWidthLog = 5;
static const UINT32 Filled = 0xFFFFFFFF;

/// @brief Make 64 bit unsigned integer from two 32 bit unsigned integers
#define MAKEU64(a, b) ((UINT64) (((UINT32) (a)) | ((UINT64) ((UINT32) (b))) << 32)) 

/*
static UINT8 lMask[] = {
	0x00,                       // 00000000
	0x80,                       // 10000000 
	0xc0,                       // 11000000
	0xe0,                       // 11100000
	0xf0,                       // 11110000
	0xf8,                       // 11111000
	0xfc,                       // 11111100
	0xfe,                       // 11111110
	0xff,                       // 11111111
};
*/
// these procedures have to be inlined because of performance reasons

//////////////////////////////////////////////////////////////////////
/// Set one bit of a bit stream to 1
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
inline void SetBit(UINT32* stream, UINT32 pos) {
	stream[pos >> WordWidthLog] |= (1 << (pos%WordWidth));
}

//////////////////////////////////////////////////////////////////////
/// Set one bit of a bit stream to 0
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
inline void ClearBit(UINT32* stream, UINT32 pos) {
	stream[pos >> WordWidthLog] &= ~(1 << (pos%WordWidth)); 
}

//////////////////////////////////////////////////////////////////////
/// Return one bit of a bit stream
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
/// @return bit at position pos of bit stream stream
inline bool GetBit(UINT32* stream, UINT32 pos)  {
	return (stream[pos >> WordWidthLog] & (1 << (pos%WordWidth))) > 0;

}

//////////////////////////////////////////////////////////////////////
/// Compare k-bit binary representation of stream at position pos with val
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
/// @param k Number of bits to compare
/// @param val Value to compare with
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
/// Store k-bit binary representation of val in stream at position pos
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
/// @param val Value to store in stream at position pos
/// @param k Number of bits of integer representation of val
inline void SetValueBlock(UINT32* stream, UINT32 pos, UINT32 val, UINT32 k) {
	const UINT32 offset = pos%WordWidth;
	const UINT32 iLoInt = pos >> WordWidthLog;
	const UINT32 iHiInt = (pos + k - 1) >> WordWidthLog;
	ASSERT(iLoInt <= iHiInt);
	const UINT32 loMask = Filled << offset;
	const UINT32 hiMask = Filled >> (WordWidth - 1 - ((pos + k - 1)%WordWidth));

	if (iLoInt == iHiInt) {
		// fits into one integer
		stream[iLoInt] &= ~(loMask & hiMask); // clear bits
		stream[iLoInt] |= val << offset; // write value
	} else {
		// must be splitted over integer boundary
		stream[iLoInt] &= ~loMask; // clear bits
		stream[iLoInt] |= val << offset; // write lower part of value
		stream[iHiInt] &= ~hiMask; // clear bits
		stream[iHiInt] |= val >> (WordWidth - offset); // write higher part of value
	}
}

//////////////////////////////////////////////////////////////////////
/// Read k-bit number from stream at position pos
/// @param stream A bit stream stored in array of unsigned integers
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
/// @param stream A bit stream stored in array of unsigned integers
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
/// @param stream A bit stream stored in array of unsigned integers
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
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
/// @param len size of search area (in bits)
/// return The distance to the next 1 in stream at position pos
inline UINT32 SeekBitRange(UINT32* stream, UINT32 pos, UINT32 len) {
	UINT32 count = 0;
	UINT32 testMask = 1 << (pos%WordWidth);
	UINT32* word = stream + (pos >> WordWidthLog);

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
/// @param stream A bit stream stored in array of unsigned integers
/// @param pos A valid zero-based position in the bit stream
/// @param len size of search area (in bits)
/// return The distance to the next 0 in stream at position pos
inline UINT32 SeekBit1Range(UINT32* stream, UINT32 pos, UINT32 len) {
	UINT32 count = 0;
	UINT32 testMask = 1 << (pos%WordWidth);
	UINT32* word = stream + (pos >> WordWidthLog);

	while (((*word & testMask) != 0) && (count < len)) {
		count++; 
		testMask <<= 1;
		if (!testMask) {
			word++; testMask = 1;

			// fast steps if all bits in a word are one
			while ((count + WordWidth <= len) && (*word == Filled)) {
				word++; 
				count += WordWidth;
			}
		}
	}
	return count;
}
/*
//////////////////////////////////////////////////////////////////////
/// BitCopy: copies k bits from source to destination
/// Note: only 8 bits are copied at a time, if speed is an issue, a more
/// complicated but faster 64 bit algorithm should be used.
inline void BitCopy(const UINT8 *sStream, UINT32 sPos, UINT8 *dStream, UINT32 dPos, UINT32 k) {
	ASSERT(k > 0);

	div_t divS = div(sPos, 8);
	div_t divD = div(dPos, 8);
	UINT32 sOff = divS.rem;
	UINT32 dOff = divD.rem;
	INT32 tmp = div(dPos + k - 1, 8).quot;

	const UINT8 *sAddr = sStream + divS.quot;
	UINT8 *dAddrS = dStream + divD.quot;
	UINT8 *dAddrE = dStream + tmp;
	UINT8 eMask;

	UINT8 destSB = *dAddrS;
	UINT8 destEB = *dAddrE;
	UINT8 *dAddr;
	UINT8 prec;
	INT32 shiftl, shiftr;

	if (dOff > sOff) {
		prec = 0;
		shiftr = dOff - sOff;
		shiftl = 8 - dOff + sOff;
	} else {
		prec = *sAddr << (sOff - dOff);
		shiftr = 8 - sOff + dOff;
		shiftl = sOff - dOff;
		sAddr++;
	}

	for (dAddr = dAddrS; dAddr < dAddrE; dAddr++, sAddr++) {
		*dAddr = prec | (*sAddr >> shiftr);
		prec = *sAddr << shiftl;
	}

	if ((sPos + k)%8 == 0) {
		*dAddr = prec;
	} else {
		*dAddr = prec | (*sAddr >> shiftr);
	}

	eMask = lMask[dOff];
	*dAddrS = (destSB & eMask) | (*dAddrS & (~eMask));

	INT32 mind = (dPos + k) % 8;
	eMask = (mind) ? lMask[mind] : lMask[8];
	*dAddrE = (destEB & (~eMask)) | (*dAddrE & eMask);
}
*/
//////////////////////////////////////////////////////////////////////
/// Compute bit position of the next 32-bit word
/// @param pos current bit stream position
/// @return bit position of next 32-bit word
inline UINT32 AlignWordPos(UINT32 pos) {
//	return ((pos + WordWidth - 1) >> WordWidthLog) << WordWidthLog;
	return DWWIDTHBITS(pos);
}

//////////////////////////////////////////////////////////////////////
/// Compute number of the 32-bit words
/// @param pos Current bit stream position
/// @return Number of 32-bit words
inline UINT32 NumberOfWords(UINT32 pos) {
	return (pos + WordWidth - 1) >> WordWidthLog;
}

#endif //PGF_BITSTREAM_H
