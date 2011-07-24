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
/// @file Decoder.cpp
/// @brief PGF decoder class implementation
/// @author C. Stamm, R. Spuler

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
// Constructor
// Read pre-header, header, and levelLength
// It might throw an IOException.
CDecoder::CDecoder(CPGFStream* stream, PGFPreHeader& preHeader, PGFHeader& header, PGFPostHeader& postHeader, UINT32*& levelLength, bool useOMP /*= true*/) THROW_
: m_stream(stream)
, m_startPos(0)
, m_streamSizeEstimation(0)
, m_encodedHeaderLength(0)
, m_currentBlockIndex(0)
, m_macroBlocksAvailable(0)
#ifdef __PGFROISUPPORT__
, m_roi(false)
#endif
{
	ASSERT(m_stream);

	int count, expected;

	// set number of threads
#ifdef LIBPGF_USE_OPENMP
	m_macroBlockLen = omp_get_num_procs();
#else
	m_macroBlockLen = 1;
#endif

	if (useOMP && m_macroBlockLen > 1) {
#ifdef LIBPGF_USE_OPENMP
		omp_set_num_threads(m_macroBlockLen);
#endif

		// create macro block array
		m_macroBlocks = new CMacroBlock*[m_macroBlockLen];
		for (int i=0; i < m_macroBlockLen; i++) m_macroBlocks[i] = new CMacroBlock(this);
	} else {
		m_macroBlocks = 0;
		m_currentBlock = new CMacroBlock(this);
	}

	// store current stream position
	m_startPos = m_stream->GetPos();

	// read magic and version
	count = expected = MagicVersionSize;
	m_stream->Read(&count, &preHeader);
	if (count != expected) ReturnWithError(MissingData);

	// read header size
	if (preHeader.version & Version6) {
		// 32 bit header size since version 6
		count = expected = 4;
	} else {
		count = expected = 2;
	}
	m_stream->Read(&count, ((UINT8*)&preHeader) + MagicVersionSize);
	if (count != expected) ReturnWithError(MissingData);

	// make sure the values are correct read
	preHeader.hSize = __VAL(preHeader.hSize);

	// check magic number
	if (memcmp(preHeader.magic, Magic, 3) != 0) {
		// error condition: wrong Magic number
		ReturnWithError(FormatCannotRead);
	}

	// read file header
	count = expected = (preHeader.hSize < HeaderSize) ? preHeader.hSize : HeaderSize;
	m_stream->Read(&count, &header);
	if (count != expected) ReturnWithError(MissingData);

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
				count = expected = ColorTableSize;
				m_stream->Read(&count, postHeader.clut);
				if (count != expected) ReturnWithError(MissingData);
				size -= count;
			}

			if (size > 0) {
				// create user data memory block
				postHeader.userDataLen = size;
				postHeader.userData = new UINT8[postHeader.userDataLen];

				// read user data
				count = expected = postHeader.userDataLen;
				m_stream->Read(&count, postHeader.userData);
				if (count != expected) ReturnWithError(MissingData);
			}
		}

		// create levelLength
		levelLength = new UINT32[header.nLevels];
		if (!levelLength) ReturnWithError(InsufficientMemory);

		// read levelLength
		count = expected = header.nLevels*WordBytes;
		m_stream->Read(&count, levelLength);
		if (count != expected) ReturnWithError(MissingData);

#ifdef PGF_USE_BIG_ENDIAN
		// make sure the values are correct read
		for (int i=0; i < header.nLevels; i++) {
			levelLength[i] = __VAL(levelLength[i]);
		}
#endif

		// compute the total size in bytes; keep attention: level length information is optional
		for (int i=0; i < header.nLevels; i++) {
			m_streamSizeEstimation += levelLength[i];
		}

	}

	// store current stream position
	m_encodedHeaderLength = UINT32(m_stream->GetPos() - m_startPos);
}

