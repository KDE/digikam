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

#ifndef PGF_SUBBAND_H
#define PGF_SUBBAND_H

#include "PGFtypes.h"

class CEncoder;
class CDecoder;
class CROIs;

//////////////////////////////////////////////////////////////////////
/// PGF wavelet channel subband class.
/// @author C. Stamm, R. Spuler
class CSubband {
	friend class CWaveletTransform;

public:
	//////////////////////////////////////////////////////////////////////
	/// Standard constructor.
	CSubband();

	//////////////////////////////////////////////////////////////////////
	/// Destructor.
	~CSubband();

	//////////////////////////////////////////////////////////////////////
	/// Allocate a memory buffer to store all wavelet coefficients of this subband.
	/// @return True if the allocation did work without any problems
	bool AllocMemory();

	//////////////////////////////////////////////////////////////////////
	/// Delete the memory buffer of this subband.
	void FreeMemory();

	/////////////////////////////////////////////////////////////////////
	/// Extracts a rectangular subregion of this subband.
	/// Write wavelet coefficients into buffer.
	/// It might throw an IOException.
	/// @param encoder An encoder instance
	/// @param quant A quantization value (linear scalar quantization)
	/// @param tile True if just a rectangular region is extracted, false if the entire subband is extracted.
	/// @param tileX Tile index in x-direction
	/// @param tileY Tile index in y-direction
	void ExtractTile(CEncoder& encoder, int quant, bool tile = false, UINT32 tileX = 0, UINT32 tileY = 0) THROW_;

	/////////////////////////////////////////////////////////////////////
	/// Decoding and dequantization of this subband.
	/// It might throw an IOException.
	/// @param decoder A decoder instance
	/// @param quantParam Dequantization value
	/// @param tile True if just a rectangular region is placed, false if the entire subband is placed.
	/// @param tileX Tile index in x-direction
	/// @param tileY Tile index in y-direction
	void PlaceTile(CDecoder& decoder, int quantParam, bool tile = false, UINT32 tileX = 0, UINT32 tileY = 0) THROW_;

	//////////////////////////////////////////////////////////////////////
	/// Perform subband quantization with given quantization parameter.
	/// A scalar quantization (with dead-zone) is used. A large quantization value
	/// results in strong quantization and therefore in big quality loss.
	/// @param quantParam A quantization parameter (larger or equal to 0)
	void Quantize(int quantParam);

	//////////////////////////////////////////////////////////////////////
	/// Perform subband dequantization with given quantization parameter.
	/// A scalar quantization (with dead-zone) is used. A large quantization value
	/// results in strong quantization and therefore in big quality loss.
	/// @param quantParam A quantization parameter (larger or equal to 0)
	/// @level Level
	void Dequantize(int quantParam, int level);

	//////////////////////////////////////////////////////////////////////
	/// Store wavelet coefficient in subband at given position.
	/// @param pos A subband position (>= 0)
	/// @param v A wavelet coefficient
	void SetData(UINT32 pos, DataT v)	{ ASSERT(pos < m_size); m_data[pos] = v; }

	//////////////////////////////////////////////////////////////////////
	/// Get a pointer to an array of all wavelet coefficients of this subband.
	/// @return Pointer to array of wavelet coefficients
	DataT* GetBuffer()					{ return m_data; }

	//////////////////////////////////////////////////////////////////////
	/// Return wavelet coefficient at given position.
	/// @param pos A subband position (>= 0)
	/// @return Wavelet coefficient
	DataT GetData(UINT32 pos) const		{ ASSERT(pos < m_size); return m_data[pos]; }

	//////////////////////////////////////////////////////////////////////
	/// Return level of this subband.
	/// @return Level of this subband
	int GetLevel() const				{ return m_level; }

	//////////////////////////////////////////////////////////////////////
	/// Return height of this subband.
	/// @return Height of this subband (in pixels)
	int GetHeight() const				{ return m_height; }

	//////////////////////////////////////////////////////////////////////
	/// Return width of this subband.
	/// @return Width of this subband (in pixels)
	int GetWidth() const				{ return m_width; }
	
