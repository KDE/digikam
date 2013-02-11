/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBSGENERATOR_H
#define THUMBSGENERATOR_H

// Qt includes

#include <QPixmap>

// Local includes

#include "maintenancetool.h"

namespace Digikam
{

class LoadingDescription;

class ThumbsGenerator : public MaintenanceTool
{
    Q_OBJECT

public:

    explicit ThumbsGenerator(bool rebuildAll = true, int albumId = -1, ProgressItem* const parent = 0);
    ~ThumbsGenerator();

private:

    void processOne();

private Q_SLOTS:

    void slotStart();
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* THUMBSGENERATOR_H */
