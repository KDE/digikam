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

#ifndef KPACTIONTHREADBASE_H
#define KPACTIONTHREADBASE_H

// Qt includes

#include <QThread>

//Local includes

#include "kpweaverobserver.h"
#include "kipiplugins_export.h"

namespace ThreadWeaver
{
    class JobCollection;
}

using namespace ThreadWeaver;

namespace KIPIPlugins
{

class KIPIPLUGINS_EXPORT KPActionThreadBase : public QThread
{
    Q_OBJECT

public:

    KPActionThreadBase(QObject* const parent=0);
    ~KPActionThreadBase();

    void cancel();
    void finish();

protected:

    void run();
    void appendJob(JobCollection* const job);

protected Q_SLOTS:

    void slotFinished();

private:

    class KPActionThreadBasePriv;
    KPActionThreadBasePriv* const d;
};

}  // namespace KIPIPlugins

#endif // KPACTIONTHREADBASE_H
