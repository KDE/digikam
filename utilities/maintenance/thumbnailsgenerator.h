/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAILSGENERATOR_H
#define THUMBNAILSGENERATOR_H

// Local includes

#include "maintenancetool.h"

namespace Digikam
{

class ThumbnailsGenerator : public MaintenanceTool
{
    Q_OBJECT

public:

    ThumbnailsGenerator(Mode mode=AllItems, int albumId=-1);
    ~ThumbnailsGenerator();

private:

    void listItemstoProcess();
    void processOne();
};

}  // namespace Digikam

#endif /* THUMBNAILSGENERATOR_H */
