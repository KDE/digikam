/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : new items finder.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef NEWITEMSFINDER_H
#define NEWITEMSFINDER_H

// Qt includes

#include <QTime>
#include <QString>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class NewItemsFinder : public ProgressItem
{
    Q_OBJECT

public:

    NewItemsFinder(bool defer = false);
    ~NewItemsFinder();

Q_SIGNALS:

    void signalComplete();

private Q_SLOTS:

    void slotScanStarted(const QString& info);
    void slotProgressValue(float);
    void slotScanCompleted();
    void slotCancel();

private:

    QTime m_duration;
};

} // namespace Digikam

#endif /* NEWITEMSFINDER_H */
