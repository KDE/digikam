/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : re-implementation of action thread using threadweaver
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * Copyright (C) 2011-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DACTIONTHREADBASE_H
#define DACTIONTHREADBASE_H

// Qt includes

#include <QThread>

//Local includes

#include "dweaverobserver.h"
#include "digikam_export.h"

namespace ThreadWeaver
{
    class JobCollection;
}

using namespace ThreadWeaver;

namespace Digikam
{

class DIGIKAM_EXPORT DActionThreadBase : public QThread
{
    Q_OBJECT

public:

    DActionThreadBase(QObject* const parent=0);
    ~DActionThreadBase();

    void cancel();
    void finish();

protected:

    void run();
    void appendJob(JobCollection* const job);

protected Q_SLOTS:

    void slotFinished();

private:

    class DActionThreadBasePriv;
    DActionThreadBasePriv* const d;
};

}  // namespace Digikam

#endif // DACTIONTHREADBASE_H
