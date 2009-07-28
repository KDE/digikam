/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GPSSEARCHVIEW_H
#define GPSSEARCHVIEW_H

// Qt includes

#include <QWidget>

class QDragEnterEvent;
class QDropEvent;
class QPixmap;

namespace Digikam
{

class SAlbum;
class ImageInfo;
class ImageInfoList;
class SearchTextBar;
class GPSSearchFolderView;
class GPSSearchViewPriv;

class GPSSearchView : public QWidget
{
    Q_OBJECT

public:

    GPSSearchView(QWidget *parent=0);
    ~GPSSearchView();

    GPSSearchFolderView* folderView() const;
    SearchTextBar* searchBar() const;

    void setActive(bool val);

private:

    void readConfig();
    void writeConfig();

    bool checkName(QString& name);
    bool checkAlbum(const QString& name) const;

    void createNewGPSSearchAlbum(const QString& name);

private Q_SLOTS:

    void slotAlbumSelected(SAlbum*);
    void slotRenameAlbum(SAlbum*);

    void slotSaveGPSSAlbum();
    void slotCheckNameEditGPSConditions();

    void slotSelectionChanged();

    void slotItemsInfo(const ImageInfoList&);

private:

    GPSSearchViewPriv* const d;
};

}  // namespace Digikam

#endif /* FUZZYSEARCHVIEW_H */
