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

    AlbumModel  *albumModel;
    TagModel    *tagModel;
    TagModel    *tagFilterModel;
    SearchModel *searchModel;
};

DigikamModelCollection::DigikamModelCollection() :
    d(new DigikamModelCollectionPriv)
{
    d->albumModel = new AlbumModel(AlbumModel::IncludeRootAlbum);

    d->tagModel = new TagModel(AbstractAlbumModel::IncludeRootAlbum);

    d->tagFilterModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    d->tagFilterModel->setCheckable(true);

    d->searchModel = new SearchModel();
}

DigikamModelCollection::~DigikamModelCollection()
{
    delete d->tagModel;
    delete d->tagFilterModel;
    delete d->albumModel;
    delete d->searchModel;

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

}
