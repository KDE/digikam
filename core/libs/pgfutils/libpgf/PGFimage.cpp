/*
 * The Progressive Graphics File; http://www.libpgf.org
 * 
 * $Date: 2007-02-03 13:04:21 +0100 (Sa, 03 Feb 2007) $
 * $Revision: 280 $
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
/// @file PGFimage.cpp
/// @brief PGF image class implementation
/// @author C. Stamm

#include "PGFimage.h"
#include "Decoder.h"
#include "Encoder.h"
#include "BitStream.h"
#include <cmath>
#include <cstring>

#define YUVoffset4		8				// 2^3
#define YUVoffset6		32				// 2^5
#define YUVoffset8		128				// 2^7
#define YUVoffset16		32768			// 2^15
//#define YUVoffset31		1073741824		// 2^30

//////////////////////////////////////////////////////////////////////
// global methods and variables
#ifdef NEXCEPTIONS
	OSError _PGF_Error_;

	OSError GetLastPGFError() {
		OSError tmp = _PGF_Error_;
		_PGF_Error_ = NoError;
		return tmp;
	}
#endif

#ifdef _DEBUG
	// allows RGB and RGBA image visualization inside Visual Studio Debugger
	struct DebugBGRImage {
		int width, height, pitch;
		BYTE *data;
	} roiimage;
#endif

//////////////////////////////////////////////////////////////////////
// Standard constructor
CPGFImage::CPGFImage() {
	Init();
}

//////////////////////////////////////////////////////////////////////
void CPGFImage::Init() {
	// init pointers
	m_decoder = nullptr;
	m_encoder = nullptr;
	m_levelLength = nullptr;

	// init members
#ifdef __PGFROISUPPORT__
	m_streamReinitialized = false;
#endif
	m_currentLevel = 0;
	m_quant = 0;
	m_userDataPos = 0;
	m_downsample = false;
	m_favorSpeedOverSize = false;
	m_useOMPinEncoder = true;
	m_useOMPinDecoder = true;
	m_cb = nullptr;
	m_cbArg = nullptr;
	m_progressMode = PM_Relative;
	m_percent = 0;
	m_userDataPolicy = UP_CacheAll;

	// init preHeader
	memcpy(m_preHeader.magic, PGFMagic, 3);
	m_preHeader.version = PGFVersion;
	m_preHeader.hSize = 0;

	// init postHeader
	m_postHeader.userData = nullptr;
	m_postHeader.userDataLen = 0;
	m_postHeader.cachedUserDataLen = 0;

	// init channels
	for (int i = 0; i < MaxChannels; i++) {
		m_channel[i] = nullptr;
		m_wtChannel[i] = nullptr;
	}

	// set image width and height
	for (int i = 0; i < MaxChannels; i++) {
		m_width[0] = 0;
		m_height[0] = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Destructor: Destroy internal data structures.
CPGFImage::~CPGFImage() {
	m_currentLevel = -100; // unusual value used as marker in Destroy()
	Destroy();
}

//////////////////////////////////////////////////////////////////////
// Destroy internal data structures. Object state after this is the same as after CPGFImage().
void CPGFImage::Destroy() {
	for (int i = 0; i < m_header.channels; i++) {
		delete m_wtChannel[i]; // also deletes m_channel
	}
	delete[] m_postHeader.userData; 
	delete[] m_levelLength;
	delete m_decoder;
	delete m_encoder;

	if (m_currentLevel != -100) Init();
}

/////////////////////////////////////////////////////////////////////////////
// Open a PGF image at current stream position: read pre-header, header, levelLength, and ckeck image type.
// Precondition: The stream has been opened for reading.
// It might throw an IOException.
// @param stream A PGF stream
void CPGFImage::Open(CPGFStream *stream) {
	ASSERT(stream);

	// create decoder and read PGFPreHeader PGFHeader PGFPostHeader LevelLengths
	m_decoder = new CDecoder(stream, m_preHeader, m_header, m_postHeader, m_levelLength, 
		m_userDataPos, m_useOMPinDecoder, m_userDataPolicy);

	if (m_header.nLevels > MaxLevel) ReturnWithError(FormatCannotRead);

	// set current level
	m_currentLevel = m_header.nLevels;

	// set image width and height
	m_width[0] = m_header.width;
	m_height[0] = m_header.height;

	// complete header
	if (!CompleteHeader()) ReturnWithError(FormatCannotRead);

	// interpret quant parameter
	if (m_header.quality > DownsampleThreshold && 
		(m_header.mode == ImageModeRGBColor || 
		 m_header.mode == ImageModeRGBA || 
		 m_header.mode == ImageModeRGB48 || 
		 m_header.mode == ImageModeCMYKColor || 
		 m_header.mode == ImageModeCMYK64 || 
		 m_header.mode == ImageModeLabColor || 
		 m_header.mode == ImageModeLab48)) {
		m_downsample = true;
		m_quant = m_header.quality - 1;
	} else {
		m_downsample = false;
		m_quant = m_header.quality;
	}

	// set channel dimensions (chrominance is subsampled by factor 2)
	if (m_downsample) {
		for (int i=1; i < m_header.channels; i++) {
			m_width[i] = (m_width[0] + 1) >> 1;
			m_height[i] = (m_height[0] + 1) >> 1;
		}
	} else {
		for (int i=1; i < m_header.channels; i++) {
			m_width[i] = m_width[0];
			m_height[i] = m_height[0];
		}
	}

	if (m_header.nLevels > 0) {
		// init wavelet subbands
		for (int i=0; i < m_header.channels; i++) {
			m_wtChannel[i] = new CWaveletTransform(m_width[i], m_height[i], m_header.nLevels);
		}

		// used in Read when PM_Absolute
		m_percent = pow(0.25, m_header.nLevels);

	} else {
		// very small image: we don't use DWT and encoding

		// read channels
		for (int c=0; c < m_header.channels; c++) {
			const UINT32 size = m_width[c]*m_height[c];
			m_channel[c] = new(std::nothrow) DataT[size];
			if (!m_channel[c]) ReturnWithError(InsufficientMemory);

			// read channel data from stream
			for (UINT32 i=0; i < size; i++) {
				int count = DataTSize;
				stream->Read(&count, &m_channel[c][i]);
				if (count != DataTSize) ReturnWithError(MissingData);
			}
		}
	}
}

////////////////////////////////////////////////////////////
bool CPGFImage::CompleteHeader() {
	// set current codec version
	m_header.version = PGFVersionNumber(PGFMajorNumber, PGFYear, PGFWeek);

	if (m_header.mode == ImageModeUnknown) {
		// undefined mode
		switch(m_header.bpp) {
		case 1: m_header.mode = ImageModeBitmap; break;
		case 8: m_header.mode = ImageModeGrayScale; break;
		case 12: m_header.mode = ImageModeRGB12; break;
		case 16: m_header.mode = ImageModeRGB16; break;
		case 24: m_header.mode = ImageModeRGBColor; break;
		case 32: m_header.mode = ImageModeRGBA; break;
		case 48: m_header.mode = ImageModeRGB48; break;
		default: m_header.mode = ImageModeRGBColor; break;
		}
	}
	if (!m_header.bpp) {
		// undefined bpp
		switch(m_header.mode) {
		case ImageModeBitmap: 
			m_header.bpp = 1;
			break;
		case ImageModeIndexedColor:
		case ImageModeGrayScale:
			m_header.bpp = 8;
			break;
		case ImageModeRGB12:
			m_header.bpp = 12;
			break;
		case ImageModeRGB16:
		case ImageModeGray16:
			m_header.bpp = 16;
			break;
		case ImageModeRGBColor:
		case ImageModeLabColor:
			m_header.bpp = 24;
			break;
		case ImageModeRGBA:
		case ImageModeCMYKColor:
		case ImageModeGray32:
			m_header.bpp = 32;
			break;
		case ImageModeRGB48:
		case ImageModeLab48:
			m_header.bpp = 48;
			break;
		case ImageModeCMYK64:
			m_header.bpp = 64;
			break;
		default:
			ASSERT(false);
			m_header.bpp = 24;
		}
	} 
	if (m_header.mode == ImageModeRGBColor && m_header.bpp == 32) {
		// change mode
		m_header.mode = ImageModeRGBA;
	}
	if (m_header.mode == ImageModeBitmap && m_header.bpp != 1) return false;
	if (m_header.mode == ImageModeIndexedColor && m_header.bpp != 8) return false;
	if (m_header.mode == ImageModeGrayScale && m_header.bpp != 8) return false;
	if (m_header.mode == ImageModeGray16 && m_header.bpp != 16) return false;
	if (m_header.mode == ImageModeGray32 && m_header.bpp != 32) return false;
	if (m_header.mode == ImageModeRGBColor && m_header.bpp != 24) return false;
	if (m_header.mode == ImageModeRGBA && m_header.bpp != 32) return false;
	if (m_header.mode == ImageModeRGB12 && m_header.bpp != 12) return false;
	if (m_header.mode == ImageModeRGB16 && m_header.bpp != 16) return false;
	if (m_header.mode == ImageModeRGB48 && m_header.bpp != 48) return false;
	if (m_header.mode == ImageModeLabColor && m_header.bpp != 24) return false;
	if (m_header.mode == ImageModeLab48 && m_header.bpp != 48) return false;
	if (m_header.mode == ImageModeCMYKColor && m_header.bpp != 32) return false;
	if (m_header.mode == ImageModeCMYK64 && m_header.bpp != 64) return false;

	// set number of channels
	if (!m_header.channels) {
		switch(m_header.mode) {
		case ImageModeBitmap: 
		case ImageModeIndexedColor:
		case ImageModeGrayScale:
		case ImageModeGray16:
		case ImageModeGray32:
			m_header.channels = 1; 
			break;
		case ImageModeRGBColor:
		case ImageModeRGB12:
		case ImageModeRGB16:
		case ImageModeRGB48:
		case ImageModeLabColor:
		case ImageModeLab48:
			m_header.channels = 3;
			break;
		case ImageModeRGBA:
		case ImageModeCMYKColor:
		case ImageModeCMYK64:
			m_header.channels = 4;
			break;
		default:
			return false;
		}
	}

	// store used bits per channel
	UINT8 bpc = m_header.bpp/m_header.channels;
	if (bpc > 31) bpc = 31;
	if (!m_header.usedBitsPerChannel || m_header.usedBitsPerChannel > bpc) {
		m_header.usedBitsPerChannel = bpc;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
/// Return user data and size of user data.
/// Precondition: The PGF image has been opened with a call of Open(...).
/// In an encoder scenario don't call this method before WriteHeader().
/// @param cachedSize [out] Size of returned user data in bytes.
/// @param pTotalSize [optional out] Pointer to return the size of user data stored in image header in bytes.
/// @return A pointer to user data or nullptr if there is no user data available.
const UINT8* CPGFImage::GetUserData(UINT32& cachedSize, UINT32* pTotalSize /*= nullptr*/) const {
	cachedSize = m_postHeader.cachedUserDataLen;
	if (pTotalSize) *pTotalSize = m_postHeader.userDataLen;
	return m_postHeader.userData;
}

