/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *               inspired from  Gwenview code (Aurelien Gateau).
 *
 * Copyright (C) 2009-2011 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef SIGNALBLOCKER_H
#define SIGNALBLOCKER_H

// Qt includes

#include <QObject>

namespace Digikam
{

/**
  * An RAII class to block and unblock signals from a QObject instance
  */
class SignalBlocker
{

public:

    explicit SignalBlocker(QObject* const object)
    {
        mObject     = object;
        mWasBlocked = object->blockSignals(true);
    }

    ~SignalBlocker()
    {
        mObject->blockSignals(mWasBlocked);
    }

private:

    bool     mWasBlocked;

    QObject* mObject;
};

} // namespace Digikam

#endif /* SIGNALBLOCKER_H */
