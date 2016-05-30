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

#include "imagepropertiesversionstab.h"

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
#include "imageversionsmodel.h"
#include "dmetadata.h"
#include "dimagehistory.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "versionswidget.h"
#include "filtershistorywidget.h"
#include "applicationsettings.h"
#include "versionsoverlays.h"

namespace Digikam
{

class ImagePropertiesVersionsTab::Private
{
public:

    Private()
    {
        versionsWidget       = 0;
        filtersHistoryWidget = 0;
    }

    VersionsWidget*       versionsWidget;
    FiltersHistoryWidget* filtersHistoryWidget;
    DImageHistory         history;
    ImageInfo             info;

    static const QString  configActiveTab;
};

const QString ImagePropertiesVersionsTab::Private::configActiveTab(QLatin1String("Version Properties Tab"));

ImagePropertiesVersionsTab::ImagePropertiesVersionsTab(QWidget* const parent)
    : QTabWidget(parent),
      d(new Private)
{
    d->versionsWidget       = new VersionsWidget(this);
    insertTab(0, d->versionsWidget, i18n("Versions"));

    d->filtersHistoryWidget = new FiltersHistoryWidget(this);
    insertTab(1, d->filtersHistoryWidget, i18n("Used Filters"));

    connect(d->versionsWidget, SIGNAL(imageSelected(ImageInfo)),
            this, SIGNAL(imageSelected(ImageInfo)));
}

ImagePropertiesVersionsTab::~ImagePropertiesVersionsTab()
{
    delete d;
}

void ImagePropertiesVersionsTab::readSettings(KConfigGroup& group)
{
    QString tab = group.readEntry(d->configActiveTab, "versions");

    if (tab == QLatin1String("versions"))
        setCurrentWidget(d->versionsWidget);
    else
        setCurrentWidget(d->filtersHistoryWidget);

    d->versionsWidget->readSettings(group);
}

void ImagePropertiesVersionsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configActiveTab, currentWidget() == d->versionsWidget ? "versions" : "filters");

    d->versionsWidget->writeSettings(group);
}

VersionsWidget* ImagePropertiesVersionsTab::versionsWidget() const
{
    return d->versionsWidget;
}

FiltersHistoryWidget* ImagePropertiesVersionsTab::filtersHistoryWidget() const
{
    return d->filtersHistoryWidget;
}

void ImagePropertiesVersionsTab::clear()
{
    d->filtersHistoryWidget->clearData();
    d->versionsWidget->setCurrentItem(ImageInfo());
}

void ImagePropertiesVersionsTab::setItem(const ImageInfo& info, const DImageHistory& history)
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

void ImagePropertiesVersionsTab::addShowHideOverlay()
{
    d->versionsWidget->addShowHideOverlay();
}

void ImagePropertiesVersionsTab::addOpenImageAction()
{
    ActionVersionsOverlay* const overlay = d->versionsWidget->addActionOverlay(QIcon::fromTheme(QLatin1String("document-open")),
                                                                               i18n("Open"),
                                                                               i18n("Open file"));

    connect(overlay, SIGNAL(activated(ImageInfo)),
            this, SIGNAL(actionTriggered(ImageInfo)));
}

void ImagePropertiesVersionsTab::addOpenAlbumAction(const ImageModel* referenceModel)
{
    ActionVersionsOverlay* const overlay = d->versionsWidget->addActionOverlay(QIcon::fromTheme(QLatin1String("folder-pictures")),
                                                                               i18n("Go To Albums"),
                                                                               i18nc("@info:tooltip", "Go to the album of this image"));
    overlay->setReferenceModel(referenceModel);

    connect(overlay, SIGNAL(activated(ImageInfo)),
            this, SIGNAL(actionTriggered(ImageInfo)));
}

void ImagePropertiesVersionsTab::setEnabledHistorySteps(int count)
{
    d->filtersHistoryWidget->setEnabledEntries(count);
}

} // namespace Digikam
