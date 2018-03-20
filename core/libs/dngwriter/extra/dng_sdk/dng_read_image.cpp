/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_read_image.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_read_image.h"

#include "dng_bottlenecks.h"
#include "dng_exceptions.h"
#include "dng_host.h"
#include "dng_image.h"
#include "dng_ifd.h"
#include "dng_lossless_jpeg.h"
#include "dng_memory.h"
#include "dng_pixel_buffer.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_row_interleaved_image::dng_row_interleaved_image (dng_image &image,
													  uint32 factor)

	:	dng_image (image.Bounds    (),
				   image.Planes    (),
				   image.PixelType ())

	,	fImage  (image )
	,	fFactor (factor)

	{

	}

/*****************************************************************************/

int32 dng_row_interleaved_image::MapRow (int32 row) const
	{

	uint32 rows = Height ();

	int32 top = Bounds ().t;

	uint32 fieldRow = row - top;

	for (uint32 field = 0; true; field++)
		{

		uint32 fieldRows = (rows - field + fFactor - 1) / fFactor;

		if (fieldRow < fieldRows)
			{

			return fieldRow * fFactor + field + top;

			}

		fieldRow -= fieldRows;

		}

	ThrowProgramError ();

	return 0;

	}

/*****************************************************************************/

void dng_row_interleaved_image::DoGet (dng_pixel_buffer &buffer) const
	{

	dng_pixel_buffer tempBuffer (buffer);

	for (int32 row = buffer.fArea.t; row < buffer.fArea.b; row++)
		{

		tempBuffer.fArea.t = MapRow (row);

		tempBuffer.fArea.b = tempBuffer.fArea.t + 1;

		tempBuffer.fData = (void *) buffer.DirtyPixel (row,
										 			   buffer.fArea.l,
										 			   buffer.fPlane);

		fImage.Get (tempBuffer);

		}

	}

/*****************************************************************************/

void dng_row_interleaved_image::DoPut (const dng_pixel_buffer &buffer)
	{

	dng_pixel_buffer tempBuffer (buffer);

	for (int32 row = buffer.fArea.t; row < buffer.fArea.b; row++)
		{

		tempBuffer.fArea.t = MapRow (row);

		tempBuffer.fArea.b = tempBuffer.fArea.t + 1;

		tempBuffer.fData = (void *) buffer.ConstPixel (row,
										 			   buffer.fArea.l,
										 			   buffer.fPlane);

		fImage.Put (tempBuffer);

		}

	}

/*****************************************************************************/

static void ReorderSubTileBlocks (dng_host &host,
								  const dng_ifd &ifd,
								  dng_pixel_buffer &buffer,
								  AutoPtr<dng_memory_block> &tempBuffer)
	{

	uint32 tempBufferSize = buffer.fArea.W () *
							buffer.fArea.H () *
							buffer.fPlanes *
							buffer.fPixelSize;

	if (!tempBuffer.Get () || tempBuffer->LogicalSize () < tempBufferSize)
		{

		tempBuffer.Reset (host.Allocate (tempBufferSize));

		}

	uint32 blockRows = ifd.fSubTileBlockRows;
	uint32 blockCols = ifd.fSubTileBlockCols;

	uint32 rowBlocks = buffer.fArea.H () / blockRows;
	uint32 colBlocks = buffer.fArea.W () / blockCols;

	int32 rowStep = buffer.fRowStep * buffer.fPixelSize;
	int32 colStep = buffer.fColStep * buffer.fPixelSize;

	int32 rowBlockStep = rowStep * blockRows;
	int32 colBlockStep = colStep * blockCols;

	uint32 blockColBytes = blockCols * buffer.fPlanes * buffer.fPixelSize;

	const uint8 *s0 = (const uint8 *) buffer.fData;
	      uint8 *d0 = tempBuffer->Buffer_uint8 ();

	for (uint32 rowBlock = 0; rowBlock < rowBlocks; rowBlock++)
		{

		uint8 *d1 = d0;

		for (uint32 colBlock = 0; colBlock < colBlocks; colBlock++)
			{

			uint8 *d2 = d1;

			for (uint32 blockRow = 0; blockRow < blockRows; blockRow++)
				{

				for (uint32 j = 0; j < blockColBytes; j++)
					{

					d2 [j] = s0 [j];

					}

				s0 += blockColBytes;

				d2 += rowStep;

				}

			d1 += colBlockStep;

			}

		d0 += rowBlockStep;

		}

	// Copy back reordered pixels.

	DoCopyBytes (tempBuffer->Buffer (),
				 buffer.fData,
				 tempBufferSize);

	}

