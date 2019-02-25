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
#define BufferLen			(BufferSize/WordWidth)	///< number of words per buffer
#define CodeBufferLen		BufferSize				///< number of words in code buffer (CodeBufferLen > BufferLen)

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
		//////////////////////////////////////////////////////////////////////
		/// Constructor: Initializes new macro block.
		CMacroBlock()
		: m_header(0)								// makes sure that IsCompletelyRead() returns true for an empty macro block
#ifdef _MSC_VER
#pragma warning( suppress : 4351 )
#endif
		, m_value()
		, m_codeBuffer()
		, m_valuePos(0)
		, m_sigFlagVector()
		{
		}

		//////////////////////////////////////////////////////////////////////
		/// Returns true if this macro block has been completely read.
		/// @return true if current value position is at block end
		bool IsCompletelyRead() const	{ return m_valuePos >= m_header.rbh.bufferSize; }

		//////////////////////////////////////////////////////////////////////
		/// Decodes already read input data into this macro block.
		/// Several macro blocks can be decoded in parallel.
		/// Call CDecoder::ReadMacroBlock before this method.
		void BitplaneDecode();

		ROIBlockHeader m_header;					///< block header
		DataT  m_value[BufferSize];					///< output buffer of values with index m_valuePos
		UINT32 m_codeBuffer[CodeBufferLen];			///< input buffer for encoded bitstream
		UINT32 m_valuePos;							///< current position in m_value

	private:
		UINT32 ComposeBitplane(UINT32 bufferSize, DataT planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits);
		UINT32 ComposeBitplaneRLD(UINT32 bufferSize, DataT planeMask, UINT32 sigPos, UINT32* refBits);
		UINT32 ComposeBitplaneRLD(UINT32 bufferSize, DataT planeMask, UINT32* sigBits, UINT32* refBits, UINT32 signPos);
		void  SetBitAtPos(UINT32 pos, DataT planeMask)			{ (m_value[pos] >= 0) ? m_value[pos] |= planeMask : m_value[pos] -= planeMask; }
		void  SetSign(UINT32 pos, bool sign)					{ m_value[pos] = -m_value[pos]*sign + m_value[pos]*(!sign); }

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
	/// @param userDataPos The stream position of the user data (metadata)
	/// @param useOMP If true, then the decoder will use multi-threading based on openMP
	/// @param userDataPolicy Policy of user data (meta-data) handling while reading PGF headers.
	CDecoder(CPGFStream* stream, PGFPreHeader& preHeader, PGFHeader& header,
		     PGFPostHeader& postHeader, UINT32*& levelLength, UINT64& userDataPos, 
			 bool useOMP, UINT32 userDataPolicy); // throws IOException

	/////////////////////////////////////////////////////////////////////
	/// Destructor
	~CDecoder();

	/////////////////////////////////////////////////////////////////////
	/// Unpartitions a rectangular region of a given subband.
	/// Partitioning scheme: The plane is partitioned in squares of side length LinBlockSize.
	/// Read wavelet coefficients from the output buffer of a macro block.
	/// It might throw an IOException.
	/// @param band A subband
	/// @param quantParam Dequantization value
	/// @param width The width of the rectangle
	/// @param height The height of the rectangle
	/// @param startPos The relative subband position of the top left corner of the rectangular region
	/// @param pitch The number of bytes in row of the subband
	void Partition(CSubband* band, int quantParam, int width, int height, int startPos, int pitch);

	/////////////////////////////////////////////////////////////////////
	/// Deccoding and dequantization of HL and LH subband (interleaved) using partitioning scheme.
	/// Partitioning scheme: The plane is partitioned in squares of side length InterBlockSize.
	/// It might throw an IOException.
	/// @param wtChannel A wavelet transform channel containing the HL and HL band
	/// @param level Wavelet transform level
	/// @param quantParam Dequantization value
	void DecodeInterleaved(CWaveletTransform* wtChannel, int level, int quantParam);

	//////////////////////////////////////////////////////////////////////
	/// Returns the length of all encoded headers in bytes.
	/// @return The length of all encoded headers in bytes
	UINT32 GetEncodedHeaderLength() const			{ return m_encodedHeaderLength; }

	////////////////////////////////////////////////////////////////////
	/// Resets stream position to beginning of PGF pre-header
	void SetStreamPosToStart()				{ ASSERT(m_stream); m_stream->SetPos(FSFromStart, m_startPos); }

	////////////////////////////////////////////////////////////////////
	/// Resets stream position to beginning of data block
	void SetStreamPosToData()				{ ASSERT(m_stream); m_stream->SetPos(FSFromStart, m_startPos + m_encodedHeaderLength); }

	////////////////////////////////////////////////////////////////////
	/// Skips a given number of bytes in the open stream.
	/// It might throw an IOException.
	void Skip(UINT64 offset);

	/////////////////////////////////////////////////////////////////////
	/// Dequantization of a single value at given position in subband.
	/// It might throw an IOException.
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
	UINT32 ReadEncodedData(UINT8* target, UINT32 len) const;

	/////////////////////////////////////////////////////////////////////
	/// Reads next block(s) from stream and decodes them
	/// It might throw an IOException.
	void DecodeBuffer();

	/////////////////////////////////////////////////////////////////////
	/// @return Stream
	CPGFStream* GetStream()							{ return m_stream; }

	/////////////////////////////////////////////////////////////////////
	/// Gets next macro block
	/// It might throw an IOException.
	void GetNextMacroBlock();

#ifdef __PGFROISUPPORT__
	/////////////////////////////////////////////////////////////////////
	/// Resets stream position to next tile.
	/// Used with ROI encoding scheme only.
	/// It might throw an IOException.
	void SkipTileBuffer();

	/////////////////////////////////////////////////////////////////////
	/// Enables region of interest (ROI) status.
	void SetROI()					{ m_roi = true; }
#endif

#ifdef TRACE
	void DumpBuffer();
#endif

private:
	void ReadMacroBlock(CMacroBlock* block); ///< throws IOException

	CPGFStream *m_stream;						///< input PGF stream
	UINT64 m_startPos;							///< stream position at the beginning of the PGF pre-header
	UINT64 m_streamSizeEstimation;				///< estimation of stream size
	UINT32 m_encodedHeaderLength;				///< stream offset from startPos to the beginning of the data part (highest level)

	CMacroBlock **m_macroBlocks;				///< array of macroblocks
	int m_currentBlockIndex;					///< index of current macro block
	int	m_macroBlockLen;						///< array length
	int	m_macroBlocksAvailable;					///< number of decoded macro blocks (including currently used macro block)
	CMacroBlock *m_currentBlock;				///< current macro block (used by main thread)

#ifdef __PGFROISUPPORT__
	bool   m_roi;								///< true: ensures region of interest (ROI) decoding
#endif
};

#endif //PGF_DECODER_H
