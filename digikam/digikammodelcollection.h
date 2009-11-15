/*
 * DigikamModelCollection.h
 *
 *  Created on: 15.11.2009
 *      Author: languitar
 */

#ifndef DIGIKAMMODELCOLLECTION_H
#define DIGIKAMMODELCOLLECTION_H

// Local includes
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "abstractalbummodel.h"

namespace Digikam
{

class DigikamModelCollectionPriv;

/**
 * This class is simply a collection of all models the build the core of the
 * digikam application.
 *
 * @author jwienke
 */
class DigikamModelCollection
{
public:
    DigikamModelCollection();
    virtual ~DigikamModelCollection();

    AlbumModel *getAlbumModel() const;

private:
    DigikamModelCollectionPriv *d;

};

}

#endif /* DIGIKAMMODELCOLLECTION_H */
