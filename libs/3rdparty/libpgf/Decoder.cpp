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

#include "Decoder.h"
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
// Decoding scheme
// input:  binary file
// output: wavelet coefficients stored in subbands
//
//                    file      (for each buffer: packedLength (16 bit), packed bits)
//                      |
//                m_codeBuffer  (for each plane: RLcodeLength (16 bit), RLcoded sigBits + m_sign, refBits)
//                |     |     |
//           m_sign  sigBits  refBits   [BufferLen, BufferLen, BufferLen]
//                |     |     |
//                   m_value	[BufferSize]
//                      |
//                   subband
//  

// Constants
#define CodeBufferBitLen		(BufferSize*WordWidth)	// max number of bits in m_codeBuffer

/////////////////////////////////////////////////////////////////////
// Default Constructor
CDecoder::CDecoder(CPGFStream* stream /*= NULL */) 
: m_stream(stream), m_startPos(0), m_encodedHeaderLength(0), m_valuePos(0), m_bufferIsAvailable(false)
#ifdef __PGFROISUPPORT__
, m_roi(false)
#endif
{
}

/////////////////////////////////////////////////////////////////////
// Constructor
// Read pre-header, header, and levelLength
// throws IOException
CDecoder::CDecoder(CPGFStream* stream, PGFPreHeader& preHeader, PGFHeader& header, PGFPostHeader& postHeader, UINT32*& levelLength) THROW_
: m_stream(stream), m_startPos(0), m_encodedHeaderLength(0), m_valuePos(0), m_bufferIsAvailable(false)
#ifdef __PGFROISUPPORT__
, m_roi(false)
#endif
{
	ASSERT(m_stream);

	int count;

	// store current stream position
	m_startPos = m_stream->GetPos();

	// read magic and version
	count = MagicVersionSize;
	m_stream->Read(&count, &preHeader);

	// read header size
	if (preHeader.version & Version6) {
		// 32 bit header size since version 6
		count = 4;
	} else {
		count = 2;
	}
	m_stream->Read(&count, ((UINT8*)&preHeader) + MagicVersionSize);

	// make sure the values are correct read
	preHeader.hSize = __VAL(preHeader.hSize);

	// check magic number
	if (memcmp(preHeader.magic, Magic, 3) != 0) {
		// error condition: wrong Magic number
		ReturnWithError(FormatCannotRead);
	}

	// read file header
	count = (preHeader.hSize < HeaderSize) ? preHeader.hSize : HeaderSize;
	m_stream->Read(&count, &header);
	// make sure the values are correct read
	header.height = __VAL(UINT32(header.height));
	header.width = __VAL(UINT32(header.width));

	// be ready to read all versions including version 0
	if (preHeader.version > 0) {
#ifndef __PGFROISUPPORT__
		// check ROI usage
		if (preHeader.version & PGFROI) ReturnWithError(FormatCannotRead);
#endif

		int size = preHeader.hSize - HeaderSize;

		if (size > 0) {
			// read post header
			if (header.mode == ImageModeIndexedColor) {
				ASSERT(size >= ColorTableSize);
				// read color table
				count = ColorTableSize;
				m_stream->Read(&count, postHeader.clut);
				ASSERT(count == ColorTableSize);
				size -= count;
			}

			if (size > 0) {
				// create user data memory block
				postHeader.userDataLen = size;
				postHeader.userData = new UINT8[postHeader.userDataLen];

				// read user data
				count = postHeader.userDataLen;
				m_stream->Read(&count, postHeader.userData);
				ASSERT(count == size);
			}
		}

		// create levelLength
		levelLength = new UINT32[header.nLevels];
		if (!levelLength) ReturnWithError(InsufficientMemory);

		// read levelLength
		count = header.nLevels*WordBytes;
		m_stream->Read(&count, levelLength);

#ifdef PGF_USE_BIG_ENDIAN 
		// make sure the values are correct read
		count /= WordBytes;
		for (int i=0; i < count; i++) {
			levelLength[i] = __VAL(levelLength[i]);
		}
#endif
	}

	// store current stream position
	m_encodedHeaderLength = UINT32(m_stream->GetPos() - m_startPos);
}

/////////////////////////////////////////////////////////////////////
// Destructor
CDecoder::~CDecoder() {
}

