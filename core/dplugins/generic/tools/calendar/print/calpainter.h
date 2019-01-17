/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : painter class to draw calendar.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_CAL_PAINTER_H
#define DIGIKAM_CAL_PAINTER_H

// Qt includes

#include <QObject>
#include <QPainter>

class QPaintDevice;

namespace Digikam
{

class CalPainter : public QObject, public QPainter
{
    Q_OBJECT

public:

    explicit CalPainter(QPaintDevice* const pd);
    ~CalPainter();

    void setImage(const QUrl& imagePath);
    void paint(int month);

Q_SIGNALS:

    void signalTotal(int total);
    void signalProgress(int value);
    void signalFinished();

public Q_SLOTS:

    void cancel();

private:

    class Private;
    Private* const d;
};

}  // Namespace Digikam

#endif // DIGIKAM_CAL_PAINTER_H
