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
/// @file Encoder.h
/// @brief PGF encoder class
/// @author C. Stamm, R. Spuler

#ifndef PGF_ENCODER_H
#define PGF_ENCODER_H

#include "PGFstream.h"
#include "BitStream.h"
#include "Subband.h"
#include "WaveletTransform.h"

/////////////////////////////////////////////////////////////////////
// Constants
#define BufferLen			(BufferSize/WordWidth)	///< number of words per buffer
#define CodeBufferLen		BufferSize				///< number of words in code buffer (CodeBufferLen > BufferLen)

/////////////////////////////////////////////////////////////////////
/// PGF encoder class.
/// @author C. Stamm
/// @brief PGF encoder
class CEncoder {
	//////////////////////////////////////////////////////////////////////
	/// PGF encoder macro block class.
	/// @author C. Stamm, I. Bauersachs
	/// @brief A macro block is an encoding unit of fixed size (uncoded)
	class CMacroBlock {
	public:
		//////////////////////////////////////////////////////////////////////
		/// Constructor: Initializes new macro block.
		/// @param encoder Pointer to outer class.
		CMacroBlock(CEncoder *encoder)
#ifdef _MSC_VER
#   pragma warning( suppress : 4351 )
#endif
		: m_value()
		, m_codeBuffer()
		, m_header(0)
		, m_encoder(encoder)
		, m_sigFlagVector()
		{
			ASSERT(m_encoder);
			Init(-1);
		}

		//////////////////////////////////////////////////////////////////////
		/// Reinitialzes this macro block (allows reusage).
		/// @param lastLevelIndex Level length directory index of last encoded level: [0, nLevels)
		void Init(int lastLevelIndex) {				// initialize for reusage
			m_valuePos = 0;
			m_maxAbsValue = 0;
			m_codePos = 0;
			m_lastLevelIndex = lastLevelIndex;
		}

		//////////////////////////////////////////////////////////////////////
		/// Encodes this macro block into internal code buffer.
		/// Several macro blocks can be encoded in parallel.
		/// Call CEncoder::WriteMacroBlock after this method.
		void BitplaneEncode();

		DataT	m_value[BufferSize];				///< input buffer of values with index m_valuePos
		UINT32	m_codeBuffer[CodeBufferLen];		///< output buffer for encoded bitstream
		ROIBlockHeader m_header;					///< block header
		UINT32	m_valuePos;							///< current buffer position
		UINT32	m_maxAbsValue;						///< maximum absolute coefficient in each buffer
		UINT32	m_codePos;							///< current position in encoded bitstream
		int		m_lastLevelIndex;					///< index of last encoded level: [0, nLevels); used because a level-end can occur before a buffer is full

	private:
		UINT32 RLESigns(UINT32 codePos, UINT32* signBits, UINT32 signLen);
		UINT32 DecomposeBitplane(UINT32 bufferSize, UINT32 planeMask, UINT32 codePos, UINT32* sigBits, UINT32* refBits, UINT32* signBits, UINT32& signLen, UINT32& codeLen);
		UINT8  NumberOfBitplanes();
		bool   GetBitAtPos(UINT32 pos, UINT32 planeMask) const	{ return (abs(m_value[pos]) & planeMask) > 0; }

		CEncoder *m_encoder;						// encoder instance
		bool	m_sigFlagVector[BufferSize+1];		// see paper from Malvar, Fast Progressive Wavelet Coder
	};

public:
	/////////////////////////////////////////////////////////////////////
	/// Write pre-header, header, post-Header, and levelLength.
	/// It might throw an IOException.
	/// @param stream A PGF stream
	/// @param preHeader A already filled in PGF pre-header
	/// @param header An already filled in PGF header
	/// @param postHeader [in] An already filled in PGF post-header (containing color table, user data, ...)
	/// @param userDataPos [out] File position of user data
	/// @param useOMP If true, then the encoder will use multi-threading based on openMP
	CEncoder(CPGFStream* stream, PGFPreHeader preHeader, PGFHeader header, const PGFPostHeader& postHeader,
		UINT64& userDataPos, bool useOMP); // throws IOException

	/////////////////////////////////////////////////////////////////////
	/// Destructor
	~CEncoder();

	/////////////////////////////////////////////////////////////////////
	/// Encoder favors speed over compression size
	void FavorSpeedOverSize() { m_favorSpeed = true; }

	/////////////////////////////////////////////////////////////////////
	/// Pad buffer with zeros and encode buffer.
	/// It might throw an IOException.
	void Flush();