//////////////////////////////////////////////////////////////////////
/// Copies data from the open stream to a target buffer.
/// It might throw an IOException.
/// @param target The target buffer
/// @param len The number of bytes to read
/// @return The number of bytes copied to the target buffer
UINT32 CDecoder::ReadEncodedData(UINT8* target, UINT32 len) const THROW_ {
	ASSERT(m_stream);

	int count = len;
	m_stream->Read(&count, target);

	return count;
}

/////////////////////////////////////////////////////////////////////
/// Unpartitions a rectangular region of a given subband.
/// Partitioning scheme: The plane is partitioned in squares of side length LinBlockSize.
/// Write wavelet coefficients into buffer.
/// It might throw an IOException.
/// @param band A subband
/// @param quantParam Dequantization value
/// @param width The width of the rectangle
/// @param height The height of the rectangle
/// @param startPos The buffer position of the top left corner of the rectangular region
/// @param pitch The number of bytes in row of the subband
void CDecoder::Partition(CSubband* band, int quantParam, int width, int height, int startPos, int pitch) THROW_ {
	ASSERT(band);

	const div_t ww = div(width, LinBlockSize);
	const div_t hh = div(height, LinBlockSize);
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
					DequantizeValue(band, pos, quantParam);
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
				DequantizeValue(band, pos, quantParam);
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
				DequantizeValue(band, pos, quantParam);
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
			DequantizeValue(band, pos, quantParam);
			pos++;
		}
		pos += wr;
	}
}

////////////////////////////////////////////////////////////////////
// Decode and dequantize HL, and LH band of one level
// LH and HH are interleaved in the codestream and must be split
// Deccoding and dequantization of HL and LH Band (interleaved) using partitioning scheme
// partitions the plane in squares of side length InterBlockSize
// throws IOException
void CDecoder::DecodeInterleaved(CWaveletTransform* wtChannel, int level, int quantParam) THROW_ {
	CSubband* hlBand = wtChannel->GetSubband(level, HL);
	CSubband* lhBand = wtChannel->GetSubband(level, LH);
	const div_t lhH = div(lhBand->GetHeight(), InterBlockSize);
	const div_t hlW = div(hlBand->GetWidth(), InterBlockSize);
	const int hlws = hlBand->GetWidth() - InterBlockSize;
	const int hlwr = hlBand->GetWidth() - hlW.rem;
	const int lhws = lhBand->GetWidth() - InterBlockSize;
	const int lhwr = lhBand->GetWidth() - hlW.rem;
	int x, y, i, j;
	int hlPos, lhPos;
	int hlBase = 0, lhBase = 0, hlBase2, lhBase2;

	ASSERT(lhBand->GetWidth() >= hlBand->GetWidth());
	ASSERT(hlBand->GetHeight() >= lhBand->GetHeight());

	hlBand->AllocMemory();
	lhBand->AllocMemory();

	// correct quantParam with normalization factor
	quantParam -= level;
	if (quantParam < 0) quantParam = 0;

	// main height
	for (i=0; i < lhH.quot; i++) {
		// main width
		hlBase2 = hlBase;
		lhBase2 = lhBase;
		for (j=0; j < hlW.quot; j++) {
			hlPos = hlBase2;
			lhPos = lhBase2;
			for (y=0; y < InterBlockSize; y++) {
				for (x=0; x < InterBlockSize; x++) {
					DequantizeValue(hlBand, hlPos, quantParam);
					DequantizeValue(lhBand, lhPos, quantParam);
					hlPos++;
					lhPos++;
				}
				hlPos += hlws;
				lhPos += lhws;
			}
			hlBase2 += InterBlockSize;
			lhBase2 += InterBlockSize;
		}
		// rest of width
		hlPos = hlBase2;
		lhPos = lhBase2;
		for (y=0; y < InterBlockSize; y++) {
			for (x=0; x < hlW.rem; x++) {
				DequantizeValue(hlBand, hlPos, quantParam);
				DequantizeValue(lhBand, lhPos, quantParam);
				hlPos++;
				lhPos++;
			}
			// width difference between HL and LH
			if (lhBand->GetWidth() > hlBand->GetWidth()) {
				DequantizeValue(lhBand, lhPos, quantParam);
			}
			hlPos += hlwr;
			lhPos += lhwr;
			hlBase += hlBand->GetWidth();
			lhBase += lhBand->GetWidth();
		}
	}
	// main width 
	hlBase2 = hlBase;
	lhBase2 = lhBase;
	for (j=0; j < hlW.quot; j++) {
		// rest of height
		hlPos = hlBase2;
		lhPos = lhBase2;
		for (y=0; y < lhH.rem; y++) {
			for (x=0; x < InterBlockSize; x++) {
				DequantizeValue(hlBand, hlPos, quantParam);
				DequantizeValue(lhBand, lhPos, quantParam);
				hlPos++;
				lhPos++;
			}
			hlPos += hlws;
			lhPos += lhws;
		}
		hlBase2 += InterBlockSize;
		lhBase2 += InterBlockSize;
	}
	// rest of height
	hlPos = hlBase2;
	lhPos = lhBase2;
	for (y=0; y < lhH.rem; y++) {
		// rest of width
		for (x=0; x < hlW.rem; x++) {
			DequantizeValue(hlBand, hlPos, quantParam);
			DequantizeValue(lhBand, lhPos, quantParam);
			hlPos++;
			lhPos++;
		}
		// width difference between HL and LH
		if (lhBand->GetWidth() > hlBand->GetWidth()) {
			DequantizeValue(lhBand, lhPos, quantParam);
		}
		hlPos += hlwr;
		lhPos += lhwr;
		hlBase += hlBand->GetWidth();
	}
	// height difference between HL and LH
	if (hlBand->GetHeight() > lhBand->GetHeight()) {
		// total width
		hlPos = hlBase;
		for (j=0; j < hlBand->GetWidth(); j++) {
			DequantizeValue(hlBand, hlPos, quantParam);
			hlPos++;
		}
	}
}

