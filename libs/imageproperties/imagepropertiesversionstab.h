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

class ImagePropertiesVersionsTab : public KTabWidget
{
    Q_OBJECT

public:

    ImagePropertiesVersionsTab(QWidget* parent);
    ~ImagePropertiesVersionsTab();

    void setupVersionsData() const;
    void setupFiltersData() const;
    int findImagePositionInList(qlonglong id) const;
    bool hasImage(qlonglong id) const;

    void clear();
    void setItem(const ImageInfo& info, const DImageHistory& history);

public Q_SLOTS:

    void slotNewVersionSelected(KUrl url);
    void showCustomContextMenu(const QPoint& position);
    void setEnabledEntries(int count);

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