/*****************************************************************************/

class dng_image_spooler: public dng_spooler
	{

	private:

		dng_host &fHost;

		const dng_ifd &fIFD;

		dng_image &fImage;

		dng_rect fTileArea;

		uint32 fPlane;
		uint32 fPlanes;

		dng_memory_block &fBlock;

		AutoPtr<dng_memory_block> &fSubTileBuffer;

		dng_rect fTileStrip;

		uint8 *fBuffer;

		uint32 fBufferCount;
		uint32 fBufferSize;

	public:

		dng_image_spooler (dng_host &host,
						   const dng_ifd &ifd,
						   dng_image &image,
						   const dng_rect &tileArea,
						   uint32 plane,
						   uint32 planes,
						   dng_memory_block &block,
						   AutoPtr<dng_memory_block> &subTileBuffer);

		virtual ~dng_image_spooler ();

		virtual void Spool (const void *data,
							uint32 count);

	private:

		// Hidden copy constructor and assignment operator.

		dng_image_spooler (const dng_image_spooler &spooler);

		dng_image_spooler & operator= (const dng_image_spooler &spooler);

	};

/*****************************************************************************/

dng_image_spooler::dng_image_spooler (dng_host &host,
									  const dng_ifd &ifd,
									  dng_image &image,
									  const dng_rect &tileArea,
									  uint32 plane,
									  uint32 planes,
									  dng_memory_block &block,
									  AutoPtr<dng_memory_block> &subTileBuffer)

	:	fHost (host)
	,	fIFD (ifd)
	,	fImage (image)
	,	fTileArea (tileArea)
	,	fPlane (plane)
	,	fPlanes (planes)
	,	fBlock (block)
	,	fSubTileBuffer (subTileBuffer)

	,	fTileStrip ()
	,	fBuffer (NULL)
	,	fBufferCount (0)
	,	fBufferSize (0)

	{

	uint32 bytesPerRow = fTileArea.W () * fPlanes * sizeof (uint16);

	uint32 stripLength = Pin_uint32 (ifd.fSubTileBlockRows,
									 fBlock.LogicalSize () / bytesPerRow,
									 fTileArea.H ());

	stripLength = stripLength / ifd.fSubTileBlockRows
							  * ifd.fSubTileBlockRows;

	fTileStrip   = fTileArea;
	fTileStrip.b = fTileArea.t + stripLength;

	fBuffer = (uint8 *) fBlock.Buffer ();

	fBufferCount = 0;
	fBufferSize  = bytesPerRow * stripLength;

	}

/*****************************************************************************/

dng_image_spooler::~dng_image_spooler ()
	{

	}

/*****************************************************************************/

void dng_image_spooler::Spool (const void *data,
							   uint32 count)
	{

	while (count)
		{

		uint32 block = Min_uint32 (count, fBufferSize - fBufferCount);

		if (block == 0)
			{
			return;
			}

		DoCopyBytes (data,
					 fBuffer + fBufferCount,
				     block);

		data = ((const uint8 *) data) + block;

		count -= block;

		fBufferCount += block;

		if (fBufferCount == fBufferSize)
			{

			fHost.SniffForAbort ();

			dng_pixel_buffer buffer;

			buffer.fArea = fTileStrip;

			buffer.fPlane  = fPlane;
			buffer.fPlanes = fPlanes;

			buffer.fRowStep   = fPlanes * fTileStrip.W ();
			buffer.fColStep   = fPlanes;
			buffer.fPlaneStep = 1;

			buffer.fData = fBuffer;

			buffer.fPixelType = ttShort;
			buffer.fPixelSize = 2;

			if (fIFD.fSubTileBlockRows > 1)
				{

				ReorderSubTileBlocks (fHost,
									  fIFD,
									  buffer,
									  fSubTileBuffer);

				}

			fImage.Put (buffer);

			uint32 stripLength = fTileStrip.H ();

			fTileStrip.t = fTileStrip.b;

			fTileStrip.b = Min_int32 (fTileStrip.t + stripLength,
									  fTileArea.b);

			fBufferCount = 0;

			fBufferSize = fTileStrip.W () *
						  fTileStrip.H () *
						  fPlanes * sizeof (uint16);

			}

		}

	}

/*****************************************************************************/

