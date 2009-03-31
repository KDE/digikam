/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries - private header
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEALBUMFILTERMODELTHREADS_H
#define IMAGEALBUMFILTERMODELTHREADS_H

// Qt includes.

#include <QThread>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASEMODELS_EXPORT ImageFilterModelWorker : public QObject
{
    Q_OBJECT

public:

    ImageFilterModelWorker(ImageFilterModelPrivate *d);

    void shutDown() { thread->quit(); thread->wait(); }

    bool checkVersion(const ImageFilterModelTodoPackage &package)
    { return d->version == package.version; }

public Q_SLOTS:

    virtual void process(ImageFilterModelTodoPackage package) = 0;

Q_SIGNALS:

    void processed(const ImageFilterModelTodoPackage &package);
    void discarded(const ImageFilterModelTodoPackage &package);

protected:

    class Thread : public QThread
    {
        public:
        Thread(QObject *parent = 0) : QThread(parent) {}
        virtual void run() { exec(); }
    };
    Thread *thread;

    ImageFilterModelPrivate *d;
};

class DIGIKAM_DATABASEMODELS_EXPORT ImageFilterModelPreparer : public ImageFilterModelWorker
{
    Q_OBJECT

public:

    ImageFilterModelPreparer(ImageFilterModelPrivate *d)
        : ImageFilterModelWorker(d) {}

    void process(ImageFilterModelTodoPackage package);
};

class DIGIKAM_DATABASEMODELS_EXPORT ImageFilterModelFilterer : public ImageFilterModelWorker
{
    Q_OBJECT

public:

    ImageFilterModelFilterer(ImageFilterModelPrivate *d)
        : ImageFilterModelWorker(d) {}

    void process(ImageFilterModelTodoPackage package);
};

} // namespace Digikam

#endif // IMAGEALBUMFILTERMODELTHREADS_H