////////////////////////////////////////////////////////////////////
/// Skip a given number of bytes in the open stream.
/// It might throw an IOException.
void CDecoder::Skip(UINT64 offset) THROW_ {
	m_stream->SetPos(FSFromCurrent, offset);
}

//////////////////////////////////////////////////////////////////////
/// Dequantization of a single value at given position in subband.
/// If encoded data is available, then stores dequantized band value into 
/// buffer m_value at position m_valuePos.
/// Otherwise reads encoded data buffer and decodes it.
/// @param band A subband
/// @param bandPos A valid position in subband band
/// @param quantParam The quantization parameter
void CDecoder::DequantizeValue(CSubband* band, UINT32 bandPos, int quantParam) {
	if (!m_bufferIsAvailable) {
		DecodeBuffer();
	}
	band->SetData(bandPos, m_value[m_valuePos] << quantParam);
	m_valuePos++;
	if (m_valuePos == BufferSize) {
		m_bufferIsAvailable = false;
	}
}

///////////////////////////////////////////////////////
// Read next block from stream and decode into buffer
// Decoding scheme: <wordLen>(16 bits) [ ROI ] data
//		ROI	  ::= <bufferSize>(15 bits) <eofTile>(1 bit)
// throws IOException
void CDecoder::DecodeBuffer() THROW_ {
	UINT16 wordLen;
	ROIBlockHeader h(BufferSize);
	int count;

#ifdef TRACE
	UINT32 filePos = (UINT32)m_stream->GetPos();
	printf("%d\n", filePos);
#endif

	// read wordLen
	count = sizeof(UINT16);
	m_stream->Read(&count, &wordLen); ASSERT(count == sizeof(UINT16));
	wordLen = __VAL(wordLen);
	ASSERT(wordLen <= BufferSize);

#ifdef __PGFROISUPPORT__
	// read ROIBlockHeader
	if (m_roi) {
		m_stream->Read(&count, &h.val); ASSERT(count == sizeof(UINT16));
		
		// convert ROIBlockHeader
		h.val = __VAL(h.val);
	}
#endif

	// read data
	count = wordLen*WordBytes;
	m_stream->Read(&count, m_codeBuffer);

#ifdef PGF_USE_BIG_ENDIAN 
	// convert data
	count /= WordBytes;
	for (int i=0; i < count; i++) {
		m_codeBuffer[i] = __VAL(m_codeBuffer[i]);
	}
#endif

#ifdef __PGFROISUPPORT__
	ASSERT(m_roi && h.bufferSize <= BufferSize || h.bufferSize == BufferSize);
#else
	ASSERT(h.bufferSize == BufferSize);
#endif
	BitplaneDecode(h.bufferSize);

	// data is available
	m_bufferIsAvailable = true;
	m_valuePos = 0;
}

