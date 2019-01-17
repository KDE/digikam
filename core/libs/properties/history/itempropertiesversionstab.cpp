/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying item versions
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

#include "itempropertiesversionstab.h"

// Qt includes

#include <QListView>
#include <QGridLayout>
#include <QLabel>
#include <QModelIndex>
#include <QUrl>
#include <QIcon>
#include <QString>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemversionsmodel.h"
#include "dmetadata.h"
#include "dimagehistory.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "versionswidget.h"
#include "filtershistorywidget.h"
#include "applicationsettings.h"
#include "versionsoverlays.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesVersionsTab::Private
{
public:

    explicit Private()
    {
        versionsWidget       = 0;
        filtersHistoryWidget = 0;
    }

    VersionsWidget*       versionsWidget;
    FiltersHistoryWidget* filtersHistoryWidget;
    DImageHistory         history;
    ItemInfo             info;

    static const QString  configActiveTab;
};

const QString ItemPropertiesVersionsTab::Private::configActiveTab(QLatin1String("Version Properties Tab"));

ItemPropertiesVersionsTab::ItemPropertiesVersionsTab(QWidget* const parent)
    : QTabWidget(parent),
      d(new Private)
{
    d->versionsWidget       = new VersionsWidget(this);
    insertTab(0, d->versionsWidget, i18n("Versions"));

    d->filtersHistoryWidget = new FiltersHistoryWidget(this);
    insertTab(1, d->filtersHistoryWidget, i18n("Used Filters"));

    connect(d->versionsWidget, SIGNAL(imageSelected(ItemInfo)),
            this, SIGNAL(imageSelected(ItemInfo)));
}

ItemPropertiesVersionsTab::~ItemPropertiesVersionsTab()
{
    delete d;
}

void ItemPropertiesVersionsTab::readSettings(KConfigGroup& group)
{
    QString tab = group.readEntry(d->configActiveTab, "versions");

    if (tab == QLatin1String("versions"))
        setCurrentWidget(d->versionsWidget);
    else
        setCurrentWidget(d->filtersHistoryWidget);

    d->versionsWidget->readSettings(group);
}

void ItemPropertiesVersionsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configActiveTab, currentWidget() == d->versionsWidget ? "versions" : "filters");

    d->versionsWidget->writeSettings(group);
}

VersionsWidget* ItemPropertiesVersionsTab::versionsWidget() const
{
    return d->versionsWidget;
}

FiltersHistoryWidget* ItemPropertiesVersionsTab::filtersHistoryWidget() const
{
    return d->filtersHistoryWidget;
}

void ItemPropertiesVersionsTab::clear()
{
    d->filtersHistoryWidget->clearData();
    d->versionsWidget->setCurrentItem(ItemInfo());
}

void ItemPropertiesVersionsTab::setItem(const ItemInfo& info, const DImageHistory& history)
{
    clear();

    if (info.isNull())
    {
        return;
    }

    d->history = history;

    if (d->history.isNull())
    {
        d->history = info.imageHistory();
    }

    d->info = info;

    d->versionsWidget->setCurrentItem(info);
    d->filtersHistoryWidget->setHistory(d->history);
}

void ItemPropertiesVersionsTab::addShowHideOverlay()
{
    d->versionsWidget->addShowHideOverlay();
}

void ItemPropertiesVersionsTab::addOpenImageAction()
{
    ActionVersionsOverlay* const overlay = d->versionsWidget->addActionOverlay(QIcon::fromTheme(QLatin1String("document-open")),
                                                                               i18n("Open"),
                                                                               i18n("Open file"));

    connect(overlay, SIGNAL(activated(ItemInfo)),
            this, SIGNAL(actionTriggered(ItemInfo)));
}

void ItemPropertiesVersionsTab::addOpenAlbumAction(const ItemModel* referenceModel)
{
    ActionVersionsOverlay* const overlay = d->versionsWidget->addActionOverlay(QIcon::fromTheme(QLatin1String("folder-pictures")),
                                                                               i18n("Go To Albums"),
                                                                               i18nc("@info:tooltip", "Go to the album of this image"));
    overlay->setReferenceModel(referenceModel);

    connect(overlay, SIGNAL(activated(ItemInfo)),
            this, SIGNAL(actionTriggered(ItemInfo)));
}

void ItemPropertiesVersionsTab::setEnabledHistorySteps(int count)
{
    d->filtersHistoryWidget->setEnabledEntries(count);
}

} // namespace Digikam