//////////////////////////////////////////////////////////////////////
/// After you've written a PGF image, you can call this method followed by GetBitmap/GetYUV
/// to get a quick reconstruction (coded -> decoded image).
/// It might throw an IOException.
/// @param level The image level of the resulting image in the internal image buffer.
void CPGFImage::Reconstruct(int level /*= 0*/) {
	if (m_header.nLevels == 0) {
		// image didn't use wavelet transform
		if (level == 0) {
			for (int i=0; i < m_header.channels; i++) {
				ASSERT(m_wtChannel[i]);
				m_channel[i] = m_wtChannel[i]->GetSubband(0, LL)->GetBuffer();
			}
		}
	} else {
		int currentLevel = m_header.nLevels;

	#ifdef __PGFROISUPPORT__
		if (ROIisSupported()) {
			// enable ROI reading
			SetROI(PGFRect(0, 0, m_header.width, m_header.height));
		}
	#endif

		while (currentLevel > level) {
			for (int i=0; i < m_header.channels; i++) {
				ASSERT(m_wtChannel[i]);
				// dequantize subbands
				if (currentLevel == m_header.nLevels) { 
					// last level also has LL band
					m_wtChannel[i]->GetSubband(currentLevel, LL)->Dequantize(m_quant);
				}
				m_wtChannel[i]->GetSubband(currentLevel, HL)->Dequantize(m_quant);
				m_wtChannel[i]->GetSubband(currentLevel, LH)->Dequantize(m_quant);
				m_wtChannel[i]->GetSubband(currentLevel, HH)->Dequantize(m_quant);

				// inverse transform from m_wtChannel to m_channel
				OSError err = m_wtChannel[i]->InverseTransform(currentLevel, &m_width[i], &m_height[i], &m_channel[i]);
				if (err != NoError) ReturnWithError(err);
				ASSERT(m_channel[i]);
			}

			currentLevel--;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Read and decode some levels of a PGF image at current stream position.
// A PGF image is structered in levels, numbered between 0 and Levels() - 1.
// Each level can be seen as a single image, containing the same content
// as all other levels, but in a different size (width, height).
// The image size at level i is double the size (width, height) of the image at level i+1.
// The image at level 0 contains the original size.
// Precondition: The PGF image has been opened with a call of Open(...).
// It might throw an IOException.
// @param level The image level of the resulting image in the internal image buffer.
// @param cb A pointer to a callback procedure. The procedure is called after reading a single level. If cb returns true, then it stops proceeding.
// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::Read(int level /*= 0*/, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) {
	ASSERT((level >= 0 && level < m_header.nLevels) || m_header.nLevels == 0); // m_header.nLevels == 0: image didn't use wavelet transform
	ASSERT(m_decoder);

#ifdef __PGFROISUPPORT__
	if (ROIisSupported() && m_header.nLevels > 0) {
		// new encoding scheme supporting ROI
		PGFRect rect(0, 0, m_header.width, m_header.height);
		Read(rect, level, cb, data);
		return;
	}
#endif

	if (m_header.nLevels == 0) {
		if (level == 0) {
			// the data has already been read during open
			// now update progress
			if (cb) {
				if ((*cb)(1.0, true, data)) ReturnWithError(EscapePressed);
			}
		}
	} else {
		const int levelDiff = m_currentLevel - level;
		double percent = (m_progressMode == PM_Relative) ? pow(0.25, levelDiff) : m_percent;

		// encoding scheme without ROI
		while (m_currentLevel > level) {
			for (int i=0; i < m_header.channels; i++) {
				CWaveletTransform* wtChannel = m_wtChannel[i];
				ASSERT(wtChannel);

				// decode file and write stream to m_wtChannel
				if (m_currentLevel == m_header.nLevels) { 
					// last level also has LL band
					wtChannel->GetSubband(m_currentLevel, LL)->PlaceTile(*m_decoder, m_quant);
				}
				if (m_preHeader.version & Version5) {
					// since version 5
					wtChannel->GetSubband(m_currentLevel, HL)->PlaceTile(*m_decoder, m_quant);
					wtChannel->GetSubband(m_currentLevel, LH)->PlaceTile(*m_decoder, m_quant);
				} else {
					// until version 4
					m_decoder->DecodeInterleaved(wtChannel, m_currentLevel, m_quant);
				}
				wtChannel->GetSubband(m_currentLevel, HH)->PlaceTile(*m_decoder, m_quant);
			}

			volatile OSError error = NoError; // volatile prevents optimizations
#ifdef LIBPGF_USE_OPENMP
			#pragma omp parallel for default(shared) 
#endif
			for (int i=0; i < m_header.channels; i++) {
				// inverse transform from m_wtChannel to m_channel
				if (error == NoError) {
					OSError err = m_wtChannel[i]->InverseTransform(m_currentLevel, &m_width[i], &m_height[i], &m_channel[i]);	
					if (err != NoError) error = err;
				}
				ASSERT(m_channel[i]);
			}
			if (error != NoError) ReturnWithError(error);

			// set new level: must be done before refresh callback
			m_currentLevel--;

			// now we have to refresh the display
			if (m_cb) m_cb(m_cbArg);

			// now update progress
			if (cb) {
				percent *= 4;
				if (m_progressMode == PM_Absolute) m_percent = percent;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}
	}
}

#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////////
/// Read and decode rectangular region of interest (ROI) of a PGF image at current stream position.
/// The origin of the coordinate axis is the top-left corner of the image.
/// All coordinates are measured in pixels.
/// It might throw an IOException.
/// @param rect [inout] Rectangular region of interest (ROI) at level 0. The rect might be cropped.
/// @param level The image level of the resulting image in the internal image buffer.
/// @param cb A pointer to a callback procedure. The procedure is called after reading a single level. If cb returns true, then it stops proceeding.
/// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::Read(PGFRect& rect, int level /*= 0*/, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) {
	ASSERT((level >= 0 && level < m_header.nLevels) || m_header.nLevels == 0); // m_header.nLevels == 0: image didn't use wavelet transform
	ASSERT(m_decoder);

	if (m_header.nLevels == 0 || !ROIisSupported()) {
		rect.left = rect.top = 0;
		rect.right = m_header.width; rect.bottom = m_header.height;
		Read(level, cb, data);
	} else {
		ASSERT(ROIisSupported());
		// new encoding scheme supporting ROI
		ASSERT(rect.left < m_header.width && rect.top < m_header.height);

		// check rectangle
		if (rect.right == 0 || rect.right > m_header.width) rect.right = m_header.width;
		if (rect.bottom == 0 || rect.bottom > m_header.height) rect.bottom = m_header.height;

		const int levelDiff = m_currentLevel - level;
		double percent = (m_progressMode == PM_Relative) ? pow(0.25, levelDiff) : m_percent;
		
		// check level difference
		if (levelDiff <= 0) {
			// it is a new read call, probably with a new ROI
			m_currentLevel = m_header.nLevels;
			m_decoder->SetStreamPosToData();
		}

		// enable ROI decoding and reading
		SetROI(rect);

		while (m_currentLevel > level) {
			for (int i=0; i < m_header.channels; i++) {
				CWaveletTransform* wtChannel = m_wtChannel[i];
				ASSERT(wtChannel);

				// get number of tiles and tile indices
				const UINT32 nTiles = wtChannel->GetNofTiles(m_currentLevel); // independent of ROI

				// decode file and write stream to m_wtChannel
				if (m_currentLevel == m_header.nLevels) { // last level also has LL band
					ASSERT(nTiles == 1);
					m_decoder->GetNextMacroBlock();
					wtChannel->GetSubband(m_currentLevel, LL)->PlaceTile(*m_decoder, m_quant);
				}
				for (UINT32 tileY=0; tileY < nTiles; tileY++) {
					for (UINT32 tileX=0; tileX < nTiles; tileX++) {
						// check relevance of tile
						if (wtChannel->TileIsRelevant(m_currentLevel, tileX, tileY)) {
							m_decoder->GetNextMacroBlock();
							wtChannel->GetSubband(m_currentLevel, HL)->PlaceTile(*m_decoder, m_quant, true, tileX, tileY);
							wtChannel->GetSubband(m_currentLevel, LH)->PlaceTile(*m_decoder, m_quant, true, tileX, tileY);
							wtChannel->GetSubband(m_currentLevel, HH)->PlaceTile(*m_decoder, m_quant, true, tileX, tileY);
						} else {
							// skip tile
							m_decoder->SkipTileBuffer();
						}
					}
				}
			}

			volatile OSError error = NoError; // volatile prevents optimizations
#ifdef LIBPGF_USE_OPENMP
			#pragma omp parallel for default(shared) 
#endif
			for (int i=0; i < m_header.channels; i++) {
				// inverse transform from m_wtChannel to m_channel
				if (error == NoError) {
					OSError err = m_wtChannel[i]->InverseTransform(m_currentLevel, &m_width[i], &m_height[i], &m_channel[i]);
					if (err != NoError) error = err;
				}
				ASSERT(m_channel[i]);
			}
			if (error != NoError) ReturnWithError(error);

			// set new level: must be done before refresh callback
			m_currentLevel--;

			// now we have to refresh the display
			if (m_cb) m_cb(m_cbArg);

			// now update progress
			if (cb) {
				percent *= 4;
				if (m_progressMode == PM_Absolute) m_percent = percent;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
/// Return ROI of channel 0 at current level in pixels.
/// The returned rect is only valid after reading a ROI.
/// @return ROI in pixels
PGFRect CPGFImage::ComputeLevelROI() const {
	if (m_currentLevel == 0) {
		return m_roi;
	} else {
		const UINT32 rLeft = LevelSizeL(m_roi.left, m_currentLevel);
		const UINT32 rRight = LevelSizeL(m_roi.right, m_currentLevel);
		const UINT32 rTop = LevelSizeL(m_roi.top, m_currentLevel);
		const UINT32 rBottom = LevelSizeL(m_roi.bottom, m_currentLevel);
		return PGFRect(rLeft, rTop, rRight - rLeft, rBottom - rTop);
	}
}

//////////////////////////////////////////////////////////////////////
/// Returns aligned ROI in pixels of current level of channel c
/// @param c A channel index
PGFRect CPGFImage::GetAlignedROI(int c /*= 0*/) const {
	PGFRect roi(0, 0, m_width[c], m_height[c]);

	if (ROIisSupported()) {
		ASSERT(m_wtChannel[c]);

		roi = m_wtChannel[c]->GetAlignedROI(m_currentLevel);
	}
	ASSERT(roi.Width() == m_width[c]);
	ASSERT(roi.Height() == m_height[c]);
	return roi;
}

//////////////////////////////////////////////////////////////////////
/// Compute ROIs for each channel and each level <= current level
/// Called inside of Read(rect, ...).
/// @param rect rectangular region of interest (ROI) at level 0
void CPGFImage::SetROI(PGFRect rect) {
	ASSERT(m_decoder);
	ASSERT(ROIisSupported());
	ASSERT(m_wtChannel[0]);

	// store ROI for a later call of GetBitmap
	m_roi = rect;

	// enable ROI decoding
	m_decoder->SetROI();

	// prepare wavelet channels for using ROI
	m_wtChannel[0]->SetROI(rect);

	if (m_downsample && m_header.channels > 1) {
		// all further channels are downsampled, therefore downsample ROI
		rect.left >>= 1;
		rect.top >>= 1;
		rect.right = (rect.right + 1) >> 1;
		rect.bottom = (rect.bottom + 1) >> 1;
	}
	for (int i=1; i < m_header.channels; i++) {
		ASSERT(m_wtChannel[i]);
		m_wtChannel[i]->SetROI(rect);
	}
}

#endif // __PGFROISUPPORT__

//////////////////////////////////////////////////////////////////////
/// Return the length of all encoded headers in bytes.
/// Precondition: The PGF image has been opened with a call of Open(...).
/// @return The length of all encoded headers in bytes
UINT32 CPGFImage::GetEncodedHeaderLength() const { 
	ASSERT(m_decoder); 
	return m_decoder->GetEncodedHeaderLength(); 
}

//////////////////////////////////////////////////////////////////////
/// Reads the encoded PGF header and copies it to a target buffer.
/// Precondition: The PGF image has been opened with a call of Open(...).
/// It might throw an IOException.
/// @param target The target buffer
/// @param targetLen The length of the target buffer in bytes
/// @return The number of bytes copied to the target buffer
UINT32 CPGFImage::ReadEncodedHeader(UINT8* target, UINT32 targetLen) const {
	ASSERT(target);
	ASSERT(targetLen > 0);
	ASSERT(m_decoder);

	// reset stream position
	m_decoder->SetStreamPosToStart();

	// compute number of bytes to read
	UINT32 len = __min(targetLen, GetEncodedHeaderLength());

	// read data
	len = m_decoder->ReadEncodedData(target, len);
	ASSERT(len >= 0 && len <= targetLen);

	return len;
}

////////////////////////////////////////////////////////////////////
/// Reset stream position to start of PGF pre-header or start of data. Must not be called before Open() or before Write(). 
/// Use this method after Read() if you want to read the same image several times, e.g. reading different ROIs.
/// @param startOfData true: you want to read the same image several times. false: resets stream position to the initial position
void CPGFImage::ResetStreamPos(bool startOfData) {
	if (startOfData) {
		ASSERT(m_decoder);
		m_decoder->SetStreamPosToData();
	} else {
		if (m_decoder) {
			m_decoder->SetStreamPosToStart();
		} else if (m_encoder) {
			m_encoder->SetStreamPosToStart();
		} else {
			ASSERT(false);
		}
	}
}

//////////////////////////////////////////////////////////////////////
/// Reads the data of an encoded PGF level and copies it to a target buffer 
/// without decoding.
/// Precondition: The PGF image has been opened with a call of Open(...).
/// It might throw an IOException.
/// @param level The image level
/// @param target The target buffer
/// @param targetLen The length of the target buffer in bytes
/// @return The number of bytes copied to the target buffer
UINT32 CPGFImage::ReadEncodedData(int level, UINT8* target, UINT32 targetLen) const {
	ASSERT(level >= 0 && level < m_header.nLevels);
	ASSERT(target);
	ASSERT(targetLen > 0);
	ASSERT(m_decoder);

	// reset stream position
	m_decoder->SetStreamPosToData();

	// position stream
	UINT64 offset = 0;

	for (int i=m_header.nLevels - 1; i > level; i--) {
		offset += m_levelLength[m_header.nLevels - 1 - i];
	}
	m_decoder->Skip(offset);

	// compute number of bytes to read
	UINT32 len = __min(targetLen, GetEncodedLevelLength(level));

	// read data
	len = m_decoder->ReadEncodedData(target, len);
	ASSERT(len >= 0 && len <= targetLen);

	return len;
}

//////////////////////////////////////////////////////////////////////
/// Set maximum intensity value for image modes with more than eight bits per channel.
/// Call this method after SetHeader, but before ImportBitmap.
/// @param maxValue The maximum intensity value.
void CPGFImage::SetMaxValue(UINT32 maxValue) {
	const BYTE bpc = m_header.bpp/m_header.channels;
	BYTE pot = 0;

	while(maxValue > 0) {
		pot++;
		maxValue >>= 1;
	}
	// store bits per channel
	if (pot > bpc) pot = bpc;
	if (pot > 31) pot = 31;
	m_header.usedBitsPerChannel = pot;
}

//////////////////////////////////////////////////////////////////////
/// Returns number of used bits per input/output image channel.
/// Precondition: header must be initialized.
/// @return number of used bits per input/output image channel.
BYTE CPGFImage::UsedBitsPerChannel() const {
	const BYTE bpc = m_header.bpp/m_header.channels;

	if (bpc > 8) {
		return m_header.usedBitsPerChannel;
	} else {
		return bpc;
	}
}

//////////////////////////////////////////////////////////////////////
/// Return major version
BYTE CPGFImage::CodecMajorVersion(BYTE version) {
	if (version & Version7) return 7;
	if (version & Version6) return 6;
	if (version & Version5) return 5;
	if (version & Version2) return 2;
	return 1;
}

//////////////////////////////////////////////////////////////////
// Import an image from a specified image buffer.
// This method is usually called before Write(...) and after SetHeader(...).
// It might throw an IOException.
// The absolute value of pitch is the number of bytes of an image row.
// If pitch is negative, then buff points to the last row of a bottom-up image (first byte on last row).
// If pitch is positive, then buff points to the first row of a top-down image (first byte).
// The sequence of input channels in the input image buffer does not need to be the same as expected from PGF. In case of different sequences you have to
// provide a channelMap of size of expected channels (depending on image mode). For example, PGF expects in RGB color mode a channel sequence BGR.
// If your provided image buffer contains a channel sequence ARGB, then the channelMap looks like { 3, 2, 1 }.
// @param pitch The number of bytes of a row of the image buffer.
// @param buff An image buffer.
// @param bpp The number of bits per pixel used in image buffer.
// @param channelMap A integer array containing the mapping of input channel ordering to expected channel ordering.
// @param cb A pointer to a callback procedure. The procedure is called after each imported buffer row. If cb returns true, then it stops proceeding.
// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::ImportBitmap(int pitch, UINT8 *buff, BYTE bpp, int channelMap[] /*= nullptr */, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) {
	ASSERT(buff);
	ASSERT(m_channel[0]);

	// color transform
	RgbToYuv(pitch, buff, bpp, channelMap, cb, data);

	if (m_downsample) {
		// Subsampling of the chrominance and alpha channels
		for (int i=1; i < m_header.channels; i++) {
			Downsample(i);
		}
	}
}

/////////////////////////////////////////////////////////////////
// Bilinerar Subsampling of channel ch by a factor 2
// Called before Write()
void CPGFImage::Downsample(int ch) {
	ASSERT(ch > 0);

	const int w = m_width[0];
	const int w2 = w/2;
	const int h2 = m_height[0]/2;
	const int oddW = w%2;				// don't use bool -> problems with MaxSpeed optimization
	const int oddH = m_height[0]%2;		// "
	int loPos = 0;
	int hiPos = w;
	int sampledPos = 0;
	DataT* buff = m_channel[ch]; ASSERT(buff);

	for (int i=0; i < h2; i++) {
		for (int j=0; j < w2; j++) {
			// compute average of pixel block
			buff[sampledPos] = (buff[loPos] + buff[loPos + 1] + buff[hiPos] + buff[hiPos + 1]) >> 2;
			loPos += 2; hiPos += 2;
			sampledPos++;
		}
		if (oddW) { 
			buff[sampledPos] = (buff[loPos] + buff[hiPos]) >> 1;
			loPos++; hiPos++;
			sampledPos++;
		}
		loPos += w; hiPos += w;
	}
	if (oddH) {
		for (int j=0; j < w2; j++) {
			buff[sampledPos] = (buff[loPos] + buff[loPos+1]) >> 1;
			loPos += 2; hiPos += 2;
			sampledPos++;
		}
		if (oddW) {
			buff[sampledPos] = buff[loPos];
		}
	}

	// downsampled image has half width and half height
	m_width[ch] = (m_width[ch] + 1)/2;
	m_height[ch] = (m_height[ch] + 1)/2;
}

//////////////////////////////////////////////////////////////////////
void CPGFImage::ComputeLevels() {
	const int maxThumbnailWidth = 20*FilterSize;
	const int m = __min(m_header.width, m_header.height);
	int s = m;

	if (m_header.nLevels < 1 || m_header.nLevels > MaxLevel) {
		m_header.nLevels = 1;
		// compute a good value depending on the size of the image
		while (s > maxThumbnailWidth) {
			m_header.nLevels++;
			s >>= 1;
		}
	}

	int levels = m_header.nLevels; // we need a signed value during level reduction

	// reduce number of levels if the image size is smaller than FilterSize*(2^levels)
	s = FilterSize*(1 << levels);	// must be at least the double filter size because of subsampling
	while (m < s) {
		levels--;
		s >>= 1;
	}
	if (levels > MaxLevel) m_header.nLevels = MaxLevel;
	else if (levels < 0) m_header.nLevels = 0;
	else m_header.nLevels = (UINT8)levels;

	// used in Write when PM_Absolute
	m_percent = pow(0.25, m_header.nLevels);

	ASSERT(0 <= m_header.nLevels && m_header.nLevels <= MaxLevel);
}

//////////////////////////////////////////////////////////////////////
/// Set PGF header and user data.
/// Precondition: The PGF image has been never opened with Open(...).
/// It might throw an IOException.
/// @param header A valid and already filled in PGF header structure
/// @param flags A combination of additional version flags. In case you use level-wise encoding then set flag = PGFROI.
/// @param userData A user-defined memory block containing any kind of cached metadata.
/// @param userDataLength The size of user-defined memory block in bytes
void CPGFImage::SetHeader(const PGFHeader& header, BYTE flags /*=0*/, const UINT8* userData /*= 0*/, UINT32 userDataLength /*= 0*/) {
	ASSERT(!m_decoder);	// current image must be closed
	ASSERT(header.quality <= MaxQuality);
	ASSERT(userDataLength <= MaxUserDataSize);
	
	// init state
#ifdef __PGFROISUPPORT__
	m_streamReinitialized = false;
#endif

	// init preHeader
	memcpy(m_preHeader.magic, PGFMagic, 3);
	m_preHeader.version = PGFVersion | flags;
	m_preHeader.hSize = HeaderSize;

	// copy header
	memcpy(&m_header, &header, HeaderSize);

	// check quality
	if (m_header.quality > MaxQuality) m_header.quality = MaxQuality;

	// complete header
	CompleteHeader();

	// check and set number of levels
	ComputeLevels();

	// check for downsample
	if (m_header.quality > DownsampleThreshold &&  (m_header.mode == ImageModeRGBColor || 
													m_header.mode == ImageModeRGBA || 
													m_header.mode == ImageModeRGB48 || 
													m_header.mode == ImageModeCMYKColor || 
													m_header.mode == ImageModeCMYK64 || 
													m_header.mode == ImageModeLabColor || 
													m_header.mode == ImageModeLab48)) {
		m_downsample = true;
		m_quant = m_header.quality - 1;
	} else {
		m_downsample = false;
		m_quant = m_header.quality;
	}

	// update header size and copy user data
	if (m_header.mode == ImageModeIndexedColor) {
		// update header size
		m_preHeader.hSize += ColorTableSize;
	}
	if (userDataLength && userData) {
		if (userDataLength > MaxUserDataSize) userDataLength = MaxUserDataSize;
		m_postHeader.userData = new(std::nothrow) UINT8[userDataLength];
		if (!m_postHeader.userData) ReturnWithError(InsufficientMemory);
		m_postHeader.userDataLen = m_postHeader.cachedUserDataLen = userDataLength;
		memcpy(m_postHeader.userData, userData, userDataLength);
		// update header size
		m_preHeader.hSize += userDataLength;
	}

	// allocate channels
	for (int i=0; i < m_header.channels; i++) {
		// set current width and height
		m_width[i] = m_header.width;
		m_height[i] = m_header.height;

		// allocate channels
		ASSERT(!m_channel[i]);
		m_channel[i] = new(std::nothrow) DataT[m_header.width*m_header.height];
		if (!m_channel[i]) {
			if (i) i--;
			while(i) {
				delete[] m_channel[i]; m_channel[i] = 0;
				i--;
			}
			ReturnWithError(InsufficientMemory);
		}
	}
}

//////////////////////////////////////////////////////////////////
/// Create wavelet transform channels and encoder. Write header at current stream position.
/// Performs forward FWT.
/// Call this method before your first call of Write(int level) or WriteImage(), but after SetHeader().
/// This method is called inside of Write(stream, ...).
/// It might throw an IOException.
/// @param stream A PGF stream
/// @return The number of bytes written into stream.
UINT32 CPGFImage::WriteHeader(CPGFStream* stream) {
	ASSERT(m_header.nLevels <= MaxLevel);
	ASSERT(m_header.quality <= MaxQuality); // quality is already initialized

	if (m_header.nLevels > 0) {
		volatile OSError error = NoError; // volatile prevents optimizations
		// create new wt channels
#ifdef LIBPGF_USE_OPENMP
		#pragma omp parallel for default(shared)
#endif
		for (int i=0; i < m_header.channels; i++) {
			DataT *temp = nullptr;
			if (error == NoError) {
				if (m_wtChannel[i]) {
					ASSERT(m_channel[i]);
					// copy m_channel to temp
					int size = m_height[i]*m_width[i];
					temp = new(std::nothrow) DataT[size];
					if (temp) {
						memcpy(temp, m_channel[i], size*DataTSize);
						delete m_wtChannel[i];	// also deletes m_channel
						m_channel[i] = nullptr;
					} else {
						error = InsufficientMemory;
					}
				}
				if (error == NoError) {
					if (temp) {
						ASSERT(!m_channel[i]);
						m_channel[i] = temp;
					}
					m_wtChannel[i] = new CWaveletTransform(m_width[i], m_height[i], m_header.nLevels, m_channel[i]);
					if (m_wtChannel[i]) {
					#ifdef __PGFROISUPPORT__
						m_wtChannel[i]->SetROI(PGFRect(0, 0, m_width[i], m_height[i]));
					#endif
					
						// wavelet subband decomposition 
						for (int l=0; error == NoError && l < m_header.nLevels; l++) {
							OSError err = m_wtChannel[i]->ForwardTransform(l, m_quant);
							if (err != NoError) error = err;
						}
					} else {
						delete[] m_channel[i];
						error = InsufficientMemory;
					}
				}
			}
		}
		if (error != NoError) {
			// free already allocated memory
			for (int i=0; i < m_header.channels; i++) {
				delete m_wtChannel[i];
			}
			ReturnWithError(error);
		}

		m_currentLevel = m_header.nLevels;

		// create encoder, write headers and user data, but not level-length area
		m_encoder = new CEncoder(stream, m_preHeader, m_header, m_postHeader, m_userDataPos, m_useOMPinEncoder);
		if (m_favorSpeedOverSize) m_encoder->FavorSpeedOverSize();

	#ifdef __PGFROISUPPORT__
		if (ROIisSupported()) {
			// new encoding scheme supporting ROI
			m_encoder->SetROI();
		}
	#endif

	} else {
		// very small image: we don't use DWT and encoding

		// create encoder, write headers and user data, but not level-length area
		m_encoder = new CEncoder(stream, m_preHeader, m_header, m_postHeader, m_userDataPos, m_useOMPinEncoder);
	}

	INT64 nBytes = m_encoder->ComputeHeaderLength();
	return (nBytes > 0) ? (UINT32)nBytes : 0;
}

//////////////////////////////////////////////////////////////////
// Encode and write next level of a PGF image at current stream position.
// A PGF image is structered in levels, numbered between 0 and Levels() - 1.
// Each level can be seen as a single image, containing the same content
// as all other levels, but in a different size (width, height).
// The image size at level i is double the size (width, height) of the image at level i+1.
// The image at level 0 contains the original size.
// It might throw an IOException.
void CPGFImage::WriteLevel() {
	ASSERT(m_encoder);
	ASSERT(m_currentLevel > 0);
	ASSERT(m_header.nLevels > 0);

#ifdef __PGFROISUPPORT__
	if (ROIisSupported()) {
		const int lastChannel = m_header.channels - 1;

		for (int i=0; i < m_header.channels; i++) {
			// get number of tiles and tile indices
			const UINT32 nTiles = m_wtChannel[i]->GetNofTiles(m_currentLevel);
			const UINT32 lastTile = nTiles - 1;

			if (m_currentLevel == m_header.nLevels) {
				// last level also has LL band
				ASSERT(nTiles == 1);
				m_wtChannel[i]->GetSubband(m_currentLevel, LL)->ExtractTile(*m_encoder);
				m_encoder->EncodeTileBuffer(); // encode macro block with tile-end = true
			}
			for (UINT32 tileY=0; tileY < nTiles; tileY++) {
				for (UINT32 tileX=0; tileX < nTiles; tileX++) {
					// extract tile to macro block and encode already filled macro blocks with tile-end = false
					m_wtChannel[i]->GetSubband(m_currentLevel, HL)->ExtractTile(*m_encoder, true, tileX, tileY);
					m_wtChannel[i]->GetSubband(m_currentLevel, LH)->ExtractTile(*m_encoder, true, tileX, tileY);
					m_wtChannel[i]->GetSubband(m_currentLevel, HH)->ExtractTile(*m_encoder, true, tileX, tileY);
					if (i == lastChannel && tileY == lastTile && tileX == lastTile) {
						// all necessary data are buffered. next call of EncodeTileBuffer will write the last piece of data of the current level.
						m_encoder->SetEncodedLevel(--m_currentLevel);
					}
					m_encoder->EncodeTileBuffer(); // encode last macro block with tile-end = true
				}
			}
		}
	} else 
#endif
	{
		for (int i=0; i < m_header.channels; i++) {
			ASSERT(m_wtChannel[i]);
			if (m_currentLevel == m_header.nLevels) { 
				// last level also has LL band
				m_wtChannel[i]->GetSubband(m_currentLevel, LL)->ExtractTile(*m_encoder);
			}
			//encoder.EncodeInterleaved(m_wtChannel[i], m_currentLevel, m_quant); // until version 4
			m_wtChannel[i]->GetSubband(m_currentLevel, HL)->ExtractTile(*m_encoder); // since version 5
			m_wtChannel[i]->GetSubband(m_currentLevel, LH)->ExtractTile(*m_encoder); // since version 5
			m_wtChannel[i]->GetSubband(m_currentLevel, HH)->ExtractTile(*m_encoder);
		}

		// all necessary data are buffered. next call of EncodeBuffer will write the last piece of data of the current level.
		m_encoder->SetEncodedLevel(--m_currentLevel);
	}
}

//////////////////////////////////////////////////////////////////////
// Return written levelLength bytes
UINT32 CPGFImage::UpdatePostHeaderSize() {
	ASSERT(m_encoder);

	INT64 offset = m_encoder->ComputeOffset(); ASSERT(offset >= 0);

	if (offset > 0) {
		// update post-header size and rewrite pre-header
		m_preHeader.hSize += (UINT32)offset;
		m_encoder->UpdatePostHeaderSize(m_preHeader);
	}

	// write dummy levelLength into stream
	return m_encoder->WriteLevelLength(m_levelLength);
}

//////////////////////////////////////////////////////////////////////
/// Encode and write an image at current stream position.
/// Call this method after WriteHeader(). 
/// In case you want to write uncached metadata, 
/// then do that after WriteHeader() and before WriteImage(). 
/// This method is called inside of Write(stream, ...).
/// It might throw an IOException.
/// @param stream A PGF stream
/// @param cb A pointer to a callback procedure. The procedure is called after writing a single level. If cb returns true, then it stops proceeding.
/// @param data Data Pointer to C++ class container to host callback procedure.
/// @return The number of bytes written into stream.
UINT32 CPGFImage::WriteImage(CPGFStream* stream, CallbackPtr cb /*= nullptr*/, void *data /*= nullptr*/) {
	ASSERT(stream);
	ASSERT(m_preHeader.hSize);

	int levels = m_header.nLevels;
	double percent = pow(0.25, levels);

	// update post-header size, rewrite pre-header, and write dummy levelLength
	UINT32 nWrittenBytes = UpdatePostHeaderSize();

	if (levels == 0) {
		// for very small images: write channels uncoded
		for (int c=0; c < m_header.channels; c++) {
			const UINT32 size = m_width[c]*m_height[c];

			// write channel data into stream
			for (UINT32 i=0; i < size; i++) {
				int count = DataTSize;
				stream->Write(&count, &m_channel[c][i]);
			}
		}

		// now update progress
		if (cb) {
			if ((*cb)(1, true, data)) ReturnWithError(EscapePressed);
		}

	} else {
		// encode quantized wavelet coefficients and write to PGF file
		// encode subbands, higher levels first
		// color channels are interleaved

		// encode all levels
		for (m_currentLevel = levels; m_currentLevel > 0; ) {
			WriteLevel(); // decrements m_currentLevel

			// now update progress
			if (cb) {
				percent *= 4;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}

		// flush encoder and write level lengths
		m_encoder->Flush();
	}

	// update level lengths
	nWrittenBytes += m_encoder->UpdateLevelLength(); // return written image bytes 

	// delete encoder
	delete m_encoder; m_encoder = nullptr;

	ASSERT(!m_encoder);

	return nWrittenBytes;
}

//////////////////////////////////////////////////////////////////
/// Encode and write an entire PGF image (header and image) at current stream position.
/// A PGF image is structered in levels, numbered between 0 and Levels() - 1.
/// Each level can be seen as a single image, containing the same content
/// as all other levels, but in a different size (width, height).
/// The image size at level i is double the size (width, height) of the image at level i+1.
/// The image at level 0 contains the original size.
/// Precondition: the PGF image contains a valid header (see also SetHeader(...)). 
/// It might throw an IOException.
/// @param stream A PGF stream
/// @param nWrittenBytes [in-out] The number of bytes written into stream are added to the input value.
/// @param cb A pointer to a callback procedure. The procedure is called after writing a single level. If cb returns true, then it stops proceeding.
/// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::Write(CPGFStream* stream, UINT32* nWrittenBytes /*= nullptr*/, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) {
	ASSERT(stream);
	ASSERT(m_preHeader.hSize);

	// create wavelet transform channels and encoder
	UINT32 nBytes = WriteHeader(stream);

	// write image
	nBytes += WriteImage(stream, cb, data);

	// return written bytes
	if (nWrittenBytes) *nWrittenBytes += nBytes;
}

#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////
// Encode and write down to given level at current stream position.
// A PGF image is structered in levels, numbered between 0 and Levels() - 1.
// Each level can be seen as a single image, containing the same content
// as all other levels, but in a different size (width, height).
// The image size at level i is double the size (width, height) of the image at level i+1.
// The image at level 0 contains the original size.
// Precondition: the PGF image contains a valid header (see also SetHeader(...)) and WriteHeader() has been called before.
// The ROI encoding scheme is used.
// It might throw an IOException.
// @param level The image level of the resulting image in the internal image buffer.
// @param cb A pointer to a callback procedure. The procedure is called after writing a single level. If cb returns true, then it stops proceeding.
// @param data Data Pointer to C++ class container to host callback procedure.
// @return The number of bytes written into stream.
UINT32 CPGFImage::Write(int level, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) {
	ASSERT(m_header.nLevels > 0);
	ASSERT(0 <= level && level < m_header.nLevels);
	ASSERT(m_encoder);
	ASSERT(ROIisSupported());

	const int levelDiff = m_currentLevel - level;
	double percent = (m_progressMode == PM_Relative) ? pow(0.25, levelDiff) : m_percent;
	UINT32 nWrittenBytes = 0;

	if (m_currentLevel == m_header.nLevels) {
		// update post-header size, rewrite pre-header, and write dummy levelLength
		nWrittenBytes = UpdatePostHeaderSize();
	} else {
		// prepare for next level: save current file position, because the stream might have been reinitialized
		if (m_encoder->ComputeBufferLength()) {
			m_streamReinitialized = true;
		}
	}

	// encoding scheme with ROI
	while (m_currentLevel > level) {
		WriteLevel();	// decrements m_currentLevel

		if (m_levelLength) {
			nWrittenBytes += m_levelLength[m_header.nLevels - m_currentLevel - 1];
		}

		// now update progress
		if (cb) {
			percent *= 4;
			if (m_progressMode == PM_Absolute) m_percent = percent;
			if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
		}
	}

	// automatically closing
	if (m_currentLevel == 0) {
		if (!m_streamReinitialized) {
			// don't write level lengths, if the stream position changed inbetween two Write operations
			m_encoder->UpdateLevelLength();
		}
		// delete encoder
		delete m_encoder; m_encoder = nullptr;
	}

	return nWrittenBytes;
}
#endif // __PGFROISUPPORT__


//////////////////////////////////////////////////////////////////
// Check for valid import image mode.
// @param mode Image mode
// @return True if an image of given mode can be imported with ImportBitmap(...)
bool CPGFImage::ImportIsSupported(BYTE mode) {
	size_t size = DataTSize;

	if (size >= 2) {
		switch(mode) {
			case ImageModeBitmap:
			case ImageModeIndexedColor:
			case ImageModeGrayScale:
			case ImageModeRGBColor:
			case ImageModeCMYKColor:
			case ImageModeHSLColor:
			case ImageModeHSBColor:
			//case ImageModeDuotone:
			case ImageModeLabColor:
			case ImageModeRGB12:
			case ImageModeRGB16:
			case ImageModeRGBA:
				return true;
		}
	}
	if (size >= 3) {
		switch(mode) {
			case ImageModeGray16:
			case ImageModeRGB48:
			case ImageModeLab48:
			case ImageModeCMYK64:
			//case ImageModeDuotone16:
				return true;
		}
	}
	if (size >=4) {
		switch(mode) {
			case ImageModeGray32:
				return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
/// Retrieves red, green, blue (RGB) color values from a range of entries in the palette of the DIB section.
/// It might throw an IOException.
/// @param iFirstColor The color table index of the first entry to retrieve.
/// @param nColors The number of color table entries to retrieve.
/// @param prgbColors A pointer to the array of RGBQUAD structures to retrieve the color table entries.
void CPGFImage::GetColorTable(UINT32 iFirstColor, UINT32 nColors, RGBQUAD* prgbColors) const {
	if (iFirstColor + nColors > ColorTableLen)	ReturnWithError(ColorTableError);

	for (UINT32 i=iFirstColor, j=0; j < nColors; i++, j++) {
		prgbColors[j] = m_postHeader.clut[i];
	}
}

//////////////////////////////////////////////////////////////////////
/// Sets the red, green, blue (RGB) color values for a range of entries in the palette (clut).
/// It might throw an IOException.
/// @param iFirstColor The color table index of the first entry to set.
/// @param nColors The number of color table entries to set.
/// @param prgbColors A pointer to the array of RGBQUAD structures to set the color table entries.
void CPGFImage::SetColorTable(UINT32 iFirstColor, UINT32 nColors, const RGBQUAD* prgbColors) {
	if (iFirstColor + nColors > ColorTableLen)	ReturnWithError(ColorTableError);

	for (UINT32 i=iFirstColor, j=0; j < nColors; i++, j++) {
		m_postHeader.clut[i] = prgbColors[j];
	}
}

//////////////////////////////////////////////////////////////////
// Buffer transform from interleaved to channel seperated format
// the absolute value of pitch is the number of bytes of an image row
// if pitch is negative, then buff points to the last row of a bottom-up image (first byte on last row)
// if pitch is positive, then buff points to the first row of a top-down image (first byte)
// bpp is the number of bits per pixel used in image buffer buff
//
// RGB is transformed into YUV format (ordering of buffer data is BGR[A])
// Y = (R + 2*G + B)/4 -128
// U = R - G
// V = B - G
//
// Since PGF Codec version 2.0 images are stored in top-down direction
//
// The sequence of input channels in the input image buffer does not need to be the same as expected from PGF. In case of different sequences you have to
// provide a channelMap of size of expected channels (depending on image mode). For example, PGF expects in RGB color mode a channel sequence BGR.
// If your provided image buffer contains a channel sequence ARGB, then the channelMap looks like { 3, 2, 1 }.
void CPGFImage::RgbToYuv(int pitch, UINT8* buff, BYTE bpp, int channelMap[], CallbackPtr cb, void *data /*=nullptr*/) {
	ASSERT(buff);
	UINT32 yPos = 0, cnt = 0;
	double percent = 0;
	const double dP = 1.0/m_header.height;
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);

	if (channelMap == nullptr) channelMap = defMap;

	switch(m_header.mode) {
	case ImageModeBitmap:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 1);
			ASSERT(bpp == 1);
			
			const UINT32 w = m_header.width;
			const UINT32 w2 = (m_header.width + 7)/8;
			DataT* y = m_channel[0]; ASSERT(y);

			// new unpacked version since version 7
			for (UINT32 h = 0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}
				cnt = 0;
				for (UINT32 j = 0; j < w2; j++) {
					UINT8 byte = buff[j];
					for (int k = 0; k < 8; k++) {
						UINT8 bit = (byte & 0x80) >> 7;
						if (cnt < w) y[yPos++] = bit;
						byte <<= 1;
						cnt++;
					}
				}
				buff += pitch;
			}
			/* old version: packed values: 8 pixels in 1 byte
			for (UINT32 h = 0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				for (UINT32 j = 0; j < w2; j++) {
					y[yPos++] = buff[j] - YUVoffset8;
				}
				// version 5 and 6
				// for (UINT32 j = w2; j < w; j++) {
				//	y[yPos++] = YUVoffset8;
				//}
				buff += pitch;
			}
			*/
		}
		break;
	case ImageModeIndexedColor:
	case ImageModeGrayScale:
	case ImageModeHSLColor:
	case ImageModeHSBColor:
	case ImageModeLabColor:
		{
			ASSERT(m_header.channels >= 1);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);
			const int channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					for (int c=0; c < m_header.channels; c++) {
						m_channel[c][yPos] = buff[cnt + channelMap[c]] - YUVoffset8;
					}
					cnt += channels;
					yPos++;
				}
				buff += pitch;	
			}
		}
		break;
	case ImageModeGray16:
	case ImageModeLab48:
		{
			ASSERT(m_header.channels >= 1);
			ASSERT(m_header.bpp == m_header.channels*16);
			ASSERT(bpp%16 == 0);

			UINT16 *buff16 = (UINT16 *)buff;
			const int pitch16 = pitch/2;
			const int channels = bpp/16; ASSERT(channels >= m_header.channels);
			const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					for (int c=0; c < m_header.channels; c++) {
						m_channel[c][yPos] = (buff16[cnt + channelMap[c]] >> shift) - yuvOffset16;
					}
					cnt += channels;
					yPos++;
				}
				buff16 += pitch16;
			}
		}
		break;
	case ImageModeRGBColor:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			const int channels = bpp/8; ASSERT(channels >= m_header.channels);
			UINT8 b, g, r;

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					b = buff[cnt + channelMap[0]];
					g = buff[cnt + channelMap[1]];
					r = buff[cnt + channelMap[2]];
					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - YUVoffset8;
					u[yPos] = r - g;
					v[yPos] = b - g;
					yPos++;
					cnt += channels;
				}
				buff += pitch;
			}	
		}
		break;
	case ImageModeRGB48:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*16);
			ASSERT(bpp%16 == 0);

			UINT16 *buff16 = (UINT16 *)buff;
			const int pitch16 = pitch/2;
			const int channels = bpp/16; ASSERT(channels >= m_header.channels);
			const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			UINT16 b, g, r;

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					b = buff16[cnt + channelMap[0]] >> shift;
					g = buff16[cnt + channelMap[1]] >> shift;
					r = buff16[cnt + channelMap[2]] >> shift;
					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - yuvOffset16;
					u[yPos] = r - g;
					v[yPos] = b - g;
					yPos++;
					cnt += channels;
				}
				buff16 += pitch16;
			}	
		}
		break;
	case ImageModeRGBA:
	case ImageModeCMYKColor:
		{
			ASSERT(m_header.channels == 4);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);
			const int channels = bpp/8; ASSERT(channels >= m_header.channels);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			DataT* a = m_channel[3]; ASSERT(a);
			UINT8 b, g, r;

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					b = buff[cnt + channelMap[0]];
					g = buff[cnt + channelMap[1]];
					r = buff[cnt + channelMap[2]];
					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - YUVoffset8;
					u[yPos] = r - g;
					v[yPos] = b - g;
					a[yPos++] = buff[cnt + channelMap[3]] - YUVoffset8;
					cnt += channels;
				}
				buff += pitch;
			}	
		}
		break;
	case ImageModeCMYK64:
		{
			ASSERT(m_header.channels == 4);
			ASSERT(m_header.bpp == m_header.channels*16);
			ASSERT(bpp%16 == 0);

			UINT16 *buff16 = (UINT16 *)buff;
			const int pitch16 = pitch/2;
			const int channels = bpp/16; ASSERT(channels >= m_header.channels);
			const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			
			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			DataT* a = m_channel[3]; ASSERT(a);
			UINT16 b, g, r;

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					b = buff16[cnt + channelMap[0]] >> shift;
					g = buff16[cnt + channelMap[1]] >> shift;
					r = buff16[cnt + channelMap[2]] >> shift;
					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - yuvOffset16;
					u[yPos] = r - g;
					v[yPos] = b - g;
					a[yPos++] = (buff16[cnt + channelMap[3]] >> shift) - yuvOffset16;
					cnt += channels;
				}
				buff16 += pitch16;
			}	
		}
		break;
