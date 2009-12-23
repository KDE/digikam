/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : collection of basic models used for views in digikam
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "digikammodelcollection.moc"

// KDE includes

#include <kiconloader.h>

// Local settings

#include "albumsettings.h"

namespace Digikam
{

class DigikamModelCollectionPriv
{

public:

    AlbumModel     *albumModel;
    TagModel       *tagModel;
    TagModel       *tagFilterModel;
    SearchModel    *searchModel;
    DateAlbumModel *dateAlbumModel;
};

DigikamModelCollection::DigikamModelCollection() :
    d(new DigikamModelCollectionPriv)
{
    d->albumModel = new AlbumModel(AlbumModel::IncludeRootAlbum);

    d->tagModel = new TagModel(AbstractAlbumModel::IncludeRootAlbum);

    d->tagFilterModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFilterModel->setCheckable(true);

    d->searchModel = new SearchModel();
    d->searchModel->addReplaceName(
                    SAlbum::getTemporaryTitle(DatabaseSearch::HaarSearch,
                                    DatabaseSearch::HaarImageSearch), i18n(
                                    "Current Fuzzy Image Search"));
    d->searchModel->addReplaceName(SAlbum::getTemporaryTitle(
                    DatabaseSearch::HaarSearch,
                    DatabaseSearch::HaarSketchSearch), i18n(
                    "Current Fuzzy Sketch Search"));
    d->searchModel->addReplaceName(SAlbum::getTemporaryTitle(
                    DatabaseSearch::AdvancedSearch), i18n("Current Search"));
    d->searchModel->addReplaceName(SAlbum::getTemporaryTitle(
                    DatabaseSearch::MapSearch), i18n("Current Map Search"));

    d->dateAlbumModel = new DateAlbumModel();
    // set icons initially
    albumSettingsChanged();

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(albumSettingsChanged()));

}

DigikamModelCollection::~DigikamModelCollection()
{
    delete d->tagModel;
    delete d->tagFilterModel;
    delete d->albumModel;
    delete d->searchModel;
    delete d->dateAlbumModel;

    delete d;
}

AlbumModel *DigikamModelCollection::getAlbumModel() const
{
    return d->albumModel;
}

TagModel *DigikamModelCollection::getTagModel() const
{
    return d->tagModel;
}

TagModel *DigikamModelCollection::getTagFilterModel() const
{
    return d->tagFilterModel;
}

SearchModel *DigikamModelCollection::getSearchModel() const
{
    return d->searchModel;
}

DateAlbumModel *DigikamModelCollection::getDateAlbumModel() const
{
    return d->dateAlbumModel;
}

void DigikamModelCollection::albumSettingsChanged()
{
    d->searchModel->setPixmapForMapSearches(SmallIcon("applications-internet", AlbumSettings::instance()->getTreeViewIconSize()));
    d->searchModel->setPixmapForHaarSearches(SmallIcon("tools-wizard", AlbumSettings::instance()->getTreeViewIconSize()));
    d->searchModel->setPixmapForNormalSearches(SmallIcon("edit-find", AlbumSettings::instance()->getTreeViewIconSize()));
    d->searchModel->setPixmapForTimelineSearches(SmallIcon("chronometer", AlbumSettings::instance()->getTreeViewIconSize()));

    d->dateAlbumModel->setPixmaps(
                    SmallIcon(
                                    "view-calendar-list",
                                    AlbumSettings::instance()->getTreeViewIconSize()),
                    SmallIcon(
                                    "view-calendar-month",
                                    AlbumSettings::instance()->getTreeViewIconSize()));
}

}
