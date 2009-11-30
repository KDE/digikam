/*
 * DigikamModelCollection.cpp
 *
 *  Created on: 15.11.2009
 *      Author: languitar
 */

#include "digikammodelcollection.h"

namespace Digikam
{

class DigikamModelCollectionPriv
{
public:
    AlbumModel *albumModel;
    TagModel *tagModel;
    TagModel *tagFilterModel;
    SearchModel *timelineSearchModel;
    SearchModel *normalSearchModel;
    SearchModel *fuzzySearchModel;
    SearchModel *mapSearchModel;

};

DigikamModelCollection::DigikamModelCollection() :
    d(new DigikamModelCollectionPriv)
{
    d->albumModel = new AlbumModel(AlbumModel::IncludeRootAlbum);

    d->tagModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);

    d->tagFilterModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFilterModel->setCheckable(true);

    d->timelineSearchModel = new SearchModel();
    d->timelineSearchModel->setSearchType(DatabaseSearch::TimeLineSearch);

    d->normalSearchModel = new SearchModel();
    d->normalSearchModel->listNormalSearches();

    d->fuzzySearchModel = new SearchModel();
    d->fuzzySearchModel->setSearchType(DatabaseSearch::HaarSearch);

    d->mapSearchModel = new SearchModel();
    d->mapSearchModel->setSearchType(DatabaseSearch::MapSearch);

}

DigikamModelCollection::~DigikamModelCollection()
{
    delete d->timelineSearchModel;
    delete d->tagModel;
    delete d->tagFilterModel;
    delete d->albumModel;
    delete d->normalSearchModel;
    delete d->fuzzySearchModel;
    delete d->mapSearchModel;

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

SearchModel *DigikamModelCollection::getTimlineSearchModel() const
{
    return d->timelineSearchModel;
}

SearchModel *DigikamModelCollection::getNormalSearchModel() const
{
    return d->normalSearchModel;
}

SearchModel *DigikamModelCollection::getFuzzySearchModel() const
{
    return d->fuzzySearchModel;
}

SearchModel *DigikamModelCollection::getMapSearchModel() const
{
    return d->mapSearchModel;
}

}
