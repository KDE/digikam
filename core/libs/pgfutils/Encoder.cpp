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

//////////////////////////////////////////////////////////////////////
/// @file Encoder.cpp
/// @brief PGF encoder class implementation
/// @author C. Stamm, R. Spuler

#include "Encoder.h"
#ifdef TRACE
	#include <stdio.h>
#endif

//////////////////////////////////////////////////////
// PGF: file structure
//
// PGFPreHeader PGFHeader [PGFPostHeader] LevelLengths Level_n-1 Level_n-2 ... Level_0
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
#define CodeBufferBitLen		(CodeBufferLen*WordWidth)	///< max number of bits in m_codeBuffer
#define MaxCodeLen				((1 << RLblockSizeLen) - 1)	///< max length of RL encoded block

//////////////////////////////////////////////////////
/// Write pre-header, header, postHeader, and levelLength.
/// It might throw an IOException.
/// @param stream A PGF stream
/// @param preHeader A already filled in PGF pre-header
/// @param header An already filled in PGF header
/// @param postHeader [in] An already filled in PGF post-header (containing color table, user data, ...)
/// @param userDataPos [out] File position of user data
/// @param useOMP If true, then the encoder will use multi-threading based on openMP
CEncoder::CEncoder(CPGFStream* stream, PGFPreHeader preHeader, PGFHeader header, const PGFPostHeader& postHeader, UINT64& userDataPos, bool useOMP)
: m_stream(stream)
, m_bufferStartPos(0)
, m_currLevelIndex(0)
, m_nLevels(header.nLevels)
, m_favorSpeed(false)
, m_forceWriting(false)
#ifdef __PGFROISUPPORT__
, m_roi(false)
#endif
{
	ASSERT(m_stream);

	int count;
	m_lastMacroBlock = 0;
	m_levelLength = nullptr;

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
		m_macroBlocks = new(std::nothrow) CMacroBlock*[m_macroBlockLen];
		if (!m_macroBlocks) ReturnWithError(InsufficientMemory);
		for (int i=0; i < m_macroBlockLen; i++) m_macroBlocks[i] = new CMacroBlock(this);
		m_currentBlock = m_macroBlocks[m_lastMacroBlock++];
	} else {
		m_macroBlocks = 0;
		m_macroBlockLen = 1;
		m_currentBlock = new CMacroBlock(this);
	}

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
	// save user data file position
	userDataPos = m_stream->GetPos();
	if (postHeader.userDataLen) {
		if (postHeader.userData) {
			// write user data
			count = postHeader.userDataLen;
			m_stream->Write(&count, postHeader.userData);
		} else {
			m_stream->SetPos(FSFromCurrent, count);
		}
	}

	// save level length file position
	m_levelLengthPos = m_stream->GetPos();
}

//////////////////////////////////////////////////////
// Destructor
CEncoder::~CEncoder() {
	if (m_macroBlocks) {
		for (int i=0; i < m_macroBlockLen; i++) delete m_macroBlocks[i];
		delete[] m_macroBlocks;
	} else {
		delete m_currentBlock;
	}
}

/////////////////////////////////////////////////////////////////////
/// Increase post-header size and write new size into stream.
/// @param preHeader An already filled in PGF pre-header
/// It might throw an IOException.
void CEncoder::UpdatePostHeaderSize(PGFPreHeader preHeader) {
	UINT64 curPos = m_stream->GetPos(); // end of user data
	int count = PreHeaderSize;

	// write preHeader
	SetStreamPosToStart();
	preHeader.hSize = __VAL(preHeader.hSize);
	m_stream->Write(&count, &preHeader);

	m_stream->SetPos(FSFromStart, curPos);
}

/////////////////////////////////////////////////////////////////////
/// Create level length data structure and write a place holder into stream.
/// It might throw an IOException.
/// @param levelLength A reference to an integer array, large enough to save the relative file positions of all PGF levels
/// @return number of bytes written into stream
UINT32 CEncoder::WriteLevelLength(UINT32*& levelLength) {
	// renew levelLength
	delete[] levelLength;
	levelLength = new(std::nothrow) UINT32[m_nLevels];
	if (!levelLength) ReturnWithError(InsufficientMemory);
	for (UINT8 l = 0; l < m_nLevels; l++) levelLength[l] = 0;
	m_levelLength = levelLength;

	// save level length file position
	m_levelLengthPos = m_stream->GetPos();

	// write dummy levelLength
	int count = m_nLevels*WordBytes;
	m_stream->Write(&count, m_levelLength);

	// save current file position
	SetBufferStartPos();

	return count;
}