dng_read_image::dng_read_image ()

	:	fCompressedBuffer   ()
	,	fUncompressedBuffer ()
	,	fSubTileBlockBuffer ()

	{

	}

/*****************************************************************************/

dng_read_image::~dng_read_image ()
	{

	}

/*****************************************************************************/

bool dng_read_image::ReadUncompressed (dng_host &host,
									   const dng_ifd &ifd,
									   dng_stream &stream,
									   dng_image &image,
									   const dng_rect &tileArea,
									   uint32 plane,
									   uint32 planes)
	{

	uint32 rows          = tileArea.H ();
	uint32 samplesPerRow = tileArea.W ();

	if (ifd.fPlanarConfiguration == pcRowInterleaved)
		{
		rows *= planes;
		}
	else
		{
		samplesPerRow *= planes;
		}

	uint32 samplesPerTile = samplesPerRow * rows;

	dng_pixel_buffer buffer;

	buffer.fArea = tileArea;

	buffer.fPlane  = plane;
	buffer.fPlanes = planes;

	buffer.fRowStep = planes * tileArea.W ();

	if (ifd.fPlanarConfiguration == pcRowInterleaved)
		{
		buffer.fColStep   = 1;
		buffer.fPlaneStep = tileArea.W ();
		}

	else
		{
		buffer.fColStep   = planes;
		buffer.fPlaneStep = 1;
		}

	buffer.fData = fUncompressedBuffer->Buffer ();

	uint32 bitDepth = ifd.fBitsPerSample [plane];

	if (bitDepth == 8)
		{

		buffer.fPixelType = ttByte;
		buffer.fPixelSize = 1;

		stream.Get (buffer.fData, samplesPerTile);

		}

	else if (bitDepth == 16)
		{

		buffer.fPixelType = ttShort;
		buffer.fPixelSize = 2;

		stream.Get (buffer.fData, samplesPerTile * 2);

		if (stream.SwapBytes ())
			{

			DoSwapBytes16 ((uint16 *) buffer.fData,
						   samplesPerTile);

			}

		}

	else if (bitDepth == 32)
		{

		buffer.fPixelType = ttLong;
		buffer.fPixelSize = 4;

		stream.Get (buffer.fData, samplesPerTile * 4);

		if (stream.SwapBytes ())
			{

			DoSwapBytes32 ((uint32 *) buffer.fData,
						   samplesPerTile);

			}

		}

	else if (bitDepth == 12)
		{

		buffer.fPixelType = ttShort;
		buffer.fPixelSize = 2;

		uint16 *p = (uint16 *) buffer.fData;

		uint32 evenSamples = samplesPerRow >> 1;

		for (uint32 row = 0; row < rows; row++)
			{

			for (uint32 j = 0; j < evenSamples; j++)
				{

				uint32 b0 = stream.Get_uint8 ();
				uint32 b1 = stream.Get_uint8 ();
				uint32 b2 = stream.Get_uint8 ();

				p [0] = (uint16) ((b0 << 4) | (b1 >> 4));
				p [1] = (uint16) (((b1 << 8) | b2) & 0x0FFF);

				p += 2;

				}

			if (samplesPerRow & 1)
				{

				uint32 b0 = stream.Get_uint8 ();
				uint32 b1 = stream.Get_uint8 ();

				p [0] = (uint16) ((b0 << 4) | (b1 >> 4));

				p += 1;

				}

			}

		}

	else if (bitDepth > 8 && bitDepth < 16)
		{

		buffer.fPixelType = ttShort;
		buffer.fPixelSize = 2;

		uint16 *p = (uint16 *) buffer.fData;

		uint32 bitMask = (1 << bitDepth) - 1;

		for (uint32 row = 0; row < rows; row++)
			{

			uint32 bitBuffer  = 0;
			uint32 bufferBits = 0;

			for (uint32 j = 0; j < samplesPerRow; j++)
				{

				while (bufferBits < bitDepth)
					{

					bitBuffer = (bitBuffer << 8) | stream.Get_uint8 ();

					bufferBits += 8;

					}

				p [j] = (uint16) ((bitBuffer >> (bufferBits - bitDepth)) & bitMask);

				bufferBits -= bitDepth;

				}

			p += samplesPerRow;

			}

		}

	else if (bitDepth > 16 && bitDepth < 32)
		{

		buffer.fPixelType = ttLong;
		buffer.fPixelSize = 4;

		uint32 *p = (uint32 *) buffer.fData;

		uint32 bitMask = (1 << bitDepth) - 1;

		for (uint32 row = 0; row < rows; row++)
			{

			uint64 bitBuffer  = 0;
			uint32 bufferBits = 0;

			for (uint32 j = 0; j < samplesPerRow; j++)
				{

				while (bufferBits < bitDepth)
					{

					bitBuffer = (bitBuffer << 8) | stream.Get_uint8 ();

					bufferBits += 8;

					}

				p [j] = ((uint32) (bitBuffer >> (bufferBits - bitDepth))) & bitMask;

				bufferBits -= bitDepth;

				}

			p += samplesPerRow;

			}

		}

	else
		{

		return false;

		}

	if (ifd.fSampleBitShift)
		{

		buffer.ShiftRight (ifd.fSampleBitShift);

		}

	if (ifd.fSubTileBlockRows > 1)
		{

		ReorderSubTileBlocks (host,
							  ifd,
							  buffer,
							  fSubTileBlockBuffer);

		}

	image.Put (buffer);

	return true;

	}

