/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-27
 * Description : a folder view for date albums.
 *
 * Copyright (C)      2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef DATEFOLDERVIEW_H
#define DATEFOLDERVIEW_H

// KDE includes

#include <kvbox.h>

// Local includes

#include "albummanager.h"
#include "statesavingobject.h"

namespace Digikam
{

class Album;
class DAlbum;
class DateAlbumModel;
class ImageFilterModel;

template <class T>
class AlbumPointer;

class DateFolderView: public KVBox, public StateSavingObject
{
    Q_OBJECT

public:

    DateFolderView(QWidget* parent, DateAlbumModel* dateAlbumModel);
    ~DateFolderView();

    void setImageModel(ImageFilterModel* model);

    void setActive(bool val);

    void gotoDate(const QDate& dt);

    void changeAlbumFromHistory(DAlbum* album);

    AlbumPointer<DAlbum> currentAlbum() const;

    virtual void setConfigGroup(const KConfigGroup& group);

    void doLoadState();
    void doSaveState();

private Q_SLOTS:

    void slotSelectionChanged(Album* selectedAlbum);
    void slotAllAlbumsLoaded();

private:

    class DateFolderViewPriv;
    DateFolderViewPriv* const d;
};

} // namespace Digikam

#endif /* DATEFOLDERVIEW_H */
