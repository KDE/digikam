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

#ifndef BATCHTHUMBSGENERATOR_H
#define BATCHTHUMBSGENERATOR_H

// Qt includes

#include <QPixmap>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class LoadingDescription;

class BatchThumbsGenerator : public ProgressItem
{
    Q_OBJECT

public:

    enum Mode
    {
        AllItems = 0,
        MissingItems,
        AlbumItems
    };

public:

    BatchThumbsGenerator(Mode mode=AllItems, int albumId=-1);
    ~BatchThumbsGenerator();

Q_SIGNALS:

    void signalProcessDone();

private:

    void complete();
    void processOne();

private Q_SLOTS:

    void slotCancel();
    void slotRun();
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    class BatchThumbsGeneratorPriv;
    BatchThumbsGeneratorPriv* const d;
};

}  // namespace Digikam

#endif /* BATCHTHUMBSGENERATOR_H */