/*****************************************************************************/

bool dng_read_image::ReadBaselineJPEG (dng_host & /* host */,
									   const dng_ifd & /* ifd */,
									   dng_stream & /* stream */,
									   dng_image & /* image */,
									   const dng_rect & /* tileArea */,
									   uint32 /* plane */,
									   uint32 /* planes */,
									   uint32 /* tileByteCount */)
	{

	// The dng_sdk does not include a baseline JPEG decoder.  Override this
	// this method to add baseline JPEG support.

	return false;

	}

/*****************************************************************************/

bool dng_read_image::ReadLosslessJPEG (dng_host &host,
									   const dng_ifd &ifd,
									   dng_stream &stream,
									   dng_image &image,
									   const dng_rect &tileArea,
									   uint32 plane,
									   uint32 planes,
									   uint32 tileByteCount)
	{

	if (fUncompressedBuffer.Get () == NULL)
		{

		uint32 bytesPerRow = tileArea.W () * planes * sizeof (uint16);

		uint32 rowsPerStrip = Pin_uint32 (ifd.fSubTileBlockRows,
										  kImageBufferSize / bytesPerRow,
										  tileArea.H ());

		rowsPerStrip = rowsPerStrip / ifd.fSubTileBlockRows
									* ifd.fSubTileBlockRows;

		uint32 bufferSize = bytesPerRow * rowsPerStrip;

		fUncompressedBuffer.Reset (host.Allocate (bufferSize));

		}

	dng_image_spooler spooler (host,
							   ifd,
							   image,
							   tileArea,
							   plane,
							   planes,
							   *fUncompressedBuffer.Get (),
							   fSubTileBlockBuffer);

	uint32 decodedSize = tileArea.W () *
						 tileArea.H () *
						 planes * sizeof (uint16);

	bool bug16 = ifd.fLosslessJPEGBug16;

	uint64 tileOffset = stream.Position ();

	DecodeLosslessJPEG (stream,
					    spooler,
					    decodedSize,
					    decodedSize,
						bug16);

	if (stream.Position () > tileOffset + tileByteCount)
		{
		ThrowBadFormat ();
		}

	return true;

	}

/*****************************************************************************/

bool dng_read_image::CanReadTile (const dng_ifd &ifd)
	{

	if (ifd.fSampleFormat [0] != sfUnsignedInteger)
		{
		return false;
		}

	switch (ifd.fCompression)
		{

		case ccUncompressed:
			{

			return ifd.fBitsPerSample [0] >= 8 &&
				   ifd.fBitsPerSample [0] <= 32;

			}

		case ccJPEG:
			{

			if (ifd.IsBaselineJPEG ())
				{

				// Baseline JPEG.

				return false;

				}

			else
				{

				// Lossless JPEG.

				return ifd.fBitsPerSample [0] >= 8 &&
					   ifd.fBitsPerSample [0] <= 16;

				}

			break;

			}

		default:
			{
			break;
			}

		}

	return false;

	}

/*****************************************************************************/

bool dng_read_image::NeedsCompressedBuffer (const dng_ifd & /* ifd */)
	{

	return false;

	}

/*****************************************************************************/

