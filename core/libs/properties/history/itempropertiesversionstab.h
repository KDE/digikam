/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying item versions
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

#ifndef DIGIKAM_ITEM_PROPERTIES_VERSIONS_TAB_H
#define DIGIKAM_ITEM_PROPERTIES_VERSIONS_TAB_H

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
class ItemInfo;
class ItemModel;
class VersionsWidget;

class ItemPropertiesVersionsTab : public QTabWidget
{
    Q_OBJECT

public:

    explicit ItemPropertiesVersionsTab(QWidget* const parent);
    ~ItemPropertiesVersionsTab();

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void clear();
    void setItem(const ItemInfo& info, const DImageHistory& history);

    VersionsWidget* versionsWidget()             const;
    FiltersHistoryWidget* filtersHistoryWidget() const;

    void addShowHideOverlay();
    void addOpenImageAction();
    void addOpenAlbumAction(const ItemModel* referenceModel);

public Q_SLOTS:

    void setEnabledHistorySteps(int count);

Q_SIGNALS:

    void imageSelected(const ItemInfo& info);
    void actionTriggered(const ItemInfo& info);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_PROPERTIES_VERSIONS_TAB_H