//////////////////////////////////////////////////////
/// Write new levelLength into stream.
/// It might throw an IOException.
/// @return Written image bytes.
UINT32 CEncoder::UpdateLevelLength() {
	UINT64 curPos = m_stream->GetPos(); // end of image

	// set file pos to levelLength
	m_stream->SetPos(FSFromStart, m_levelLengthPos);

	if (m_levelLength) {
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
	} else {
		int count = m_currLevelIndex*WordBytes;
		m_stream->SetPos(FSFromCurrent, count);
	}

	// begin of image
	UINT32 retValue = UINT32(curPos - m_stream->GetPos());

	// restore file position
	m_stream->SetPos(FSFromStart, curPos);

	return retValue;
}

/////////////////////////////////////////////////////////////////////
/// Partitions a rectangular region of a given subband.
/// Partitioning scheme: The plane is partitioned in squares of side length LinBlockSize.
/// Write wavelet coefficients from subband into the input buffer of a macro block.
/// It might throw an IOException.
/// @param band A subband
/// @param width The width of the rectangle
/// @param height The height of the rectangle
/// @param startPos The absolute subband position of the top left corner of the rectangular region
/// @param pitch The number of bytes in row of the subband
void CEncoder::Partition(CSubband* band, int width, int height, int startPos, int pitch) {
	ASSERT(band);

	const div_t hh = div(height, LinBlockSize);
	const div_t ww = div(width, LinBlockSize);
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
					WriteValue(band, pos);
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
				WriteValue(band, pos);
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
				WriteValue(band, pos);
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
			WriteValue(band, pos);
			pos++;
		}
		pos += wr;
	}
}

//////////////////////////////////////////////////////
/// Pad buffer with zeros and encode buffer.
/// It might throw an IOException.
void CEncoder::Flush() {
	if (m_currentBlock->m_valuePos > 0) {
		// pad buffer with zeros
		memset(&(m_currentBlock->m_value[m_currentBlock->m_valuePos]), 0, (BufferSize - m_currentBlock->m_valuePos)*DataTSize);
		m_currentBlock->m_valuePos = BufferSize;

		// encode buffer
		m_forceWriting = true;	// makes sure that the following EncodeBuffer is really written into the stream
		EncodeBuffer(ROIBlockHeader(m_currentBlock->m_valuePos, true));
	}
}

/////////////////////////////////////////////////////////////////////
// Stores band value from given position bandPos into buffer m_value at position m_valuePos
// If buffer is full encode it to file
// It might throw an IOException.
void CEncoder::WriteValue(CSubband* band, int bandPos) {
	if (m_currentBlock->m_valuePos == BufferSize) {
		EncodeBuffer(ROIBlockHeader(BufferSize, false));
	}
	DataT val = m_currentBlock->m_value[m_currentBlock->m_valuePos++] = band->GetData(bandPos);
	UINT32 v = abs(val);
	if (v > m_currentBlock->m_maxAbsValue) m_currentBlock->m_maxAbsValue = v;
}

