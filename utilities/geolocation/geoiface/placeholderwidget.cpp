/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-12-05
 * @brief  Placeholder widget for when backends are activated
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "placeholderwidget.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

namespace GeoIface
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

    d->messageLabel = new QLabel(i18n("GeoIface"), this);
}

PlaceholderWidget::~PlaceholderWidget()
{
}

void PlaceholderWidget::setMessage(const QString& message)
{
    d->messageLabel->setText(message);
}

} /* namespace GeoIface */
