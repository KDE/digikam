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

#ifndef PGF_WAVELETTRANSFORM_H
#define PGF_WAVELETTRANSFORM_H

#include "PGFtypes.h"
#include "Subband.h"

//////////////////////////////////////////////////////////////////////
// Constants
#define FilterWidth			5					// number of coefficients of the row wavelet filter
#define FilterHeight		3					// number of coefficients of the column wavelet filter

#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////////
/// PGF ROI and tile support. This is a helper class for CWaveletTransform.
/// @author C. Stamm
class CROIs {
	friend class CWaveletTransform;

	//////////////////////////////////////////////////////////////////////
	/// Constructor: Creates a ROI helper object
	/// @param levels The number of levels (>= 0)
	CROIs(int levels) : m_nLevels(levels), m_ROIs(0), m_indices(0) { ASSERT(levels > 0); }

	//////////////////////////////////////////////////////////////////////
	/// Destructor
	~CROIs() { Destroy(); }

	void Destroy()								{ delete[] m_ROIs; m_ROIs = 0; delete[] m_indices; m_indices = 0; }
	void CreateROIs();
	void CreateIndices();
	void ComputeIndices(UINT32 width, UINT32 height, const PGFRect& rect);
	void SetROI(int level, const PGFRect& rect)	{ ASSERT(m_ROIs); ASSERT(level >= 0 && level < m_nLevels); m_ROIs[level] = rect; }
	const PGFRect& GetIndices(int level) const	{ ASSERT(m_indices); ASSERT(level >= 0 && level < m_nLevels); return m_indices[level]; }
	UINT32 Left(int level) const				{ ASSERT(m_ROIs); ASSERT(level >= 0 && level < m_nLevels); return m_ROIs[level].left; }
	UINT32 Top(int level) const					{ ASSERT(m_ROIs); ASSERT(level >= 0 && level < m_nLevels); return m_ROIs[level].top; }
	bool ROIisSupported() const					{ return m_ROIs != 0; }
	void ComputeTileIndex(UINT32 width, UINT32 height, UINT32 pos, bool horizontal, bool isMin);

public:
	//////////////////////////////////////////////////////////////////////
	/// Returns the number of tiles at a given level.
	/// @param level A wavelet transform pyramid level (>= 0 && < Levels())
	UINT32 GetNofTiles(int level) const			{ ASSERT(level >= 0 && level < m_nLevels); return 1 << (m_nLevels - level - 1); }

	//////////////////////////////////////////////////////////////////////
	/// Return region of interest at a given level.
	/// @param level A wavelet transform pyramid level (>= 0 && < Levels())
	const PGFRect& GetROI(int level) const		{ ASSERT(m_ROIs); ASSERT(level >= 0 && level < m_nLevels); return m_ROIs[level]; }

private:
	int      m_nLevels;			// number of levels of the image
	PGFRect	*m_ROIs;			// array of region of interests (ROI)
	PGFRect *m_indices;			// array of tile indices

};
#endif //__PGFROISUPPORT__


//////////////////////////////////////////////////////////////////////
/// PGF wavelet transform class.
/// @author C. Stamm, R. Spuler
class CWaveletTransform {
	friend class CSubband;

public:
	//////////////////////////////////////////////////////////////////////
	/// Constructor: Constructs a wavelet transform pyramid of given size and levels.
	/// @param width The width of the original image (at level 0) in pixels
	/// @param height The height of the original image (at level 0) in pixels
	/// @param levels The number of levels (>= 0)
	/// @param data Input data of subband LL at level 0
	CWaveletTransform(UINT32 width, UINT32 height, int levels, DataT* data = NULL);

	//////////////////////////////////////////////////////////////////////
	/// Destructor
	~CWaveletTransform() { Destroy(); }
	
	//////////////////////////////////////////////////////////////////////
	/// Compute fast forward wavelet transform of LL subband at given level and
	/// stores result on all 4 subbands of level + 1.
	/// @param level A wavelet transform pyramid level (>= 0 && < Levels())
	void ForwardTransform(int level);

	//////////////////////////////////////////////////////////////////////
	/// Compute fast inverse wavelet transform of all 4 subbands of given level and
	/// stores result in LL subband of level - 1.
	/// @param level A wavelet transform pyramid level (> 0 && <= Levels())
	/// @param width A pointer to the returned width of subband LL (in pixels)
	/// @param height A pointer to the returned height of subband LL (in pixels)
	/// @param data A pointer to the returned array of image data
	void InverseTransform(int level, UINT32* width, UINT32* height, DataT** data);

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
	/// Compute and store ROIs for each level
	/// @param rect rectangular region of interest (ROI)
	void SetROI(const PGFRect& rect);

	//////////////////////////////////////////////////////////////////////
	/// For each subband set a referenct to ROI information
	void SetROI();

	//////////////////////////////////////////////////////////////////////
	/// Get tile indices of a ROI at given level.
	/// @param level A valid subband level.
	const PGFRect& GetTileIndices(int level) const	{ return m_ROIs.GetIndices(level); }

	//////////////////////////////////////////////////////////////////////
	/// Get number of tiles in x- or y-direction at given level.
	/// @param level A valid subband level.
	UINT32 GetNofTiles(int level) const				{ return m_ROIs.GetNofTiles(level); }

	//////////////////////////////////////////////////////////////////////
	/// Return ROI at given level.
	/// @param level A valid subband level.
	const PGFRect& GetROI(int level) const			{ return m_ROIs.GetROI(level); }

#endif // __PGFROISUPPORT__

private:
	void Destroy() { delete[] m_subband; m_subband = 0; 
		#ifdef __PGFROISUPPORT__
		m_ROIs.Destroy(); 
		#endif
	}
	void InitSubbands(UINT32 width, UINT32 height, DataT* data);
	void ForwardRow(DataT* buff, UINT32 width);
	void InverseRow(DataT* buff, UINT32 width);
	void LinearToMallat(int destLevel,DataT* loRow, DataT* hiRow, UINT32 width);
	void MallatToLinear(int srcLevel, DataT* loRow, DataT* hiRow, UINT32 width);

#ifdef __PGFROISUPPORT__
	CROIs		m_ROIs;							// ROI information
#endif //__PGFROISUPPORT__

	int			m_nLevels;						// number of transform levels: one more than the number of level in PGFimage
	CSubband	(*m_subband)[NSubbands];		// quadtree of subbands: LL HL												
												//                       LH HH
};

#endif //PGF_WAVELETTRANSFORM_H
