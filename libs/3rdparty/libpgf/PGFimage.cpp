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

#include "PGFimage.h"
#include "Decoder.h"
#include "Encoder.h"
#include <cmath>

#define YUVoffset4		8				// 2^3
#define YUVoffset6		32				// 2^5
#define YUVoffset8		128				// 2^7
#define YUVoffset16		32768			// 2^15
#define YUVoffset31		1073741824		// 2^30
#define MaxValue		2147483648		// 2^MaxBitPlanes = 2^31

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

//////////////////////////////////////////////////////////////////////
// Standard constructor: It is used to create a PGF instance for opening and reading.
CPGFImage::CPGFImage() 
: m_decoder(0), m_levelLength(0), m_quant(0), m_downsample(false), m_cb(0), m_cbArg(0)
{

	// init preHeader
	memcpy(m_preHeader.magic, Magic, 3);
	m_preHeader.version = PGFVersion;
	m_preHeader.hSize = 0;

	// init postHeader
	m_postHeader.userData = 0;
	m_postHeader.userDataLen = 0;

	// init channels
	for (int i=0; i < MaxChannels; i++) {
		m_channel[i] = 0;
		m_wtChannel[i] = 0;
	}

	// set image width and height
	m_width[0] = 0;
	m_height[0] = 0;
}

//////////////////////////////////////////////////////////////////////
// Destructor: Destroy internal data structures.
CPGFImage::~CPGFImage() {
	Destroy();
}

//////////////////////////////////////////////////////////////////////
// Destroy internal data structures.
// Destructor calls this method during destruction.
void CPGFImage::Destroy() {
	Close();

	for (int i=0; i < m_header.channels; i++) {
		delete m_wtChannel[i]; m_wtChannel[i]=0;
		m_channel[i] = 0;
	}
	delete[] m_postHeader.userData; m_postHeader.userData = 0; m_postHeader.userDataLen = 0;
	delete[] m_levelLength; m_levelLength = 0;
}

//////////////////////////////////////////////////////////////////////
// Close PGF image after opening and reading.
// Destructor calls this method during destruction.
void CPGFImage::Close() {
	delete m_decoder; m_decoder = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Open a PGF image at current stream position: read pre-header, header, levelLength, and ckeck image type.
// Precondition: The stream has been opened for reading.
// It might throw an IOException.
// @param stream A PGF stream
void CPGFImage::Open(CPGFStream *stream) THROW_ {
	ASSERT(stream);

	m_decoder = new CDecoder(stream, m_preHeader, m_header, m_postHeader, m_levelLength);
	if (!m_decoder) ReturnWithError(InsufficientMemory);

	if (m_header.nLevels > MaxLevel) ReturnWithError(FormatCannotRead);

	Init();

	ASSERT(m_decoder);
}

////////////////////////////////////////////////////////////
// Initialize an open pgf file
void CPGFImage::Init() THROW_ {
	// set current level
	m_currentLevel = m_header.nLevels;

	// set image width and height
	m_width[0] = m_header.width;
	m_height[0] = m_header.height;

	// set or correct image mode
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
	} else if (m_header.mode == ImageModeRGBColor && m_header.bpp == 32) {
		// change mode
		m_header.mode = ImageModeRGBA;
	}
	ASSERT(m_header.mode != ImageModeBitmap || m_header.bpp == 1);
	ASSERT(m_header.mode != ImageModeGrayScale || m_header.bpp == 8);
	ASSERT(m_header.mode != ImageModeGray16 || m_header.bpp == 16);
	ASSERT(m_header.mode != ImageModeRGBColor || m_header.bpp == 24);
	ASSERT(m_header.mode != ImageModeRGBA || m_header.bpp == 32);
	ASSERT(m_header.mode != ImageModeRGB12 || m_header.bpp == 12);
	ASSERT(m_header.mode != ImageModeRGB16 || m_header.bpp == 16);
	ASSERT(m_header.mode != ImageModeRGB48 || m_header.bpp == 48);
	ASSERT(m_header.mode != ImageModeLabColor || m_header.bpp == 24);
	ASSERT(m_header.mode != ImageModeLab48 || m_header.bpp == 48);
	ASSERT(m_header.mode != ImageModeCMYKColor || m_header.bpp == 32);
	ASSERT(m_header.mode != ImageModeCMYK64 || m_header.bpp == 64);

	// set number of channels
	if (!m_header.channels) {
		switch(m_header.mode) {
		case ImageModeBitmap: 
		case ImageModeIndexedColor:
		case ImageModeGrayScale:
		case ImageModeGray16:
		case ImageModeGray31:
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
		}
	}

	// interprete quant parameter
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
			m_width[i] = (m_width[0] + 1)/2;
			m_height[i] = (m_height[0] + 1)/2;
		}
	} else {
		for (int i=1; i < m_header.channels; i++) {
			m_width[i] = m_width[0];
			m_height[i] = m_height[0];
		}
	}

	// init wavelet subbands
	for (int i=0; i < m_header.channels; i++) {
		m_wtChannel[i] = new CWaveletTransform(m_width[i], m_height[i], m_header.nLevels);
		if (!m_wtChannel[i]) ReturnWithError(InsufficientMemory);
	}

	// set background
	/*
	if (m_header.mode == ImageModeRGBA &&
		(m_header.background.rgbtBlue != DefaultBGColor ||
		m_header.background.rgbtGreen != DefaultBGColor ||
		m_header.background.rgbtRed != DefaultBGColor)) {
		m_backgroundSet = true;
	}
	*/
}

