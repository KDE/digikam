/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : collection of basic models used for views in digikam
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

#include "digikammodelcollection.h"

// Qt includes

#include <QIcon>

// Local settings

#include "applicationsettings.h"

namespace Digikam
{

class DigikamModelCollection::Private
{

public:

    Private()
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
    ImageVersionsModel* imageVersionModel;
};

DigikamModelCollection::DigikamModelCollection() :
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
    d->imageVersionModel = new ImageVersionsModel();

    // set icons initially
    slotApplicationSettingsChanged();

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));
}

DigikamModelCollection::~DigikamModelCollection()
{
    delete d->tagModel;
    delete d->tagFilterModel;
    delete d->albumModel;
    delete d->searchModel;
    delete d->dateAlbumModel;
    delete d->imageVersionModel;
    delete d;
}

AlbumModel* DigikamModelCollection::getAlbumModel() const
{
    return d->albumModel;
}

TagModel* DigikamModelCollection::getTagModel() const
{
    return d->tagModel;
}

TagModel* DigikamModelCollection::getTagFilterModel() const
{
    return d->tagFilterModel;
}

TagModel* DigikamModelCollection::getTagFacesModel() const
{
    return d->tagFacesModel;
}

SearchModel* DigikamModelCollection::getSearchModel() const
{
    return d->searchModel;
}

DateAlbumModel* DigikamModelCollection::getDateAlbumModel() const
{
    return d->dateAlbumModel;
}

ImageVersionsModel* DigikamModelCollection::getImageVersionsModel() const
{
    return d->imageVersionModel;
}

void DigikamModelCollection::slotApplicationSettingsChanged()
{
    d->dateAlbumModel->setPixmaps(QIcon::fromTheme(QLatin1String("view-calendar-list")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()),
                                  QIcon::fromTheme(QLatin1String("view-calendar")).pixmap(ApplicationSettings::instance()->getTreeViewIconSize()));
}

} // namespace Digikam