/////////////////////////////////////////////////////////////////////
// Encode buffer and write data into stream.
// h contains buffer size and flag indicating end of tile.
// Encoding scheme: <wordLen>(16 bits) [ ROI ] data
//		ROI	  ::= <bufferSize>(15 bits) <eofTile>(1 bit)
// It might throw an IOException.
void CEncoder::EncodeBuffer(ROIBlockHeader h) {
	ASSERT(m_currentBlock);
#ifdef __PGFROISUPPORT__
	ASSERT(m_roi && h.rbh.bufferSize <= BufferSize || h.rbh.bufferSize == BufferSize);
#else
	ASSERT(h.rbh.bufferSize == BufferSize);
#endif
	m_currentBlock->m_header = h;

	// macro block management
	if (m_macroBlockLen == 1) {
		m_currentBlock->BitplaneEncode();
		WriteMacroBlock(m_currentBlock);
	} else {
		// save last level index
		int lastLevelIndex = m_currentBlock->m_lastLevelIndex;

		if (m_forceWriting || m_lastMacroBlock == m_macroBlockLen) {
			// encode macro blocks
			/*
			volatile OSError error = NoError;
			#ifdef LIBPGF_USE_OPENMP
			#pragma omp parallel for ordered default(shared)
			#endif
			for (int i=0; i < m_lastMacroBlock; i++) {
				if (error == NoError) {
					m_macroBlocks[i]->BitplaneEncode();
					#ifdef LIBPGF_USE_OPENMP
					#pragma omp ordered
					#endif
					{
						try {
							WriteMacroBlock(m_macroBlocks[i]);
						} catch (IOException& e) {
							error = e.error;
						}
						delete m_macroBlocks[i]; m_macroBlocks[i] = 0;
					}
				}
			}
			if (error != NoError) ReturnWithError(error);
			*/
#ifdef LIBPGF_USE_OPENMP
			#pragma omp parallel for default(shared) //no declared exceptions in next block
#endif
			for (int i=0; i < m_lastMacroBlock; i++) {
				m_macroBlocks[i]->BitplaneEncode();
			}
			for (int i=0; i < m_lastMacroBlock; i++) {
				WriteMacroBlock(m_macroBlocks[i]);
			}

			// prepare for next round
			m_forceWriting = false;
			m_lastMacroBlock = 0;
		}
		// re-initialize macro block
		m_currentBlock = m_macroBlocks[m_lastMacroBlock++];
		m_currentBlock->Init(lastLevelIndex);
	}
}

/////////////////////////////////////////////////////////////////////
// Write encoded macro block into stream.
// It might throw an IOException.
void CEncoder::WriteMacroBlock(CMacroBlock* block) {
	ASSERT(block);
#ifdef __PGFROISUPPORT__
	ROIBlockHeader h = block->m_header;
#endif
	UINT16 wordLen = UINT16(NumberOfWords(block->m_codePos)); ASSERT(wordLen <= CodeBufferLen);
	int count = sizeof(UINT16);

#ifdef TRACE
	//UINT32 filePos = (UINT32)m_stream->GetPos();
	//printf("EncodeBuffer: %d\n", filePos);
#endif

#ifdef PGF_USE_BIG_ENDIAN
	// write wordLen
	UINT16 wl = __VAL(wordLen);
	m_stream->Write(&count, &wl); ASSERT(count == sizeof(UINT16));

#ifdef __PGFROISUPPORT__
	// write ROIBlockHeader
	if (m_roi) {
		count = sizeof(ROIBlockHeader);
		h.val = __VAL(h.val);
		m_stream->Write(&count, &h.val); ASSERT(count == sizeof(ROIBlockHeader));
	}
#endif // __PGFROISUPPORT__

	// convert data
	for (int i=0; i < wordLen; i++) {
		block->m_codeBuffer[i] = __VAL(block->m_codeBuffer[i]);
	}
#else
	// write wordLen
	m_stream->Write(&count, &wordLen); ASSERT(count == sizeof(UINT16));

#ifdef __PGFROISUPPORT__
	// write ROIBlockHeader
	if (m_roi) {
		count = sizeof(ROIBlockHeader);
		m_stream->Write(&count, &h.val); ASSERT(count == sizeof(ROIBlockHeader));
	}
#endif // __PGFROISUPPORT__
#endif // PGF_USE_BIG_ENDIAN

	// write encoded data into stream
	count = wordLen*WordBytes;
	m_stream->Write(&count, block->m_codeBuffer);

	// store levelLength
	if (m_levelLength) {
		// store level length
		// EncodeBuffer has been called after m_lastLevelIndex has been updated
		ASSERT(m_currLevelIndex < m_nLevels);
		m_levelLength[m_currLevelIndex] += (UINT32)ComputeBufferLength();
		m_currLevelIndex = block->m_lastLevelIndex + 1;

	}

	// prepare for next buffer
	SetBufferStartPos();

	// reset values
	block->m_valuePos = 0;
	block->m_maxAbsValue = 0;
}

