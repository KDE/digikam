/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : image properties side bar using data from 
 *               digiKam database.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// KDE includes

#include <kurl.h>

// Local includes

#include "imageinfolist.h"
#include "imagepropertiessidebar.h"
#include "digikam_export.h"

class QWidget;
class QRect;

namespace Digikam
{

class DImg;
class SidebarSplitter;
class ImageInfo;
class ImageChangeset;
class ImagePropertiesSideBarDBPriv;

class ImagePropertiesSideBarDB : public ImagePropertiesSideBar
{
    Q_OBJECT

public:

    ImagePropertiesSideBarDB(QWidget* parent, SidebarSplitter *splitter,
                             KMultiTabBarPosition side=KMultiTabBar::Left,
                             bool mimimizedDefault=false);

    ~ImagePropertiesSideBarDB();

    virtual void itemChanged(const KUrl& url, const QRect &rect = QRect(), DImg *img = 0);

    virtual void itemChanged(const ImageInfo &info, const QRect &rect = QRect(), DImg *img = 0);
    virtual void itemChanged(const ImageInfoList &infos);

    void populateTags(void);
    void refreshTagsView();

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);

public Q_SLOTS:

    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();

    virtual void slotNoCurrentItem();

private Q_SLOTS:

    void slotChangedTab(QWidget* tab);
    void slotFileMetadataChanged(const KUrl &url);
    void slotImageChangeDatabase(const ImageChangeset &changeset);

private:

    void itemChanged(const KUrl& url, const ImageInfo &info, const QRect &rect, DImg *img);
    void itemChanged(const ImageInfoList infos, const QRect &rect, DImg *img);
    void setImagePropertiesInformation(const KUrl& url);

private:

    ImagePropertiesSideBarDBPriv* const d;
};

}  // namespace Digikam

#endif  // IMAGEPROPERTIESSIDEBARDB_H
