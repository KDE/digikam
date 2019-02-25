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
/// @file WaveletTransform.cpp
/// @brief PGF wavelet transform class implementation
/// @author C. Stamm

#include "WaveletTransform.h"

#define c1 1	// best value 1
#define c2 2	// best value 2

//////////////////////////////////////////////////////////////////////////
// Constructor: Constructs a wavelet transform pyramid of given size and levels.
// @param width The width of the original image (at level 0) in pixels
// @param height The height of the original image (at level 0) in pixels
// @param levels The number of levels (>= 0)
// @param data Input data of subband LL at level 0
CWaveletTransform::CWaveletTransform(UINT32 width, UINT32 height, int levels, DataT* data) 
: 
#ifdef __PGFROISUPPORT__
  m_indices(nullptr),
#endif
  m_nLevels(levels + 1) // m_nLevels in CPGFImage determines the number of FWT steps; this.m_nLevels determines the number subband-planes
, m_subband(nullptr)
{
	ASSERT(m_nLevels > 0 && m_nLevels <= MaxLevel + 1);
	InitSubbands(width, height, data);
}

/////////////////////////////////////////////////////////////////////
// Initialize size subbands on all levels
void CWaveletTransform::InitSubbands(UINT32 width, UINT32 height, DataT* data) {
	if (m_subband) Destroy();

	// create subbands
	m_subband = new CSubband[m_nLevels][NSubbands];

	// init subbands
	UINT32 loWidth = width;
	UINT32 hiWidth = width;
	UINT32 loHeight = height;
	UINT32 hiHeight = height;

	for (int level = 0; level < m_nLevels; level++) {
		m_subband[level][LL].Initialize(loWidth, loHeight, level, LL);	// LL
		m_subband[level][HL].Initialize(hiWidth, loHeight, level, HL);	//    HL
		m_subband[level][LH].Initialize(loWidth, hiHeight, level, LH);	// LH
		m_subband[level][HH].Initialize(hiWidth, hiHeight, level, HH);	//    HH
		hiWidth = loWidth >> 1;			hiHeight = loHeight >> 1;
		loWidth = (loWidth + 1) >> 1;	loHeight = (loHeight + 1) >> 1;
	}
	if (data) {
		m_subband[0][LL].SetBuffer(data);
	}
}

//////////////////////////////////////////////////////////////////////////
// Compute fast forward wavelet transform of LL subband at given level and
// stores result in all 4 subbands of level + 1.
// Wavelet transform used in writing a PGF file
// Forward Transform of srcBand and split and store it into subbands on destLevel
// low pass filter at even positions: 1/8[-1, 2, (6), 2, -1]
// high pass filter at odd positions: 1/4[-2, (4), -2]
// @param level A wavelet transform pyramid level (>= 0 && < Levels())
// @param quant A quantization value (linear scalar quantization)
// @return error in case of a memory allocation problem
OSError CWaveletTransform::ForwardTransform(int level, int quant) {
	ASSERT(level >= 0 && level < m_nLevels - 1);
	const int destLevel = level + 1;
	ASSERT(m_subband[destLevel]);
	CSubband* srcBand = &m_subband[level][LL]; ASSERT(srcBand);
	const UINT32 width = srcBand->GetWidth();
	const UINT32 height = srcBand->GetHeight();
	DataT* src = srcBand->GetBuffer(); ASSERT(src);
	DataT *row0, *row1, *row2, *row3;

	// Allocate memory for next transform level
	for (int i=0; i < NSubbands; i++) {
		if (!m_subband[destLevel][i].AllocMemory()) return InsufficientMemory;
	}

 	if (height >= FilterSize) { // changed from FilterSizeH to FilterSize
		// top border handling
		row0 = src; row1 = row0 + width; row2 = row1 + width;
		ForwardRow(row0, width);
		ForwardRow(row1, width);
		ForwardRow(row2, width);
		for (UINT32 k=0; k < width; k++) {
			row1[k] -= ((row0[k] + row2[k] + c1) >> 1); // high pass
			row0[k] += ((row1[k] + c1) >> 1); // low pass
		}
		InterleavedToSubbands(destLevel, row0, row1, width);
		row0 = row1; row1 = row2; row2 += width; row3 = row2 + width;

		// middle part
		for (UINT32 i=3; i < height-1; i += 2) {
			ForwardRow(row2, width);
			ForwardRow(row3, width);
			for (UINT32 k=0; k < width; k++) {
				row2[k] -= ((row1[k] + row3[k] + c1) >> 1); // high pass filter
				row1[k] += ((row0[k] + row2[k] + c2) >> 2); // low pass filter
			}
			InterleavedToSubbands(destLevel, row1, row2, width);
			row0 = row2; row1 = row3; row2 = row3 + width; row3 = row2 + width;
		}

		// bottom border handling
		if (height & 1) {
			for (UINT32 k=0; k < width; k++) {
				row1[k] += ((row0[k] + c1) >> 1); // low pass
			}
			InterleavedToSubbands(destLevel, row1, nullptr, width);
			row0 = row1; row1 += width;
		} else {
			ForwardRow(row2, width);
			for (UINT32 k=0; k < width; k++) {
				row2[k] -= row1[k]; // high pass
				row1[k] += ((row0[k] + row2[k] + c2) >> 2); // low pass
			}
			InterleavedToSubbands(destLevel, row1, row2, width);
			row0 = row1; row1 = row2; row2 += width;
		}
	} else {
		// if height is too small
		row0 = src; row1 = row0 + width;
		// first part
		for (UINT32 k=0; k < height; k += 2) {
			ForwardRow(row0, width);
			ForwardRow(row1, width);
			InterleavedToSubbands(destLevel, row0, row1, width);
			row0 += width << 1; row1 += width << 1;
		}
		// bottom
		if (height & 1) {
			InterleavedToSubbands(destLevel, row0, nullptr, width);
		}
	}

	if (quant > 0) {
		// subband quantization (without LL)
		for (int i=1; i < NSubbands; i++) {
			m_subband[destLevel][i].Quantize(quant);
		}
		// LL subband quantization
		if (destLevel == m_nLevels - 1) {
			m_subband[destLevel][LL].Quantize(quant);
		}
	}

	// free source band
	srcBand->FreeMemory();
	return NoError;
}

