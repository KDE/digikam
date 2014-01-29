/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-01-28
 * Description : drag and drop handling for Showfoto
 *
 * Copyright (C) 2014 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef SHOWFOTODRAGDROPHANDLER_H
#define SHOWFOTODRAGDROPHANDLER_H

// KDE includes

#include <kmenu.h>

// Local includes

#include "abstractitemdragdrophandler.h"
#include "showfotoimagemodel.h"
#include "ddragobjects.h"

class KJob;

namespace ShowFoto
{

class ShowfotoDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ShowfotoDragDropHandler(ShowfotoImageModel* const model);

    ShowfotoImageModel*    model() const;

    virtual bool           dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);
    virtual QStringList    mimeTypes() const;
    virtual QMimeData*     createMimeData(const QList<QModelIndex> &);

Q_SIGNALS:

    void dioResult(KJob*);
    void signalDroppedUrls(const KUrl::List& droppedUrls);

private:

    QAction*   addGroupAction(KMenu* const menu);
    QAction*   addCancelAction(KMenu* const menu);
};

} // namespace Digikam

#endif // SHOWFOTODRAGDROPHANDLER_H
