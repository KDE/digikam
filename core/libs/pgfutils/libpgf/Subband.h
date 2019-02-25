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
/// @file Subband.h
/// @brief PGF wavelet subband class
/// @author C. Stamm

#ifndef PGF_SUBBAND_H
#define PGF_SUBBAND_H

#include "PGFtypes.h"

class CEncoder;
class CDecoder;
class CRoiIndices;

//////////////////////////////////////////////////////////////////////
/// PGF wavelet channel subband class.
/// @author C. Stamm, R. Spuler
/// @brief Wavelet channel class
class CSubband {
	friend class CWaveletTransform;
	friend class CRoiIndices;

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
	/// @param tile True if just a rectangular region is extracted, false if the entire subband is extracted.
	/// @param tileX Tile index in x-direction
	/// @param tileY Tile index in y-direction
	void ExtractTile(CEncoder& encoder, bool tile = false, UINT32 tileX = 0, UINT32 tileY = 0);

	/////////////////////////////////////////////////////////////////////
	/// Decoding and dequantization of this subband.
	/// It might throw an IOException.
	/// @param decoder A decoder instance
	/// @param quantParam Dequantization value
	/// @param tile True if just a rectangular region is placed, false if the entire subband is placed.
	/// @param tileX Tile index in x-direction
	/// @param tileY Tile index in y-direction
	void PlaceTile(CDecoder& decoder, int quantParam, bool tile = false, UINT32 tileX = 0, UINT32 tileY = 0);

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
	void Dequantize(int quantParam);

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
	/// Set data buffer position to given position + one row.
	/// @param pos Given position
	void IncBuffRow(UINT32 pos)	{ m_dataPos = pos + BufferWidth(); }

#endif

private:
	void Initialize(UINT32 width, UINT32 height, int level, Orientation orient);
	void WriteBuffer(DataT val)			{ ASSERT(m_dataPos < m_size); m_data[m_dataPos++] = val; }
	void SetBuffer(DataT* b)			{ ASSERT(b); m_data = b; }
	DataT ReadBuffer()					{ ASSERT(m_dataPos < m_size); return m_data[m_dataPos++]; }

	UINT32 GetBuffPos() const			{ return m_dataPos; }

#ifdef __PGFROISUPPORT__
	UINT32 BufferWidth() const			{ return m_ROI.Width(); }
	void TilePosition(UINT32 tileX, UINT32 tileY, UINT32& left, UINT32& top, UINT32& w, UINT32& h) const;
	void TileIndex(bool topLeft, UINT32 xPos, UINT32 yPos, UINT32& tileX, UINT32& tileY, UINT32& x, UINT32& y) const;
	const PGFRect& GetAlignedROI() const { return m_ROI; }
	void SetNTiles(UINT32 nTiles)		{ m_nTiles = nTiles; }
	void SetAlignedROI(const PGFRect& roi);
	void InitBuffPos(UINT32 left = 0, UINT32 top = 0)	{ m_dataPos = top*BufferWidth() + left; ASSERT(m_dataPos < m_size); }
#else
	void InitBuffPos()					{ m_dataPos = 0; }
#endif

private:
	UINT32 m_width;					///< width in pixels
	UINT32 m_height;				///< height in pixels
	UINT32 m_size;					///< size of data buffer m_data
	int m_level;					///< recursion level
	Orientation m_orientation;		///< 0=LL, 1=HL, 2=LH, 3=HH L=lowpass filtered, H=highpass filterd
	UINT32 m_dataPos;				///< current position in m_data
	DataT* m_data;					///< buffer

#ifdef __PGFROISUPPORT__
	PGFRect m_ROI;					///< region of interest (block aligned)
	UINT32	m_nTiles;				///< number of tiles in one dimension in this subband
#endif
};

#endif //PGF_SUBBAND_H