//////////////////////////////////////////////////////////////
// Forward transform one row
// low pass filter at even positions: 1/8[-1, 2, (6), 2, -1]
// high pass filter at odd positions: 1/4[-2, (4), -2]
void CWaveletTransform::ForwardRow(DataT* src, UINT32 width) {
	if (width >= FilterSize) {
		UINT32 i = 3;

		// left border handling
		src[1] -= ((src[0] + src[2] + c1) >> 1); // high pass
		src[0] += ((src[1] + c1) >> 1); // low pass
		
		// middle part
		for (; i < width-1; i += 2) {
			src[i] -= ((src[i-1] + src[i+1] + c1) >> 1); // high pass
			src[i-1] += ((src[i-2] + src[i] + c2) >> 2); // low pass
		}

		// right border handling
		if (width & 1) {
			src[i-1] += ((src[i-2] + c1) >> 1); // low pass
		} else {
			src[i] -= src[i-1]; // high pass
			src[i-1] += ((src[i-2] + src[i] + c2) >> 2); // low pass
		}
	}
}

/////////////////////////////////////////////////////////////////
// Copy transformed and interleaved (L,H,L,H,...) rows loRow and hiRow to subbands LL,HL,LH,HH
void CWaveletTransform::InterleavedToSubbands(int destLevel, DataT* loRow, DataT* hiRow, UINT32 width) {
	const UINT32 wquot = width >> 1;
	const bool wrem = (width & 1);
	CSubband &ll = m_subband[destLevel][LL], &hl = m_subband[destLevel][HL];
	CSubband &lh = m_subband[destLevel][LH], &hh = m_subband[destLevel][HH];

	if (hiRow) {
		for (UINT32 i=0; i < wquot; i++) {
			ll.WriteBuffer(*loRow++);	// first access, than increment
			hl.WriteBuffer(*loRow++);
			lh.WriteBuffer(*hiRow++);	// first access, than increment
			hh.WriteBuffer(*hiRow++);
		}
		if (wrem) {
			ll.WriteBuffer(*loRow);
			lh.WriteBuffer(*hiRow);
		}
	} else {
		for (UINT32 i=0; i < wquot; i++) {
			ll.WriteBuffer(*loRow++);	// first access, than increment
			hl.WriteBuffer(*loRow++);
		}
		if (wrem) ll.WriteBuffer(*loRow);
	}
}

