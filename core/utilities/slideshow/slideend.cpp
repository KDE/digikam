/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow end view
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideend.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPalette>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

SlideEnd::SlideEnd(QWidget* const parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setAutoFillBackground(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    palette.setColor(foregroundRole(), Qt::white);
    setPalette(palette);

    QFont fn(font());
    fn.setPointSize(fn.pointSize() + 10);
    fn.setBold(true);
    setFont(fn);

    QLabel* const logoLabel = new QLabel(this);
    logoLabel->setAlignment(Qt::AlignTop);

    QPixmap logo;

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        logo = QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48));
    }
    else
    {
        logo = QIcon::fromTheme(QLatin1String("showfoto")).pixmap(QSize(48,48));
    }

    logoLabel->setPixmap(logo);

    QLabel* const textLabel = new QLabel(i18n("Slideshow Completed.\nClick To Exit\nor press ESC..."), this);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(logoLabel, 1, 1, 1, 1);
    grid->addWidget(textLabel, 1, 2, 1, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
}

SlideEnd::~SlideEnd()
{
}

}  // namespace Digikam