////////////////////////////////////////////////////////
// Encode buffer of given size using bit plane coding.
// A buffer contains bufferLen UINT32 values, thus, bufferSize bits per bit plane.
// Following coding scheme is used:
//		Buffer		::= <nPlanes>(5 bits) foreach(plane i): Plane[i]
//		Plane[i]	::= [ Sig1 | Sig2 ] [DWORD alignment] refBits
//		Sig1		::= 1 <codeLen>(15 bits) codedSigAndSignBits
//		Sig2		::= 0 <sigLen>(15 bits) [Sign1 | Sign2 ] [DWORD alignment] sigBits
//		Sign1		::= 1 <codeLen>(15 bits) codedSignBits
//		Sign2		::= 0 <signLen>(15 bits) [DWORD alignment] signBits
void CEncoder::CMacroBlock::BitplaneEncode() {
	UINT8	nPlanes;
	UINT32	sigLen, codeLen = 0, wordPos, refLen, signLen;
	UINT32  sigBits[BufferLen] = { 0 };
	UINT32  refBits[BufferLen] = { 0 };
	UINT32  signBits[BufferLen] = { 0 };
	UINT32  planeMask;
	UINT32	bufferSize = m_header.rbh.bufferSize; ASSERT(bufferSize <= BufferSize);
	bool	useRL;

#ifdef TRACE
	//printf("which thread: %d\n", omp_get_thread_num());
#endif

	// clear significance vector
	for (UINT32 k=0; k < bufferSize; k++) {
		m_sigFlagVector[k] = false;
	}
	m_sigFlagVector[bufferSize] = true; // sentinel

	// clear output buffer
	for (UINT32 k=0; k < bufferSize; k++) {
		m_codeBuffer[k] = 0;
	}
	m_codePos = 0;

	// compute number of bit planes and split buffer into separate bit planes
	nPlanes = NumberOfBitplanes();

	// write number of bit planes to m_codeBuffer
	// <nPlanes>
	SetValueBlock(m_codeBuffer, 0, nPlanes, MaxBitPlanesLog);
	m_codePos += MaxBitPlanesLog;

	// loop through all bit planes
	if (nPlanes == 0) nPlanes = MaxBitPlanes + 1;
	planeMask = 1 << (nPlanes - 1);

	for (int plane = nPlanes - 1; plane >= 0; plane--) {
		// clear significant bitset
		for (UINT32 k=0; k < BufferLen; k++) {
			sigBits[k] = 0;
		}

		// split bitplane in significant bitset and refinement bitset
		sigLen = DecomposeBitplane(bufferSize, planeMask, m_codePos + RLblockSizeLen + 1, sigBits, refBits, signBits, signLen, codeLen);

		if (sigLen > 0 && codeLen <= MaxCodeLen && codeLen < AlignWordPos(sigLen) + AlignWordPos(signLen) + 2*RLblockSizeLen) {
			// set RL code bit
			// <1><codeLen>
			SetBit(m_codeBuffer, m_codePos++);

			// write length codeLen to m_codeBuffer
			SetValueBlock(m_codeBuffer, m_codePos, codeLen, RLblockSizeLen);
			m_codePos += RLblockSizeLen + codeLen;
		} else {
		#ifdef TRACE
			//printf("new\n");
			//for (UINT32 i=0; i < bufferSize; i++) {
			//	printf("%s", (GetBit(sigBits, i)) ? "1" : "_");
			//	if (i%120 == 119) printf("\n");
			//}
			//printf("\n");
		#endif // TRACE

			// run-length coding wasn't efficient enough
			// we don't use RL coding for sigBits
			// <0><sigLen>
			ClearBit(m_codeBuffer, m_codePos++);

			// write length sigLen to m_codeBuffer
			ASSERT(sigLen <= MaxCodeLen);
			SetValueBlock(m_codeBuffer, m_codePos, sigLen, RLblockSizeLen);
			m_codePos += RLblockSizeLen;

			if (m_encoder->m_favorSpeed || signLen == 0) {
				useRL = false;
			} else {
				// overwrite m_codeBuffer
				useRL = true;
				// run-length encode m_sign and append them to the m_codeBuffer
				codeLen = RLESigns(m_codePos + RLblockSizeLen + 1, signBits, signLen);
			}

			if (useRL && codeLen <= MaxCodeLen && codeLen < signLen) {
				// RL encoding of m_sign was efficient
				// <1><codeLen><codedSignBits>_
				// write RL code bit
				SetBit(m_codeBuffer, m_codePos++);

				// write codeLen to m_codeBuffer
				SetValueBlock(m_codeBuffer, m_codePos, codeLen, RLblockSizeLen);

				// compute position of sigBits
				wordPos = NumberOfWords(m_codePos + RLblockSizeLen + codeLen);
				ASSERT(0 <= wordPos && wordPos < CodeBufferLen);
			} else {
				// RL encoding of signBits wasn't efficient
				// <0><signLen>_<signBits>_
				// clear RL code bit
				ClearBit(m_codeBuffer, m_codePos++);

				// write signLen to m_codeBuffer
				ASSERT(signLen <= MaxCodeLen);
				SetValueBlock(m_codeBuffer, m_codePos, signLen, RLblockSizeLen);

				// write signBits to m_codeBuffer
				wordPos = NumberOfWords(m_codePos + RLblockSizeLen);
				ASSERT(0 <= wordPos && wordPos < CodeBufferLen);
				codeLen = NumberOfWords(signLen);

				for (UINT32 k=0; k < codeLen; k++) {
					m_codeBuffer[wordPos++] = signBits[k];
				}
			}

			// write sigBits
			// <sigBits>_
			ASSERT(0 <= wordPos && wordPos < CodeBufferLen);
			refLen = NumberOfWords(sigLen);

			for (UINT32 k=0; k < refLen; k++) {
				m_codeBuffer[wordPos++] = sigBits[k];
			}
			m_codePos = wordPos << WordWidthLog;
		}

		// append refinement bitset (aligned to word boundary)
		// _<refBits>
		wordPos = NumberOfWords(m_codePos);
		ASSERT(0 <= wordPos && wordPos < CodeBufferLen);
		refLen = NumberOfWords(bufferSize - sigLen);

		for (UINT32 k=0; k < refLen; k++) {
			m_codeBuffer[wordPos++] = refBits[k];
		}
		m_codePos = wordPos << WordWidthLog;
		planeMask >>= 1;
	}
	ASSERT(0 <= m_codePos && m_codePos <= CodeBufferBitLen);
}

