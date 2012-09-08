/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-09-07
 * Description : Qt Model for ImportUI - drag and drop handling
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#ifndef IMPORTDRAGDROP_H
#define IMPORTDRAGDROP_H

// Local includes

#include "abstractitemdragdrophandler.h"
#include "importimagemodel.h"
#include "album.h"

class KJob;

namespace Digikam
{

class ImportDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    ImportDragDropHandler(ImportImageModel* const model);

    ImportImageModel* model() const;

    virtual bool dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);
    virtual QStringList mimeTypes() const;
    virtual QMimeData* createMimeData(const QList<QModelIndex> &);

Q_SIGNALS:

    void dioResult(KJob*);
};

} // namespace Digikam

#endif // IMPORTDRAGDROP_H