///////////////////////////////////////////////////////
// Read next block from stream but don't decode into buffer
// Encoding scheme: <wordLen>(16 bits) [ ROI ] data
//		ROI	  ::= <bufferSize>(15 bits) <eofTile>(1 bit)
// throws IOException
void CDecoder::SkipBuffer() THROW_ {
	UINT16 wordLen;
	int count;

	// read wordLen
	count = sizeof(UINT16);
	m_stream->Read(&count, &wordLen); ASSERT(count == sizeof(UINT16));
	wordLen = __VAL(wordLen);
	ASSERT(wordLen <= BufferSize);

#ifdef __PGFROISUPPORT__
	if (m_roi) {
		// skip ROIBlockHeader
		m_stream->SetPos(FSFromCurrent, count);
	}
#endif

	// skip data
	m_stream->SetPos(FSFromCurrent, wordLen*WordBytes);
}

//////////////////////////////////////////////////////////////////////
// Decode block into buffer of given size using bit plane coding.
// A buffer contains bufferLen UINT32 values, thus, bufferSize bits per bit plane.
// Following coding scheme is used: 
//		Buffer		::= <nPlanes>(5 bits) foreach(plane i): Plane[i]  
//		Plane[i]	::= [ Sig1 | Sig2 ] [DWORD alignment] refBits
//		Sig1		::= 1 <codeLen>(15 bits) codedSigAndSignBits 
//		Sig2		::= 0 <sigLen>(15 bits) [Sign1 | Sign2 ] sigBits 
//		Sign1		::= 1 <codeLen>(15 bits) [DWORD alignment] codedSignBits
//		Sign2		::= 0 <signLen>(15 bits) [DWORD alignment] signBits
void CDecoder::BitplaneDecode(UINT32 bufferSize) {
	ASSERT(bufferSize <= BufferSize);
	const UINT32 bufferLen = AlignWordPos(bufferSize)/WordWidth;

	UINT32 nPlanes, k;
	UINT32 codePos = 0, codeLen, sigLen, sigPos, signLen, wordPos;
	int    plane;
	UINT32 sigBits[BufferLen];
	UINT32 signBits[BufferLen];
	UINT32 planeMask;

	// clear significance vector
	for (k=0; k < bufferLen; k++) {
		m_sigFlagVector[k] = 0;
	}

	// clear buffer
	for (k=0; k < bufferSize; k++) {
		m_value[k] = 0;
	}

	// read number of bit planes
	nPlanes = GetValueBlock(m_codeBuffer, 0, MaxBitPlanesLog); 
	codePos += MaxBitPlanesLog;

	// loop through all bit planes
	if (nPlanes == 0) nPlanes = MaxBitPlanes + 1;
	ASSERT(0 < nPlanes && nPlanes <= MaxBitPlanes + 1);
	planeMask = 1 << (nPlanes - 1);

	for (plane = nPlanes - 1; plane >= 0; plane--) {
		// read RL code
		if (GetBit(m_codeBuffer, codePos)) {
			// RL coding of sigBits is used
			codePos++;

			// read codeLen
			codeLen = GetValueBlock(m_codeBuffer, codePos, RLblockSizeLen);

			// run-length decode significant bits and signs from m_codeBuffer and store them to sigBits
			ASSERT(codeLen < (1 << RLblockSizeLen)); 
			m_codePos = codePos + RLblockSizeLen;
			sigLen = RLDSigsAndSigns(bufferSize, codeLen, sigBits, signBits); ASSERT(sigLen <= bufferSize);
			//printf("%d\n", sigLen);

			// adjust code buffer position
			codePos += RLblockSizeLen + codeLen;
			codePos = AlignWordPos(codePos); ASSERT(codePos < CodeBufferBitLen);

			// read refinement bits from m_codeBuffer and compose bit plane
			sigLen = ComposeBitplane(bufferSize, planeMask, sigBits, &m_codeBuffer[codePos >> WordWidthLog], signBits);

			// adjust code buffer position
			codePos += bufferSize - sigLen;
			codePos = AlignWordPos(codePos); ASSERT(codePos < CodeBufferBitLen);
		} else {
			// no RL coding is used
			codePos++;

			// read sigLen
			sigLen = GetValueBlock(m_codeBuffer, codePos, RLblockSizeLen); ASSERT(sigLen <= BufferSize);
			codePos += RLblockSizeLen; ASSERT(codePos < CodeBufferBitLen);

			// read RL code for signBits
			if (GetBit(m_codeBuffer, codePos)) {
				// RL coding is used
				codePos++;

				// read codeLen
				codeLen = GetValueBlock(m_codeBuffer, codePos, RLblockSizeLen);
				codePos += RLblockSizeLen;

				// RL decode signBits
				m_codePos = codePos;
				signLen = RLDSigns(bufferSize, codeLen, signBits);

				// adjust code buffer position
				codePos += codeLen;
				codePos = AlignWordPos(codePos); ASSERT(codePos < CodeBufferBitLen);
			} else {
				// RL coding of signBits was not efficient and therefore not used
				codePos++;

				// read signLen
				signLen = GetValueBlock(m_codeBuffer, codePos, RLblockSizeLen); ASSERT(signLen <= bufferSize);
				
				// adjust code buffer position
				codePos += RLblockSizeLen; 
				codePos = AlignWordPos(codePos); ASSERT(codePos < CodeBufferBitLen);

				// read signBits
				wordPos = codePos >> WordWidthLog; ASSERT(0 <= wordPos && wordPos < BufferSize);
				signLen = AlignWordPos(signLen) >> WordWidthLog;
				for (k=0; k < signLen; k++) {
					signBits[k] = m_codeBuffer[wordPos];
					wordPos++;
				}

				// adjust code buffer position
				codePos = wordPos << WordWidthLog; ASSERT(codePos < CodeBufferBitLen);
			}

			sigPos = codePos; // position of sigBits
			codePos += sigLen;

			codePos = AlignWordPos(codePos);
			ASSERT(codePos < CodeBufferBitLen);

			// read significant and refinement bitset from m_codeBuffer
			sigLen = ComposeBitplane(bufferSize, planeMask, &m_codeBuffer[sigPos >> WordWidthLog], &m_codeBuffer[codePos >> WordWidthLog], signBits);
			
			// adjust code buffer position
			codePos += bufferSize - sigLen;
			codePos = AlignWordPos(codePos);
		}
		planeMask >>= 1;
	}
}

