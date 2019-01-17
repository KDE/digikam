/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : factory of basic models used for views in digikam
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "dmodelfactory.h"

// Qt includes

#include <QIcon>

// Local settings

#include "applicationsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN DModelFactory::Private
{

public:

    explicit Private()
    {
        albumModel        = 0;
        tagModel          = 0;
        tagFilterModel    = 0;
        tagFacesModel     = 0;
        searchModel       = 0;
        dateAlbumModel    = 0;
        imageVersionModel = 0;
    }

    AlbumModel*         albumModel;
    TagModel*           tagModel;
    TagModel*           tagFilterModel;
    TagModel*           tagFacesModel;
    SearchModel*        searchModel;
    DateAlbumModel*     dateAlbumModel;
    ItemVersionsModel* imageVersionModel;
};

DModelFactory::DModelFactory() :
    d(new Private)
{
    d->albumModel        = new AlbumModel(AlbumModel::IncludeRootAlbum);
    d->tagModel          = new TagModel(AbstractAlbumModel::IncludeRootAlbum);
    d->tagFilterModel    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFilterModel->setAddExcludeTristate(true);
    d->tagFacesModel     = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFacesModel->setTagCount(TagModel::FaceTagCount);

    d->searchModel       = new SearchModel();
    d->dateAlbumModel    = new DateAlbumModel();
    d->imageVersionModel = new ItemVersionsModel();

    // set icons initially
    slotApplicationSettingsChanged();

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));
}

DModelFactory::~DModelFactory()
{
    delete d->tagModel;
    delete d->tagFilterModel;
    delete d->albumModel;
    delete d->searchModel;
    delete d->dateAlbumModel;
    delete d->imageVersionModel;
    delete d;
}

AlbumModel* DModelFactory::getAlbumModel() const
{
    return d->albumModel;
}

TagModel* DModelFactory::getTagModel() const
{
    return d->tagModel;
}

TagModel* DModelFactory::getTagFilterModel() const
{
    return d->tagFilterModel;
}

TagModel* DModelFactory::getTagFacesModel() const
{
    return d->tagFacesModel;
}

SearchModel* DModelFactory::getSearchModel() const
{
    return d->searchModel;
}

DateAlbumModel* DModelFactory::getDateAlbumModel() const
{
    return d->dateAlbumModel;
}

ItemVersionsModel* DModelFactory::getItemVersionsModel() const
{
    return d->imageVersionModel;
}

void DModelFactory::slotApplicationSettingsChanged()
{
    d->dateAlbumModel->setPixmaps(QIcon::fromTheme(QLatin1String("view-calendar-list")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()),
                                  QIcon::fromTheme(QLatin1String("view-calendar")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()));
}

} // namespace Digikam