//////////////////////////////////////////////////////////////////////////
// Compute fast inverse wavelet transform of all 4 subbands of given level and
// stores result in LL subband of level - 1.
// Inverse wavelet transform used in reading a PGF file
// Inverse Transform srcLevel and combine to destBand
// low-pass coefficients at even positions, high-pass coefficients at odd positions
// inverse filter for even positions: 1/4[-1, (4), -1]
// inverse filter for odd positions: 1/8[-1, 4, (6), 4, -1]
// @param srcLevel A wavelet transform pyramid level (> 0 && <= Levels())
// @param w [out] A pointer to the returned width of subband LL (in pixels)
// @param h [out] A pointer to the returned height of subband LL (in pixels)
// @param data [out] A pointer to the returned array of image data
// @return error in case of a memory allocation problem
OSError CWaveletTransform::InverseTransform(int srcLevel, UINT32* w, UINT32* h, DataT** data) {
	ASSERT(srcLevel > 0 && srcLevel < m_nLevels);
	const int destLevel = srcLevel - 1;
	ASSERT(m_subband[destLevel]);
	CSubband* destBand = &m_subband[destLevel][LL];
	UINT32 width, height;

	// allocate memory for the results of the inverse transform 
	if (!destBand->AllocMemory()) return InsufficientMemory;
	DataT *origin = destBand->GetBuffer(), *row0, *row1, *row2, *row3;

#ifdef __PGFROISUPPORT__
	PGFRect destROI = destBand->GetAlignedROI();	
	const UINT32 destWidth  = destROI.Width();  // destination buffer width
	const UINT32 destHeight = destROI.Height(); // destination buffer height
	width = destWidth;		// destination working width
	height = destHeight;	// destination working height

	// update destination ROI
	if (destROI.top & 1) {
		destROI.top++;
		origin += destWidth;
		height--;
	}
	if (destROI.left & 1) {
		destROI.left++;
		origin++;
		width--;
	}

	// init source buffer position
	const UINT32 leftD = destROI.left >> 1;
	const UINT32 left0 = m_subband[srcLevel][LL].GetAlignedROI().left;
	const UINT32 left1 = m_subband[srcLevel][HL].GetAlignedROI().left;
	const UINT32 topD = destROI.top >> 1;
	const UINT32 top0 = m_subband[srcLevel][LL].GetAlignedROI().top;
	const UINT32 top1 = m_subband[srcLevel][LH].GetAlignedROI().top;
	ASSERT(m_subband[srcLevel][LH].GetAlignedROI().left == left0);
	ASSERT(m_subband[srcLevel][HH].GetAlignedROI().left == left1);
	ASSERT(m_subband[srcLevel][HL].GetAlignedROI().top == top0);
	ASSERT(m_subband[srcLevel][HH].GetAlignedROI().top == top1);

	UINT32 srcOffsetX[2] = { 0, 0 };
	UINT32 srcOffsetY[2] = { 0, 0 };

	if (leftD >= __max(left0, left1)) {
		srcOffsetX[0] = leftD - left0;
		srcOffsetX[1] = leftD - left1;
	} else {
		if (left0 <= left1) {
			const UINT32 dx = (left1 - leftD) << 1;
			destROI.left += dx;
			origin += dx;
			width -= dx;
			srcOffsetX[0] = left1 - left0;
		} else {
			const UINT32 dx = (left0 - leftD) << 1;
			destROI.left += dx;
			origin += dx;
			width -= dx;
			srcOffsetX[1] = left0 - left1;
		}
	}
	if (topD >= __max(top0, top1)) {
		srcOffsetY[0] = topD - top0;
		srcOffsetY[1] = topD - top1;
	} else {
		if (top0 <= top1) {
			const UINT32 dy = (top1 - topD) << 1;
			destROI.top += dy;
			origin += dy*destWidth;
			height -= dy;
			srcOffsetY[0] = top1 - top0;
		} else {
			const UINT32 dy = (top0 - topD) << 1;
			destROI.top += dy;
			origin += dy*destWidth;
			height -= dy;
			srcOffsetY[1] = top0 - top1;
		}
	}

	m_subband[srcLevel][LL].InitBuffPos(srcOffsetX[0], srcOffsetY[0]);
	m_subband[srcLevel][HL].InitBuffPos(srcOffsetX[1], srcOffsetY[0]);
	m_subband[srcLevel][LH].InitBuffPos(srcOffsetX[0], srcOffsetY[1]);
	m_subband[srcLevel][HH].InitBuffPos(srcOffsetX[1], srcOffsetY[1]);

#else
	width = destBand->GetWidth();
	height = destBand->GetHeight();
	PGFRect destROI(0, 0, width, height);
	const UINT32 destWidth = width; // destination buffer width
	const UINT32 destHeight = height; // destination buffer height

	// init source buffer position
	for (int i = 0; i < NSubbands; i++) {
		m_subband[srcLevel][i].InitBuffPos();
	}
#endif

	if (destHeight >= FilterSize) { // changed from FilterSizeH to FilterSize
		// top border handling
		row0 = origin; row1 = row0 + destWidth;
		SubbandsToInterleaved(srcLevel, row0, row1, width);
		for (UINT32 k = 0; k < width; k++) {
			row0[k] -= ((row1[k] + c1) >> 1); // even
		}

		// middle part
		row2 = row1 + destWidth; row3 = row2 + destWidth;
		for (UINT32 i = destROI.top + 2; i < destROI.bottom - 1; i += 2) {
			SubbandsToInterleaved(srcLevel, row2, row3, width);
			for (UINT32 k = 0; k < width; k++) {
				row2[k] -= ((row1[k] + row3[k] + c2) >> 2); // even
				row1[k] += ((row0[k] + row2[k] + c1) >> 1); // odd
			}
			InverseRow(row0, width);
			InverseRow(row1, width);
			row0 = row2; row1 = row3; row2 = row1 + destWidth; row3 = row2 + destWidth;
		}

		// bottom border handling
		if (height & 1) {
			SubbandsToInterleaved(srcLevel, row2, nullptr, width);
			for (UINT32 k = 0; k < width; k++) {
				row2[k] -= ((row1[k] + c1) >> 1); // even
				row1[k] += ((row0[k] + row2[k] + c1) >> 1); // odd
			}
			InverseRow(row0, width);
			InverseRow(row1, width);
			InverseRow(row2, width);
			row0 = row1; row1 = row2; row2 += destWidth;
		} else {
			for (UINT32 k = 0; k < width; k++) {
				row1[k] += row0[k];
			}
			InverseRow(row0, width);
			InverseRow(row1, width);
			row0 = row1; row1 += destWidth;
		}
	} else {
		// height is too small
		row0 = origin; row1 = row0 + destWidth;
		// first part
		for (UINT32 k = 0; k < height; k += 2) {
			SubbandsToInterleaved(srcLevel, row0, row1, width);
			InverseRow(row0, width);
			InverseRow(row1, width);
			row0 += destWidth << 1; row1 += destWidth << 1;
		}
		// bottom
		if (height & 1) {
			SubbandsToInterleaved(srcLevel, row0, nullptr, width);
			InverseRow(row0, width);
		}
	}

	// free memory of the current srcLevel
	for (int i = 0; i < NSubbands; i++) {
		m_subband[srcLevel][i].FreeMemory();
	}

	// return info
	*w = destWidth;
	*h = destHeight;
	*data = destBand->GetBuffer();
	return NoError;
}