void dng_read_image::ReadTile (dng_host &host,
						       const dng_ifd &ifd,
						       dng_stream &stream,
						       dng_image &image,
						       const dng_rect &tileArea,
						       uint32 plane,
						       uint32 planes,
						       uint32 tileByteCount)
	{

	switch (ifd.fCompression)
		{

		case ccUncompressed:
			{

			if (ReadUncompressed (host,
								  ifd,
								  stream,
								  image,
								  tileArea,
								  plane,
								  planes))
				{

				return;

				}

			break;

			}

		case ccJPEG:
			{

			if (ifd.IsBaselineJPEG ())
				{

				// Baseline JPEG.

				if (ReadBaselineJPEG (host,
									  ifd,
									  stream,
									  image,
									  tileArea,
									  plane,
									  planes,
									  tileByteCount))
					{

					return;

					}

				}

			else
				{

				// Otherwise is should be lossless JPEG.

				if (ReadLosslessJPEG (host,
									  ifd,
									  stream,
									  image,
									  tileArea,
									  plane,
									  planes,
									  tileByteCount))
					{

					return;

					}

				}

			break;

			}

		default:
			break;

		}

	ThrowBadFormat ();

	}

/*****************************************************************************/

bool dng_read_image::CanRead (const dng_ifd &ifd)
	{

	if (ifd.fImageWidth  < 1 ||
		ifd.fImageLength < 1)
		{
		return false;
		}

	if (ifd.fSamplesPerPixel < 1)
		{
		return false;
		}

	if (ifd.fBitsPerSample [0] < 1)
		{
		return false;
		}

	for (uint32 j = 1; j < Min_uint32 (ifd.fSamplesPerPixel,
									   kMaxSamplesPerPixel); j++)
		{

		if (ifd.fBitsPerSample [j] !=
			ifd.fBitsPerSample [0])
			{
			return false;
			}

		if (ifd.fSampleFormat [j] !=
			ifd.fSampleFormat [0])
			{
			return false;
			}

		}

	if ((ifd.fPlanarConfiguration != pcInterleaved   ) &&
		(ifd.fPlanarConfiguration != pcPlanar        ) &&
		(ifd.fPlanarConfiguration != pcRowInterleaved))
		{
		return false;
		}

	if (ifd.fUsesStrips == ifd.fUsesTiles)
		{
		return false;
		}

	uint32 tileCount = ifd.TilesPerImage ();

	if (tileCount < 1)
		{
		return false;
		}

	bool needTileByteCounts = (ifd.TileByteCount (ifd.TileArea (0, 0)) == 0);

	if (tileCount == 1)
		{

		if (needTileByteCounts)
			{

			if (ifd.fTileByteCount [0] < 1)
				{
				return false;
				}

			}

		}

	else
		{

		if (ifd.fTileOffsetsCount != tileCount)
			{
			return false;
			}

		if (needTileByteCounts)
			{

			if (ifd.fTileByteCountsCount != tileCount)
				{
				return false;
				}

			}

		}

	if (!CanReadTile (ifd))
		{
		return false;
		}

	return true;

	}

/*****************************************************************************/

