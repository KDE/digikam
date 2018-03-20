/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-13
 * Description : printer thread.
 *
 * Copyright (C) 2008      by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CALPRINTER_H
#define CALPRINTER_H

// Qt includes

#include <QMap>
#include <QThread>
#include <QUrl>

class QPrinter;

namespace Digikam
{

class CalPainter;

class CalPrinter : public QThread
{
    Q_OBJECT

public:

    CalPrinter(QPrinter* const printer,
               QMap<int, QUrl>& months,
               QObject* const parent);

    virtual ~CalPrinter();

protected:

    void run();

Q_SIGNALS:

    void pageChanged(int page);
    void totalBlocks(int total);
    void blocksFinished(int finished);

public Q_SLOTS:

    void cancel();

private:

    class Private;
    Private* const d;
};

}  // Namespace Digikam

#endif // CALPRINTER_H
