/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-13
 * Description : printer thread.
 *
 * Copyright (C) 2008 by Orgad Shaneh <orgads at gmail dot com>
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

CalPrinter::CalPrinter(QPrinter* const printer,
                       QMap<int, QUrl>& months,
                       QObject* const parent)
    : QThread(parent)
{
    printer_   = printer;
    painter_   = new CalPainter(printer_);
    months_    = months;
    cancelled_ = false;
}

CalPrinter::~CalPrinter()
{
    delete painter_;
}

void CalPrinter::run()
{
    connect(painter_, SIGNAL(signalTotal(int)),
            this, SIGNAL(totalBlocks(int)));

    connect(painter_, SIGNAL(signalProgress(int)),
            this, SIGNAL(blocksFinished(int)));

    int currPage = 0;

    foreach(const int month, months_.keys())
    {
        emit pageChanged(currPage);

        if (currPage)
        {
            printer_->newPage();
        }

        ++currPage;

        painter_->setImage(months_.value(month));
        painter_->paint(month);

        if (cancelled_)
        {
            break;
        }
    }

    emit pageChanged(currPage);
}

void CalPrinter::cancel()
{
    painter_->cancel();
    cancelled_ = true;
}

}  // Namespace Digikam
