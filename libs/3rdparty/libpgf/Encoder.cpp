/*
 * The Progressive Graphics File; http://www.libpgf.org
 * 
 * $Date: 2007-02-03 13:04:21 +0100 (Sa, 03 Feb 2007) $
 * $Revision: 280 $
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

#include "Encoder.h"
#ifdef TRACE
	#include <stdio.h>
#endif

//////////////////////////////////////////////////////
// PGF: file structure
//
// PGFPreHeader PGFHeader PGFPostHeader LevelLengths Level_n-1 Level_n-2 ... Level_0
// PGFPostHeader ::= [ColorTable] [UserData]
// LevelLengths  ::= UINT32[nLevels]

//////////////////////////////////////////////////////
// Encoding scheme
// input: wavelet coefficients stored in subbands
// output: binary file
//
//                   subband
//                      |
//                   m_value	[BufferSize]
//                |     |     |
//           m_sign  sigBits  refBits   [BufferSize, BufferLen, BufferLen]
//                |     |     |
//                m_codeBuffer  (for each plane: RLcodeLength (16 bit), RLcoded sigBits + m_sign, refBits)
//                      |
//                    file      (for each buffer: packedLength (16 bit), packed bits)
//  

// Constants
#define CodeBufferBitLen		(BufferSize*WordWidth)		// max number of bits in m_codeBuffer
#define MaxCodeLen				((1 << RLblockSizeLen) - 1)	// max length of RL encoded block

//////////////////////////////////////////////////////
// Constructor
// Write pre-header, header, postHeader, and levelLength.
// Throws IOException
// preHeader and header must not be references, because on BigEndian platforms they are modified
CEncoder::CEncoder(CPGFStream* stream, PGFPreHeader preHeader, PGFHeader header, const PGFPostHeader& postHeader, UINT32*& levelLength) THROW_
: m_stream(stream)
, m_startPosition(0)
, m_valuePos(0)
, m_maxAbsValue(0)
#ifdef __PGFROISUPPORT__
, m_roi(false)
#endif
{
	ASSERT(m_stream);

	int count;

	// save file position
	m_startPosition = m_stream->GetPos();

	// write preHeader
	preHeader.hSize = __VAL(preHeader.hSize);
	count = PreHeaderSize;
	m_stream->Write(&count, &preHeader);

	// write file header
	header.height = __VAL(header.height);
	header.width = __VAL(header.width);
	count = HeaderSize;
	m_stream->Write(&count, &header);

	// write postHeader
	if (header.mode == ImageModeIndexedColor) {
		// write color table
		count = ColorTableSize;
		m_stream->Write(&count, (void *)postHeader.clut);
	}
	if (postHeader.userData && postHeader.userDataLen) {
		// write user data
		count = postHeader.userDataLen;
		m_stream->Write(&count, postHeader.userData);
	}

	m_currLevelIndex = 0;
	m_isLevelEncoded = false;

	// renew levelLength
	delete[] levelLength;
	levelLength = new UINT32[header.nLevels];
	if (!levelLength) ReturnWithError(InsufficientMemory);
	for (UINT8 l = 0; l < header.nLevels; l++) levelLength[l] = 0;
	m_levelLength = levelLength;

	// write dummy levelLength
	m_levelLengthPos = m_stream->GetPos();
	count = header.nLevels*WordBytes;
	m_stream->Write(&count, m_levelLength);

	// save current file position
	m_currPosition = m_stream->GetPos();
}

//////////////////////////////////////////////////////
// Destructor
CEncoder::~CEncoder() {	
}

/////////////////////////////////////////////////////////////////////
/// Partitions a rectangular region of a given subband.
/// Partitioning scheme: The plane is partitioned in squares of side length LinBlockSize.
/// Write wavelet coefficients into buffer.
/// It might throw an IOException.
/// @param band A subband
/// @param width The width of the rectangle
/// @param height The height of the rectangle
/// @param startPos The buffer position of the top left corner of the rectangular region
/// @param pitch The number of bytes in row of the subband
void CEncoder::Partition(CSubband* band, int width, int height, int startPos, int pitch) THROW_ {
	ASSERT(band);

	const div_t hh = div(height, LinBlockSize);
	const div_t ww = div(width, LinBlockSize);
	const int ws = pitch - LinBlockSize;
	const int wr = pitch - ww.rem;
	int x, y, i, j;
	int pos, base = startPos, base2;

	// main height
	for (i=0; i < hh.quot; i++) {
		// main width
		base2 = base;
		for (j=0; j < ww.quot; j++) {
			pos = base2;
			for (y=0; y < LinBlockSize; y++) {
				for (x=0; x < LinBlockSize; x++) {
					WriteValue(band, pos);
					pos++;
				}
				pos += ws;
			}
			base2 += LinBlockSize;
		}
		// rest of width
		pos = base2;
		for (y=0; y < LinBlockSize; y++) {
			for (x=0; x < ww.rem; x++) {
				WriteValue(band, pos);
				pos++;
			}
			pos += wr;
			base += pitch;
		}
	}
	// main width 
	base2 = base;
	for (j=0; j < ww.quot; j++) {
		// rest of height
		pos = base2;
		for (y=0; y < hh.rem; y++) {
			for (x=0; x < LinBlockSize; x++) {
				WriteValue(band, pos);
				pos++;
			}
			pos += ws;
		}
		base2 += LinBlockSize;
	}
	// rest of height
	pos = base2;
	for (y=0; y < hh.rem; y++) {
		// rest of width
		for (x=0; x < ww.rem; x++) {
			WriteValue(band, pos);
			pos++;
		}
		pos += wr;
	}
}

//////////////////////////////////////////////////////
// Pad buffer with zeros, encode buffer, write levelLength into header
// Return number of bytes written into stream
// Throws IOException
UINT32 CEncoder::Flush() THROW_ {
	UINT32 retValue;

#ifdef __PGFROISUPPORT__
	if (!m_roi) {
#endif
		// pad buffer with zeros
		while (m_valuePos < BufferSize) {
			m_value[m_valuePos] = 0; 
			m_valuePos++;
		}
#ifdef __PGFROISUPPORT__
	}
#endif

	// encode buffer
	EncodeBuffer(ROIBlockHeader(m_valuePos, true));

	retValue = UINT32(m_stream->GetPos() - m_startPosition);

	if (m_levelLength) {
		// append levelLength to file, directly after post-header
		UINT64 curPos = m_stream->GetPos();

		// set file pos to levelLength
		m_stream->SetPos(FSFromStart, m_levelLengthPos);
#ifdef PGF_USE_BIG_ENDIAN 
		UINT32 levelLength;
		int count = WordBytes;
		
		for (int i=0; i < m_currLevelIndex; i++) {
			levelLength = __VAL(UINT32(m_levelLength[i]));
			m_stream->Write(&count, &levelLength);
		}
#else
		int count = m_currLevelIndex*WordBytes;
		
		m_stream->Write(&count, m_levelLength);
#endif //PGF_USE_BIG_ENDIAN 

		// restore file position
		m_stream->SetPos(FSFromStart, curPos);
	}

	return retValue;
}

/////////////////////////////////////////////////////////////////////
// Stores band value from given position bandPos into buffer m_value at position m_valuePos
// If buffer is full encode it to file
// Throws IOException
void CEncoder::WriteValue(CSubband* band, int bandPos) THROW_ {
	if (m_valuePos == BufferSize) {
		EncodeBuffer(ROIBlockHeader(BufferSize, false));
	}
	m_value[m_valuePos] = band->GetData(bandPos);
	UINT32 v = abs(m_value[m_valuePos]);
	if (v > m_maxAbsValue) m_maxAbsValue = v;
	m_valuePos++;
}

///////////////////////////////////////////////////////
// Encode buffer and write data into stream.
// h contains buffer size and flag indicating end of tile.
// Encoding scheme: <wordLen>(16 bits) [ ROI ] data
//		ROI	  ::= <bufferSize>(15 bits) <eofTile>(1 bit)
// Throws IOException
void CEncoder::EncodeBuffer(ROIBlockHeader h) THROW_ {
#ifdef __PGFROISUPPORT__
	ASSERT(m_roi && h.bufferSize <= BufferSize || h.bufferSize == BufferSize);
#else
	ASSERT(h.bufferSize == BufferSize);
#endif

	UINT32 codeLen = BitplaneEncode(h.bufferSize);
	UINT16 wordLen = UINT16(AlignWordPos(codeLen)/WordWidth);
	ASSERT(wordLen <= BufferSize);
	
	int count = sizeof(UINT16);

#ifdef PGF_USE_BIG_ENDIAN 
	// write wordLen
	UINT16 wordLen2 = __VAL(wordLen);
	m_stream->Write(&count, &wordLen2);

	#ifdef __PGFROISUPPORT__
		// write ROIBlockHeader
		if (m_roi) {
			h.val = __VAL(h.val);
			m_stream->Write(&count, &h.val); ASSERT(count == sizeof(UINT16));
		}
	#endif // __PGFROISUPPORT__

	// convert data
	for (int i=0; i < wordLen; i++) {
		m_codeBuffer[i] = __VAL(m_codeBuffer[i]);
	}
#else
	// write wordLen
	m_stream->Write(&count, &wordLen); ASSERT(count == sizeof(UINT16));

	#ifdef __PGFROISUPPORT__
		// write ROIBlockHeader
		if (m_roi) {
			m_stream->Write(&count, &h.val); ASSERT(count == sizeof(UINT16));
		}
	#endif // __PGFROISUPPORT__
#endif // PGF_USE_BIG_ENDIAN 

	// write data
	count = wordLen*WordBytes;
	m_stream->Write(&count, m_codeBuffer);

	// store levelLength
	if (m_levelLength && m_isLevelEncoded) {
		m_isLevelEncoded = false;
		UINT64 streamPos = m_stream->GetPos();
		ASSERT(streamPos - m_currPosition <= UINT_MAX);
		m_levelLength[m_currLevelIndex++] = UINT32(streamPos - m_currPosition);
		m_currPosition = streamPos;
	}

	// reset values
	m_valuePos = 0;
	m_maxAbsValue = 0;
}

////////////////////////////////////////////////////////
// Encode buffer of given size using bit plane coding.
// A buffer contains bufferLen UINT32 values, thus, bufferSize bits per bit plane.
// Following coding scheme is used: 
//		Buffer		::= <nPlanes>(5 bits) foreach(plane i): Plane[i]  
//		Plane[i]	::= [ Sig1 | Sig2 ] [DWORD alignment] refBits
//		Sig1		::= 1 <codeLen>(15 bits) codedSigAndSignBits 
//		Sig2		::= 0 <sigLen>(15 bits) [Sign1 | Sign2 ] sigBits 
//		Sign1		::= 1 <codeLen>(15 bits) [DWORD alignment] codedSignBits
//		Sign2		::= 0 <signLen>(15 bits) [DWORD alignment] signBits
// returns number of bits in m_codeBuffer
UINT32 CEncoder::BitplaneEncode(UINT32 bufferSize) {
	ASSERT(bufferSize <= BufferSize);
	const UINT32 bufferLen = AlignWordPos(bufferSize)/WordWidth;

	UINT8	nPlanes;
	UINT32	sigLen, codeLen = 0, codePos = 0, wordPos, refLen, signLen, k;
	int     plane;
	UINT32  sigBits[BufferLen];
	UINT32  refBits[BufferLen];
	UINT32  signBits[BufferLen];
	UINT32  planeMask;
	bool	useRL;

	// clear significance vector
	for (k=0; k < bufferLen; k++) {
		m_sigFlagVector[k] = 0;
	}

	// compute number of bit planes and split buffer into separate bit planes
	nPlanes = NumberOfBitplanes();

	// write number of bit planes to m_codeBuffer
	SetValueBlock(m_codeBuffer, 0, nPlanes, MaxBitPlanesLog);
	codePos += MaxBitPlanesLog;

	// loop through all bit planes
	if (nPlanes == 0) nPlanes = MaxBitPlanes + 1;
	planeMask = 1 << (nPlanes - 1);

	for (plane = nPlanes - 1; plane >= 0; plane--) {
		// clear significant bitset
		for (k=0; k < bufferLen; k++) {
			sigBits[k] = 0;
		}

		// split bitplane in significant bitset and refinement bitset
		sigLen = DecomposeBitplane(bufferSize, planeMask, sigBits, refBits, signBits, signLen);

		if (sigLen > 0) {
			useRL = true;
			// run-length encode significant bits and signs and append them to the m_codeBuffer
			m_codePos = codePos + RLblockSizeLen + 1; 
			codeLen = RLESigsAndSigns(sigBits, sigLen, signBits, signLen);
		} else {
			useRL = false;
		}

		if (useRL && codeLen <= MaxCodeLen && codeLen < AlignWordPos(sigLen) + AlignWordPos(signLen) + 2*RLblockSizeLen) {
			// set RL code bit
			SetBit(m_codeBuffer, codePos);
			codePos++;

			// write length codeLen to m_codeBuffer
			SetValueBlock(m_codeBuffer, codePos, codeLen, RLblockSizeLen);
			codePos += RLblockSizeLen + codeLen;
		} else {
			#ifdef TRACE
			printf("new\n");
			for (UINT32 i=0; i < bufferSize; i++) {
				printf("%s", (GetBit(sigBits, i)) ? "1" : "_");
				if (i%120 == 119) printf("\n");
			}
			printf("\n");
			#endif // TRACE

			// run-length coding wasn't efficient enough
			// we don't use RL coding for sigBits
			ClearBit(m_codeBuffer, codePos);
			codePos++;

			// write length sigLen to m_codeBuffer
			ASSERT(sigLen <= MaxCodeLen); 
			SetValueBlock(m_codeBuffer, codePos, sigLen, RLblockSizeLen);
			codePos += RLblockSizeLen;

			// overwrite m_codeBuffer
			if (signLen > 0) {
				useRL = true;
				// run-length encode m_sign and append them to the m_codeBuffer
				m_codePos = codePos + RLblockSizeLen + 1;
				codeLen = RLESigns(signBits, signLen);
			} else {
				useRL = false;
			}

			if (useRL && codeLen <= MaxCodeLen && codeLen < signLen) {
				// RL encoding of m_sign was efficient
				// write RL code bit
				SetBit(m_codeBuffer, codePos);
				codePos++;
				
				// write codeLen to m_codeBuffer
				SetValueBlock(m_codeBuffer, codePos, codeLen, RLblockSizeLen);

				// adjust code buffer position
				codePos += codeLen + RLblockSizeLen;
				codePos = AlignWordPos(codePos);

				// compute position of sigBits
				wordPos = codePos >> WordWidthLog;
				ASSERT(0 <= wordPos && wordPos < BufferSize);
			} else {
				// RL encoding of signBits wasn't efficient
				// clear RL code bit
				ClearBit(m_codeBuffer, codePos);
				codePos++;

				// write signLen to m_codeBuffer
				ASSERT(signLen <= MaxCodeLen); 
				SetValueBlock(m_codeBuffer, codePos, signLen, RLblockSizeLen);
				codePos += RLblockSizeLen;

				// write signBits to m_codeBuffer
				codePos = AlignWordPos(codePos);
				wordPos = codePos >> WordWidthLog;
				ASSERT(0 <= wordPos && wordPos < BufferSize);
				codeLen = AlignWordPos(signLen) >> WordWidthLog;

				for (k=0; k < codeLen; k++) {
					m_codeBuffer[wordPos] = signBits[k];
					wordPos++;
				}
				
			}

			// write sigBits
			ASSERT(0 <= wordPos && wordPos < BufferSize);
			refLen = AlignWordPos(sigLen) >> WordWidthLog;

			for (k=0; k < refLen; k++) {
				m_codeBuffer[wordPos] = sigBits[k];
				wordPos++;
			}
			codePos = wordPos << WordWidthLog;
		}

		// append refinement bitset (aligned to word boundary)
		codePos = AlignWordPos(codePos);
		wordPos = codePos >> WordWidthLog;
		ASSERT(0 <= wordPos && wordPos < BufferSize);
		refLen = AlignWordPos(bufferSize - sigLen)/WordWidth;

		for (k=0; k < refLen; k++) {
			m_codeBuffer[wordPos] = refBits[k];
			wordPos++;
		}
		codePos = wordPos << WordWidthLog;
		planeMask >>= 1;
	}
	ASSERT(0 <= codePos && codePos <= CodeBufferBitLen);
	return codePos;
}

//////////////////////////////////////////////////////////
// Split bitplane of length bufferSize into significant and refinement bitset
// returns length [bits] of significant bits
// input:  m_value
// output: sigBits, refBits, signBits, signLen [bits]
UINT32 CEncoder::DecomposeBitplane(UINT32 bufferSize, UINT32 planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits, UINT32& signLen) {
	ASSERT(sigBits);
	ASSERT(refBits);
	ASSERT(signBits);

	UINT32 sigRunLen;
	UINT32 sigPos = 0;
	UINT32 valuePos = 0, valueEnd;
	UINT32 refPos = 0;

	signLen = 0;

	while (valuePos < bufferSize) {
		// search next 1 in m_sigFlagVector
		sigRunLen = SeekBitRange(m_sigFlagVector, valuePos, bufferSize - valuePos);

		// search 1's in m_value[plane][valuePos..valuePos+sigRunLen)
		// these 1's are significant bits
		valueEnd = valuePos + sigRunLen;
		while (valuePos < valueEnd) {
			// search 0's
			while (valuePos < valueEnd && !GetBitAtPos(valuePos, planeMask)) {
				valuePos++;
				// write 0 to sigBits
				sigPos++;
			}
			if (valuePos < valueEnd) {
				// write a 1 to sigBits
				SetBit(sigBits, sigPos); 
				sigPos++;

				// copy the sign bit
				if (m_value[valuePos] < 0) {
					SetBit(signBits, signLen);
				} else {
					ClearBit(signBits, signLen);
				}
				signLen++;

				// update m_sigFlagVector
				SetBit(m_sigFlagVector, valuePos);
				valuePos++;
			}
		}
		// refinement bit
		if (valuePos < bufferSize) {
			// write one refinement bit
			if (GetBitAtPos(valuePos, planeMask)) {
				SetBit(refBits, refPos);
			} else {
				ClearBit(refBits, refPos);
			}
			refPos++;
			valuePos++;
		}
	}

	ASSERT(sigPos <= bufferSize);
	ASSERT(refPos <= bufferSize);
	ASSERT(signLen <= bufferSize);
	ASSERT(valuePos == bufferSize);

	return sigPos;
}


///////////////////////////////////////////////////////
// Compute number of bit planes needed
UINT8 CEncoder::NumberOfBitplanes() {
	UINT8 cnt = 0;

	// determine number of bitplanes for max value
	if (m_maxAbsValue > 0) {
		while (m_maxAbsValue > 0) {
			m_maxAbsValue >>= 1; cnt++;
		}
		if (cnt == MaxBitPlanes + 1) cnt = 0;
		// end cs
		ASSERT(cnt <= MaxBitPlanes);
		ASSERT((cnt >> MaxBitPlanesLog) == 0);
		return cnt;
	} else {
		return 1;
	}
}

////////////////////////////////////////////////////////
// Adaptive run-Length encoder for significant bits with a lot of zeros and sign bits.
// sigLen and signLen are in bits
// Returns length of output in bits.
// - Encode run of 2^k zeros by a single 0.
// - Encode run of count 0's followed by a 1 with codeword: 1<count>x
// - x is 0: if a positive sign is stored, otherwise 1
// - Store each bit in m_codeBuffer[m_codePos] and increment m_codePos.
UINT32 CEncoder::RLESigsAndSigns(UINT32* sigBits, UINT32 sigLen, UINT32* signBits, UINT32 signLen) {
	ASSERT(sigBits);
	ASSERT(signBits);
	ASSERT(m_codePos < CodeBufferBitLen);
	ASSERT(0 < sigLen && sigLen <= BufferSize);
	ASSERT(signLen <= BufferSize);

	const UINT32 outStartPos = m_codePos;

	UINT32 k = 3;
	UINT32 runlen = 1 << k; // = 2^k
	UINT32 count = 0;
	UINT32 sigPos = 0, signPos = 0;

	while (sigPos < sigLen) {
		// search next 1 in sigBits starting at position sigPos
		count = SeekBitRange(sigBits, sigPos, __min(runlen, sigLen - sigPos));
		// count 0's found
		if (count == runlen) {
			// encode run of 2^k zeros by a single 0
			sigPos += count; 
			ClearBit(m_codeBuffer, m_codePos); m_codePos++;
			// adapt k (double the zero run-length)
			if (k < WordWidth) {
				k++;
				runlen <<= 1;
			}
		} else {
			// encode run of count 0's followed by a 1
			// with codeword: 1<count>(signBits[signPos])
			sigPos += count + 1;
			SetBit(m_codeBuffer, m_codePos); 
			m_codePos++;
			if (k > 0) {
				SetValueBlock(m_codeBuffer, m_codePos, count, k);
				m_codePos += k;
			}
			// sign bit
			if (GetBit(signBits, signPos)) {
				SetBit(m_codeBuffer, m_codePos);
			} else {
				ClearBit(m_codeBuffer, m_codePos);
			}
			signPos++; m_codePos++;

			// adapt k (half the zero run-length)
			if (k > 0) {
				k--; 
				runlen >>= 1;
			}
		}
	}
	ASSERT(sigPos == sigLen || sigPos == sigLen + 1);
	ASSERT(signPos == signLen || signPos == signLen + 1);
	ASSERT(m_codePos >= outStartPos && m_codePos < CodeBufferBitLen);
	return m_codePos - outStartPos;
}

//////////////////////////////////////////////////////
// Adaptive Run-Length encoder for long sequences of ones.
// Returns length of output in bits.
// - Encode run of 2^k ones by a single 1.
// - Encode run of count 1's followed by a 0 with codeword: 0<count>.
// - Store each bit in m_codeBuffer[m_codePos] and increment m_codePos.
UINT32 CEncoder::RLESigns(UINT32* signBits, UINT32 signLen) {
	ASSERT(signBits);
	ASSERT(0 <= m_codePos && m_codePos < CodeBufferBitLen);
	ASSERT(0 < signLen && signLen <= BufferSize);
	
	const UINT32  outStartPos = m_codePos;
	UINT32 k = 0;
	UINT32 runlen = 1 << k; // = 2^k
	UINT32 count = 0;
	UINT32 signPos = 0;

	while (signPos < signLen) {
		// search next 0 in signBits starting at position signPos
		count = SeekBit1Range(signBits, signPos, __min(runlen, signLen - signPos));
		// count 1's found
		if (count == runlen) {
			// encode run of 2^k ones by a single 1
			signPos += count; 
			SetBit(m_codeBuffer, m_codePos); m_codePos++;
			// adapt k (double the 1's run-length)
			if (k < WordWidth) {
				k++; 
				runlen <<= 1;
			}
		} else {
			// encode run of count 1's followed by a 0
			// with codeword: 0(count)
			signPos += count + 1;
			ClearBit(m_codeBuffer, m_codePos); m_codePos++;
			if (k > 0) {
				SetValueBlock(m_codeBuffer, m_codePos, count, k);
				m_codePos += k;
			}
			// adapt k (half the 1's run-length)
			if (k > 0) {
				k--; 
				runlen >>= 1;
			}
		}
	}
	ASSERT(signPos == signLen || signPos == signLen + 1);
	ASSERT(m_codePos >= outStartPos && m_codePos < CodeBufferBitLen);
	return m_codePos - outStartPos;
}

//////////////////////////////////////////////////////
#ifdef TRACE
void CEncoder::DumpBuffer() const {
	printf("\nDump\n");
	for (UINT32 i=0; i < BufferSize; i++) {
		printf("%d", m_value[i]);
	}
	printf("\n");
}
#endif //TRACE


