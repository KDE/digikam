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

#include "Subband.h"
#include "Encoder.h"
#include "Decoder.h"

/////////////////////////////////////////////////////////////////////
// Default constructor
CSubband::CSubband() : m_size(0), m_data(0)
#ifdef __PGFROISUPPORT__
, m_ROIs(0), m_dataWidth(0) 
#endif
{
}

/////////////////////////////////////////////////////////////////////
// Destructor
CSubband::~CSubband() {
	FreeMemory();
}

/////////////////////////////////////////////////////////////////////
// Initialize subband parameters
void CSubband::Initialize(UINT32 width, UINT32 height, int level, Orientation orient) {
	ASSERT(!m_data);

	m_width = width;
	m_height = height;
	m_size = m_width*m_height;
	m_level = level;
	m_orientation = orient;
	m_data = 0;
	m_dataPos = 0;
#ifdef __PGFROISUPPORT__
	m_dataWidth = width;
#endif
}


/////////////////////////////////////////////////////////////////////
// Allocate a memory buffer to store all wavelet coefficients of this subband.
// @return True if the allocation did work without any problems
bool CSubband::AllocMemory() {
	UINT32 oldSize = m_size;

#ifdef __PGFROISUPPORT__
	if (m_ROIs) {
		// reset dataWidth and size
		const PGFRect& roi = m_ROIs->GetROI(m_level);
		m_dataWidth = __min(m_width, roi.right) - roi.left;
		ASSERT(m_dataWidth > 0);
		m_size = m_dataWidth*(__min(m_height, roi.bottom) - roi.top);
	}
#endif
	ASSERT(m_size > 0);

	if (m_data) {
		if (oldSize >= m_size) {
			return false;
		} else {
			delete[] m_data;
			m_data = new DataT[m_size];
			return (m_data != 0);
		}
	} else {
		m_data = new DataT[m_size];
		return (m_data != 0);
	}
}

/////////////////////////////////////////////////////////////////////
// Delete the memory buffer of this subband.
void CSubband::FreeMemory() {
	if (m_data) {
		delete[] m_data; m_data = 0;
	}
}

