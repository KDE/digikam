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

namespace Digikam
{

class ImagePropertiesVersionsTab::ImagePropertiesVersionsTabPriv
{
public:

    ImagePropertiesVersionsTabPriv()
    {
        versionsWidget          = 0;
        filtersHistoryWidget    = 0;
        versionsList            = 0;
    }

    VersionsWidget*                 versionsWidget;
    FiltersHistoryWidget*           filtersHistoryWidget;
    QList<QPair<ImageInfo, int> >*  versionsList;
    QString                         currentSelectedImage;
    int                             currentSelectedImageListPosition;
    
};

ImagePropertiesVersionsTab::ImagePropertiesVersionsTab(QWidget* parent)
                          : KTabWidget(parent), d(new ImagePropertiesVersionsTabPriv)
{
    d->versionsList = new QList<QPair<ImageInfo, int> >;
    
    d->versionsWidget = new VersionsWidget(this);
    insertTab(0, d->versionsWidget, i18n("Versions"));

    d->filtersHistoryWidget = new FiltersHistoryWidget(this);
    insertTab(1, d->filtersHistoryWidget, i18n("Used Filters"));

    connect(d->versionsWidget, SIGNAL(newVersionSelected(KUrl)),
            this, SLOT(slotNewVersionSelected(KUrl)));

    //connect(d->versionsWidget, SIGNAL(doubleClicked(QModelIndex)),
    //        this, SLOT(slotViewItemSelected(QModelIndex)));
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
    
    //QList<DImageHistory::Entry>* filtersList = new QList<DImageHistory::Entry>;
    
    kDebug() << "Getting data for versions model";
    d->versionsList->clear();
    
    foreach(ImageInfo inf, selectedImage)
    {
        d->currentSelectedImage = inf.fileUrl().path();

        if(hasImage(inf))
        {
            break;
        }

        QList<QPair<ImageInfo, int> > infoList = inf.allAvailableVersions();
        if(infoList.isEmpty())
        {
            d->versionsList->append(qMakePair(inf,0));
        }
        else
        {
            d->versionsList->append(infoList);
            d->currentSelectedImageListPosition = findImagePositionInList(inf);
        }
    }

    setupVersionsData();
    setupFiltersData();
}

int ImagePropertiesVersionsTab::findImagePositionInList(Digikam::ImageInfo& info) const
{
    for(int i = 0; i < d->versionsList->size(); i++)
    {
        if(d->versionsList->at(i).first == info)
            return i;
    }
    return -1;
}

bool ImagePropertiesVersionsTab::hasImage(ImageInfo& info) const
{
    for(int i = 0; i < d->versionsList->size(); i++)
    {
        if(d->versionsList->at(i).first == info)
            return true;
    }
    return false;
}

void ImagePropertiesVersionsTab::slotViewItemSelected(QModelIndex index)
{
    d->versionsWidget->slotViewItemSelected(index);
}

void ImagePropertiesVersionsTab::disableEntry(bool disable)
{
    d->filtersHistoryWidget->disableEntry(disable);
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
            l.append(qMakePair(d->versionsList->at(i).first.fileUrl().path(), d->versionsList->at(i).second));
        }

        d->versionsWidget->setupModelData(l);
        d->versionsWidget->setCurrentSelectedImage(d->currentSelectedImage);
    }
}

void ImagePropertiesVersionsTab::setupFiltersData() const
{
    if(!d->versionsList->isEmpty() && d->currentSelectedImageListPosition > -1 && d->currentSelectedImageListPosition < d->versionsList->size())
        d->filtersHistoryWidget->setModelData(d->versionsList->at(d->currentSelectedImageListPosition).first.imageHistory().entries());
}

void ImagePropertiesVersionsTab::slotNewVersionSelected(KUrl url)
{
    ImageInfo current;
    ImageInfo newOne;
    for(int i = 0; i < d->versionsList->size(); i++)
    {
        if(d->versionsList->at(i).first.fileUrl() == url)
        {
            newOne = d->versionsList->at(i).first;
            d->currentSelectedImageListPosition = i;
        }
        else if(d->versionsList->at(i).first.fileUrl() == d->currentSelectedImage)
        {
            current = d->versionsList->at(i).first;
        }
    }
    current.setVisible(false);
    newOne.setVisible(true);

    d->currentSelectedImage = url.path();

    setupFiltersData();
    
    emit setCurrentUrlSignal(url);
    emit updateMainViewSignal();
}

} // namespace Digikam