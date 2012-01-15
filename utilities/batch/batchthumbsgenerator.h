/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QObject>
#include <QPixmap>

// Local includes

#include "progressmanager.h"

class KUrl;

namespace Digikam
{

class LoadingDescription;

class BatchThumbsGenerator : public ProgressItem
{
    Q_OBJECT

public:

    explicit BatchThumbsGenerator(bool rebuildAll=true);
    BatchThumbsGenerator(int albumId);
    ~BatchThumbsGenerator();

Q_SIGNALS:

    void signalRebuildAllThumbsDone();

private:

    void complete();
    void processOne();

private Q_SLOTS:

    void slotCancel();
    void slotRebuildThumbs();
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    class BatchThumbsGeneratorPriv;
    BatchThumbsGeneratorPriv* const d;
};

}  // namespace Digikam

#endif /* BATCHTHUMBSGENERATOR_H */
