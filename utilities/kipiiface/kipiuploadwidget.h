/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select an image collection 
 *               to upload new items using digiKam album folder views
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KIPIUPLOADWIDGET_H
#define KIPIUPLOADWIDGET_H

// LibKIPI includes.

#include <libkipi/imagecollection.h>
#include <libkipi/uploadwidget.h>

// Local includes.

#include "albummanager.h"
#include "searchtextbar.h"

class QWidget;
class QTreeWidget;

namespace Digikam
{
class KipiInterface;
class KipiUploadWidgetPriv;

class KipiUploadWidget : public KIPI::UploadWidget
{
    Q_OBJECT

public:

    KipiUploadWidget(KipiInterface *iface, QWidget *parent=0);
    ~KipiUploadWidget();

    KIPI::ImageCollection selectedImageCollection() const;

    void setCurrentAlbumUrl(const KUrl& albumUrl);
    KUrl currentAlbumUrl() const;

private:

    void populateTreeView(const AlbumList& aList, QTreeWidget *view);

private slots: 

    void slotSearchTextChanged(const SearchTextSettings&);
    void slotNewAlbum();
    void slotAlbumAdded(Album* album);

private:

    KipiUploadWidgetPriv* const d;
};

}  // namespace Digikam

#endif  // KIPIUPLOADWIDGET_H
