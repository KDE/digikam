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

namespace Digikam 
{

class ImagePropertiesVersionsTab::ImagePropertiesVersionsTabPriv
{
public:

    ImagePropertiesVersionsTabPriv()
    {
        view     = 0;
        model    = 0;
        layout   = 0;
        delegate = 0;
    }

    QListView*                       view;
    ImageVersionsModel*              model;
    QGridLayout*                     layout;
    QHBoxLayout*                     iconTextLayout;
    QLabel*                          headerText;
    QLabel*                          headerTextIcon;
    ImagePropertiesVersionsDelegate* delegate;
};

ImagePropertiesVersionsTab::ImagePropertiesVersionsTab(QWidget* parent, ImageVersionsModel* model)
                           : QWidget(parent), d(new ImagePropertiesVersionsTabPriv)
{
    d->view           = new QListView(this);
    d->layout         = new QGridLayout(this);
    d->model          = model;
    d->delegate       = new ImagePropertiesVersionsDelegate(0);
    d->headerText     = new QLabel(this);
    d->headerTextIcon = new QLabel(this);
    d->iconTextLayout = new QHBoxLayout();

    setLayout(d->layout);

    d->headerText->setText(i18n("Available versions"));
    d->headerTextIcon->setPixmap(SmallIcon("image-x-generic"));
    d->headerTextIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    d->view->setItemDelegate(d->delegate);
    d->view->setSelectionRectVisible(true);
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->setSelectionBehavior(QAbstractItemView::SelectItems);

    d->iconTextLayout->addWidget(d->headerTextIcon);
    d->iconTextLayout->addWidget(d->headerText);

    d->layout->addLayout(d->iconTextLayout, 0, 0, 1, 1);
    d->layout->addWidget(d->view,           1, 0, 1, 1);

    connect(d->view, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotViewItemSelected(QModelIndex)));

    connect(d->delegate, SIGNAL(throbberUpdated()),
            d->model, SLOT(slotAnimationStep()));
}

ImagePropertiesVersionsTab::~ImagePropertiesVersionsTab()
{
    delete d->delegate;
    delete d;
}

void ImagePropertiesVersionsTab::slotDigikamViewNoCurrentItem()
{
    d->model->clearModelData();
}

void ImagePropertiesVersionsTab::slotDigikamViewImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList& allImages)
{
    Q_UNUSED(hasPrevious)
    Q_UNUSED(hasNext)
    Q_UNUSED(allImages)

    QString path;
    QList<QVariant>* list = new QList<QVariant>;

    kDebug() << "Getting data for versions model";

    foreach(ImageInfo inf, selectedImage)
    {
        kDebug() << inf.imageHistory().originalFileName();
        if(!inf.imageHistory().originalFileName().isEmpty())
        {
            path = inf.imageHistory().originalFilePath();
            path.append("/");
            path.append(inf.imageHistory().originalFileName());
            kDebug() << "Original file path: " << path;
            list->append(path);
        }
        list->append(inf.filePath());
    }

    d->model->setupModelData(list);
    d->view->setModel(d->model);
    d->view->update();
}

void ImagePropertiesVersionsTab::slotViewItemSelected(QModelIndex index)
{
    KUrl url(index.data(Qt::DisplayRole).toString());
    emit setCurrentUrlSignal(url);
}

} // namespace Digikam
