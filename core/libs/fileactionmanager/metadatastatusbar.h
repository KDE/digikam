/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-21
 * Description : a bar to indicate pending metadata
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef METADATASTATUSBAR_H
#define METADATASTATUSBAR_H

// Qt includes

#include <QWidget>

// Local includes

#include "metadatahubmngr.h"

namespace Digikam
{

class MetadataStatusBar : public QWidget
{
    Q_OBJECT

public:

    explicit MetadataStatusBar(QWidget* const parent);
    ~MetadataStatusBar();

public Q_SLOTS:

    void slotSettingsChanged();
    void slotSetPendingItems(int number);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // METADATASTATUSBAR_H