	/////////////////////////////////////////////////////////////////////
	/// Increase post-header size and write new size into stream.
	/// @param preHeader An already filled in PGF pre-header
	/// It might throw an IOException.
	void UpdatePostHeaderSize(PGFPreHeader preHeader);

	/////////////////////////////////////////////////////////////////////
	/// Create level length data structure and write a place holder into stream.
	/// It might throw an IOException.
	/// @param levelLength A reference to an integer array, large enough to save the relative file positions of all PGF levels
	/// @return number of bytes written into stream
	UINT32 WriteLevelLength(UINT32*& levelLength);

	/////////////////////////////////////////////////////////////////////
	/// Write new levelLength into stream.
	/// It might throw an IOException.
	/// @return Written image bytes.
	UINT32 UpdateLevelLength();

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
	void Partition(CSubband* band, int width, int height, int startPos, int pitch);

	/////////////////////////////////////////////////////////////////////
	/// Informs the encoder about the encoded level.
	/// @param currentLevel encoded level [0, nLevels)
	void SetEncodedLevel(int currentLevel) { ASSERT(currentLevel >= 0); m_currentBlock->m_lastLevelIndex = m_nLevels - currentLevel - 1; m_forceWriting = true; }

	/////////////////////////////////////////////////////////////////////
	/// Write a single value into subband at given position.
	/// It might throw an IOException.
	/// @param band A subband
	/// @param bandPos A valid position in subband band
	void WriteValue(CSubband* band, int bandPos);

	/////////////////////////////////////////////////////////////////////
	/// Compute stream length of header.
	/// @return header length
	INT64 ComputeHeaderLength() const { return m_levelLengthPos - m_startPosition; }

	/////////////////////////////////////////////////////////////////////
	/// Compute stream length of encoded buffer.
	/// @return encoded buffer length
	INT64 ComputeBufferLength() const { return m_stream->GetPos() - m_bufferStartPos; }

	/////////////////////////////////////////////////////////////////////
	/// Compute file offset between real and expected levelLength position.
	/// @return file offset
	INT64 ComputeOffset() const { return m_stream->GetPos() - m_levelLengthPos; }

	////////////////////////////////////////////////////////////////////
	/// Resets stream position to beginning of PGF pre-header
	void SetStreamPosToStart() { ASSERT(m_stream); m_stream->SetPos(FSFromStart, m_startPosition); }

	/////////////////////////////////////////////////////////////////////
	/// Save current stream position as beginning of current level.
	void SetBufferStartPos() { m_bufferStartPos = m_stream->GetPos(); }

#ifdef __PGFROISUPPORT__
	/////////////////////////////////////////////////////////////////////
	/// Encodes tile buffer and writes it into stream
	/// It might throw an IOException.
	void EncodeTileBuffer()	{ ASSERT(m_currentBlock && m_currentBlock->m_valuePos >= 0 && m_currentBlock->m_valuePos <= BufferSize); EncodeBuffer(ROIBlockHeader(m_currentBlock->m_valuePos, true)); }

	/////////////////////////////////////////////////////////////////////
	/// Enables region of interest (ROI) status.
	void SetROI()					{ m_roi = true; }
#endif

#ifdef TRACE
	void DumpBuffer() const;
#endif

private:
	void EncodeBuffer(ROIBlockHeader h); // throws IOException
	void WriteMacroBlock(CMacroBlock* block); // throws IOException

	CPGFStream *m_stream;						///< output PMF stream
	UINT64	m_startPosition;					///< stream position of PGF start (PreHeader)
	UINT64  m_levelLengthPos;					///< stream position of Metadata
	UINT64  m_bufferStartPos;					///< stream position of encoded buffer

	CMacroBlock **m_macroBlocks;				///< array of macroblocks
	int		m_macroBlockLen;					///< array length
	int		m_lastMacroBlock;					///< array index of the last created macro block
	CMacroBlock *m_currentBlock;				///< current macro block (used by main thread)

	UINT32* m_levelLength;						///< temporary saves the level index
	int     m_currLevelIndex;					///< counts where (=index) to save next value
	UINT8	m_nLevels;							///< number of levels
	bool	m_favorSpeed;						///< favor speed over size
	bool	m_forceWriting;						///< all macro blocks have to be written into the stream
#ifdef __PGFROISUPPORT__
	bool	m_roi;								///< true: ensures region of interest (ROI) encoding
#endif
};

#endif //PGF_ENCODER