void dng_read_image::Read (dng_host &host,
						   const dng_ifd &ifd,
						   dng_stream &stream,
						   dng_image &image)
	{

	uint32 tileIndex;

	// Deal with row interleaved images.

	if (ifd.fRowInterleaveFactor > 1 &&
		ifd.fRowInterleaveFactor < ifd.fImageLength)
		{

		dng_ifd tempIFD (ifd);

		tempIFD.fRowInterleaveFactor = 1;

		dng_row_interleaved_image tempImage (image,
											 ifd.fRowInterleaveFactor);

		Read (host,
			  tempIFD,
			  stream,
			  tempImage);

		return;

		}

	// Figure out inner and outer samples.

	uint32 innerSamples = 1;
	uint32 outerSamples = 1;

	if (ifd.fPlanarConfiguration == pcPlanar)
		{
		outerSamples = ifd.fSamplesPerPixel;
		}
	else
		{
		innerSamples = ifd.fSamplesPerPixel;
		}

	// Calculate number of tiles to read.

	uint32 tilesAcross = ifd.TilesAcross ();
	uint32 tilesDown   = ifd.TilesDown   ();

	uint32 tileCount = tilesAcross * tilesDown * outerSamples;

	// Find the tile offsets.

	dng_memory_data tileOffsetData (tileCount * sizeof (uint64));

	uint64 *tileOffset = tileOffsetData.Buffer_uint64 ();

	if (tileCount <= dng_ifd::kMaxTileInfo)
		{

		for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
			{

			tileOffset [tileIndex] = ifd.fTileOffset [tileIndex];

			}

		}

	else
		{

		stream.SetReadPosition (ifd.fTileOffsetsOffset);

		for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
			{

			tileOffset [tileIndex] = stream.TagValue_uint32 (ifd.fTileOffsetsType);

			}

		}

	// Quick validity check on tile offsets.

	for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
		{

		#if qDNGValidate

		if (tileOffset [tileIndex] < 8)
			{

			ReportWarning ("Tile/Strip offset less than 8");

			}

		#endif

		if (tileOffset [tileIndex] >= stream.Length ())
			{

			ThrowBadFormat ();

			}

		}

	// Buffer to hold the tile byte counts, if needed.

	dng_memory_data tileByteCountData;

	uint32 *tileByteCount = NULL;

	// If we can compute the number of bytes needed to store the
	// data, we can split the read for each tile into sub-tiles.

	uint32 subTileLength = ifd.fTileLength;

	if (ifd.TileByteCount (ifd.TileArea (0, 0)) != 0)
		{

		uint32 bytesPerPixel = TagTypeSize (ifd.PixelType ());

		uint32 bytesPerRow = ifd.fTileWidth * innerSamples * bytesPerPixel;

		subTileLength = Pin_uint32 (ifd.fSubTileBlockRows,
									kImageBufferSize / bytesPerRow,
									ifd.fTileLength);

		subTileLength = subTileLength / ifd.fSubTileBlockRows
									  * ifd.fSubTileBlockRows;

		fUncompressedBuffer.Reset (host.Allocate (subTileLength * bytesPerRow));

		}

	// Else we need to know the byte counts.

	else
		{

		tileByteCountData.Allocate (tileCount * sizeof (uint32));

		tileByteCount = tileByteCountData.Buffer_uint32 ();

		if (tileCount <= dng_ifd::kMaxTileInfo)
			{

			for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
				{

				tileByteCount [tileIndex] = ifd.fTileByteCount [tileIndex];

				}

			}

		else
			{

			stream.SetReadPosition (ifd.fTileByteCountsOffset);

			for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
				{

				tileByteCount [tileIndex] = stream.TagValue_uint32 (ifd.fTileByteCountsType);

				}

			}

		// Quick validity check on tile byte counts.

		for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
			{

			if (tileByteCount [tileIndex] < 1 ||
				tileByteCount [tileIndex] > stream.Length ())
				{

				ThrowBadFormat ();

				}

			}

		}

	// See if we need to allocate the compressed tile data buffer.

	if (tileByteCount && NeedsCompressedBuffer (ifd))
		{

		// Find maximum compressed tile size.

		uint32 maxTileByteCount = 0;

		for (tileIndex = 0; tileIndex < tileCount; tileIndex++)
			{

			maxTileByteCount = Max_uint32 (maxTileByteCount,
										   tileByteCount [tileIndex]);

			}

		// Allocate buffer that size.

		if (maxTileByteCount)
			{

			fCompressedBuffer.Reset (host.Allocate (maxTileByteCount));

			}

		}

	// Now read in each tile.

	tileIndex = 0;

	for (uint32 plane = 0; plane < outerSamples; plane++)
		{

		if (plane >= image.Planes ())
			{
			return;		// Don't waste time reading planes we are not saving.
			}

		for (uint32 rowIndex = 0; rowIndex < tilesDown; rowIndex++)
			{

			for (uint32 colIndex = 0; colIndex < tilesAcross; colIndex++)
				{

				stream.SetReadPosition (tileOffset [tileIndex]);

				dng_rect tileArea = ifd.TileArea (rowIndex, colIndex);

				uint32 subTileCount = (tileArea.H () + subTileLength - 1) /
									  subTileLength;

				for (uint32 subIndex = 0; subIndex < subTileCount; subIndex++)
					{

					host.SniffForAbort ();

					dng_rect subArea (tileArea);

					subArea.t = tileArea.t + subIndex * subTileLength;

					subArea.b = Min_int32 (subArea.t + subTileLength,
										   tileArea.b);

					uint32 subByteCount;

					if (tileByteCount)
						{
						subByteCount = tileByteCount [tileIndex];
						}
					else
						{
						subByteCount = ifd.TileByteCount (subArea);
						}

					ReadTile (host,
							  ifd,
							  stream,
							  image,
							  subArea,
							  plane,
							  innerSamples,
							  subByteCount);

					}

				tileIndex++;

				}

			}

		}

	}

/*****************************************************************************/
