/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying image versions
 *
 * Copyright (C) 2010-2012 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGE_PROPERTIES_VERSIONS_TAB_H
#define IMAGE_PROPERTIES_VERSIONS_TAB_H

// Qt includes

#include <QWidget>
#include <QModelIndex>
#include <QPoint>
#include <QTabWidget>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"

class KConfigGroup;

namespace Digikam
{

class FiltersHistoryWidget;
class ImageInfo;
class ImageModel;
class VersionsWidget;

class ImagePropertiesVersionsTab : public QTabWidget
{
    Q_OBJECT

public:

    explicit ImagePropertiesVersionsTab(QWidget* const parent);
    ~ImagePropertiesVersionsTab();

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void clear();
    void setItem(const ImageInfo& info, const DImageHistory& history);

    VersionsWidget* versionsWidget()             const;
    FiltersHistoryWidget* filtersHistoryWidget() const;

    void addShowHideOverlay();
    void addOpenImageAction();
    void addOpenAlbumAction(const ImageModel* referenceModel);

public Q_SLOTS:

    void setEnabledHistorySteps(int count);

Q_SIGNALS:

    void imageSelected(const ImageInfo& info);
    void actionTriggered(const ImageInfo& info);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGE_PROPERTIES_VERSIONS_TAB_H