/////////////////////////////////////////////////////////////////////
// Perform subband quantization with given quantization parameter.
// A scalar quantization (with dead-zone) is used. A large quantization value
// results in strong quantization and therefore in big quality loss.
// @param quantParam A quantization parameter (larger or equal to 0)
void CSubband::Quantize(int quantParam) {
	if (m_orientation == LL) {
		quantParam -= (m_level + 1);
		// uniform rounding quantization
		if (quantParam > 0) {
			quantParam--;
			for (UINT32 i=0; i < m_size; i++) {
				if (m_data[i] < 0) {
					m_data[i] = -(((-m_data[i] >> quantParam) + 1) >> 1);
				} else {
					m_data[i] = ((m_data[i] >> quantParam) + 1) >> 1;
				}
			}
		}
	} else {
		if (m_orientation == HH) {
			quantParam -= (m_level - 1);
		} else {
			quantParam -= m_level;
		}
		// uniform deadzone quantization
		if (quantParam > 0) {
			int threshold = ((1 << quantParam) * 7)/5;	// good value
			quantParam--;
			for (UINT32 i=0; i < m_size; i++) {
				if (m_data[i] < -threshold) {
					m_data[i] = -(((-m_data[i] >> quantParam) + 1) >> 1);
				} else if (m_data[i] > threshold) {
					m_data[i] = ((m_data[i] >> quantParam) + 1) >> 1;
				} else {
					m_data[i] = 0;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
/// Perform subband dequantization with given quantization parameter.
/// A scalar quantization (with dead-zone) is used. A large quantization value
/// results in strong quantization and therefore in big quality loss.
/// @param quantParam A quantization parameter (larger or equal to 0)
/// @param level Level
void CSubband::Dequantize(int quantParam, int /*level*/) {
	if (m_orientation == LL) {
		quantParam -= m_level + 1;
	} else if (m_orientation == HH) {
		quantParam -= m_level - 1;
	} else {
		quantParam -= m_level;
	}
	if (quantParam > 0) {
		for (UINT32 i=0; i < m_size; i++) {
			m_data[i] <<= quantParam;
		}
	}
}

/////////////////////////////////////////////////////////////////////
/// Extracts a rectangular subregion of this subband.
/// Write wavelet coefficients into buffer.
/// It might throw an IOException.
/// @param encoder An encoder instance
/// @param quant A quantization value (linear scalar quantization)
/// @param tile True if just a rectangular region is extracted, false if the entire subband is extracted.
/// @param tileX Tile index in x-direction
/// @param tileY Tile index in y-direction
void CSubband::ExtractTile(CEncoder& encoder, int quant, bool tile /*= false*/, UINT32 tileX /*= 0*/, UINT32 tileY /*= 0*/) THROW_ {
	// quantize subband
	if (tileX == 0 && tileY == 0) Quantize(quant);

#ifdef __PGFROISUPPORT__
	if (tile) {
		// compute tile position and size
		UINT32 xPos, yPos, w, h;
		TilePosition(tileX, tileY, xPos, yPos, w, h);

		// write values into buffer using partitiong scheme
		encoder.Partition(this, w, h, xPos + yPos*m_width, m_width);
	} else 
#endif
	{
		// write values into buffer using partitiong scheme
		encoder.Partition(this, m_width, m_height, 0, m_width);
	}
}

/////////////////////////////////////////////////////////////////////
/// Decoding and dequantization of this subband.
/// It might throw an IOException.
/// @param decoder A decoder instance
/// @param quantParam Dequantization value
/// @param tile True if just a rectangular region is placed, false if the entire subband is placed.
/// @param tileX Tile index in x-direction
/// @param tileY Tile index in y-direction
void CSubband::PlaceTile(CDecoder& decoder, int quantParam, bool tile /*= false*/, UINT32 tileX /*= 0*/, UINT32 tileY /*= 0*/) THROW_ {
	// allocate memory
	AllocMemory();

	// correct quantParam with normalization factor
	if (m_orientation == LL) {
		quantParam -= m_level + 1;
	} else if (m_orientation == HH) {
		quantParam -= m_level - 1;
	} else {
		quantParam -= m_level;
	}
	if (quantParam < 0) quantParam = 0;

#ifdef __PGFROISUPPORT__
	if (tile) {
		// compute tile position and size
		const PGFRect& roi = m_ROIs->GetROI(m_level);
		UINT32 xPos, yPos, w, h;
		TilePosition(tileX, tileY, xPos, yPos, w, h);

		// read values into buffer using partitiong scheme
		decoder.Partition(this, quantParam, w, h, (xPos - roi.left) + (yPos - roi.top)*m_dataWidth, m_dataWidth);
	} else 
#endif
	{
		// read values into buffer using partitiong scheme
		decoder.Partition(this, quantParam, m_width, m_height, 0, m_width);
	}
}



#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////////
/// Compute tile position and size.
/// @param tileX Tile index in x-direction
/// @param tileY Tile index in y-direction
/// @param xPos [out] Offset to left
/// @param yPos [out] Offset to top
/// @param w [out] Tile width
/// @param h [out] Tile height
void CSubband::TilePosition(UINT32 tileX, UINT32 tileY, UINT32& xPos, UINT32& yPos, UINT32& w, UINT32& h) const {
	// example
	// band = HH, w = 30, ldTiles = 2 -> 4 tiles in a row/column
	// --> tile widths
	// 8 7 8 7
	// 
	// tile partitioning scheme
	// 0 1 2 3
	// 4 5 6 7
	// 8 9 A B
	// C D E F

	UINT32 nTiles = m_ROIs->GetNofTiles(m_level);
	ASSERT(tileX < nTiles); ASSERT(tileY < nTiles);
	UINT32 m;
	UINT32 left = 0, right = nTiles;
	UINT32 top = 0, bottom = nTiles;

	xPos = 0;
	yPos = 0;
	w = m_width;
	h = m_height;

	while (nTiles > 1) {
		// compute xPos and w with binary search
		m = (left + right) >> 1;
		if (tileX >= m) {
			xPos += (w + 1) >> 1;
			w >>= 1;
			left = m;
		} else {
			w = (w + 1) >> 1;
			right = m;
		}
		// compute yPos and h with binary search
		m = (top + bottom) >> 1;
		if (tileY >= m) {
			yPos += (h + 1) >> 1;
			h >>= 1;
			top = m;
		} else {
			h = (h + 1) >> 1;
			bottom = m;
		}
		nTiles >>= 1;
	}
	ASSERT(xPos < m_width && (xPos + w <= m_width));
	ASSERT(yPos < m_height && (yPos + h <= m_height));
}

#endif
