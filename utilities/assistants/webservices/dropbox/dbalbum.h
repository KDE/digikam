/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DBALBUM_H
#define DBALBUM_H

// Local includes

#include "newalbumdialog.h"

namespace Digikam
{

class DBFolder;

class DBNewAlbum : public NewAlbumDialog
{
    Q_OBJECT

public:

    explicit DBNewAlbum(QWidget* const parent, const QString& pluginName);
    ~DBNewAlbum();

    void getFolderTitle(DBFolder& folder);
};

} //namespace Digikam

#endif /*DBALBUM_H*/