//////////////////////////////////////////////////////////
// Split bitplane of length bufferSize into significant and refinement bitset
// returns length [bits] of significant bits
// input:  bufferSize, planeMask, codePos
// output: sigBits, refBits, signBits, signLen [bits], codeLen [bits]
// RLE
// - Encode run of 2^k zeros by a single 0.
// - Encode run of count 0's followed by a 1 with codeword: 1<count>x
// - x is 0: if a positive sign is stored, otherwise 1
// - Store each bit in m_codeBuffer[codePos] and increment codePos.
UINT32 CEncoder::CMacroBlock::DecomposeBitplane(UINT32 bufferSize, UINT32 planeMask, UINT32 codePos, UINT32* sigBits, UINT32* refBits, UINT32* signBits, UINT32& signLen, UINT32& codeLen) {
	ASSERT(sigBits);
	ASSERT(refBits);
	ASSERT(signBits);
	ASSERT(codePos < CodeBufferBitLen);

	UINT32 sigPos = 0;
	UINT32 valuePos = 0, valueEnd;
	UINT32 refPos = 0;

	// set output value
	signLen = 0;

	// prepare RLE of Sigs and Signs
	const UINT32 outStartPos = codePos;
	UINT32 k = 3;
	UINT32 runlen = 1 << k; // = 2^k
	UINT32 count = 0;

	while (valuePos < bufferSize) {
		// search next 1 in m_sigFlagVector using searching with sentinel
		valueEnd = valuePos;
		while(!m_sigFlagVector[valueEnd]) { valueEnd++; }

		// search 1's in m_value[plane][valuePos..valueEnd)
		// these 1's are significant bits
		while (valuePos < valueEnd) {
			if (GetBitAtPos(valuePos, planeMask)) {
				// RLE encoding
				// encode run of count 0's followed by a 1
				// with codeword: 1<count>(signBits[signPos])
				SetBit(m_codeBuffer, codePos++);
				if (k > 0) {
					SetValueBlock(m_codeBuffer, codePos, count, k);
					codePos += k;

					// adapt k (half the zero run-length)
					k--;
					runlen >>= 1;
				}

				// copy and write sign bit
				if (m_value[valuePos] < 0) {
					SetBit(signBits, signLen++);
					SetBit(m_codeBuffer, codePos++);
				} else {
					ClearBit(signBits, signLen++);
					ClearBit(m_codeBuffer, codePos++);
				}

				// write a 1 to sigBits
				SetBit(sigBits, sigPos++);

				// update m_sigFlagVector
				m_sigFlagVector[valuePos] = true;

				// prepare for next run
				count = 0;
			} else {
				// RLE encoding
				count++;
				if (count == runlen) {
					// encode run of 2^k zeros by a single 0
					ClearBit(m_codeBuffer, codePos++);
					// adapt k (double the zero run-length)
					if (k < WordWidth) {
						k++;
						runlen <<= 1;
					}

					// prepare for next run
					count = 0;
				}

				// write 0 to sigBits
				sigPos++;
			}
			valuePos++;
		}
		// refinement bit
		if (valuePos < bufferSize) {
			// write one refinement bit
			if (GetBitAtPos(valuePos++, planeMask)) {
				SetBit(refBits, refPos);
			} else {
				ClearBit(refBits, refPos);
			}
			refPos++;
		}
	}
	// RLE encoding of the rest of the plane
	// encode run of count 0's followed by a 1
	// with codeword: 1<count>(signBits[signPos])
	SetBit(m_codeBuffer, codePos++);
	if (k > 0) {
		SetValueBlock(m_codeBuffer, codePos, count, k);
		codePos += k;
	}
	// write dmmy sign bit
	SetBit(m_codeBuffer, codePos++);

	// write word filler zeros

	ASSERT(sigPos <= bufferSize);
	ASSERT(refPos <= bufferSize);
	ASSERT(signLen <= bufferSize);
	ASSERT(valuePos == bufferSize);
	ASSERT(codePos >= outStartPos && codePos < CodeBufferBitLen);
	codeLen = codePos - outStartPos;

	return sigPos;
}


