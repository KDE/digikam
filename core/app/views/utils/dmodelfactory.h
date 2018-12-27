/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

#ifndef DIGIKAM_DMODEL_FACTORY_H
#define DIGIKAM_DMODEL_FACTORY_H

// Qt includes

#include <QObject>

// Local includes

#include "abstractalbummodel.h"
#include "albumfiltermodel.h"
#include "albummodel.h"
#include "itemversionsmodel.h"

namespace Digikam
{

/**
 * This class is simply a factory of all models that build the core of the
 * digikam application.
 *
 * @author jwienke
 */
class DModelFactory: public QObject
{
    Q_OBJECT

public:

    DModelFactory();
    virtual ~DModelFactory();

    AlbumModel*        getAlbumModel()        const;
    TagModel*          getTagModel()          const;
    TagModel*          getTagFilterModel()    const;
    TagModel*          getTagFacesModel()     const;
    SearchModel*       getSearchModel()       const;
    DateAlbumModel*    getDateAlbumModel()    const;
    ItemVersionsModel* getItemVersionsModel() const;

private Q_SLOTS:

    void slotApplicationSettingsChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DMODEL_FACTORY_H
