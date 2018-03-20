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

#include "calprinter.h"

// Qt includes

#include <QPrinter>

// Local includes

#include "calpainter.h"

namespace Digikam
{

class CalPrinter::Private
{
public:

    explicit Private()
      : cancelled(false),
        printer(0),
        painter(0)
    {
    }

    bool             cancelled;

    QMap<int, QUrl>  months;
    QPrinter*        printer;

    CalPainter*      painter;
};

CalPrinter::CalPrinter(QPrinter* const printer,
                       QMap<int, QUrl>& months,
                       QObject* const parent)
    : QThread(parent),
      d(new Private)
{
    d->printer   = printer;
    d->painter   = new CalPainter(d->printer);
    d->months    = months;
    d->cancelled = false;
}

CalPrinter::~CalPrinter()
{
    delete d->painter;
    delete d;
}

void CalPrinter::run()
{
    connect(d->painter, SIGNAL(signalTotal(int)),
            this, SIGNAL(totalBlocks(int)));

    connect(d->painter, SIGNAL(signalProgress(int)),
            this, SIGNAL(blocksFinished(int)));

    int currPage = 0;

    foreach(const int month, d->months.keys())
    {
        emit pageChanged(currPage);

        if (currPage)
        {
            d->printer->newPage();
        }

        ++currPage;

        d->painter->setImage(d->months.value(month));
        d->painter->paint(month);

        if (d->cancelled)
        {
            break;
        }
    }

    d->painter->end();

    emit pageChanged(currPage);
}

void CalPrinter::cancel()
{
    d->painter->cancel();
    d->cancelled = true;
}

}  // Namespace Digikam
