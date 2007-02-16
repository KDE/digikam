/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at gmail dot com>
 *         Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2004-11-17
 * Description : image properties side bar using data from 
 *               digiKam database.
 *
 * Copyright 2004-2006 by Gilles Caulier
 * Copyright 2007 by Gilles Caulier and Marcel Wiesweg
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
 
#ifndef IMAGEPROPERTIESSIDEBARDB_H
#define IMAGEPROPERTIESSIDEBARDB_H

// Qt includes.

#include <qptrlist.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "imagepropertiessidebar.h"
#include "digikam_export.h"

class QSplitter;
class QWidget;
class QRect;

namespace Digikam
{

class DImg;
class AlbumIconView;
class AlbumIconItem;
class ImageInfo;
class NavigateBarTab;
class ImagePropertiesSideBarDBPriv;

class DIGIKAM_EXPORT ImagePropertiesSideBarDB : public ImagePropertiesSideBar
{
    Q_OBJECT

public:

    ImagePropertiesSideBarDB(QWidget* parent, const char *name, QSplitter *splitter, Side side=Left,
                             bool mimimizedDefault=false);

    ~ImagePropertiesSideBarDB();

    virtual void itemChanged(const KURL& url, const QRect &rect = QRect(), DImg *img = 0);

    virtual void itemChanged(ImageInfo *info, const QRect &rect = QRect(), DImg *img = 0);
    virtual void itemChanged(QPtrList<ImageInfo> infos);

    void takeImageInfoOwnership(bool takeOwnership);

    void populateTags(void);

signals:

    void signalFirstItem(void);
    void signalPrevItem(void);
    void signalNextItem(void);
    void signalLastItem(void);
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);

public slots:

    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();

    virtual void slotNoCurrentItem(void);

private slots:

    void slotChangedTab(QWidget* tab);
    void slotThemeChanged();
    void slotFileMetadataChanged(const KURL &url);

private:

    void itemChanged(const KURL& url, ImageInfo *info,
                     const QRect &rect, DImg *img);
    void itemChanged(QPtrList<ImageInfo> infos, const QRect &rect, DImg *img);

private:

    ImagePropertiesSideBarDBPriv* d;
};

}  // NameSpace Digikam

#endif  // IMAGEPROPERTIESSIDEBARDB_H
