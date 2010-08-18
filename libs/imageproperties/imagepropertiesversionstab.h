/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying image versions
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGEPROPERTIESVERSIONSTAB_H
#define IMAGEPROPERTIESVERSIONSTAB_H

// Qt includes

#include <QWidget>
#include <QModelIndex>
#include <QPoint>

// KDE includes

#include <KUrl>
#include <KTabWidget>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"

namespace Digikam 
{

class ImageInfoList;
class ImageInfo;

class DIGIKAM_EXPORT ImagePropertiesVersionsTab : public KTabWidget
{
    Q_OBJECT

public:

    ImagePropertiesVersionsTab(QWidget* parent);
    ~ImagePropertiesVersionsTab();

    void setupVersionsData() const;
    void setupFiltersData() const;
    int findImagePositionInList(qlonglong id) const;
    bool hasImage(qlonglong id) const;

//    void setCurrentURL(const KUrl& url = KUrl());
//    void setImageHistory(const DImageHistory& history);

public Q_SLOTS:

    //versions tab slots
    void slotDigikamViewNoCurrentItem();
    void slotDigikamViewImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList &allImages) const;
    void slotUpdateImageInfo(const ImageInfo& info);
    void slotViewItemSelected(QModelIndex index);
    void slotNewVersionSelected(KUrl url);

    //filters history tab slots
    void showCustomContextMenu(const QPoint& position);
    void setModelData(const QList<DImageHistory::Entry>& entries);
    void disableEntries(int count);
    void enableEntries(int count);

Q_SIGNALS:

    void setCurrentUrlSignal(const KUrl& url);
    void updateMainViewSignal();
    void setCurrentIdSignal(qlonglong id);

private:

    class ImagePropertiesVersionsTabPriv;
    ImagePropertiesVersionsTabPriv* const d;
};

} // namespace Digikam

#endif // IMAGEPROPERTIESVERSIONSTAB_H
