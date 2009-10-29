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

#ifndef PGF_ENCODER_H
#define PGF_ENCODER_H

#include "PGFtypes.h"
#include "BitStream.h"
#include "Subband.h"
#include "WaveletTransform.h"
#include "Stream.h"

/////////////////////////////////////////////////////////////////////
// Constants
#define BufferLen			(BufferSize/WordWidth)	// number of words per buffer

/////////////////////////////////////////////////////////////////////
/// PGF encoder class.
/// @author C. Stamm, R. Spuler
class CEncoder {
public:
	/////////////////////////////////////////////////////////////////////
	/// Write pre-header, header, postHeader, and levelLength.
	/// It might throw an IOException.
	/// @param stream A PGF stream
	/// @param preHeader A already filled in PGF pre header
	/// @param header An already filled in PGF header
	/// @param postHeader [in] A already filled in PGF post header (containing color table, user data, ...)
	/// @param levelLength A reference to an integer array, large enough to save the relative file positions of all PGF levels
	CEncoder(CPGFStream* stream, PGFPreHeader preHeader, PGFHeader header, const PGFPostHeader& postHeader, UINT32*& levelLength) THROW_; // throws IOException

	/////////////////////////////////////////////////////////////////////
	/// Destructor
	~CEncoder();

	/////////////////////////////////////////////////////////////////////
	/// Pad buffer with zeros, encode buffer, write levelLength into header
	/// Return number of bytes written into stream
	/// It might throw an IOException.
	UINT32 Flush() THROW_;

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
	void Partition(CSubband* band, int width, int height, int startPos, int pitch) THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Set or clear flag. The flag must be set to true as soon a wavelet
	/// transform level is encoded.
	/// @param b A flag value
	void SetLevelIsEncoded(bool b) { m_isLevelEncoded = b; }

	/////////////////////////////////////////////////////////////////////
	/// Write a single value into subband at given position.
	/// It might throw an IOException.
	/// @param band A subband
	/// @param bandPos A valid position in subband band
	void WriteValue(CSubband* band, int bandPos) THROW_;

#ifdef __PGFROISUPPORT__
	/////////////////////////////////////////////////////////////////////
	/// Encodes tile buffer and writes it into stream
	/// It might throw an IOException.
	void EncodeTileBuffer() THROW_	{ ASSERT(m_valuePos >= 0 && m_valuePos <= BufferSize); EncodeBuffer(ROIBlockHeader(m_valuePos, true)); }

	/////////////////////////////////////////////////////////////////////
	/// Enables region of interest (ROI) status.
	void SetROI()					{ m_roi = true; }
#endif

#ifdef TRACE
	void DumpBuffer() const;
#endif

private:
	UINT32 BitplaneEncode(UINT32 bufferSize);
	UINT32 RLESigsAndSigns(UINT32* sigBits, UINT32 sigLen, UINT32* signBits, UINT32 signLen);
	UINT32 RLESigns(UINT32* signBits, UINT32 signLen);
	UINT32 DecomposeBitplane(UINT32 bufferSize, UINT32 planeMask, UINT32* sigBits, UINT32* refBits, UINT32* signBits, UINT32& signLen);
	UINT8 NumberOfBitplanes();
	bool  GetBitAtPos(UINT32 pos, UINT32 planeMask) const	{ return (abs(m_value[pos]) & planeMask) > 0; }	
	void  EncodeBuffer(ROIBlockHeader h) THROW_; // throws IOException

protected:
	CPGFStream *m_stream;
	UINT64	m_startPosition;					// file position of PGF start (PreHeader)
	UINT64  m_levelLengthPos;					// file position of Metadata
	UINT64  m_currPosition;						// Already accumulated size from lower levels
	
	DataT	m_value[BufferSize];				// buffer of values with index m_valuePos
	UINT32	m_codeBuffer[BufferSize];			// buffer for encoded bitstream

	UINT32	m_sigFlagVector[BufferLen];			// see paper from Malvar, Fast Progressive Wavelet Coder

	UINT32	m_valuePos;							// current buffer position
	UINT32	m_codePos;							// current bit position in m_codeBuffer
	UINT32	m_maxAbsValue;						// maximum absolute coefficient in each buffer

	UINT32* m_levelLength;						// temporary saves the level index
	int     m_currLevelIndex;					// counts where (=index) to save next value
	bool    m_isLevelEncoded;					// Already enough data available to set level-block-size
#ifdef __PGFROISUPPORT__
	bool	m_roi;								// true: ensures region of interest (ROI) encoding
#endif
};

#endif //PGF_ENCODER
