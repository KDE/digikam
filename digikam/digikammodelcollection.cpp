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

};

DigikamModelCollection::DigikamModelCollection() :
    d(new DigikamModelCollectionPriv)
{
    d->albumModel = new AlbumModel(AlbumModel::IncludeRootAlbum);
}

DigikamModelCollection::~DigikamModelCollection()
{
    delete d->albumModel;
    delete d;
}

AlbumModel *DigikamModelCollection::getAlbumModel() const
{
    return d->albumModel;
}

}