////////////////////////////////////////////////////////////////////
// Reconstruct bitplane from significant bitset and refinement bitset
// returns length [bits] of sigBits
// input:  sigBits, refBits
// output: m_value
UINT32 CDecoder::ComposeBitplane(UINT32 bufferSize, UINT32 planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits) {
	ASSERT(sigBits);
	ASSERT(refBits);
	ASSERT(signBits);

	UINT32 valPos = 0, signPos = 0, refPos = 0;
	UINT32 sigPos = 0, sigEnd, sigRunLen;
	UINT32 zerocnt;

	while (valPos < bufferSize) {
		// search next 1 in m_sigFlagVector
		sigRunLen = SeekBitRange(m_sigFlagVector, valPos, bufferSize - valPos);

		// search 1's in sigBits[valuePos..valuePos+sigRunLen)
		// these 1's are significant bits
		sigEnd = sigPos + sigRunLen;
		while (sigPos < sigEnd) {
			// search 0's
			zerocnt = SeekBitRange(sigBits, sigPos, sigEnd - sigPos);
			sigPos += zerocnt;
			valPos += zerocnt;
			if (sigPos < sigEnd) {
				// write bit to m_value
				SetBitAtPos(valPos, planeMask);

				// copy sign bit
				SetSign(valPos, GetBit(signBits, signPos)); 
				signPos++;

				// update significance flag vector
				SetBit(m_sigFlagVector, valPos);
				sigPos++; 
				valPos++;
			}
		}
		// refinement bit
		if (valPos < bufferSize) {
			// write one refinement bit
			if (GetBit(refBits, refPos)) {
				SetBitAtPos(valPos, planeMask);
			}
			refPos++;
			valPos++;
		}
	}
	ASSERT(sigPos <= bufferSize);
	ASSERT(refPos <= bufferSize);
	ASSERT(signPos <= bufferSize);
	ASSERT(valPos == bufferSize);

	return sigPos;
}