/////////////////////////////////////////////////////////////////////
// Destructor
CDecoder::~CDecoder() {
	if (m_macroBlocks) {
		for (int i=0; i < m_macroBlockLen; i++) delete m_macroBlocks[i];
		delete[] m_macroBlocks;
	} else {
		delete m_currentBlock;
	}
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
	int pos, base = startPos, base2;

	// main height
	for (int i=0; i < hh.quot; i++) {
		// main width
		base2 = base;
		for (int j=0; j < ww.quot; j++) {
			pos = base2;
			for (int y=0; y < LinBlockSize; y++) {
				for (int x=0; x < LinBlockSize; x++) {
					DequantizeValue(band, pos, quantParam);
					pos++;
				}
				pos += ws;
			}
			base2 += LinBlockSize;
		}
		// rest of width
		pos = base2;
		for (int y=0; y < LinBlockSize; y++) {
			for (int x=0; x < ww.rem; x++) {
				DequantizeValue(band, pos, quantParam);
				pos++;
			}
			pos += wr;
			base += pitch;
		}
	}
	// main width
	base2 = base;
	for (int j=0; j < ww.quot; j++) {
		// rest of height
		pos = base2;
		for (int y=0; y < hh.rem; y++) {
			for (int x=0; x < LinBlockSize; x++) {
				DequantizeValue(band, pos, quantParam);
				pos++;
			}
			pos += ws;
		}
		base2 += LinBlockSize;
	}
	// rest of height
	pos = base2;
	for (int y=0; y < hh.rem; y++) {
		// rest of width
		for (int x=0; x < ww.rem; x++) {
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
// It might throw an IOException.
void CDecoder::DecodeInterleaved(CWaveletTransform* wtChannel, int level, int quantParam) THROW_ {
	CSubband* hlBand = wtChannel->GetSubband(level, HL);
	CSubband* lhBand = wtChannel->GetSubband(level, LH);
	const div_t lhH = div(lhBand->GetHeight(), InterBlockSize);
	const div_t hlW = div(hlBand->GetWidth(), InterBlockSize);
	const int hlws = hlBand->GetWidth() - InterBlockSize;
	const int hlwr = hlBand->GetWidth() - hlW.rem;
	const int lhws = lhBand->GetWidth() - InterBlockSize;
	const int lhwr = lhBand->GetWidth() - hlW.rem;
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
	for (int i=0; i < lhH.quot; i++) {
		// main width
		hlBase2 = hlBase;
		lhBase2 = lhBase;
		for (int j=0; j < hlW.quot; j++) {
			hlPos = hlBase2;
			lhPos = lhBase2;
			for (int y=0; y < InterBlockSize; y++) {
				for (int x=0; x < InterBlockSize; x++) {
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
		for (int y=0; y < InterBlockSize; y++) {
			for (int x=0; x < hlW.rem; x++) {
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
	for (int j=0; j < hlW.quot; j++) {
		// rest of height
		hlPos = hlBase2;
		lhPos = lhBase2;
		for (int y=0; y < lhH.rem; y++) {
			for (int x=0; x < InterBlockSize; x++) {
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
	for (int y=0; y < lhH.rem; y++) {
		// rest of width
		for (int x=0; x < hlW.rem; x++) {
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
		for (int j=0; j < hlBand->GetWidth(); j++) {
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
	if (!m_macroBlocksAvailable) {
		DecodeBuffer();
		ASSERT(m_currentBlock);
		ASSERT(m_currentBlock->m_valuePos == 0);
		ASSERT(m_macroBlocksAvailable);
	}
	band->SetData(bandPos, m_currentBlock->m_value[m_currentBlock->m_valuePos] << quantParam);
	m_currentBlock->m_valuePos++;
	if (m_currentBlock->m_valuePos == BufferSize) {
		// current block has been read
		m_macroBlocksAvailable--;
		if (m_macroBlocksAvailable)
			m_currentBlock = m_macroBlocks[++m_currentBlockIndex];
	}
}

//////////////////////////////////////////////////////////////////////
// Read next block from stream and store it in the given block
// It might throw an IOException.
void CDecoder::ReadMacroBlock(CMacroBlock* block) THROW_ {
	ASSERT(block);

	UINT16 wordLen;
	ROIBlockHeader h(BufferSize);
	int count, expected;

#ifdef TRACE
	//UINT32 filePos = (UINT32)m_stream->GetPos();
	//printf("DecodeBuffer: %d\n", filePos);
#endif

	// read wordLen
	count = expected = sizeof(UINT16);
	m_stream->Read(&count, &wordLen);
	if (count != expected) ReturnWithError(MissingData);
	wordLen = __VAL(wordLen);
	if (wordLen > BufferSize)
		ReturnWithError(FormatCannotRead);

#ifdef __PGFROISUPPORT__
	// read ROIBlockHeader
	if (m_roi) {
		m_stream->Read(&count, &h.val);
		if (count != expected) ReturnWithError(MissingData);

		// convert ROIBlockHeader
		h.val = __VAL(h.val);
	}
#endif
	// save header
	block->m_header = h;

	// read data
	count = expected = wordLen*WordBytes;
	m_stream->Read(&count, block->m_codeBuffer);
	if (count != expected) ReturnWithError(MissingData);

#ifdef PGF_USE_BIG_ENDIAN
	// convert data
	count /= WordBytes;
	for (int i=0; i < count; i++) {
		block->m_codeBuffer[i] = __VAL(block->m_codeBuffer[i]);
	}
#endif

#ifdef __PGFROISUPPORT__
	ASSERT(m_roi && h.rbh.bufferSize <= BufferSize || h.rbh.bufferSize == BufferSize);
#else
	ASSERT(h.rbh.bufferSize == BufferSize);
#endif
}

//////////////////////////////////////////////////////////////////////
// Read next block from stream but don't decode into macro block
// Encoding scheme: <wordLen>(16 bits) [ ROI ] data
//		ROI	  ::= <bufferSize>(15 bits) <eofTile>(1 bit)
// It might throw an IOException.
void CDecoder::SkipTileBuffer() THROW_ {
	// check if pre-decoded data is available
	if (m_macroBlocksAvailable) {
		// current block is not used
		m_macroBlocksAvailable--;
		if (m_macroBlocksAvailable)
			m_currentBlock = m_macroBlocks[++m_currentBlockIndex];
		return;
	}

	UINT16 wordLen;
	int count, expected;

	// read wordLen
	count = expected = sizeof(wordLen);
	m_stream->Read(&count, &wordLen);
	if (count != expected) ReturnWithError(MissingData);
	wordLen = __VAL(wordLen);
	ASSERT(wordLen <= BufferSize);

#ifdef __PGFROISUPPORT__
	if (m_roi) {
		// skip ROIBlockHeader
		m_stream->SetPos(FSFromCurrent, sizeof(ROIBlockHeader));
	}
#endif

	// skip data
	m_stream->SetPos(FSFromCurrent, wordLen*WordBytes);
}

//////////////////////////////////////////////////////////////////////
// Read next block from stream and decode into macro block
// It might throw an IOException.
void CDecoder::DecodeTileBuffer() THROW_ {
	if (m_macroBlocksAvailable) {
		// current block has been read
		m_macroBlocksAvailable--;
		if (m_macroBlocksAvailable)
			m_currentBlock = m_macroBlocks[++m_currentBlockIndex];
	} else {
		DecodeBuffer();
		ASSERT(m_currentBlock);
		ASSERT(m_currentBlock->m_valuePos == 0);
		ASSERT(m_macroBlocksAvailable);
	}
}

//////////////////////////////////////////////////////////////////////
// Read next block from stream and decode into macro block
// Decoding scheme: <wordLen>(16 bits) [ ROI ] data
//		ROI	  ::= <bufferSize>(15 bits) <eofTile>(1 bit)
// It might throw an IOException.
void CDecoder::DecodeBuffer() THROW_ {
	ASSERT(m_macroBlocksAvailable == 0);

	// macro block management
	if (m_macroBlockLen == 1) {
		ASSERT(m_currentBlock);
		ReadMacroBlock(m_currentBlock);
		m_currentBlock->BitplaneDecode();
		m_macroBlocksAvailable = 1;
	} else {
		for (int i=0; i < m_macroBlockLen; i++) {
			// read sequentially several blocks
			try {
				ReadMacroBlock(m_macroBlocks[i]);
				m_macroBlocksAvailable++;
			} catch(const IOException &ex) {
				if (ex.error == MissingData) {
					break; // no further levels available
				} else {
					throw;
				}
			}
		}

		// decode in parallel
		#pragma omp parallel for default(shared) //no declared exceptions in next block
		for (int i=0; i < m_macroBlocksAvailable; i++) {
			m_macroBlocks[i]->BitplaneDecode();
		}

		m_currentBlockIndex = 0;
		m_currentBlock = m_macroBlocks[m_currentBlockIndex];
	}
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
void CDecoder::CMacroBlock::BitplaneDecode() {
	UINT32 bufferSize = m_header.rbh.bufferSize; ASSERT(bufferSize <= BufferSize);

	UINT32 nPlanes;
	UINT32 codePos = 0, codeLen, sigLen, sigPos, signLen, signPos;
	DataT planeMask;

	// clear significance vector
	for (UINT32 k=0; k < bufferSize; k++) {
		m_sigFlagVector[k] = false;
	}
	m_sigFlagVector[bufferSize] = true; // sentinel

	// clear output buffer
	for (UINT32 k=0; k < BufferSize; k++) {
		m_value[k] = 0;
	}

	// read number of bit planes
	nPlanes = GetValueBlock(m_codeBuffer, 0, MaxBitPlanesLog);
	codePos += MaxBitPlanesLog;

	// loop through all bit planes
	if (nPlanes == 0) nPlanes = MaxBitPlanes + 1;
	ASSERT(0 < nPlanes && nPlanes <= MaxBitPlanes + 1);
	planeMask = 1 << (nPlanes - 1);

	for (int plane = nPlanes - 1; plane >= 0; plane--) {
		// read RL code
		if (GetBit(m_codeBuffer, codePos)) {
			// RL coding of sigBits is used
			codePos++;

			// read codeLen
			codeLen = GetValueBlock(m_codeBuffer, codePos, RLblockSizeLen); ASSERT(codeLen < (1 << RLblockSizeLen));

			// position of encoded sigBits and signBits
			sigPos = codePos + RLblockSizeLen; ASSERT(sigPos < CodeBufferBitLen);

			// refinement bits
			codePos = AlignWordPos(sigPos + codeLen); ASSERT(codePos < CodeBufferBitLen);

			// run-length decode significant bits and signs from m_codeBuffer and
			// read refinement bits from m_codeBuffer and compose bit plane
			sigLen = ComposeBitplaneRLD(bufferSize, planeMask, sigPos, &m_codeBuffer[codePos >> WordWidthLog]);

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

				// sign bits
				signPos = codePos + RLblockSizeLen; ASSERT(signPos < CodeBufferBitLen);

				// significant bits
				sigPos = AlignWordPos(signPos + codeLen); ASSERT(sigPos < CodeBufferBitLen);

				// refinement bits
				codePos = AlignWordPos(sigPos + sigLen); ASSERT(codePos < CodeBufferBitLen);

				// read significant and refinement bitset from m_codeBuffer
				sigLen = ComposeBitplaneRLD(bufferSize, planeMask, &m_codeBuffer[sigPos >> WordWidthLog], &m_codeBuffer[codePos >> WordWidthLog], &m_codeBuffer[signPos >> WordWidthLog]);

			} else {
				// RL coding of signBits was not efficient and therefore not used
				codePos++;

				// read signLen
				signLen = AlignWordPos(GetValueBlock(m_codeBuffer, codePos, RLblockSizeLen)); ASSERT(signLen <= bufferSize);

				// sign bits
				signPos = AlignWordPos(codePos + RLblockSizeLen); ASSERT(signPos < CodeBufferBitLen);

				// significant bits
				sigPos = signPos + signLen; ASSERT(sigPos < CodeBufferBitLen);

				// refinement bits
				codePos = AlignWordPos(sigPos + sigLen); ASSERT(codePos < CodeBufferBitLen);

				// read significant and refinement bitset from m_codeBuffer
				sigLen = ComposeBitplane(bufferSize, planeMask, &m_codeBuffer[sigPos >> WordWidthLog], &m_codeBuffer[codePos >> WordWidthLog], &m_codeBuffer[signPos >> WordWidthLog]);
			}
		}

		// start of next chunk
		codePos = AlignWordPos(codePos + bufferSize - sigLen); ASSERT(codePos < CodeBufferBitLen);

		// next plane
		planeMask >>= 1;
	}

	m_valuePos = 0;
}

////////////////////////////////////////////////////////////////////
// Reconstruct bitplane from significant bitset and refinement bitset
// returns length [bits] of sigBits
// input:  sigBits, refBits, signBits
// output: m_value
UINT32 CDecoder::CMacroBlock::ComposeBitplane(UINT32 bufferSize, DataT planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits) {
	ASSERT(sigBits);
	ASSERT(refBits);
	ASSERT(signBits);

	UINT32 valPos = 0, signPos = 0, refPos = 0;
	UINT32 sigPos = 0, sigEnd;
	UINT32 zerocnt;

	while (valPos < bufferSize) {
		// search next 1 in m_sigFlagVector using searching with sentinel
		sigEnd = valPos;
		while(!m_sigFlagVector[sigEnd]) { sigEnd++; }
		sigEnd -= valPos;
		sigEnd += sigPos;

		// search 1's in sigBits[sigPos..sigEnd)
		// these 1's are significant bits
		while (sigPos < sigEnd) {
			// search 0's
			zerocnt = SeekBitRange(sigBits, sigPos, sigEnd - sigPos);
			sigPos += zerocnt;
			valPos += zerocnt;
			if (sigPos < sigEnd) {
				// write bit to m_value
				SetBitAtPos(valPos, planeMask);

				// copy sign bit
				SetSign(valPos, GetBit(signBits, signPos++));

				// update significance flag vector
				m_sigFlagVector[valPos++] = true;
				sigPos++;
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

////////////////////////////////////////////////////////////////////
// Reconstruct bitplane from significant bitset and refinement bitset
// returns length [bits] of decoded significant bits
// input:  RL encoded sigBits and signBits in m_codeBuffer, refBits
// output: m_value
// RLE:
// - Decode run of 2^k zeros by a single 0.
// - Decode run of count 0's followed by a 1 with codeword: 1<count>x
// - x is 0: if a positive sign has been stored, otherwise 1
// - Read each bit from m_codeBuffer[codePos] and increment codePos.
UINT32 CDecoder::CMacroBlock::ComposeBitplaneRLD(UINT32 bufferSize, DataT planeMask, UINT32 codePos, UINT32* refBits) {
	ASSERT(refBits);

	UINT32 valPos = 0, refPos = 0;
	UINT32 sigPos = 0, sigEnd;
	UINT32 k = 3;
	UINT32 runlen = 1 << k; // = 2^k
	UINT32 count = 0, rest = 0;
	bool set1 = false;

	while (valPos < bufferSize) {
		// search next 1 in m_sigFlagVector using searching with sentinel
		sigEnd = valPos;
		while(!m_sigFlagVector[sigEnd]) { sigEnd++; }
		sigEnd -= valPos;
		sigEnd += sigPos;

		while (sigPos < sigEnd) {
			if (rest || set1) {
				// rest of last run
				sigPos += rest;
				valPos += rest;
				rest = 0;
			} else {
				// decode significant bits
				if (GetBit(m_codeBuffer, codePos++)) {
					// extract counter and generate zero run of length count
					if (k > 0) {
						// extract counter
						count = GetValueBlock(m_codeBuffer, codePos, k);
						codePos += k;
						if (count > 0) {
							sigPos += count;
							valPos += count;
						}

						// adapt k (half run-length interval)
						k--;
						runlen >>= 1;
					}

					set1 = true;

				} else {
					// generate zero run of length 2^k
					sigPos += runlen;
					valPos += runlen;

					// adapt k (double run-length interval)
					if (k < WordWidth) {
						k++;
						runlen <<= 1;
					}
				}
			}

			if (sigPos < sigEnd) {
				if (set1) {
					set1 = false;

					// write 1 bit
					SetBitAtPos(valPos, planeMask);

					// set sign bit
					SetSign(valPos, GetBit(m_codeBuffer, codePos++));

					// update significance flag vector
					m_sigFlagVector[valPos++] = true;
					sigPos++;
				}
			} else {
				rest = sigPos - sigEnd;
				sigPos = sigEnd;
				valPos -= rest;
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
	ASSERT(valPos == bufferSize);

	return sigPos;
}

////////////////////////////////////////////////////////////////////
// Reconstruct bitplane from significant bitset, refinement bitset, and RL encoded sign bits
// returns length [bits] of sigBits
// input:  sigBits, refBits, RL encoded signBits
// output: m_value
// RLE:
// decode run of 2^k 1's by a single 1
// decode run of count 1's followed by a 0 with codeword: 0<count>
UINT32 CDecoder::CMacroBlock::ComposeBitplaneRLD(UINT32 bufferSize, DataT planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits) {
	ASSERT(sigBits);
	ASSERT(refBits);
	ASSERT(signBits);

	UINT32 valPos = 0, signPos = 0, refPos = 0;
	UINT32 sigPos = 0, sigEnd;
	UINT32 zerocnt, count = 0;
	UINT32 k = 0;
	UINT32 runlen = 1 << k; // = 2^k
	bool signBit = false;
	bool zeroAfterRun = false;

	while (valPos < bufferSize) {
		// search next 1 in m_sigFlagVector using searching with sentinel
		sigEnd = valPos;
		while(!m_sigFlagVector[sigEnd]) { sigEnd++; }
		sigEnd -= valPos;
		sigEnd += sigPos;

		// search 1's in sigBits[sigPos..sigEnd)
		// these 1's are significant bits
		while (sigPos < sigEnd) {
			// search 0's
			zerocnt = SeekBitRange(sigBits, sigPos, sigEnd - sigPos);
			sigPos += zerocnt;
			valPos += zerocnt;
			if (sigPos < sigEnd) {
				// write bit to m_value
				SetBitAtPos(valPos, planeMask);

				// check sign bit
				if (count == 0) {
					// all 1's have been set
					if (zeroAfterRun) {
						// finish the run with a 0
						signBit = false;
						zeroAfterRun = false;
					} else {
						// decode next sign bit
						if (GetBit(signBits, signPos++)) {
							// generate 1's run of length 2^k
							count = runlen - 1;
							signBit = true;

							// adapt k (double run-length interval)
							if (k < WordWidth) {
								k++;
								runlen <<= 1;
							}
						} else {
							// extract counter and generate 1's run of length count
							if (k > 0) {
								// extract counter
								count = GetValueBlock(signBits, signPos, k);
								signPos += k;

								// adapt k (half run-length interval)
								k--;
								runlen >>= 1;
							}
							if (count > 0) {
								count--;
								signBit = true;
								zeroAfterRun = true;
							} else {
								signBit = false;
							}
						}
					}
				} else {
					ASSERT(count > 0);
					ASSERT(signBit);
					count--;
				}

				// copy sign bit
				SetSign(valPos, signBit);

				// update significance flag vector
				m_sigFlagVector[valPos++] = true;
				sigPos++;
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

////////////////////////////////////////////////////////////////////
#ifdef TRACE
void CDecoder::DumpBuffer() {
	//printf("\nDump\n");
	//for (int i=0; i < BufferSize; i++) {
	//	printf("%d", m_value[i]);
	//}
}
#endif //TRACE
