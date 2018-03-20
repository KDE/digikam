/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-09-07
 * Description : Qt Model for ImportUI - drag and drop handling
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QMenu>

// Local includes

#include "abstractitemdragdrophandler.h"
#include "importimagemodel.h"
#include "album.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImportDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ImportDragDropHandler(ImportImageModel* const model);

    ImportImageModel* model() const;

    virtual bool           dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);
    virtual QStringList    mimeTypes() const;
    virtual QMimeData*     createMimeData(const QList<QModelIndex> &);

private:

    enum DropAction
    {
        NoAction,
        CopyAction,
        MoveAction,
        GroupAction,
        AssignTagAction
    };

private:

    QAction*   addGroupAction(QMenu* const menu);
    QAction*   addCancelAction(QMenu* const menu);
    DropAction copyOrMove(const QDropEvent* e, QWidget* const view, bool allowMove = true, bool askForGrouping = false);
};

} // namespace Digikam

#endif // IMPORTDRAGDROP_H
