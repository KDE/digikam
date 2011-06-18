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
/// @file Decoder.h
/// @brief PGF decoder class
/// @author C. Stamm, R. Spuler

#ifndef PGF_DECODER_H
#define PGF_DECODER_H

#include "PGFstream.h"
#include "BitStream.h"
#include "Subband.h"
#include "WaveletTransform.h"

/////////////////////////////////////////////////////////////////////
// Constants
#define BufferLen			(BufferSize/WordWidth)	// number of words per buffer

/////////////////////////////////////////////////////////////////////
/// PGF decoder class.
/// @author C. Stamm, R. Spuler
/// @brief PGF decoder
class CDecoder {
	//////////////////////////////////////////////////////////////////////
	/// PGF decoder macro block class.
	/// @author C. Stamm, I. Bauersachs
	/// @brief A macro block is a decoding unit of fixed size (uncoded)
	class CMacroBlock {
	public:
		CMacroBlock(CDecoder *decoder)
		: m_header(0)
        , m_valuePos(0)
		, m_decoder(decoder)
		{
			ASSERT(m_decoder);
		}

		void  BitplaneDecode();						// several macro blocks can be encoded in parallel

		ROIBlockHeader m_header;					// block header
		DataT  m_value[BufferSize];					// output buffer of values with index m_valuePos
		UINT32 m_codeBuffer[BufferSize];			// input buffer for encoded bitstream
		UINT32 m_valuePos;							// current position in m_value

	private:
		UINT32 ComposeBitplane(UINT32 bufferSize, DataT planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits);
		UINT32 ComposeBitplaneRLD(UINT32 bufferSize, DataT planeMask, UINT32 sigPos, UINT32* refBits);
		UINT32 ComposeBitplaneRLD(UINT32 bufferSize, DataT planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits);
		void  SetBitAtPos(UINT32 pos, DataT planeMask)			{ (m_value[pos] >= 0) ? m_value[pos] |= planeMask : m_value[pos] -= planeMask; }
		void  SetSign(UINT32 pos, bool sign)					{ m_value[pos] = -m_value[pos]*sign + m_value[pos]*(!sign); }

		CDecoder *m_decoder;						// outer class
		bool m_sigFlagVector[BufferSize+1];			// see paper from Malvar, Fast Progressive Wavelet Coder
	};

public:
	/////////////////////////////////////////////////////////////////////
	/// Constructor: Read pre-header, header, and levelLength at current stream position.
	/// It might throw an IOException.
	/// @param stream A PGF stream
	/// @param preHeader [out] A PGF pre-header
	/// @param header [out] A PGF header
	/// @param postHeader [out] A PGF post-header
	/// @param levelLength The location of the levelLength array. The array is allocated in this method. The caller has to delete this array.
	/// @param useOMP If true, then the decoder will use multi-threading based on openMP
	CDecoder(CPGFStream* stream, PGFPreHeader& preHeader, PGFHeader& header, PGFPostHeader& postHeader, UINT32*& levelLength, bool useOMP = true) THROW_; // throws IOException

	/////////////////////////////////////////////////////////////////////
	/// Destructor
	~CDecoder();

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
	void Partition(CSubband* band, int quantParam, int width, int height, int startPos, int pitch) THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Deccoding and dequantization of HL and LH subband (interleaved) using partitioning scheme.
	/// Partitioning scheme: The plane is partitioned in squares of side length InterBlockSize.
	/// It might throw an IOException.
	/// @param wtChannel A wavelet transform channel containing the HL and HL band
	/// @param level Wavelet transform level
	/// @param quantParam Dequantization value
	void DecodeInterleaved(CWaveletTransform* wtChannel, int level, int quantParam) THROW_;

	//////////////////////////////////////////////////////////////////////
	/// Return the length of all encoded headers in bytes.
	/// @return The length of all encoded headers in bytes
	UINT32 GetEncodedHeaderLength() const			{ return m_encodedHeaderLength; }

	////////////////////////////////////////////////////////////////////
	/// Reset stream position to beginning of PGF pre header
	void SetStreamPosToStart() THROW_				{ ASSERT(m_stream); m_stream->SetPos(FSFromStart, m_startPos); }

	////////////////////////////////////////////////////////////////////
	/// Reset stream position to beginning of data block
	void SetStreamPosToData() THROW_				{ ASSERT(m_stream); m_stream->SetPos(FSFromStart, m_startPos + m_encodedHeaderLength); }

	////////////////////////////////////////////////////////////////////
	/// Skip a given number of bytes in the open stream.
	/// It might throw an IOException.
	void Skip(UINT64 offset) THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Dequantization of a single value at given position in subband.
	/// @param band A subband
	/// @param bandPos A valid position in subband band
	/// @param quantParam The quantization parameter
	void DequantizeValue(CSubband* band, UINT32 bandPos, int quantParam);

	//////////////////////////////////////////////////////////////////////
	/// Copies data from the open stream to a target buffer.
	/// It might throw an IOException.
	/// @param target The target buffer
	/// @param len The number of bytes to read
	/// @return The number of bytes copied to the target buffer
	UINT32 ReadEncodedData(UINT8* target, UINT32 len) const THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Reads stream and decodes tile buffer
	/// It might throw an IOException.
	void DecodeBuffer() THROW_;

#ifdef __PGFROISUPPORT__
	/////////////////////////////////////////////////////////////////////
	/// Reads stream and decodes tile buffer
	/// It might throw an IOException.
	void DecodeTileBuffer() THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Resets stream position to next tile.
	/// It might throw an IOException.
	void SkipTileBuffer() THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Enables region of interest (ROI) status.
	void SetROI()					{ m_roi = true; }
#endif

#ifdef TRACE
	void DumpBuffer();
#endif

private:
	void ReadMacroBlock(CMacroBlock* block) THROW_; // throws IOException

	CPGFStream *m_stream;						// input pgf stream
	UINT64 m_startPos;							// stream position at the beginning of the PGF pre header
	UINT64 m_streamSizeEstimation;				// estimation of stream size
	UINT32 m_encodedHeaderLength;				// stream offset from startPos to the beginning of the data part (highest level)

	CMacroBlock **m_macroBlocks;				// array of macroblocks
	int m_currentBlockIndex;					// index of current macro block
	int	m_macroBlockLen;						// array length
	int	m_macroBlocksAvailable;					// number of decoded macro blocks
	CMacroBlock *m_currentBlock;				// current macro block (used by main thread)

#ifdef __PGFROISUPPORT__
	bool   m_roi;								// true: ensures region of interest (ROI) decoding
#endif
};

#endif //PGF_DECODER_H
