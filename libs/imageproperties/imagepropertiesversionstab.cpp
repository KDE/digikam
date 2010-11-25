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

#include "imagepropertiesversionstab.moc"

// Qt includes

#include <QListView>
#include <QGridLayout>
#include <QLabel>
#include <QModelIndex>

// KDE includes

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>

// Local includes

#include "imagepropertiesversionstab.h"
#include "imageversionsmodel.h"
#include "dmetadata.h"
#include "dimagehistory.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imagepropertiesversionsdelegate.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnailloadthread.h"
#include "versionswidget.h"
#include "filtershistorywidget.h"
#include "albumsettings.h"

namespace Digikam
{

class ImagePropertiesVersionsTab::ImagePropertiesVersionsTabPriv
{
public:

    ImagePropertiesVersionsTabPriv()
    {
        versionsWidget       = 0;
        filtersHistoryWidget = 0;
    }

    VersionsWidget*                versionsWidget;
    FiltersHistoryWidget*          filtersHistoryWidget;
    QList<QPair<qlonglong, int> >  versionsList;
    DImageHistory                  history;
    ImageInfo                      info;
    QString                        currentSelectedImagePath;
    int                            currentSelectedImageListPosition;
    qlonglong                      currentSelectedImageId;
};

ImagePropertiesVersionsTab::ImagePropertiesVersionsTab(QWidget* parent)
    : KTabWidget(parent), d(new ImagePropertiesVersionsTabPriv)
{
    d->versionsWidget = new VersionsWidget(this);
    insertTab(0, d->versionsWidget, i18n("Versions"));

    d->filtersHistoryWidget = new FiltersHistoryWidget(this);
    insertTab(1, d->filtersHistoryWidget, i18n("Used Filters"));

    connect(d->versionsWidget, SIGNAL(newVersionSelected(KUrl)),
            this, SLOT(slotNewVersionSelected(KUrl)));
}

ImagePropertiesVersionsTab::~ImagePropertiesVersionsTab()
{
    delete d;
}

void ImagePropertiesVersionsTab::clear()
{
    d->versionsList.clear();
    d->filtersHistoryWidget->clearData();
    d->versionsWidget->slotDigikamViewNoCurrentItem();
}

/*
 * TODO: Database-less support?
void ImagePropertiesMetaDataTab::setCurrentURL(const KUrl& url)
{
    if (url.isEmpty())
    {
        clear();
        return;
    }
}
*/

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
    d->currentSelectedImagePath = info.filePath();
    d->currentSelectedImageId   = info.id();

    if (!hasImage(info.id()))
    {
        QList<QPair<qlonglong, int> > infoList = info.allAvailableVersions();

        if (infoList.isEmpty())
        {
            d->versionsList.append(qMakePair(info.id(),0));
        }
        else
        {
            d->versionsList.append(infoList);
            d->currentSelectedImageListPosition = findImagePositionInList(info.id());
        }

        setupVersionsData();
    }

    d->filtersHistoryWidget->setHistory(d->history);
}

int ImagePropertiesVersionsTab::findImagePositionInList(qlonglong id) const
{
    for (int i = 0; i < d->versionsList.size(); i++)
    {
        if (d->versionsList.at(i).first == id)
        {
            return i;
        }
    }

    return -1;
}

bool ImagePropertiesVersionsTab::hasImage(qlonglong id) const
{
    for (int i = 0; i < d->versionsList.size(); i++)
    {
        if (d->versionsList.at(i).first == id)
        {
            return true;
        }
    }

    return false;
}

void ImagePropertiesVersionsTab::setEnabledEntries(int count)
{
    d->filtersHistoryWidget->setEnabledEntries(count);
}

void ImagePropertiesVersionsTab::showCustomContextMenu(const QPoint& position)
{
    d->filtersHistoryWidget->showCustomContextMenu(position);
}

void ImagePropertiesVersionsTab::setupVersionsData() const
{
    if (!d->versionsList.isEmpty())
    {
        QList<QPair<QString, int> > l;

        for (int i = 0; i < d->versionsList.size(); i++)
        {
            l.append(qMakePair(ImageInfo(d->versionsList.at(i).first).filePath(), d->versionsList.at(i).second));
        }

        d->versionsWidget->setupModelData(l);
        d->versionsWidget->setCurrentSelectedImage(d->currentSelectedImagePath);
    }
}

void ImagePropertiesVersionsTab::setupFiltersData() const
{
    if (!d->versionsList.isEmpty()
        && d->currentSelectedImageListPosition > -1
        && d->currentSelectedImageListPosition < d->versionsList.size())
    {
        ImageInfo info(d->versionsList.at(d->currentSelectedImageListPosition).first);
        d->filtersHistoryWidget->setHistory(info.imageHistory());
    }
    else
    {
        d->filtersHistoryWidget->setHistory(DImageHistory());
    }
}

void ImagePropertiesVersionsTab::slotNewVersionSelected(KUrl url)
{
    qlonglong selectedImage = ImageInfo(url).id();
    qlonglong current = 0;
    qlonglong newOne = 0;

    for (int i = 0; i < d->versionsList.size(); i++)
    {
        if (d->versionsList.at(i).first == selectedImage)
        {
            newOne = d->versionsList.at(i).first;
            d->currentSelectedImageListPosition = i;
        }
        else if (d->versionsList.at(i).first == d->currentSelectedImageId)
        {
            current = d->versionsList.at(i).first;
        }
    }

    ImageInfo newOneInfo(newOne);

    /*
     * TODO
    if(!AlbumSettings::instance()->getShowAllVersions())
    {
        newOneInfo.setVisible(true);
        ImageInfo(current).setVisible(false);
    }
    else if(!newOneInfo.isVisible())
    {
        newOneInfo.setVisible(true);
    }
    */

    d->currentSelectedImagePath = url.path();
    d->currentSelectedImageId = newOne;

    emit setCurrentIdSignal(newOne);
    //emit updateMainViewSignal();
    emit setCurrentUrlSignal(url);
}

} // namespace Digikam
