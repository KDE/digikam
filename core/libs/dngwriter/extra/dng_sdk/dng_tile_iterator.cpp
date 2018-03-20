/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_tile_iterator.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_tile_iterator.h"

#include "dng_exceptions.h"
#include "dng_image.h"
#include "dng_pixel_buffer.h"
#include "dng_tag_types.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_tile_iterator::dng_tile_iterator (const dng_image &image,
									  const dng_rect &area)

	:	fArea           ()
	,	fTileWidth      (0)
	,	fTileHeight     (0)
	,	fTileTop        (0)
	,	fTileLeft       (0)
	,	fRowLeft        (0)
	,	fLeftPage       (0)
	,	fRightPage      (0)
	,	fTopPage        (0)
	,	fBottomPage     (0)
	,	fHorizontalPage (0)
	,	fVerticalPage   (0)

	{

	Initialize (image.RepeatingTile (),
				area & image.Bounds ());

	}

/*****************************************************************************/

dng_tile_iterator::dng_tile_iterator (const dng_point &tileSize,
						   			  const dng_rect &area)

	:	fArea           ()
	,	fTileWidth      (0)
	,	fTileHeight     (0)
	,	fTileTop        (0)
	,	fTileLeft       (0)
	,	fRowLeft        (0)
	,	fLeftPage       (0)
	,	fRightPage      (0)
	,	fTopPage        (0)
	,	fBottomPage     (0)
	,	fHorizontalPage (0)
	,	fVerticalPage   (0)

	{

	dng_rect tile (area);

	tile.b = Min_int32 (tile.b, tile.t + tileSize.v);
	tile.r = Min_int32 (tile.r, tile.l + tileSize.h);

	Initialize (tile,
				area);

	}

/*****************************************************************************/

dng_tile_iterator::dng_tile_iterator (const dng_rect &tile,
						   			  const dng_rect &area)

	:	fArea           ()
	,	fTileWidth      (0)
	,	fTileHeight     (0)
	,	fTileTop        (0)
	,	fTileLeft       (0)
	,	fRowLeft        (0)
	,	fLeftPage       (0)
	,	fRightPage      (0)
	,	fTopPage        (0)
	,	fBottomPage     (0)
	,	fHorizontalPage (0)
	,	fVerticalPage   (0)

	{

	Initialize (tile,
				area);

	}

/*****************************************************************************/

void dng_tile_iterator::Initialize (const dng_rect &tile,
						 			const dng_rect &area)
	{

	fArea = area;

	if (area.IsEmpty ())
		{

		fVerticalPage =  0;
		fBottomPage   = -1;

		return;

		}

	int32 vOffset = tile.t;
	int32 hOffset = tile.l;

	int32 tileHeight = tile.b - vOffset;
	int32 tileWidth  = tile.r - hOffset;

	fTileHeight = tileHeight;
	fTileWidth  = tileWidth;

	fLeftPage  = (fArea.l - hOffset    ) / tileWidth;
	fRightPage = (fArea.r - hOffset - 1) / tileWidth;

	fHorizontalPage = fLeftPage;

	fTopPage    = (fArea.t - vOffset    ) / tileHeight;
	fBottomPage = (fArea.b - vOffset - 1) / tileHeight;

	fVerticalPage = fTopPage;

	fTileLeft = fHorizontalPage * tileWidth  + hOffset;
	fTileTop  = fVerticalPage   * tileHeight + vOffset;

	fRowLeft = fTileLeft;

	}

/*****************************************************************************/

bool dng_tile_iterator::GetOneTile (dng_rect &tile)
	{

	if (fVerticalPage > fBottomPage)
		{
		return false;
		}

	if (fVerticalPage > fTopPage)
		tile.t = fTileTop;
	else
		tile.t = fArea.t;

	if (fVerticalPage < fBottomPage)
		tile.b = fTileTop + fTileHeight;
	else
		tile.b = fArea.b;

	if (fHorizontalPage > fLeftPage)
		tile.l = fTileLeft;
	else
		tile.l = fArea.l;

	if (fHorizontalPage < fRightPage)
		tile.r = fTileLeft + fTileWidth;
	else
		tile.r = fArea.r;

	if (fHorizontalPage < fRightPage)
		{
		fHorizontalPage++;
		fTileLeft += fTileWidth;
		}

	else
		{

		fVerticalPage++;
		fTileTop += fTileHeight;

		fHorizontalPage = fLeftPage;
		fTileLeft = fRowLeft;

		}

	return true;

	}

/*****************************************************************************/