///////////////////////////////////////////////////////
// Compute number of bit planes needed
UINT8 CEncoder::CMacroBlock::NumberOfBitplanes() {
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

//////////////////////////////////////////////////////
// Adaptive Run-Length encoder for long sequences of ones.
// Returns length of output in bits.
// - Encode run of 2^k ones by a single 1.
// - Encode run of count 1's followed by a 0 with codeword: 0<count>.
// - Store each bit in m_codeBuffer[codePos] and increment codePos.
UINT32 CEncoder::CMacroBlock::RLESigns(UINT32 codePos, UINT32* signBits, UINT32 signLen) {
	ASSERT(signBits);
	ASSERT(0 <= codePos && codePos < CodeBufferBitLen);
	ASSERT(0 < signLen && signLen <= BufferSize);

	const UINT32  outStartPos = codePos;
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
			SetBit(m_codeBuffer, codePos++);
			// adapt k (double the 1's run-length)
			if (k < WordWidth) {
				k++;
				runlen <<= 1;
			}
		} else {
			// encode run of count 1's followed by a 0
			// with codeword: 0(count)
			signPos += count + 1;
			ClearBit(m_codeBuffer, codePos++);
			if (k > 0) {
				SetValueBlock(m_codeBuffer, codePos, count, k);
				codePos += k;
			}
			// adapt k (half the 1's run-length)
			if (k > 0) {
				k--;
				runlen >>= 1;
			}
		}
	}
	ASSERT(signPos == signLen || signPos == signLen + 1);
	ASSERT(codePos >= outStartPos && codePos < CodeBufferBitLen);
	return codePos - outStartPos;
}

//////////////////////////////////////////////////////
#ifdef TRACE
void CEncoder::DumpBuffer() const {
	//printf("\nDump\n");
	//for (UINT32 i=0; i < BufferSize; i++) {
	//	printf("%d", m_value[i]);
	//}
	//printf("\n");
}
#endif //TRACE