#ifdef __PGF32SUPPORT__
	case ImageModeGray32:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 32);
			ASSERT(bpp == 32);
			ASSERT(DataTSize == sizeof(UINT32));

			DataT* y = m_channel[0]; ASSERT(y);

			UINT32 *buff32 = (UINT32 *)buff;
			const int pitch32 = pitch/4;
			const int shift = 31 - UsedBitsPerChannel(); ASSERT(shift >= 0);
			const DataT yuvOffset31 = 1 << (UsedBitsPerChannel() - 1);

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				for (UINT32 w=0; w < m_header.width; w++) {
					y[yPos++] = (buff32[w] >> shift) - yuvOffset31;
				}
				buff32 += pitch32;
			}
		}
		break;
#endif
	case ImageModeRGB12:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*4);
			ASSERT(bpp == m_header.channels*4);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);

			UINT8 rgb = 0, b, g, r;

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					if (w%2 == 0) {
						// even pixel position
						rgb = buff[cnt];
						b = rgb & 0x0F;
						g = (rgb & 0xF0) >> 4;
						cnt++;
						rgb = buff[cnt];
						r = rgb & 0x0F;
					} else {
						// odd pixel position
						b = (rgb & 0xF0) >> 4;
						cnt++;
						rgb = buff[cnt];
						g = rgb & 0x0F;
						r = (rgb & 0xF0) >> 4;
						cnt++;
					}

					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - YUVoffset4;
					u[yPos] = r - g;
					v[yPos] = b - g;
					yPos++;
				}
				buff += pitch;
			}	
		}
		break;
	case ImageModeRGB16:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == 16);
			ASSERT(bpp == 16);
			
			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);

			UINT16 *buff16 = (UINT16 *)buff;
			UINT16 rgb, b, g, r;
			const int pitch16 = pitch/2;

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}
				for (UINT32 w=0; w < m_header.width; w++) {
					rgb = buff16[w]; 
					r = (rgb & 0xF800) >> 10;	// highest 5 bits
					g = (rgb & 0x07E0) >> 5;	// middle 6 bits
					b = (rgb & 0x001F) << 1;	// lowest 5 bits
					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - YUVoffset6;
					u[yPos] = r - g;
					v[yPos] = b - g;
					yPos++;
				}

				buff16 += pitch16;
			}	
		}
		break;
	default:
		ASSERT(false);
	}
}