//////////////////////////////////////////////////////////////////////
// Inverse Wavelet Transform of one row
// low-pass coefficients at even positions, high-pass coefficients at odd positions
// inverse filter for even positions: 1/4[-1, (4), -1]
// inverse filter for odd positions: 1/8[-1, 4, (6), 4, -1]
void CWaveletTransform::InverseRow(DataT* dest, UINT32 width) {
	if (width >= FilterSize) {
		UINT32 i = 2;

		// left border handling
		dest[0] -= ((dest[1] + c1) >> 1); // even

		// middle part
		for (; i < width - 1; i += 2) {
			dest[i] -= ((dest[i-1] + dest[i+1] + c2) >> 2); // even
			dest[i-1] += ((dest[i-2] + dest[i] + c1) >> 1); // odd
		}

		// right border handling
		if (width & 1) {
			dest[i] -= ((dest[i-1] + c1) >> 1); // even
			dest[i-1] += ((dest[i-2] + dest[i] + c1) >> 1); // odd
		} else {
			dest[i-1] += dest[i-2]; // odd
		}
	}
}

///////////////////////////////////////////////////////////////////
// Copy transformed coefficients from subbands LL,HL,LH,HH to interleaved format (L,H,L,H,...)
void CWaveletTransform::SubbandsToInterleaved(int srcLevel, DataT* loRow, DataT* hiRow, UINT32 width) {
	const UINT32 wquot = width >> 1;
	const bool wrem = (width & 1);
	CSubband &ll = m_subband[srcLevel][LL], &hl = m_subband[srcLevel][HL];
	CSubband &lh = m_subband[srcLevel][LH], &hh = m_subband[srcLevel][HH];

	if (hiRow) {
	#ifdef __PGFROISUPPORT__
		const bool storePos = wquot < ll.BufferWidth();
		UINT32 llPos = 0, hlPos = 0, lhPos = 0, hhPos = 0;

		if (storePos) {
			// save current src buffer positions
			llPos = ll.GetBuffPos(); 
			hlPos = hl.GetBuffPos(); 
			lhPos = lh.GetBuffPos(); 
			hhPos = hh.GetBuffPos(); 
		}
	#endif

		for (UINT32 i=0; i < wquot; i++) {
			*loRow++ = ll.ReadBuffer();// first access, than increment
			*loRow++ = hl.ReadBuffer();// first access, than increment
			*hiRow++ = lh.ReadBuffer();// first access, than increment
			*hiRow++ = hh.ReadBuffer();// first access, than increment
		}

		if (wrem) {
			*loRow++ = ll.ReadBuffer();// first access, than increment
			*hiRow++ = lh.ReadBuffer();// first access, than increment
		}

	#ifdef __PGFROISUPPORT__
		if (storePos) {
			// increment src buffer positions
			ll.IncBuffRow(llPos); 
			hl.IncBuffRow(hlPos); 
			lh.IncBuffRow(lhPos); 
			hh.IncBuffRow(hhPos); 
		}
	#endif

	} else {
	#ifdef __PGFROISUPPORT__
		const bool storePos = wquot < ll.BufferWidth();
		UINT32 llPos = 0, hlPos = 0;

		if (storePos) {
			// save current src buffer positions
			llPos = ll.GetBuffPos(); 
			hlPos = hl.GetBuffPos(); 
		}
	#endif

		for (UINT32 i=0; i < wquot; i++) {
			*loRow++ = ll.ReadBuffer();// first access, than increment
			*loRow++ = hl.ReadBuffer();// first access, than increment
		}
		if (wrem) *loRow++ = ll.ReadBuffer();

	#ifdef __PGFROISUPPORT__
		if (storePos) {
			// increment src buffer positions
			ll.IncBuffRow(llPos); 
			hl.IncBuffRow(hlPos); 
		}
	#endif
	}
}

