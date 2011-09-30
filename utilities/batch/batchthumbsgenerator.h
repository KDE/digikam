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

#include <QPixmap>
#include <QCloseEvent>

// Local includes

#include "dprogressdlg.h"

class QWidget;
class QPixmap;

class KUrl;

namespace Digikam
{

class LoadingDescription;

class BatchThumbsGenerator : public DProgressDlg
{
    Q_OBJECT

public:

    explicit BatchThumbsGenerator(QWidget* parent, bool rebuildAll=true);
    BatchThumbsGenerator(QWidget* parent, int albumId);
    ~BatchThumbsGenerator();

Q_SIGNALS:

    void signalRebuildAllThumbsDone();

private:

    void abort();
    void complete();
    void processOne();

protected:

    void closeEvent(QCloseEvent* e);

protected Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotRebuildThumbs();
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    class BatchThumbsGeneratorPriv;
    BatchThumbsGeneratorPriv* const d;
};

}  // namespace Digikam

#endif /* BATCHTHUMBSGENERATOR_H */
