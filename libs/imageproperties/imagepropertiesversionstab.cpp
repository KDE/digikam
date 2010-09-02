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
        versionsList         = 0;
    }

    VersionsWidget*                versionsWidget;
    FiltersHistoryWidget*          filtersHistoryWidget;
    QList<QPair<qlonglong, int> >* versionsList;
    QString                        currentSelectedImagePath;
    int                            currentSelectedImageListPosition;
    qlonglong                      currentSelectedImageId;
};

ImagePropertiesVersionsTab::ImagePropertiesVersionsTab(QWidget* parent)
                          : KTabWidget(parent), d(new ImagePropertiesVersionsTabPriv)
{
    d->versionsList = new QList<QPair<qlonglong, int> >;

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

void ImagePropertiesVersionsTab::slotDigikamViewNoCurrentItem()
{
    d->versionsWidget->slotDigikamViewNoCurrentItem();
}

void ImagePropertiesVersionsTab::slotDigikamViewImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList& allImages) const
{
    Q_UNUSED(hasPrevious)
    Q_UNUSED(hasNext)
    Q_UNUSED(allImages)

    QString path;

    kDebug() << "Getting data for versions model";
    d->versionsList->clear();

    foreach(ImageInfo inf, selectedImage)
    {
        d->currentSelectedImagePath = inf.fileUrl().path();
        d->currentSelectedImageId = inf.id();

        if(hasImage(inf.id()))
        {
            break;
        }

        QList<QPair<qlonglong, int> > infoList = inf.allAvailableVersions();
        if(infoList.isEmpty())
        {
            d->versionsList->append(qMakePair(inf.id(), 0));
            d->currentSelectedImageListPosition = 0;
        }
        else
        {
            d->versionsList->append(infoList);
            d->currentSelectedImageListPosition = findImagePositionInList(inf.id());
        }
    }

    setupVersionsData();
    setupFiltersData();
}

int ImagePropertiesVersionsTab::findImagePositionInList(qlonglong id) const
{
    for(int i = 0; i < d->versionsList->size(); i++)
    {
        if(d->versionsList->at(i).first == id)
            return i;
    }
    return -1;
}

bool ImagePropertiesVersionsTab::hasImage(qlonglong id) const
{
    for(int i = 0; i < d->versionsList->size(); i++)
    {
        if(d->versionsList->at(i).first == id)
            return true;
    }
    return false;
}

void ImagePropertiesVersionsTab::slotViewItemSelected(QModelIndex index)
{
    d->versionsWidget->slotViewItemSelected(index);
}

void ImagePropertiesVersionsTab::disableEntries(int count)
{
    d->filtersHistoryWidget->disableEntries(count);
}

void ImagePropertiesVersionsTab::enableEntries(int count)
{
    d->filtersHistoryWidget->enableEntries(count);
}

void ImagePropertiesVersionsTab::setModelData(const QList<DImageHistory::Entry>& entries)
{
    d->filtersHistoryWidget->setModelData(entries);
}

void ImagePropertiesVersionsTab::showCustomContextMenu(const QPoint& position)
{
    d->filtersHistoryWidget->showCustomContextMenu(position);
}

void ImagePropertiesVersionsTab::setupVersionsData() const
{
    if(!d->versionsList->isEmpty())
    {
        QList<QPair<QString, int> > l;
        for(int i = 0; i < d->versionsList->size(); i++)
        {
            l.append(qMakePair(ImageInfo(d->versionsList->at(i).first).fileUrl().path(), d->versionsList->at(i).second));
        }

        d->versionsWidget->setupModelData(l);
        d->versionsWidget->setCurrentSelectedImage(d->currentSelectedImagePath);
    }
}

void ImagePropertiesVersionsTab::setupFiltersData() const
{
    if(!d->versionsList->isEmpty() && d->currentSelectedImageListPosition > -1 && d->currentSelectedImageListPosition < d->versionsList->size())
        d->filtersHistoryWidget->setModelData(ImageInfo(d->versionsList->at(d->currentSelectedImageListPosition).first).imageHistory().entries());
}

void ImagePropertiesVersionsTab::slotNewVersionSelected(KUrl url)
{
    qlonglong selectedImage = ImageInfo(url).id();
    qlonglong current;
    qlonglong newOne;

    for(int i = 0; i < d->versionsList->size(); i++)
    {
        if(d->versionsList->at(i).first == selectedImage)
        {
            newOne = d->versionsList->at(i).first;
            d->currentSelectedImageListPosition = i;
        }
        else if(d->versionsList->at(i).first == d->currentSelectedImageId)
        {
            current = d->versionsList->at(i).first;
        }
    }

    ImageInfo newOneInfo(newOne);

    if(!AlbumSettings::instance()->getShowAllVersions())
    {
        newOneInfo.setVisible(true);
        ImageInfo(current).setVisible(false);
    }
    else if(!newOneInfo.isVisible())
    {
        newOneInfo.setVisible(true);
    }

    d->currentSelectedImagePath = url.path();
    d->currentSelectedImageId = newOne;

    setupFiltersData();

    emit setCurrentIdSignal(newOne);
    //emit updateMainViewSignal();
    emit setCurrentUrlSignal(url);
    
}

void ImagePropertiesVersionsTab::slotUpdateImageInfo(const ImageInfo& info)
{
    QString path;

    kDebug() << "Getting data for versions model";
    d->versionsList->clear();

    d->currentSelectedImagePath = info.fileUrl().path();
    d->currentSelectedImageId = info.id();

    if(!hasImage(info.id()))
    {
        QList<QPair<qlonglong, int> > infoList = info.allAvailableVersions();
        if(infoList.isEmpty())
        {
            d->versionsList->append(qMakePair(info.id(),0));
        }
        else
        {
            d->versionsList->append(infoList);
            d->currentSelectedImageListPosition = findImagePositionInList(info.id());
        }

        setupVersionsData();
        setupFiltersData();
    }
}

} // namespace Digikam
