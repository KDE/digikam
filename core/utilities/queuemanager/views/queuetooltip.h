/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-03
 * Description : queue tool tip
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUETOOLTIP_H
#define QUEUETOOLTIP_H

// Local includes

#include "imageinfo.h"
#include "ditemtooltip.h"

namespace Digikam
{

class QueueListView;
class QueueListViewItem;

class QueueToolTip : public DItemToolTip
{
public:

    explicit QueueToolTip(QueueListView* const view);
    ~QueueToolTip();

    void setQueueItem(QueueListViewItem* const item);

private:

    QRect   repositionRect();
    QString tipContents();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* QUEUETOOLTIP_H */
