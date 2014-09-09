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

#ifndef DIGIKAMMODELCOLLECTION_H
#define DIGIKAMMODELCOLLECTION_H

// Qt includes

#include <QObject>

// Local includes

#include "abstractalbummodel.h"
#include "albumfiltermodel.h"
#include "albummodel.h"
#include "imageversionsmodel.h"

namespace Digikam
{

/**
 * This class is simply a collection of all models that build the core of the
 * digikam application.
 *
 * @author jwienke
 */
class DigikamModelCollection: public QObject
{
    Q_OBJECT

public:

    DigikamModelCollection();
    virtual ~DigikamModelCollection();

    AlbumModel*         getAlbumModel()         const;
    TagModel*           getTagModel()           const;
    TagModel*           getTagFilterModel()     const;
    TagModel*           getTagFacesModel()      const;
    SearchModel*        getSearchModel()        const;
    DateAlbumModel*     getDateAlbumModel()     const;
    ImageVersionsModel* getImageVersionsModel() const;

private Q_SLOTS:

    void slotApplicationSettingsChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* DIGIKAMMODELCOLLECTION_H */
