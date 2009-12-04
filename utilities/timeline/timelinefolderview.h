/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Searches dates folder view used by timeline
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TIMELINEFOLDERVIEW_H
#define TIMELINEFOLDERVIEW_H

// Local includes
#include "albumtreeview.h"
#include "searchmodificationhelper.h"

namespace Digikam
{

class TimeLineFolderViewNewPriv;
class TimeLineFolderViewNew : public SearchTreeView
{
    Q_OBJECT
public:
    TimeLineFolderViewNew(QWidget *parent, SearchModel *searchModel,
                          SearchModificationHelper *searchModificationHelper);
    virtual ~TimeLineFolderViewNew();

private:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    TimeLineFolderViewNewPriv *d;

};

}

#endif /* TIMELINEFOLDERVIEW_H */
