/*
 * The Progressive Graphics File; http://www.libpgf.org
 * 
 * $Date: 2006-05-18 16:03:32 +0200 (Do, 18 Mai 2006) $
 * $Revision: 194 $
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
/// @file WaveletTransform.h
/// @brief PGF wavelet transform class
/// @author C. Stamm

#ifndef PGF_WAVELETTRANSFORM_H
#define PGF_WAVELETTRANSFORM_H

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

#if defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#endif

#include "PGFtypes.h"
#include "Subband.h"

//////////////////////////////////////////////////////////////////////
// Constants
const UINT32 FilterSizeL = 5;					///< number of coefficients of the low pass filter
const UINT32 FilterSizeH = 3;					///< number of coefficients of the high pass filter
const UINT32 FilterSize = __max(FilterSizeL, FilterSizeH);

#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////////
/// PGF ROI and tile support. This is a helper class for CWaveletTransform.
/// @author C. Stamm
/// @brief ROI indices
class CRoiIndices {
};
#endif //__PGFROISUPPORT__


//////////////////////////////////////////////////////////////////////
/// PGF wavelet transform class.
/// @author C. Stamm, R. Spuler
/// @brief PGF wavelet transform
class CWaveletTransform {
	friend class CSubband;

public:
	//////////////////////////////////////////////////////////////////////
	/// Constructor: Constructs a wavelet transform pyramid of given size and levels.
	/// @param width The width of the original image (at level 0) in pixels
	/// @param height The height of the original image (at level 0) in pixels
	/// @param levels The number of levels (>= 0)
	/// @param data Input data of subband LL at level 0
	CWaveletTransform(UINT32 width, UINT32 height, int levels, DataT* data = nullptr);

	//////////////////////////////////////////////////////////////////////
	/// Destructor
	~CWaveletTransform() { Destroy(); }
	
	//////////////////////////////////////////////////////////////////////
	/// Compute fast forward wavelet transform of LL subband at given level and
	/// stores result in all 4 subbands of level + 1.
	/// @param level A wavelet transform pyramid level (>= 0 && < Levels())
	/// @param quant A quantization value (linear scalar quantization)
	/// @return error in case of a memory allocation problem
	OSError ForwardTransform(int level, int quant);

	//////////////////////////////////////////////////////////////////////
	/// Compute fast inverse wavelet transform of all 4 subbands of given level and
	/// stores result in LL subband of level - 1.
	/// @param level A wavelet transform pyramid level (> 0 && <= Levels())
	/// @param width A pointer to the returned width of subband LL (in pixels)
	/// @param height A pointer to the returned height of subband LL (in pixels)
	/// @param data A pointer to the returned array of image data
	/// @return error in case of a memory allocation problem
	OSError InverseTransform(int level, UINT32* width, UINT32* height, DataT** data);

	//////////////////////////////////////////////////////////////////////
	/// Get pointer to one of the 4 subband at a given level.
	/// @param level A wavelet transform pyramid level (>= 0 && <= Levels())
	/// @param orientation A quarter of the subband (LL, LH, HL, HH)
	CSubband* GetSubband(int level, Orientation orientation) {
		ASSERT(level >= 0 && level < m_nLevels);
		return &m_subband[level][orientation];
	}
	
#ifdef __PGFROISUPPORT__
	//////////////////////////////////////////////////////////////////////
	/// Compute and store ROIs for nLevels
	/// @param rect rectangular region of interest (ROI) at level 0
	void SetROI(PGFRect rect);

	//////////////////////////////////////////////////////////////////////
	/// Checks the relevance of a given tile at given level.
	/// @param level A valid subband level.
	/// @param tileX x-index of the given tile
	/// @param tileY y-index of the given tile
	const bool TileIsRelevant(int level, UINT32 tileX, UINT32 tileY) const { ASSERT(m_indices); ASSERT(level >= 0 && level < m_nLevels); return m_indices[level].IsInside(tileX, tileY); }

	//////////////////////////////////////////////////////////////////////
	/// Get number of tiles in x- or y-direction at given level. 
	/// This number is independent of the given ROI.
	/// @param level A valid subband level.
	UINT32 GetNofTiles(int level) const { ASSERT(level >= 0 && level < m_nLevels); return 1 << (m_nLevels - level - 1); }

	//////////////////////////////////////////////////////////////////////
	/// Return ROI at given level.
	/// @param level A valid subband level.
	const PGFRect& GetAlignedROI(int level) const		{ return m_subband[level][LL].GetAlignedROI(); }

#endif // __PGFROISUPPORT__

private:
	void Destroy() { 
		delete[] m_subband; m_subband = nullptr;
	#ifdef __PGFROISUPPORT__
		delete[] m_indices; m_indices = nullptr;
	#endif
	}
	void InitSubbands(UINT32 width, UINT32 height, DataT* data);
	void ForwardRow(DataT* buff, UINT32 width);
	void InverseRow(DataT* buff, UINT32 width);
	void InterleavedToSubbands(int destLevel, DataT* loRow, DataT* hiRow, UINT32 width);
	void SubbandsToInterleaved(int srcLevel, DataT* loRow, DataT* hiRow, UINT32 width);

#ifdef __PGFROISUPPORT__
	PGFRect *m_indices;							///< array of length m_nLevels of tile indices
#endif //__PGFROISUPPORT__

	int			m_nLevels;						///< number of LL levels: one more than header.nLevels in PGFimage
	CSubband	(*m_subband)[NSubbands];		///< quadtree of subbands: LL HL LH HH
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__)
#pragma clang diagnostic pop
#endif

#endif //PGF_WAVELETTRANSFORM_H