//////////////////////////////////////////////////////////////////
// Get image data in interleaved format: (ordering of RGB data is BGR[A])
// Upsampling, YUV to RGB transform and interleaving are done here to reduce the number 
// of passes over the data.
// The absolute value of pitch is the number of bytes of an image row of the given image buffer.
// If pitch is negative, then the image buffer must point to the last row of a bottom-up image (first byte on last row).
// if pitch is positive, then the image buffer must point to the first row of a top-down image (first byte).
// The sequence of output channels in the output image buffer does not need to be the same as provided by PGF. In case of different sequences you have to
// provide a channelMap of size of expected channels (depending on image mode). For example, PGF provides a channel sequence BGR in RGB color mode.
// If your provided image buffer expects a channel sequence ARGB, then the channelMap looks like { 3, 2, 1 }.
// It might throw an IOException.
// @param pitch The number of bytes of a row of the image buffer.
// @param buff An image buffer.
// @param bpp The number of bits per pixel used in image buffer.
// @param channelMap A integer array containing the mapping of PGF channel ordering to expected channel ordering.
// @param cb A pointer to a callback procedure. The procedure is called after each copied buffer row. If cb returns true, then it stops proceeding.
// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::GetBitmap(int pitch, UINT8* buff, BYTE bpp, int channelMap[] /*= nullptr */, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) const {
	ASSERT(buff);
	UINT32 w = m_width[0];  // width of decoded image
	UINT32 h = m_height[0]; // height of decoded image
	UINT32 yw = w;			// y-channel width
	UINT32 uw = m_width[1];	// u-channel width
	UINT32 roiOffsetX = 0;
	UINT32 roiOffsetY = 0;
	UINT32 yOffset = 0;
	UINT32 uOffset = 0;