//////////////////////////////////////////////////////////////////////
/// Return user data and size of user data.
/// @param size [out] Size of user data in bytes.
/// @return A pointer to user data or NULL if there is no user data.
const UINT8* CPGFImage::GetUserData(UINT32& size) const {
	size = m_postHeader.userDataLen;
	return m_postHeader.userData;
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
void CPGFImage::Read(int level /*= 0*/, CallbackPtr cb /*= NULL*/, void *data /*=NULL*/) THROW_ {
	ASSERT((level >= 0 && level < m_header.nLevels) || m_header.nLevels == 0); // m_header.nLevels == 0: image didn't use wavelet transform
	ASSERT(m_decoder);
	int i;

#ifdef __PGFROISUPPORT__
	if (ROIisSupported() && m_header.nLevels > 0) {
		// new encoding scheme supporting ROI
		PGFRect rect(0, 0, m_header.width, m_header.height);
		Read(rect, level, cb, data);
		return;
	}
#endif

	if (m_header.nLevels == 0) {
		// image didn't use wavelet transform
		for (i=0; i < m_header.channels; i++) {
			ASSERT(m_wtChannel[i]);
			// decode file and write stream to m_channel
			m_wtChannel[i]->GetSubband(0, LL)->PlaceTile(*m_decoder, m_quant);
			m_channel[i] = m_wtChannel[i]->GetSubband(0, LL)->GetBuffer();
		}
	} else {
		const int levelDiff = m_currentLevel - level;
		double p, percent = pow(0.25, levelDiff);

		// encoding scheme without ROI
		while (m_currentLevel > level) {
			p = percent;
			for (i=0; i < m_header.channels; i++) {
				ASSERT(m_wtChannel[i]);
				// decode file and write stream to m_wtChannel
				if (m_currentLevel == m_header.nLevels) { 
					// last level also has LL band
					m_wtChannel[i]->GetSubband(m_currentLevel, LL)->PlaceTile(*m_decoder, m_quant);
				}
				if (m_preHeader.version & Version5) {
					// since version 5
					m_wtChannel[i]->GetSubband(m_currentLevel, HL)->PlaceTile(*m_decoder, m_quant);
					m_wtChannel[i]->GetSubband(m_currentLevel, LH)->PlaceTile(*m_decoder, m_quant);
				} else {
					// until version 4
					m_decoder->DecodeInterleaved(m_wtChannel[i], m_currentLevel, m_quant);
				}
				m_wtChannel[i]->GetSubband(m_currentLevel, HH)->PlaceTile(*m_decoder, m_quant);

				// inverse transform from m_wtChannel to m_channel
				m_wtChannel[i]->InverseTransform(m_currentLevel, &m_width[i], &m_height[i], &m_channel[i]);
				ASSERT(m_channel[i]);

				// now update progress
				if (i < m_header.channels - 1 && cb) {
					percent += 3*p/m_header.channels;
					(*cb)(percent, false, data);
				}
			}
			// set new level: must be done before refresh callback
			m_currentLevel--;

			// now we have to refresh the display
			if (m_cb) m_cb(m_cbArg);

			// now update progress
			if (cb) {
				percent += 3*p/m_header.channels;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
/// After you've written a PGF image, you can call this method followed by GetBitmap/GetYUV
/// to get a quick reconstruction (coded -> decoded image).
void CPGFImage::Reconstruct() THROW_ {
	int i;

	if (m_header.nLevels == 0) {
		// image didn't use wavelet transform
		for (i=0; i < m_header.channels; i++) {
			ASSERT(m_wtChannel[i]);
			m_channel[i] = m_wtChannel[i]->GetSubband(0, LL)->GetBuffer();
		}
	} else {
		int currentLevel = m_header.nLevels;

		// old encoding scheme without ROI
		while (currentLevel > 0) {
			for (i=0; i < m_header.channels; i++) {
				ASSERT(m_wtChannel[i]);
				// dequantize subbands
				if (currentLevel == m_header.nLevels) { 
					// last level also has LL band
					m_wtChannel[i]->GetSubband(currentLevel, LL)->Dequantize(m_quant, currentLevel);
				}
				m_wtChannel[i]->GetSubband(currentLevel, HL)->Dequantize(m_quant, currentLevel);
				m_wtChannel[i]->GetSubband(currentLevel, LH)->Dequantize(m_quant, currentLevel);
				m_wtChannel[i]->GetSubband(currentLevel, HH)->Dequantize(m_quant, currentLevel);

				// inverse transform from m_wtChannel to m_channel
				m_wtChannel[i]->InverseTransform(currentLevel, &m_width[i], &m_height[i], &m_channel[i]);
				ASSERT(m_channel[i]);
			}
			// now we have to refresh the display
			if (m_cb) m_cb(m_cbArg);

			currentLevel--;
		}
	}
}

#ifdef __PGFROISUPPORT__
//////////////////////////////////////////////////////////////////////
/// Compute ROIs for each channel and each level
/// @param rect rectangular region of interest (ROI)
void CPGFImage::SetROI(PGFRect rect) {
	ASSERT(m_decoder);
	ASSERT(ROIisSupported());

	// store ROI for a later call of GetBitmap
	m_roi = rect;

	// enable ROI decoding
	m_decoder->SetROI();

	// enlarge ROI because of border artefacts
	const UINT32 dx = FilterWidth/2*(1 << m_currentLevel);
	const UINT32 dy = FilterHeight/2*(1 << m_currentLevel);

	if (rect.left < dx) rect.left = 0;
	else rect.left -= dx;
	if (rect.top < dy) rect.top = 0;
	else rect.top -= dy;
	rect.right += dx;
	if (rect.right > m_header.width) rect.right = m_header.width;
	rect.bottom += dy;
	if (rect.bottom > m_header.height) rect.bottom = m_header.height;

	// prepare wavelet channels for using ROI
	ASSERT(m_wtChannel[0]);
	m_wtChannel[0]->SetROI(rect);
	if (m_downsample && m_header.channels > 1) {
		// all further channels are downsampled, therefore downsample ROI
		rect.left >>= 1;
		rect.top >>= 1;
		rect.right >>= 1;
		rect.bottom >>= 1;
	}
	for (int i=1; i < m_header.channels; i++) {
		ASSERT(m_wtChannel[i]);
		m_wtChannel[i]->SetROI(rect);
	}
}

//////////////////////////////////////////////////////////////////////
/// Read a rectangular region of interest of a PGF image at current stream position.
/// The origin of the coordinate axis is the top-left corner of the image.
/// All coordinates are measured in pixels.
/// It might throw an IOException.
/// @param rect [inout] Rectangular region of interest (ROI). The rect might be cropped.
/// @param level The image level of the resulting image in the internal image buffer.
/// @param cb A pointer to a callback procedure. The procedure is called after reading a single level. If cb returns true, then it stops proceeding.
/// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::Read(PGFRect& rect, int level /*= 0*/, CallbackPtr cb /*= NULL*/, void *data /*=NULL*/) THROW_ {
	ASSERT((level >= 0 && level < m_header.nLevels) || m_header.nLevels == 0); // m_header.nLevels == 0: image didn't use wavelet transform
	ASSERT(m_decoder);
	int i;

	if (m_header.nLevels == 0 || !ROIisSupported()) {
		rect.left = rect.top = 0;
		rect.right = m_header.width; rect.bottom = m_header.height;
		Read(level, cb, data);
	} else {
		ASSERT(ROIisSupported());
		// new encoding scheme supporting ROI
		ASSERT(rect.left < m_header.width && rect.top < m_header.height);
		const int levelDiff = m_currentLevel - level;
		double p, percent = pow(0.25, levelDiff);
		
		// check level difference
		if (levelDiff <= 0) {
			// it is a new read call, probably with a new ROI
			m_currentLevel = m_header.nLevels;
			m_decoder->SetStreamPosToData();
		}

		// check rectangle
		if (rect.right == 0 || rect.right > m_header.width) rect.right = m_header.width;
		if (rect.bottom == 0 || rect.bottom > m_header.height) rect.bottom = m_header.height;
		
		// enable ROI decoding and reading
		SetROI(rect);

		while (m_currentLevel > level) {
			p = percent;
			for (i=0; i < m_header.channels; i++) {
				ASSERT(m_wtChannel[i]);

				// get number of tiles and tile indices
				const UINT32 nTiles = m_wtChannel[i]->GetNofTiles(m_currentLevel);
				const PGFRect& tileIndices = m_wtChannel[i]->GetTileIndices(m_currentLevel);

				// decode file and write stream to m_wtChannel
				if (m_currentLevel == m_header.nLevels) { // last level also has LL band
					ASSERT(nTiles == 1);
					m_decoder->DecodeTileBuffer();
					m_wtChannel[i]->GetSubband(m_currentLevel, LL)->PlaceTile(*m_decoder, m_quant);
				}
				for (UINT32 tileY=0; tileY < nTiles; tileY++) {
					for (UINT32 tileX=0; tileX < nTiles; tileX++) {
						// check relevance of tile
						if (tileIndices.IsInside(tileX, tileY)) {
							m_decoder->DecodeTileBuffer();
							m_wtChannel[i]->GetSubband(m_currentLevel, HL)->PlaceTile(*m_decoder, m_quant, true, tileX, tileY);
							m_wtChannel[i]->GetSubband(m_currentLevel, LH)->PlaceTile(*m_decoder, m_quant, true, tileX, tileY);
							m_wtChannel[i]->GetSubband(m_currentLevel, HH)->PlaceTile(*m_decoder, m_quant, true, tileX, tileY);
						} else {
							// skip tile
							m_decoder->SkipTileBuffer();
						}
					}
				}

				// inverse transform from m_wtChannel to m_channel
				m_wtChannel[i]->InverseTransform(m_currentLevel, &m_width[i], &m_height[i], &m_channel[i]);
				ASSERT(m_channel[i]);

				// now update progress
				if (i < m_header.channels - 1 && cb) {
					percent += 3*p/m_header.channels;
					(*cb)(percent, false, data);
				}
			}
			// set new level: must be done before refresh callback
			m_currentLevel--;

			// now we have to refresh the display
			if (m_cb) m_cb(m_cbArg);

			// now update progress
			if (cb) {
				percent += 3*p/m_header.channels;
				if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
			}
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////
/// Return the length of all encoded headers in bytes.
/// @return The length of all encoded headers in bytes
UINT32 CPGFImage::GetEncodedHeaderLength() const { 
	ASSERT(m_decoder); 
	return m_decoder->GetEncodedHeaderLength(); 
}

//////////////////////////////////////////////////////////////////////
/// Reads the encoded PGF headers and copies it to a target buffer.
/// Precondition: The PGF image has been opened with a call of Open(...).
/// It might throw an IOException.
/// @param target The target buffer
/// @param targetLen The length of the target buffer in bytes
/// @return The number of bytes copied to the target buffer
UINT32 CPGFImage::ReadEncodedHeader(UINT8* target, UINT32 targetLen) const THROW_ {
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
/// Reset stream position to beginning of PGF pre header
void CPGFImage::ResetStreamPos() THROW_ {
	ASSERT(m_decoder);
	return m_decoder->SetStreamPosToStart(); 
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
UINT32 CPGFImage::ReadEncodedData(int level, UINT8* target, UINT32 targetLen) const THROW_ {
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

//////////////////////////////////////////////////////////////////
// Set background of an RGB image with transparency channel or reset to default background.
// @param bg A pointer to a background color or NULL (reset to default background)
void CPGFImage::SetBackground(const RGBTRIPLE* bg) { 
	if (bg) { 
		m_header.background = *bg; 
//		m_backgroundSet = true; 
	} else {
		m_header.background.rgbtBlue = DefaultBGColor;
		m_header.background.rgbtGreen = DefaultBGColor;
		m_header.background.rgbtRed = DefaultBGColor;
//		m_backgroundSet = false;
	}
}

//////////////////////////////////////////////////////////////////////
/// Set PGF header and user data.
/// Precondition: The PGF image has been closed with Close(...) or never opened with Open(...).
/// It might throw an IOException.
/// @param header A valid and already filled in PGF header structure
/// @param flags A combination of additional version flags
/// @param userData A user-defined memory block
/// @param userDataLength The size of user-defined memory block in bytes
void CPGFImage::SetHeader(const PGFHeader& header, BYTE flags /*=0*/, UINT8* userData /*= 0*/, UINT32 userDataLength /*= 0*/) THROW_ {
	ASSERT(!m_decoder);	// current image must be closed
	ASSERT(header.quality <= MaxQuality);
	int i;

	// init preHeader
	memcpy(m_preHeader.magic, Magic, 3);
	m_preHeader.version = PGFVersion | flags;
	m_preHeader.hSize = HeaderSize;

	// copy header
	memcpy(&m_header, &header, HeaderSize);

	// misuse background value to store bits per channel
	BYTE bpc = m_header.bpp/m_header.channels;
	if (bpc > 8) {
		if (bpc > 31) bpc = 31;
		m_header.background.rgbtBlue = bpc;
	}

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
		m_preHeader.hSize += ColorTableSize;
	}
	if (userDataLength && userData) {
		m_postHeader.userData = new UINT8[userDataLength];
		m_postHeader.userDataLen = userDataLength;
		memcpy(m_postHeader.userData, userData, userDataLength);
		m_preHeader.hSize += userDataLength;
	}

	// allocate channels
	for (i=0; i < m_header.channels; i++) {
		// set current width and height
		m_width[i] = m_header.width;
		m_height[i] = m_header.height;

		// allocate channels
		ASSERT(!m_channel[i]);
		m_channel[i] = new DataT[m_header.width*m_header.height];
		if (!m_channel[i]) ReturnWithError(InsufficientMemory);
	}
}

//////////////////////////////////////////////////////////////////////
/// Set maximum intensity value for image modes with more than eight bits per channel.
/// Don't call this method before SetHeader.
/// @param maxValue The maximum intensity value.
void CPGFImage::SetMaxValue(UINT32 maxValue) {
	BYTE pot = 0;

	while(maxValue > 0) {
		pot++;
		maxValue >>= 1;
	}
	// store bits per channel
	if (pot > 31) pot = 31;
	m_header.background.rgbtBlue = pot;
}

//////////////////////////////////////////////////////////////////////
/// Returns number of used bits per input/output image channel.
/// Precondition: header must be initialized.
/// @return number of used bits per input/output image channel.
BYTE CPGFImage::UsedBitsPerChannel() const {
	BYTE bpc = m_header.bpp/m_header.channels;

	if (bpc > 8) {
		return m_header.background.rgbtBlue;
	} else {
		return bpc;
	}
}

//////////////////////////////////////////////////////////////////////
/// Returns highest supported version
BYTE CPGFImage::Version() const {
	if (m_preHeader.version & Version6) return 6;
	if (m_preHeader.version & Version5) return 5;
	if (m_preHeader.version & Version2) return 2;
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
void CPGFImage::ImportBitmap(int pitch, UINT8 *buff, BYTE bpp, int channelMap[] /*= NULL */, CallbackPtr cb /*= NULL*/, void *data /*=NULL*/) THROW_ {
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
void CPGFImage::Downsample(int ch) {
	ASSERT(ch > 0);

	const int w = m_width[0];
	const int w2 = w/2;
	const int h2 = m_height[0]/2;
	const int oddW = w%2;				// don't use bool -> problems with MaxSpeed optimization
	const int oddH = m_height[0]%2;		// "
	int i, j;
	int loPos = 0;
	int hiPos = w;
	int sampledPos = 0;
	DataT* buff = m_channel[ch]; ASSERT(buff);

	for (i=0; i < h2; i++) {
		for (j=0; j < w2; j++) {
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
		for (j=0; j < w2; j++) {
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
/// Compute and return number of levels. During PGF::Write the return
/// value of this method is used in case the parameter levels is not a
/// positive value. 
/// A PGF image is structered in levels, numbered between 0 and Levels() - 1.
/// Each level can be seen as a single image, containing the same content
/// as all other levels, but in a different size (width, height).
/// The image size at level i is double the size (width, height) of the image at level i+1.
/// The image at level 0 contains the original size.
/// @param width Original image width
/// @param height Original image height
/// @return Number of PGF levels
BYTE CPGFImage::ComputeLevels(UINT32 width, UINT32 height) {
	const int maxThumbnailWidth = 20*FilterWidth;
	const int m = __min(width, height);
	int s = m, levels = 1;

	// compute a good value depending on the size of the image
	while (s > maxThumbnailWidth) {
		levels++;
		s = s/2;
	}

	// reduce number of levels if the image size is smaller than FilterWidth*2^levels
	s = FilterWidth*(1 << levels);	// must be at least the double filter size because of subsampling
	while (m < s) {
		levels--;
		s = s/2;
	}
	if (levels > MaxLevel) levels = MaxLevel;
	if (levels < 0) levels = 0;
	ASSERT(0 <= levels && levels <= MaxLevel);
	
	return (BYTE)levels;
}

//////////////////////////////////////////////////////////////////
// Encode and write a PGF image at current stream position.
// A PGF image is structered in levels, numbered between 0 and Levels() - 1.
// Each level can be seen as a single image, containing the same content
// as all other levels, but in a different size (width, height).
// The image size at level i is double the size (width, height) of the image at level i+1.
// The image at level 0 contains the original size.
// Precondition: the PGF image contains a valid header (see also SetHeader(...)).
// It might throw an IOException.
// @param stream A PGF stream
// @param levels The positive number of levels used in layering or 0 meaning a useful number of levels is computed.
// @param cb A pointer to a callback procedure. The procedure is called after reading a single level. If cb returns true, then it stops proceeding.
// @param nWrittenBytes [in-out] The number of bytes written into stream are added to the input value.
// @param data Data Pointer to C++ class container to host callback procedure.
void CPGFImage::Write(CPGFStream* stream, int levels /* = 0*/, CallbackPtr cb /*= NULL*/, UINT32* nWrittenBytes /*= NULL*/, void *data /*=NULL*/) THROW_ {
	ASSERT(stream);
	ASSERT(m_preHeader.hSize);
	int i;
	UINT32 nBytes = 0;
	DataT *temp;

	// check and set number of levels
	if (levels < 1) {
		levels = m_header.nLevels = ComputeLevels(m_header.width, m_header.height);
	} else {
		if (levels > MaxLevel) levels = MaxLevel;
		m_header.nLevels = levels;
	}
	ASSERT(m_header.nLevels <= MaxLevel);
	ASSERT(m_header.quality <= MaxQuality); // quality is already initialized

	// create new wt channels
	for (i=0; i < m_header.channels; i++) {
		temp = NULL;
		if (m_wtChannel[i]) {
			ASSERT(m_channel[i]);
			// copy m_channel to temp
			int size = m_height[i]*m_width[i];
			temp = new DataT[size]; 
			if (!temp) ReturnWithError(InsufficientMemory);
			memcpy(temp, m_channel[i], size*sizeof(INT16));
			delete m_wtChannel[i];	// also deletes m_channel
		}
		if (temp) m_channel[i] = temp;
		m_wtChannel[i] = new CWaveletTransform(m_width[i], m_height[i], levels, m_channel[i]);
		if (!m_wtChannel[i]) {
			delete temp;
			ReturnWithError(InsufficientMemory);
		}

		// wavelet subband decomposition 
		for (m_currentLevel=0; m_currentLevel < levels; m_currentLevel++) {
			m_wtChannel[i]->ForwardTransform(m_currentLevel);
		}
	}
	double percent = pow(0.25, levels - 1);

	// open encoder and eventually write headers and levelLength
	CEncoder encoder(stream, m_preHeader, m_header, m_postHeader, m_levelLength);

	if (levels > 0) {
		// encode quantized wavelet coefficients and write to PGF file
		// encode subbands, higher levels first
		// color channels are interleaved

#ifdef __PGFROISUPPORT__
		if (ROIisSupported()) {
			// new encoding scheme supporting ROI
			encoder.SetROI();

			for (m_currentLevel = (UINT8)levels; m_currentLevel > 0; m_currentLevel--) {
				for (i=0; i < m_header.channels; i++) {
					m_wtChannel[i]->SetROI();

					// get number of tiles and tile indices
					const UINT32 nTiles = m_wtChannel[i]->GetNofTiles(m_currentLevel);

					if (m_currentLevel == levels) {
						ASSERT(nTiles == 1);
						m_wtChannel[i]->GetSubband(m_currentLevel, LL)->ExtractTile(encoder, m_quant);
						encoder.EncodeTileBuffer();
					}
					for (UINT32 tileY=0; tileY < nTiles; tileY++) {
						for (UINT32 tileX=0; tileX < nTiles; tileX++) {
							m_wtChannel[i]->GetSubband(m_currentLevel, HL)->ExtractTile(encoder, m_quant, true, tileX, tileY);
							m_wtChannel[i]->GetSubband(m_currentLevel, LH)->ExtractTile(encoder, m_quant, true, tileX, tileY);
							m_wtChannel[i]->GetSubband(m_currentLevel, HH)->ExtractTile(encoder, m_quant, true, tileX, tileY);
							encoder.EncodeTileBuffer();
						}
					}
				}

				// all necessary data of a level is buffered!
				encoder.SetLevelIsEncoded(true);

				// now update progress
				if (cb) {
					percent *= 4;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
		} else 
#endif
		{
			// encoding scheme without ROI
			m_currentLevel = (UINT8)levels;
			for (i=0; i < m_header.channels; i++) {
				m_wtChannel[i]->GetSubband(m_currentLevel, LL)->ExtractTile(encoder, m_quant);
				//encoder.EncodeInterleaved(m_wtChannel[i], m_currentLevel, m_quant); // until version 4
				m_wtChannel[i]->GetSubband(m_currentLevel, HL)->ExtractTile(encoder, m_quant); // since version 5
				m_wtChannel[i]->GetSubband(m_currentLevel, LH)->ExtractTile(encoder, m_quant); // since version 5
				m_wtChannel[i]->GetSubband(m_currentLevel, HH)->ExtractTile(encoder, m_quant);
			}
			// all necessary data buffered!
			encoder.SetLevelIsEncoded(true);
			m_currentLevel--;

			for (; m_currentLevel > 0; m_currentLevel--) {
				for (i=0; i < m_header.channels; i++) {
					//encoder.EncodeInterleaved(m_wtChannel[i], m_currentLevel, m_quant); // until version 4
					m_wtChannel[i]->GetSubband(m_currentLevel, HL)->ExtractTile(encoder, m_quant); // since version 5
					m_wtChannel[i]->GetSubband(m_currentLevel, LH)->ExtractTile(encoder, m_quant); // since version 5
					m_wtChannel[i]->GetSubband(m_currentLevel, HH)->ExtractTile(encoder, m_quant);
				}
				// all necessary data of a level buffered!
				encoder.SetLevelIsEncoded(true);

				// now update progress
				if (cb) {
					percent *= 4;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
				}
			}
		}
	} else {
		// write untransformed m_channel
		for (i=0; i < m_header.channels; i++) {
			m_wtChannel[i]->GetSubband(0, LL)->ExtractTile(encoder, m_quant);
		}
	}
	
	// flush encoder
	nBytes = encoder.Flush();
	if (nWrittenBytes) *nWrittenBytes += nBytes;
}

//////////////////////////////////////////////////////////////////
// Check for valid import image mode.
// @param mode Image mode
// @return True if an image of given mode can be imported with ImportBitmap(...)
bool CPGFImage::ImportIsSupported(BYTE mode) {
	size_t size = sizeof(DataT);

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
			case ImageModeRGBA:
				return true;
		}
	}
	if (size >= 3) {
		switch(mode) {
			case ImageModeGray16:
			case ImageModeRGB16:
			case ImageModeRGB48:
			case ImageModeLab48:
			case ImageModeCMYK64:
			//case ImageModeDuotone16:
				return true;
		}
	}
	if (size >=4) {
		switch(mode) {
			case ImageModeGray31:
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
void CPGFImage::GetColorTable(UINT32 iFirstColor, UINT32 nColors, RGBQUAD* prgbColors) const THROW_ {
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
void CPGFImage::SetColorTable(UINT32 iFirstColor, UINT32 nColors, const RGBQUAD* prgbColors) THROW_ {
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
void CPGFImage::RgbToYuv(int pitch, UINT8* buff, BYTE bpp, int channelMap[], CallbackPtr cb, void *data /*=NULL*/) THROW_ {
	ASSERT(buff);
	int yPos = 0, cnt = 0;
	double percent = 0;
	const double dP = 1.0/m_header.height;
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);

	if (channelMap == NULL) channelMap = defMap;

	switch(m_header.mode) {
	case ImageModeBitmap:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 1);
			ASSERT(bpp == 1);
			
			const UINT32 w2 = (m_header.width + 7)/8;
			DataT* y = m_channel[0]; ASSERT(y);

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				for (UINT32 w=0; w < w2; w++) {
					y[yPos++] = buff[w] - YUVoffset8;
				}
				buff += pitch;	
			}
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
			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				cnt = 0;
				for (UINT32 w=0; w < m_header.width; w++) {
					for (int c=0; c < m_header.channels; c++) {
						m_channel[c][yPos] = buff16[cnt + channelMap[c]] - yuvOffset16;
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
			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);

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
					b = buff16[cnt + channelMap[0]];
					g = buff16[cnt + channelMap[1]];
					r = buff16[cnt + channelMap[2]];
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
			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			
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
					b = buff16[cnt + channelMap[0]];
					g = buff16[cnt + channelMap[1]];
					r = buff16[cnt + channelMap[2]];
					// Yuv
					y[yPos] = ((b + (g << 1) + r) >> 2) - yuvOffset16;
					u[yPos] = r - g;
					v[yPos] = b - g;
					a[yPos++] = buff16[cnt + channelMap[3]] - yuvOffset16;
					cnt += channels;
				}
				buff16 += pitch16;
			}	
		}
		break;
	case ImageModeGray31:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 32);
			ASSERT(bpp == 32);

			DataT* y = m_channel[0]; ASSERT(y);

			UINT32 *buff32 = (UINT32 *)buff;
			const int pitch32 = pitch/4;
			const int yuvOffset31 = 1 << (UsedBitsPerChannel() - 1);

			for (UINT32 h=0; h < m_header.height; h++) {
				if (cb) {
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					percent += dP;
				}

				for (UINT32 w=0; w < m_header.width; w++) {
					ASSERT(buff32[cnt] < MaxValue);
					y[yPos++] = buff32[w] - yuvOffset31;
				}
				buff32 += pitch32;
			}
		}
		break;
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
void CPGFImage::GetBitmap(int pitch, UINT8* buff, BYTE bpp, int channelMap[] /*= NULL */, CallbackPtr cb /*= NULL*/, void *data /*=NULL*/) const THROW_ {
	ASSERT(buff);
	UINT32 w = m_width[0];
	UINT32 h = m_height[0];
	UINT8* targetBuff = 0;	// used if ROI is used
	UINT8* buffStart = 0;	// used if ROI is used
	int targetPitch = 0;	// used if ROI is used

#ifdef __PGFROISUPPORT__
	const PGFRect& roi = (ROIisSupported()) ? m_wtChannel[0]->GetROI(m_currentLevel) : PGFRect(0, 0, w, h); // roi is usually larger than m_roi
	const PGFRect levelRoi(LevelWidth(m_roi.left, m_currentLevel), LevelHeight(m_roi.top, m_currentLevel), LevelWidth(m_roi.Width(), m_currentLevel), LevelHeight(m_roi.Height(), m_currentLevel));
	ASSERT(w == roi.Width() && h == roi.Height());
	ASSERT(roi.left <= levelRoi.left && levelRoi.right <= roi.right); 
	ASSERT(roi.top <= levelRoi.top && levelRoi.bottom <= roi.bottom); 

	if (ROIisSupported() && (levelRoi.Width() < w || levelRoi.Height() < h)) {
		// ROI is used -> create a temporary image buffer for roi
		// compute pitch
		targetPitch = pitch;
		pitch = AlignWordPos(w*bpp)/8;

		// create temporary output buffer
		targetBuff = buff;
		buff = buffStart = new UINT8[pitch*h];
	}
#endif

	const bool wOdd = (1 == w%2);

	const double dP = 1.0/h;
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);
	if (channelMap == NULL) channelMap = defMap;
	int sampledPos = 0, yPos = 0;
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

			for (i=0; i < h; i++) {
				for (j=0; j < w2; j++) {
					buff[j] = Clamp(y[yPos++] + YUVoffset8);
				}
				buff += pitch;

				if (cb) {
					percent += dP;
					if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
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

			int cnt, channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (i=0; i < h; i++) {
				cnt = 0;
				for (j=0; j < w; j++) {
					for (int c=0; c < m_header.channels; c++) {
						buff[cnt + channelMap[c]] = Clamp(m_channel[c][yPos] + YUVoffset8);
					}
					cnt += channels;
					yPos++;
				}
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

			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			const int shift = UsedBitsPerChannel() - 8;
			int cnt, channels;

			if (bpp%16 == 0) {
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					cnt = 0;
					for (j=0; j < w; j++) {
						for (int c=0; c < m_header.channels; c++) {
							buff16[cnt + channelMap[c]] = Clamp16(m_channel[c][yPos] + yuvOffset16);
						}
						cnt += channels;
						yPos++;
					}
					buff16 += pitch16;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				channels = bpp/8; ASSERT(channels >= m_header.channels);
				
				for (i=0; i < h; i++) {
					cnt = 0;
					for (j=0; j < w; j++) {
						for (int c=0; c < m_header.channels; c++) {
							buff[cnt + channelMap[c]] = Clamp16(m_channel[c][yPos] + yuvOffset16) >> shift;
						}
						cnt += channels;
						yPos++;
					}
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
				*buffu = &buff[channelMap[2]],
				*buffv = &buff[channelMap[0]];
			UINT8 g;
			int cnt, channels = bpp/8;
			if(m_downsample){
				for (i=0; i < h; i++) {
					if (i%2) sampledPos -= (w + 1)/2;
					cnt = 0;
					for (j=0; j < w; j++) {
						// image was downsampled
						uAvg = u[sampledPos];
						vAvg = v[sampledPos];
						// Yuv
						buffg[cnt] = g = Clamp(y[yPos] + YUVoffset8 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buffu[cnt] = Clamp(uAvg + g);
						buffv[cnt] = Clamp(vAvg + g);
						yPos++;
						cnt += channels;
						if (j%2) sampledPos++;
					}
					buffg += pitch;
					buffu += pitch;
					buffv += pitch;
					if (wOdd) sampledPos++;
					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}else{
				for (i=0; i < h; i++) {
					cnt = 0;
					for (j = 0; j < w; j++) {
						uAvg = u[yPos];
						vAvg = v[yPos];
						// Yuv
						buffg[cnt] = g = Clamp(y[yPos] + YUVoffset8 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buffu[cnt] = Clamp(uAvg + g);
						buffv[cnt] = Clamp(vAvg + g);
						yPos++;
						cnt += channels;
					}
					buffg += pitch;
					buffu += pitch;
					buffv += pitch;

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

			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			const int shift = UsedBitsPerChannel() - 8;

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			UINT16 g;
			int cnt, channels;

			if (bpp >= 48 && bpp%16 == 0) {
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

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
						// Yuv
						buff16[cnt + channelMap[1]] = g = Clamp16(y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buff16[cnt + channelMap[2]] = Clamp16(uAvg + g);
						buff16[cnt + channelMap[0]] = Clamp16(vAvg + g);
						yPos++; 
						cnt += channels;
						if (j%2) sampledPos++;
					}
					buff16 += pitch16;
					if (wOdd) sampledPos++;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				channels = bpp/8; ASSERT(channels >= m_header.channels);

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
						// Yuv
						g = Clamp16(y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buff[cnt + channelMap[1]] = g >> shift; 
						buff[cnt + channelMap[2]] = Clamp16(uAvg + g) >> shift;
						buff[cnt + channelMap[0]] = Clamp16(vAvg + g) >> shift;
						yPos++; 
						cnt += channels;
						if (j%2) sampledPos++;
					}
					buff += pitch;
					if (wOdd) sampledPos++;

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
			int cnt, channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (i=0; i < h; i++) {
				if (i%2) sampledPos -= (w + 1)/2;
				cnt = 0;
				for (j=0; j < w; j++) {
					if (m_downsample) {
						// image was downsampled
						uAvg = a[sampledPos];
						vAvg = b[sampledPos];
					} else {
						uAvg = a[yPos];
						vAvg = b[yPos];
					}
					buff[cnt + channelMap[0]] = Clamp(l[yPos] + YUVoffset8);
					buff[cnt + channelMap[1]] = Clamp(uAvg + YUVoffset8); 
					buff[cnt + channelMap[2]] = Clamp(vAvg + YUVoffset8);
					cnt += channels;
					yPos++;
					if (j%2) sampledPos++;
				}
				buff += pitch;
				if (wOdd) sampledPos++;

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

			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			const int shift = UsedBitsPerChannel() - 8;

			DataT* l = m_channel[0]; ASSERT(l);
			DataT* a = m_channel[1]; ASSERT(a);
			DataT* b = m_channel[2]; ASSERT(b);
			int cnt, channels;

			if (bpp%16 == 0) {
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					if (i%2) sampledPos -= (w + 1)/2;
					cnt = 0;
					for (j=0; j < w; j++) {
						if (m_downsample) {
							// image was downsampled
							uAvg = a[sampledPos];
							vAvg = b[sampledPos];
						} else {
							uAvg = a[yPos];
							vAvg = b[yPos];
						}
						buff16[cnt + channelMap[0]] = Clamp16(l[yPos] + yuvOffset16);
						buff16[cnt + channelMap[1]] = Clamp16(uAvg + yuvOffset16);
						buff16[cnt + channelMap[2]] = Clamp16(vAvg + yuvOffset16);
						cnt += channels;
						yPos++;
						if (j%2) sampledPos++;
					}
					buff16 += pitch16;
					if (wOdd) sampledPos++;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				channels = bpp/8; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					if (i%2) sampledPos -= (w + 1)/2;
					cnt = 0;
					for (j=0; j < w; j++) {
						if (m_downsample) {
							// image was downsampled
							uAvg = a[sampledPos];
							vAvg = b[sampledPos];
						} else {
							uAvg = a[yPos];
							vAvg = b[yPos];
						}
						buff[cnt + channelMap[0]] = Clamp16(l[yPos] + yuvOffset16) >> shift;
						buff[cnt + channelMap[1]] = Clamp16(uAvg + yuvOffset16) >> shift;
						buff[cnt + channelMap[2]] = Clamp16(vAvg + yuvOffset16) >> shift;
						cnt += channels;
						yPos++;
						if (j%2) sampledPos++;
					}
					buff += pitch;
					if (wOdd) sampledPos++;

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
			int cnt, channels = bpp/8; ASSERT(channels >= m_header.channels);

			for (i=0; i < h; i++) {
				if (i%2) sampledPos -= (w + 1)/2;
				cnt = 0;
				for (j=0; j < w; j++) {
					if (m_downsample) {
						// image was downsampled
						uAvg = u[sampledPos];
						vAvg = v[sampledPos];
						aAvg = Clamp(a[sampledPos] + YUVoffset8);
					} else {
						uAvg = u[yPos];
						vAvg = v[yPos];
						aAvg = Clamp(a[yPos] + YUVoffset8);
					}
					// Yuv
					buff[cnt + channelMap[1]] = g = Clamp(y[yPos] + YUVoffset8 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
					buff[cnt + channelMap[2]] = Clamp(uAvg + g);
					buff[cnt + channelMap[0]] = Clamp(vAvg + g);
					buff[cnt + channelMap[3]] = aAvg;
					yPos++; 
					cnt += channels;
					if (j%2) sampledPos++;
				}
				buff += pitch;
				if (wOdd) sampledPos++;

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

			const int yuvOffset16 = 1 << (UsedBitsPerChannel() - 1);
			const int shift = UsedBitsPerChannel() - 8;

			DataT* y = m_channel[0]; ASSERT(y);
			DataT* u = m_channel[1]; ASSERT(u);
			DataT* v = m_channel[2]; ASSERT(v);
			DataT* a = m_channel[3]; ASSERT(a);
			UINT16 g, aAvg;
			int cnt, channels;

			if (bpp%16 == 0) {
				UINT16 *buff16 = (UINT16 *)buff;
				int pitch16 = pitch/2;
				channels = bpp/16; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					if (i%2) sampledPos -= (w + 1)/2;
					cnt = 0;
					for (j=0; j < w; j++) {
						if (m_downsample) {
							// image was downsampled
							uAvg = u[sampledPos];
							vAvg = v[sampledPos];
							aAvg = Clamp16(a[sampledPos] + yuvOffset16);
						} else {
							uAvg = u[yPos];
							vAvg = v[yPos];
							aAvg = Clamp16(a[yPos] + yuvOffset16);
						}
						// Yuv
						buff16[cnt + channelMap[1]] = g = Clamp16(y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buff16[cnt + channelMap[2]] = Clamp16(uAvg + g);
						buff16[cnt + channelMap[0]] = Clamp16(vAvg + g);
						buff16[cnt + channelMap[3]] = aAvg;
						yPos++; 
						cnt += channels;
						if (j%2) sampledPos++;
					}
					buff16 += pitch16;
					if (wOdd) sampledPos++;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp%8 == 0);
				channels = bpp/8; ASSERT(channels >= m_header.channels);

				for (i=0; i < h; i++) {
					if (i%2) sampledPos -= (w + 1)/2;
					cnt = 0;
					for (j=0; j < w; j++) {
						if (m_downsample) {
							// image was downsampled
							uAvg = u[sampledPos];
							vAvg = v[sampledPos];
							aAvg = Clamp16(a[sampledPos] + yuvOffset16);
						} else {
							uAvg = u[yPos];
							vAvg = v[yPos];
							aAvg = Clamp16(a[yPos] + yuvOffset16);
						}
						// Yuv
						g = Clamp16(y[yPos] + yuvOffset16 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
						buff[cnt + channelMap[1]] = g >> shift; 
						buff[cnt + channelMap[2]] = Clamp16(uAvg + g) >> shift;
						buff[cnt + channelMap[0]] = Clamp16(vAvg + g) >> shift;
						buff[cnt + channelMap[3]] = aAvg >> shift;
						yPos++; 
						cnt += channels;
						if (j%2) sampledPos++;
					}
					buff += pitch;
					if (wOdd) sampledPos++;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;
		}
	case ImageModeGray31:
		{
			ASSERT(m_header.channels == 1);
			ASSERT(m_header.bpp == 32);

			const int yuvOffset31 = 1 << (UsedBitsPerChannel() - 1);
			const int shift = UsedBitsPerChannel() - 8;

			DataT* y = m_channel[0]; ASSERT(y);

			if (bpp == 32) {
				UINT32 *buff32 = (UINT32 *)buff;
				int pitch32 = pitch/4;

				for (i=0; i < h; i++) {
					for (j=0; j < w; j++) {
						buff32[j] = Clamp31(y[yPos++] + yuvOffset31);
					}
					buff32 += pitch32;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			} else {
				ASSERT(bpp == 8);
				
				for (i=0; i < h; i++) {
					for (j=0; j < w; j++) {
						buff[j] = Clamp31(y[yPos++] + yuvOffset31) >> shift;
					}
					buff += pitch;

					if (cb) {
						percent += dP;
						if ((*cb)(percent, true, data)) ReturnWithError(EscapePressed);
					}
				}
			}
			break;	
		}
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
			int cnt;

			for (i=0; i < h; i++) {
				cnt = 0;
				for (j=0; j < w; j++) {
					// Yuv
					uAvg = u[yPos];
					vAvg = v[yPos];
					yval = Clamp4(y[yPos++] + YUVoffset4 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
					if (j%2 == 0) {
						buff[cnt] = Clamp4(vAvg + yval) | (yval << 4);
						cnt++;
						buff[cnt] = Clamp4(uAvg + yval);
					} else {
						buff[cnt] |= Clamp4(vAvg + yval) << 4;
						cnt++;
						buff[cnt] = yval | (Clamp4(uAvg + yval) << 4);
						cnt++;
					}
				}
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
				for (j=0; j < w; j++) {
					// Yuv
					uAvg = u[yPos];
					vAvg = v[yPos];
					yval = Clamp6(y[yPos++] + YUVoffset6 - ((uAvg + vAvg ) >> 2)); // must be logical shift operator
					buff16[j] = (yval << 5) | ((Clamp6(uAvg + yval) >> 1) << 11) | (Clamp6(vAvg + yval) >> 1);
				}
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

#ifdef __PGFROISUPPORT__
	if (targetBuff) {
		// copy valid ROI (m_roi) from temporary buffer (roi) to target buffer
		if (bpp%8 == 0) {
			BYTE bypp = bpp/8;
			buff = buffStart + (levelRoi.top - roi.top)*pitch + (levelRoi.left - roi.left)*bypp;
			w = levelRoi.Width()*bypp;
			h = levelRoi.Height();

			for (i=0; i < h; i++) {
				for (j=0; j < w; j++) {
					targetBuff[j] = buff[j];
				}
				targetBuff += targetPitch;
				buff += pitch;
			}
		} else {
			// to do
		}

		delete[] buffStart;
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
void CPGFImage::GetYUV(int pitch, DataT* buff, BYTE bpp, int channelMap[] /*= NULL*/, CallbackPtr cb /*= NULL*/, void *data /*=NULL*/) const THROW_ {
	ASSERT(buff);
	const UINT32 w = m_width[0];
	const UINT32 h = m_height[0];
	const bool wOdd = (1 == w%2);
	const int bits = m_header.bpp/m_header.channels;
	const int dataBits = sizeof(DataT)*8; ASSERT(dataBits == 16 || dataBits == 32);
	const int pitch2 = pitch/sizeof(DataT);
	const int yuvOffset = (dataBits == 16) ? YUVoffset8 : YUVoffset16;
	const double dP = 1.0/h;
	
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);
	if (channelMap == NULL) channelMap = defMap;
	int sampledPos = 0, yPos = 0;
	DataT uAvg, vAvg;
	double percent = 0;
	UINT32 i, j;

	if (m_header.channels == 3) { 
		ASSERT(m_header.bpp == m_header.channels*bits);
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
					aAvg = Clamp(a[sampledPos] + yuvOffset);
				} else {
					uAvg = u[yPos];
					vAvg = v[yPos];
					aAvg = Clamp(a[yPos] + yuvOffset);
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
void CPGFImage::ImportYUV(int pitch, DataT *buff, BYTE bpp, int channelMap[] /*= NULL*/, CallbackPtr cb /*= NULL*/, void *data /*=NULL*/) THROW_ {
	ASSERT(buff);
	const double dP = 1.0/m_header.height;
	const int bits = m_header.bpp/m_header.channels;
	const int dataBits = sizeof(DataT)*8; ASSERT(dataBits == 16 || dataBits == 32);
	const int pitch2 = pitch/sizeof(DataT);
	const int yuvOffset = (dataBits == 16) ? YUVoffset8 : YUVoffset16;

	int yPos = 0, cnt = 0;
	double percent = 0;
	int defMap[] = { 0, 1, 2, 3, 4, 5, 6, 7 }; ASSERT(sizeof(defMap)/sizeof(defMap[0]) == MaxChannels);

	if (channelMap == NULL) channelMap = defMap;

	if (m_header.channels == 3)	{
		ASSERT(m_header.bpp == m_header.channels*bits);
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
		ASSERT(m_header.bpp == m_header.channels*bits);
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

