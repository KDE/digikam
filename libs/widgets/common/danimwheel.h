/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-01-04
 * Description : an animated wheel
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DANIM_WHEEL_H
#define DANIM_WHEEL_H

// Qt includes.

#include <QObject>
#include <QPoint>

// Local includes.

#include "digikam_export.h"

class QPainter;

namespace Digikam
{

class DAnimWheelPriv;

class DIGIKAM_EXPORT DAnimWheel : public QObject
{
  Q_OBJECT

public:

    DAnimWheel(QObject* parent, int wheelSize);
    ~DAnimWheel();

    void start();
    void cont();
    void stop();
    bool running() const;
    int  wheelSize() const;

    void drawWheel(QPainter *p, const QPoint& center);

Q_SIGNALS:

    void signalWheelTimeOut();

private:

    DAnimWheelPriv* const d;
};

} // namespace Digikam

#endif /* DANIM_WHEEL_H */