#ifdef __PGFROISUPPORT__
	const PGFRect& roi = GetAlignedROI(); // in pixels, roi is usually larger than levelRoi
	ASSERT(w == roi.Width() && h == roi.Height());
	const PGFRect levelRoi = ComputeLevelROI();
	ASSERT(roi.left <= levelRoi.left && levelRoi.right <= roi.right); 
	ASSERT(roi.top <= levelRoi.top && levelRoi.bottom <= roi.bottom); 

	if (ROIisSupported() && (levelRoi.Width() < w || levelRoi.Height() < h)) {
		// ROI is used 
		w = levelRoi.Width();
		h = levelRoi.Height();
		roiOffsetX = levelRoi.left - roi.left;
		roiOffsetY = levelRoi.top - roi.top;
		yOffset = roiOffsetX + roiOffsetY*yw;

		if (m_downsample) {
			const PGFRect& downsampledRoi = GetAlignedROI(1);
			uOffset = levelRoi.left/2 - downsampledRoi.left + (levelRoi.top/2 - downsampledRoi.top)*m_width[1];
		} else {
			uOffset = yOffset;
		}
	}
#endif

	const double dP = 1.0/h;
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);
	if (channelMap == nullptr) channelMap = defMap;
	DataT uAvg, vAvg;
	double percent = 0;
	UINT32 i, j;

	switch(m_header.mode) {
	case ImageModeBitmap:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 1);
			ASSERT(bpp == 1);

			const UINT32 w2 = (w + 7)/8;
			DataT* y = m_channel[0]; ASSERT(y);

			if (m_preHeader.version & Version7) {
				// new unpacked version has a little better compression ratio
				// since version 7
				for (i = 0; i < h; i++) {
					UINT32 cnt = 0;
					for (j = 0; j < w2; j++) {
						UINT8 byte = 0;
						for (int k = 0; k < 8; k++) {
							byte <<= 1;
							UINT8 bit = 0;
							if (cnt < w) {
								bit = y[yOffset + cnt] & 1;
							}
							byte |= bit;
							cnt++;
						}
						buff[j] = byte;
					}
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				// old versions
				// packed pixels: 8 pixel in 1 byte of channel[0]
				if (!(m_preHeader.version & Version5)) yw = w2; // not version 5 or 6
				yOffset = roiOffsetX/8 + roiOffsetY*yw; // 1 byte in y contains 8 pixel values
				for (i = 0; i < h; i++) {
					for (j = 0; j < w2; j++) {
						buff[j] = Clamp8(y[yOffset + j] + YUVoffset8);
					}
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
	case ImageModeIndexedColor:
	case ImageModeGrayScale:
	case ImageModeHSLColor:
	case ImageModeHSBColor:
		{
			ASSERT(m_header.channels >= 1);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);

			UINT32 cnt, channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (i=0; i < h; i++) {
				UINT32 yPos = yOffset;
				cnt = 0;
				for (j=0; j < w; j++) {
					for (UINT32 c=0; c < m_header.channels; c++) {
						buff[cnt + channelMap[c]] = Clamp8(m_channel[c][yPos] + YUVoffset8);
					}
					cnt += channels;
					yPos++;
				}
				yOffset += yw;
				buff += pitch;

				if (cb) {
					percent += dP;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
			break;
		}
	case ImageModeGray16:
		{
			ASSERT(m_header.channels >= 1);
			ASSERT(m_header.bpp == m_header.channels*16);

			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			UINT32 cnt, channels;

			if (bpp%16 == 0) {
				const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						for (UINT32 c=0; c < m_header.channels; c++) {
							buff16[cnt + channelMap[c]] = Clamp16((m_channel[c][yPos] + yuvOffset16) << shift);
						}
						cnt += channels;
						yPos++;
					}
					yOffset += yw;
					buff16 += pitch16;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				const int shift = __max(0, UsedBitsPerChannel() - 8);
				channels = bpp/8; ASSERT(channels >= m_header.channels);
				
				for (i=0; i < h; i++) {
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						for (UINT32 c=0; c < m_header.channels; c++) {
							buff[cnt + channelMap[c]] = Clamp8((m_channel[c][yPos] + yuvOffset16) >> shift);
						}
						cnt += channels;
						yPos++;
					}
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
	case ImageModeRGBColor:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);
			ASSERT(bpp >= m_header.bpp);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			UINT8 *buffg = &buff[channelMap[1]],
				  *buffr = &buff[channelMap[2]],
				  *buffb = &buff[channelMap[0]];
			UINT8 g;
			UINT32 cnt, channels = bpp/8;

			if (m_downsample) {
				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						// u and v are downsampled
						uAvg = u[uPos];
						vAvg = v[uPos];
						// Yuv
						buffg[cnt] = g = Clamp8(y[yPos] + YUVoffset8 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buffr[cnt] = Clamp8(uAvg + g);
						buffb[cnt] = Clamp8(vAvg + g);
						cnt += channels;
						if (j & 1) uPos++;
						yPos++;
					}
					if (i & 1) uOffset += uw;
					yOffset += yw;
					buffb += pitch;
					buffg += pitch;
					buffr += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}

			} else {
				for (i=0; i < h; i++) {
					cnt = 0;
					UINT32 yPos = yOffset;
					for (j = 0; j < w; j++) {
						uAvg = u[yPos];
						vAvg = v[yPos];
						// Yuv
						buffg[cnt] = g = Clamp8(y[yPos] + YUVoffset8 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buffr[cnt] = Clamp8(uAvg + g);
						buffb[cnt] = Clamp8(vAvg + g);
						cnt += channels;
						yPos++;
					}
					yOffset += yw;
					buffb += pitch;
					buffg += pitch;
					buffr += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
	case ImageModeRGB48:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == 48);

			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			UINT32 cnt, channels;
			DataT g;

			if (bpp >= 48 && bpp%16 == 0) {
				const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						uAvg = u[uPos];
						vAvg = v[uPos];
						// Yuv
						g = y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2); // must be logical shift operator
						buff16[cnt + channelMap[1]] = Clamp16(g << shift);
						buff16[cnt + channelMap[2]] = Clamp16((uAvg + g) << shift);
						buff16[cnt + channelMap[0]] = Clamp16((vAvg + g) << shift);
						cnt += channels;
						if (!m_downsample || (j & 1)) uPos++;
						yPos++;
					}
					if (!m_downsample || (i & 1)) uOffset += uw;
					yOffset += yw;
					buff16 += pitch16;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				const int shift = __max(0, UsedBitsPerChannel() - 8);
				channels = bpp/8; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						uAvg = u[uPos];
						vAvg = v[uPos];
						// Yuv
						g = y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2); // must be logical shift operator
						buff[cnt + channelMap[1]] = Clamp8(g >> shift); 
						buff[cnt + channelMap[2]] = Clamp8((uAvg + g) >> shift);
						buff[cnt + channelMap[0]] = Clamp8((vAvg + g) >> shift);
						cnt += channels;
						if (!m_downsample || (j & 1)) uPos++;
						yPos++;
					}
					if (!m_downsample || (i & 1)) uOffset += uw;
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
	case ImageModeLabColor:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);

			DataT* l = m_channel[0]; ASSERT(l);
			DataT* a = m_channel[1]; ASSERT(a);
			DataT* b = m_channel[2]; ASSERT(b);
			UINT32 cnt, channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (i=0; i < h; i++) {
				UINT32 uPos = uOffset;
				UINT32 yPos = yOffset;
				cnt = 0;
				for (j=0; j < w; j++) {
					uAvg = a[uPos];
					vAvg = b[uPos];
					buff[cnt + channelMap[0]] = Clamp8(l[yPos] + YUVoffset8);
					buff[cnt + channelMap[1]] = Clamp8(uAvg + YUVoffset8); 
					buff[cnt + channelMap[2]] = Clamp8(vAvg + YUVoffset8);
					cnt += channels;
					if (!m_downsample || (j & 1)) uPos++;
					yPos++;
				}
				if (!m_downsample || (i & 1)) uOffset += uw;
				yOffset += yw;
				buff += pitch;

				if (cb) {
					percent += dP;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
			break;
		}
	case ImageModeLab48:
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*16);

			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

			DataT* l = m_channel[0]; ASSERT(l);
			DataT* a = m_channel[1]; ASSERT(a);
			DataT* b = m_channel[2]; ASSERT(b);
			UINT32 cnt, channels;

			if (bpp%16 == 0) {
				const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						uAvg = a[uPos];
						vAvg = b[uPos];
						buff16[cnt + channelMap[0]] = Clamp16((l[yPos] + yuvOffset16) << shift);
						buff16[cnt + channelMap[1]] = Clamp16((uAvg + yuvOffset16) << shift);
						buff16[cnt + channelMap[2]] = Clamp16((vAvg + yuvOffset16) << shift);
						cnt += channels;
						if (!m_downsample || (j & 1)) uPos++;
						yPos++;
					}
					if (!m_downsample || (i & 1)) uOffset += uw;
					yOffset += yw;
					buff16 += pitch16;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				const int shift = __max(0, UsedBitsPerChannel() - 8);
				channels = bpp/8; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						uAvg = a[uPos];
						vAvg = b[uPos];
						buff[cnt + channelMap[0]] = Clamp8((l[yPos] + yuvOffset16) >> shift);
						buff[cnt + channelMap[1]] = Clamp8((uAvg + yuvOffset16) >> shift);
						buff[cnt + channelMap[2]] = Clamp8((vAvg + yuvOffset16) >> shift);
						cnt += channels;
						if (!m_downsample || (j & 1)) uPos++;
						yPos++;
					}
					if (!m_downsample || (i & 1)) uOffset += uw;
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
	case ImageModeRGBA:
	case ImageModeCMYKColor:
		{
			ASSERT(m_header.channels == 4);
			ASSERT(m_header.bpp == m_header.channels*8);
			ASSERT(bpp%8 == 0);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			DataT* a = m_channel[3]; ASSERT(a);
			UINT8 g, aAvg;
			UINT32 cnt, channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (i=0; i < h; i++) {
				UINT32 uPos = uOffset;
				UINT32 yPos = yOffset;
				cnt = 0;
				for (j=0; j < w; j++) {
					uAvg = u[uPos];
					vAvg = v[uPos];
					aAvg = Clamp8(a[uPos] + YUVoffset8);
					// Yuv
					buff[cnt + channelMap[1]] = g = Clamp8(y[yPos] + YUVoffset8 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
					buff[cnt + channelMap[2]] = Clamp8(uAvg + g);
					buff[cnt + channelMap[0]] = Clamp8(vAvg + g);
					buff[cnt + channelMap[3]] = aAvg;
					cnt += channels;
					if (!m_downsample || (j & 1)) uPos++;
					yPos++;
				}
				if (!m_downsample || (i & 1)) uOffset += uw;
				yOffset += yw;
				buff += pitch;

				if (cb) {
					percent += dP;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
			break;
		}
	case ImageModeCMYK64: 
		{
			ASSERT(m_header.channels == 4);
			ASSERT(m_header.bpp == 64);

			const DataT yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			DataT* a = m_channel[3]; ASSERT(a);
			DataT g, aAvg;
			UINT32 cnt, channels;

			if (bpp%16 == 0) {
				const int shift = 16 - UsedBitsPerChannel(); ASSERT(shift >= 0);
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						uAvg = u[uPos];
						vAvg = v[uPos];
						aAvg = a[uPos] + yuvOffset16;
						// Yuv
						g = y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2); // must be logical shift operator
						buff16[cnt + channelMap[1]] = Clamp16(g << shift);
						buff16[cnt + channelMap[2]] = Clamp16((uAvg + g) << shift);
						buff16[cnt + channelMap[0]] = Clamp16((vAvg + g) << shift);
						buff16[cnt + channelMap[3]] = Clamp16(aAvg << shift);
						cnt += channels;
						if (!m_downsample || (j & 1)) uPos++;
						yPos++;
					}
					if (!m_downsample || (i & 1)) uOffset += uw;
					yOffset += yw;
					buff16 += pitch16;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				const int shift = __max(0, UsedBitsPerChannel() - 8);
				channels = bpp/8; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					UINT32 uPos = uOffset;
					UINT32 yPos = yOffset;
					cnt = 0;
					for (j=0; j < w; j++) {
						uAvg = u[uPos];
						vAvg = v[uPos];
						aAvg = a[uPos] + yuvOffset16;
						// Yuv
						g = y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2); // must be logical shift operator
						buff[cnt + channelMap[1]] = Clamp8(g >> shift); 
						buff[cnt + channelMap[2]] = Clamp8((uAvg + g) >> shift);
						buff[cnt + channelMap[0]] = Clamp8((vAvg + g) >> shift);
						buff[cnt + channelMap[3]] = Clamp8(aAvg >> shift);
						cnt += channels;
						if (!m_downsample || (j & 1)) uPos++;
						yPos++;
					}
					if (!m_downsample || (i & 1)) uOffset += uw;
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
#ifdef __PGF32SUPPORT__
	case ImageModeGray32:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 32);

			const int yuvOffset31 = 1 << (UsedBitsPerChannel() - 1);
			DataT* y = m_channel[0]; ASSERT(y);

			if (bpp == 32) {
				const int shift = 31 - UsedBitsPerChannel(); ASSERT(shift >= 0);
				UINT32 *buff32 = (UINT32 *)buff;
				int pitch32 = pitch/4;

				for (i=0; i < h; i++) {
					UINT32 yPos = yOffset;
					for (j = 0; j < w; j++) {
						buff32[j] = Clamp31((y[yPos++] + yuvOffset31) << shift);
					}
					yOffset += yw;
					buff32 += pitch32;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else if (bpp == 16) {
				const int usedBits = UsedBitsPerChannel();
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;

				if (usedBits < 16) {
					const int shift = 16 - usedBits;
					for (i=0; i < h; i++) {
						UINT32 yPos = yOffset;
						for (j = 0; j < w; j++) {
							buff16[j] = Clamp16((y[yPos++] + yuvOffset31) << shift);
						}
						yOffset += yw;
						buff16 += pitch16;

						if (cb) {
							percent += dP;
							if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
						}
					}
				} else {
					const int shift = __max(0, usedBits - 16);
					for (i=0; i < h; i++) {
						UINT32 yPos = yOffset;
						for (j = 0; j < w; j++) {
							buff16[j] = Clamp16((y[yPos++] + yuvOffset31) >> shift);
						}
						yOffset += yw;
						buff16 += pitch16;

						if (cb) {
							percent += dP;
							if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
						}
					}
				}
			} else {
				ASSERT(bpp == 8);
				const int shift = __max(0, UsedBitsPerChannel() - 8);
				
				for (i=0; i < h; i++) {
					UINT32 yPos = yOffset;
					for (j = 0; j < w; j++) {
						buff[j] = Clamp8((y[yPos++] + yuvOffset31) >> shift);
					}
					yOffset += yw;
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;	
		}
#endif
	case ImageModeRGB12: 
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == m_header.channels*4);
			ASSERT(bpp == m_header.channels*4);
			ASSERT(!m_downsample);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			UINT16 yval;
			UINT32 cnt;

			for (i=0; i < h; i++) {
				UINT32 yPos = yOffset;
				cnt = 0;
				for (j=0; j < w; j++) {
					// Yuv
					uAvg = u[yPos];
					vAvg = v[yPos];
					yval = Clamp4(y[yPos] + YUVoffset4 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
					if (j%2 == 0) {
						buff[cnt] = UINT8(Clamp4(vAvg + yval) | (yval << 4));
						cnt++;
						buff[cnt] = Clamp4(uAvg + yval);
					} else {
						buff[cnt] |= Clamp4(vAvg + yval) << 4;
						cnt++;
						buff[cnt] = UINT8(yval | (Clamp4(uAvg + yval) << 4));
						cnt++;
					}
					yPos++;
				}
				yOffset += yw;
				buff += pitch;

				if (cb) {
					percent += dP;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
			break;
		}
	case ImageModeRGB16: 
		{
			ASSERT(m_header.channels == 3);
			ASSERT(m_header.bpp == 16);
			ASSERT(bpp == 16);
			ASSERT(!m_downsample);

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			UINT16 yval;
			UINT16 *buff16 = (UINT16 *)buff;
			int pitch16 = pitch/2;

			for (i=0; i < h; i++) {
				UINT32 yPos = yOffset;
				for (j = 0; j < w; j++) {
					// Yuv
					uAvg = u[yPos];
					vAvg = v[yPos];
					yval = Clamp6(y[yPos++] + YUVoffset6 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
					buff16[j] = (yval << 5) | ((Clamp6(uAvg + yval) >> 1) << 11) | (Clamp6(vAvg + yval) >> 1);
				}
				yOffset += yw;
				buff16 += pitch16;

				if (cb) {
					percent += dP;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
			break;
		}
	default:
		ASSERT(false);
	}

#ifdef _DEBUG
	// display ROI (RGB) in debugger
	roiimage.width = w;
	roiimage.height = h;
	if (pitch > 0) {
		roiimage.pitch = pitch;
		roiimage.data = buff;
	} else {
		roiimage.pitch = -pitch;
		roiimage.data = buff + (h - 1)*pitch;
	}
#endif

}			

//////////////////////////////////////////////////////////////////////
/// Get YUV image data in interleaved format: (ordering is YUV[A])
/// The absolute value of pitch is the number of bytes of an image row of the given image buffer.
/// If pitch is negative, then the image buffer must point to the last row of a bottom-up image (first byte on last row).
/// if pitch is positive, then the image buffer must point to the first row of a top-down image (first byte).
/// The sequence of output channels in the output image buffer does not need to be the same as provided by PGF. In case of different sequences you have to
/// provide a channelMap of size of expected channels (depending on image mode). For example, PGF provides a channel sequence BGR in RGB color mode.
/// If your provided image buffer expects a channel sequence VUY, then the channelMap looks like { 2, 1, 0 }.
/// It might throw an IOException.
/// @param pitch The number of bytes of a row of the image buffer.
/// @param buff An image buffer.
/// @param bpp The number of bits per pixel used in image buffer.
/// @param channelMap A integer array containing the mapping of PGF channel ordering to expected channel ordering.
/// @param cb A pointer to a callback procedure. The procedure is called after each copied buffer row. If cb returns true, then it stops proceeding.
void CPGFImage::GetYUV(int pitch, DataT* buff, BYTE bpp, int channelMap[] /*= nullptr*/, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) const {
	ASSERT(buff);
	const UINT32 w = m_width[0];
	const UINT32 h = m_height[0];
	const bool wOdd = (1 == w%2);
	const int dataBits = DataTSize*8; ASSERT(dataBits == 16 || dataBits == 32);
	const int pitch2 = pitch/DataTSize;
	const int yuvOffset = (dataBits == 16) ? YUVoffset8 : YUVoffset16;
	const double dP = 1.0/h;

	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);
	if (channelMap == nullptr) channelMap = defMap;
	int sampledPos = 0, yPos = 0;
	DataT uAvg, vAvg;
	double percent = 0;
	UINT32 i, j;

	if (m_header.channels == 3) { 
		ASSERT(bpp%dataBits == 0);

		DataT* y = m_channel[0]; ASSERT(y);
		DataT* u = m_channel[1]; ASSERT(u);
		DataT* v = m_channel[2]; ASSERT(v);
		int cnt, channels = bpp/dataBits; ASSERT(channels >= m_header.channels);

		for (i=0; i < h; i++) {
			if (i%2) sampledPos -= (w + 1)/2;
			cnt = 0;
			for (j=0; j < w; j++) {
				if (m_downsample) {
					// image was downsampled
					uAvg = u[sampledPos];
					vAvg = v[sampledPos];
				} else {
					uAvg = u[yPos];
					vAvg = v[yPos];
				}
				buff[cnt + channelMap[0]] = y[yPos];
				buff[cnt + channelMap[1]] = uAvg;
				buff[cnt + channelMap[2]] = vAvg;
				yPos++; 
				cnt += channels;
				if (j%2) sampledPos++;
			}
			buff += pitch2;
			if (wOdd) sampledPos++;

			if (cb) {
				percent += dP;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}
	} else if (m_header.channels == 4) {
		ASSERT(m_header.bpp == m_header.channels*8);
		ASSERT(bpp%dataBits == 0);

		DataT* y = m_channel[0]; ASSERT(y);
		DataT* u = m_channel[1]; ASSERT(u);
		DataT* v = m_channel[2]; ASSERT(v);
		DataT* a = m_channel[3]; ASSERT(a);
		UINT8 aAvg;
		int cnt, channels = bpp/dataBits; ASSERT(channels >= m_header.channels);

		for (i=0; i < h; i++) {
			if (i%2) sampledPos -= (w + 1)/2;
			cnt = 0;
			for (j=0; j < w; j++) {
				if (m_downsample) {
					// image was downsampled
					uAvg = u[sampledPos];
					vAvg = v[sampledPos];
					aAvg = Clamp8(a[sampledPos] + yuvOffset);
				} else {
					uAvg = u[yPos];
					vAvg = v[yPos];
					aAvg = Clamp8(a[yPos] + yuvOffset);
				}
				// Yuv
				buff[cnt + channelMap[0]] = y[yPos];
				buff[cnt + channelMap[1]] = uAvg;
				buff[cnt + channelMap[2]] = vAvg;
				buff[cnt + channelMap[3]] = aAvg;
				yPos++; 
				cnt += channels;
				if (j%2) sampledPos++;
			}
			buff += pitch2;
			if (wOdd) sampledPos++;

			if (cb) {
				percent += dP;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
/// Import a YUV image from a specified image buffer.
/// The absolute value of pitch is the number of bytes of an image row.
/// If pitch is negative, then buff points to the last row of a bottom-up image (first byte on last row).
/// If pitch is positive, then buff points to the first row of a top-down image (first byte).
/// The sequence of input channels in the input image buffer does not need to be the same as expected from PGF. In case of different sequences you have to
/// provide a channelMap of size of expected channels (depending on image mode). For example, PGF expects in RGB color mode a channel sequence BGR.
/// If your provided image buffer contains a channel sequence VUY, then the channelMap looks like { 2, 1, 0 }.
/// It might throw an IOException.
/// @param pitch The number of bytes of a row of the image buffer.
/// @param buff An image buffer.
/// @param bpp The number of bits per pixel used in image buffer.
/// @param channelMap A integer array containing the mapping of input channel ordering to expected channel ordering.
/// @param cb A pointer to a callback procedure. The procedure is called after each imported buffer row. If cb returns true, then it stops proceeding.
void CPGFImage::ImportYUV(int pitch, DataT *buff, BYTE bpp, int channelMap[] /*= nullptr*/, CallbackPtr cb /*= nullptr*/, void *data /*=nullptr*/) {
	ASSERT(buff);
	const double dP = 1.0/m_header.height;
	const int dataBits = DataTSize*8; ASSERT(dataBits == 16 || dataBits == 32);
	const int pitch2 = pitch/DataTSize;
	const int yuvOffset = (dataBits == 16) ? YUVoffset8 : YUVoffset16;

	int yPos = 0, cnt = 0;
	double percent = 0;
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);

	if (channelMap == nullptr) channelMap = defMap;

	if (m_header.channels == 3)	{
		ASSERT(bpp%dataBits == 0);

		DataT* y = m_channel[0]; ASSERT(y);
		DataT* u = m_channel[1]; ASSERT(u);
		DataT* v = m_channel[2]; ASSERT(v);
		const int channels = bpp/dataBits; ASSERT(channels >= m_header.channels);

		for (UINT32 h=0; h < m_header.height; h++) {
			if (cb) {
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				percent += dP;
			}

			cnt = 0;
			for (UINT32 w=0; w < m_header.width; w++) {
				y[yPos] = buff[cnt + channelMap[0]];
				u[yPos] = buff[cnt + channelMap[1]];
				v[yPos] = buff[cnt + channelMap[2]];
				yPos++;
				cnt += channels;
			}
			buff += pitch2;
		}	
	} else if (m_header.channels == 4) {
		ASSERT(bpp%dataBits == 0);

		DataT* y = m_channel[0]; ASSERT(y);
		DataT* u = m_channel[1]; ASSERT(u);
		DataT* v = m_channel[2]; ASSERT(v);
		DataT* a = m_channel[3]; ASSERT(a);
		const int channels = bpp/dataBits; ASSERT(channels >= m_header.channels);

		for (UINT32 h=0; h < m_header.height; h++) {
			if (cb) {
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				percent += dP;
			}

			cnt = 0;
			for (UINT32 w=0; w < m_header.width; w++) {
				y[yPos] = buff[cnt + channelMap[0]];
				u[yPos] = buff[cnt + channelMap[1]];
				v[yPos] = buff[cnt + channelMap[2]];
				a[yPos] = buff[cnt + channelMap[3]] - yuvOffset;
				yPos++;
				cnt += channels;
			}
			buff += pitch2;
		}	
	}

	if (m_downsample) {
		// Subsampling of the chrominance and alpha channels
		for (int i=1; i < m_header.channels; i++) {
			Downsample(i);
		}
	}
}