#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////////
/// Compute and store ROIs for nLevels
/// @param roi rectangular region of interest at level 0
void CWaveletTransform::SetROI(PGFRect roi) {
	const UINT32 delta = (FilterSize >> 1) << m_nLevels;

	// create tile indices
	delete[] m_indices;
	m_indices = new PGFRect[m_nLevels];

	// enlarge rect: add margin
	roi.left = (roi.left > delta) ? roi.left - delta : 0;
	roi.top  = (roi.top  > delta) ? roi.top  - delta : 0;
	roi.right += delta; 
	roi.bottom += delta; 

	for (int l = 0; l < m_nLevels; l++) {
		PGFRect alignedROI;
		PGFRect& indices = m_indices[l];
		UINT32 nTiles = GetNofTiles(l);
		CSubband& subband = m_subband[l][LL];

		// use roi to determine the necessary tile indices (for all subbands the same) and aligned ROI for LL subband
		subband.SetNTiles(nTiles); // must be called before TileIndex()
		subband.TileIndex(true, roi.left, roi.top, indices.left, indices.top, alignedROI.left, alignedROI.top);
		subband.TileIndex(false, roi.right, roi.bottom, indices.right, indices.bottom, alignedROI.right, alignedROI.bottom);
		subband.SetAlignedROI(alignedROI);
		ASSERT(l == 0 ||
			(m_indices[l-1].left >= 2*m_indices[l].left &&
			m_indices[l-1].top >= 2*m_indices[l].top &&
			m_indices[l-1].right <= 2*m_indices[l].right &&
			m_indices[l-1].bottom <= 2*m_indices[l].bottom));

		// determine aligned ROI of other three subbands
		PGFRect aroi;
		UINT32 w, h;
		for (int b = 1; b < NSubbands; b++) {
			CSubband& sb = m_subband[l][b];
			sb.SetNTiles(nTiles); // must be called before TilePosition()
			sb.TilePosition(indices.left, indices.top, aroi.left, aroi.top, w, h);
			sb.TilePosition(indices.right - 1, indices.bottom - 1, aroi.right, aroi.bottom, w, h);
			aroi.right += w;
			aroi.bottom += h;
			sb.SetAlignedROI(aroi);
		}

		// use aligned ROI of LL subband for next level
		roi.left = alignedROI.left >> 1;
		roi.top = alignedROI.top >> 1;
		roi.right = (alignedROI.right + 1) >> 1;
		roi.bottom = (alignedROI.bottom + 1) >> 1;
	}
}

#endif // __PGFROISUPPORT__
