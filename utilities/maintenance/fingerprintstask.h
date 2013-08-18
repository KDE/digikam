/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for finger-prints generator.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FINGERPRINTS_TASK_H
#define FINGERPRINTS_TASK_H

// Qt includes

#include <QPixmap>
#include <QThread>

// KDE includes

#include <threadweaver/Job.h>

using namespace ThreadWeaver;

namespace Digikam
{

class LoadingDescription;
class DImg;

class FingerprintsTask : public Job
{
    Q_OBJECT

public:

    FingerprintsTask();
    ~FingerprintsTask();

    void setItem(const QString& path);

Q_SIGNALS:

    void signalFinished(const QImage&);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FINGERPRINTS_TASK_H */