//////////////////////////////////////////////////////////////////////
// Adaptive run-Length decoder for significant bits with a lot of zeros and sign bits.
// Returns length of sigBits.
// - Decode run of 2^k zeros by a single 0.
// - Decode run of count 0's followed by a 1 with codeword: 1<count>x
// - x is 0: if a positive sign has been stored, otherwise 1
// - Read each bit from m_codeBuffer[m_codePos] and increment m_codePos.
UINT32 CDecoder::RLDSigsAndSigns(UINT32 bufferSize, UINT32 codeLen, UINT32* sigBits, UINT32* signBits) {
	ASSERT(sigBits);
	ASSERT(signBits);
	ASSERT(m_codePos < CodeBufferBitLen);
	const UINT32 inEndPos = m_codePos + codeLen; ASSERT(inEndPos < CodeBufferBitLen);

	UINT32 k = 3;
	UINT32 runlen = 1 << k; // = 2^k
	UINT32 count = 0;
	UINT32 sigPos = 0, signPos = 0;

	ASSERT(inEndPos < CodeBufferBitLen);
	while (m_codePos < inEndPos) {
		if (GetBit(m_codeBuffer, m_codePos)) {
			m_codePos++;
			
			// extract counter and generate zero run of length count
			if (k > 0) {
				// extract counter
				count = GetValueBlock(m_codeBuffer, m_codePos, k); 
				m_codePos += k;
				if (count > 0) {
					ClearBitBlock(sigBits, sigPos, count); 
					sigPos += count;
				}
			}
			// write 1 bit
			if (sigPos < bufferSize) {
				SetBit(sigBits, sigPos); sigPos++;
			}

			// get sign bit
			if (GetBit(m_codeBuffer, m_codePos)) {
				SetBit(signBits, signPos);
			} else {
				ClearBit(signBits, signPos);
			}
			signPos++; m_codePos++;

			// adapt k (half run-length interval)
			if (k > 0) {
				k--;
				runlen >>= 1;
			}
		} else {
			m_codePos++;
			// generate zero run of length 2^k
			//ASSERT(sigPos + runlen <= BufferSize);
			ClearBitBlock(sigBits, sigPos, runlen); 
			sigPos += runlen;

			// adapt k (double run-length interval)
			if (k < WordWidth) {
				k++;
				runlen <<= 1;
			}
		}
	}
	return sigPos;
}

/////////////////////////////////////////////////////
// Adaptive run-length decoder.
// Returns signLen in bits.
// decodes codeLen bits from m_codeBuffer at m_codePos
// decode run of 2^k 1's by a single 1
// decode run of count 1's followed by a 0 with codeword: 0<count>
// output is stored in signBits
UINT32 CDecoder::RLDSigns(UINT32 bufferSize, UINT32 codeLen, UINT32* signBits) {
	ASSERT(signBits);
	ASSERT(0 <= m_codePos && m_codePos < CodeBufferBitLen);
	const UINT32 inEndPos = m_codePos + codeLen; ASSERT(inEndPos < CodeBufferBitLen);

	UINT32 k = 0;
	UINT32 runlen = 1 << k; // = 2^k
	UINT32 count = 0, signPos = 0;

	while (m_codePos < inEndPos) {
		if (GetBit(m_codeBuffer, m_codePos)) {
			m_codePos++;
			// generate 1's run of length 2^k
			SetBitBlock(signBits, signPos, runlen); 
			signPos += runlen;
			
			// adapt k (double run-length interval)
			if (k < WordWidth) {
				k++; 
				runlen <<= 1;
			}
		} else {
			m_codePos++;
			// extract counter and generate zero run of length count
			if (k > 0) {
				// extract counter
				count = GetValueBlock(m_codeBuffer, m_codePos, k); 
				m_codePos += k;
				if (count > 0) {
					SetBitBlock(signBits, signPos, count); 
					signPos += count;
				}
			}
			if (signPos < bufferSize) {
				ClearBit(signBits, signPos); 
				signPos++;
			}
			
			// adapt k (half run-length interval)
			if (k > 0) {
				k--; 
				runlen >>= 1;
			}
		}
	}
	return signPos;
}

////////////////////////////////////////////////////////////////////
#ifdef TRACE
void CDecoder::DumpBuffer() {
	printf("\nDump\n");
	for (int i=0; i < BufferSize; i++) {
		printf("%d", m_value[i]);
	}
}
#endif //TRACE
