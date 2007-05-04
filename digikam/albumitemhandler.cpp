/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-10-15
 * Description : album item handler.
 * 
 * Copyright (C) 2003 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Local includes.

#include "albummanager.h"
#include "albumitemhandler.h"

namespace Digikam
{

AlbumItemHandler::AlbumItemHandler()
{
}

AlbumItemHandler::~AlbumItemHandler()
{
}

void AlbumItemHandler::emitItemsSelected(bool val)
{
    AlbumManager::instance()->emitAlbumItemsSelected(val);
}

}  // namespace Digikam