	//////////////////////////////////////////////////////////////////////
	/// Return orientation of this subband.
	/// LL LH
	/// HL HH
	/// @return Orientation of this subband (LL, HL, LH, HH)
	Orientation GetOrientation() const	{ return m_orientation; }

#ifdef __PGFROISUPPORT__
	/////////////////////////////////////////////////////////////////////
	/// Return data buffer line width.
	UINT32 BufferWidth() const	{ return m_dataWidth; }

	/////////////////////////////////////////////////////////////////////
	/// Set data buffer position to given position + one row.
	/// @param pos Given position
	void IncBuffRow(UINT32 pos)	{ m_dataPos = pos + m_dataWidth; }

#endif

private:
	void Initialize(UINT32 width, UINT32 height, int level, Orientation orient);
	void WriteBuffer(DataT val)			{ ASSERT(m_dataPos < m_size); m_data[m_dataPos++] = val; }
	void SetBuffer(DataT* b)			{ ASSERT(b); m_data = b; }
	DataT ReadBuffer()					{ ASSERT(m_dataPos < m_size); return m_data[m_dataPos++]; }

	UINT32 GetBuffPos() const			{ return m_dataPos; }

#ifdef __PGFROISUPPORT__
//	void IncBuffPos(UINT32 x, UINT32 y)	{ ASSERT(x < m_width); ASSERT(y < m_height); m_dataPos += y*m_width + x; }
	void TilePosition(UINT32 tileX, UINT32 tileY, UINT32& left, UINT32& top, UINT32& w, UINT32& h) const;
	void SetROI(CROIs* roi)				{ ASSERT(roi); m_ROIs = roi; }
	void InitBuffPos(UINT32 left = 0, UINT32 top = 0)	{ m_dataPos = top*m_dataWidth + left; ASSERT(m_dataPos < m_size); }
#else
	void InitBuffPos()					{ m_dataPos = 0; }
#endif

//	void PutData(INT16* buff, UINT32 width, bool low);
//	void GetData(INT16* buff, UINT32 width, bool low);
//	UINT32 GetSize() const				{ return m_size; }
//	void SetWidth(UINT32 w)				{ m_width = w; }
//	void SetHeight(UINT32 h)			{ m_height = h; }
//	void SetSize(UINT32 s)				{ m_size = s; }
//	void SetLevel(int l)				{ m_level = l; }
//	void SetOrientation(Orientation o)	{ m_orientation = o; }

private:
	UINT32 m_width;
	UINT32 m_height;
	UINT32 m_size;					// size of data buffer m_data
	int m_level;					// recursion level
	Orientation m_orientation;		// 0=LL, 1=HL, 2=LH, 3=HH L=lowpass filtered , H=highpass filterd
	UINT32 m_dataPos;				// current position in m_data
	DataT* m_data;					// buffer

#ifdef __PGFROISUPPORT__
	CROIs*	 m_ROIs;				// ROI information
	UINT32   m_dataWidth;			// row width of the data buffer
#endif
};
/*
///////////////////////////////////////////////////////////
// store line of wavelet coefficients
// because the wavelet coefficients in buff are interleaved
// low determines whether the lowpass or highpass data is stored
inline void CSubband::PutData(INT16* buff, int width, bool low) {
	ASSERT(m_dataPos + width/2 <= (UINT32)m_size);
	int start = (low) ? 0 : 1;

	for (int i=start; i < width; i += 2) {
		m_data[m_dataPos] = buff[i];
		m_dataPos++;
	}
}

//////////////////////////////////////////////////////////////
// get line of wavelet coefficients
// if low is true get the even coefficients in buff
inline void CSubband::GetData(INT16* buff, int width, bool low) {
	ASSERT(m_dataPos + width/2 <= (UINT32)m_size);
	int start = (low) ? 0 : 1;
	//if (low) start = 0; else start = 1;
	for (int i=start; i < width; i += 2) {
		buff[i] = m_data[m_dataPos];
		m_dataPos++;
	}
}
*/

#endif //PGF_SUBBAND_H
