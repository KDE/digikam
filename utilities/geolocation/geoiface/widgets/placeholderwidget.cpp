/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Placeholder widget for when backends are activated
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "placeholderwidget.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class PlaceholderWidget::Private
{
public:

    Private()
    {
        messageLabel = 0;
    }

    QLabel* messageLabel;
};

PlaceholderWidget::PlaceholderWidget(QWidget* const parent)
    : QFrame(parent),
      d(new Private)
{
    QVBoxLayout* const vboxlayout = new QVBoxLayout();
    setLayout(vboxlayout);

    d->messageLabel = new QLabel(i18n("Geolocation Interface"), this);
}

PlaceholderWidget::~PlaceholderWidget()
{
}

void PlaceholderWidget::setMessage(const QString& message)
{
    d->messageLabel->setText(message);
}

} // namespace Digikam
