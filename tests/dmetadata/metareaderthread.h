/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-08-14
 * Description : CLI tool to test to load metadata from images through multi-core threads.
 *
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef META_READER_THREAD_H
#define META_READER_THREAD_H

// Qt includes

#include <QList>
#include <QUrl>

// Local includes

#include "actionthreadbase.h"

using namespace Digikam;

class MetaReaderThread : public ActionThreadBase
{
    Q_OBJECT

public:

    MetaReaderThread(QObject* const parent);
    ~MetaReaderThread() {};

    void readMetadata(const QList<QUrl>& list, const QString& direction);

protected Q_SLOTS:

    void slotJobFinished();

Q_SIGNALS:

    void done();
};

#endif // META_READER_THREAD_H
