/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow error view
 *
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideerror.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPalette>

// KDE includes

#include <kdialog.h>
#include <klocale.h>

namespace Digikam
{

class SlideError::Private
{

public:

    Private() :
        errorMsg(0)
    {
    }

    QLabel* errorMsg;
};

SlideError::SlideError(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setAutoFillBackground(true);

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);

    d->errorMsg = new QLabel(this);
    d->errorMsg->setAlignment(Qt::AlignCenter);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->errorMsg, 1, 0, 1, 3 );
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
}

SlideError::~SlideError()
{
    delete d;
}

void SlideError::setCurrentUrl(const KUrl& url)
{
    d->errorMsg->setText(i18n("An error has occurred to show item\n%1", url.fileName()));
}

}  // namespace Digikam
