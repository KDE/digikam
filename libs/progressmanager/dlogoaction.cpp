/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : a tool bar action object to display animated logo
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dlogoaction.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPalette>
#include <QPixmap>
#include <QBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "daboutdata.h"
#include "dactivelabel.h"

namespace Digikam
{

class DLogoAction::Private
{
public:

    Private()
    {
        alignOnright  = true;
        progressTimer = 0;
        urlLabel      = 0;
        progressCount = 0;
    }

    bool          alignOnright;

    int           progressCount;         // Position of animation.

    QTimer*       progressTimer;

    QPixmap       progressPixmap;

    DActiveLabel* urlLabel;
};

DLogoAction::DLogoAction(QObject* const parent, bool alignOnright)
    : QWidgetAction(parent),
      d(new Private)
{
    setText(QLatin1String("digikam.org"));

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        setIcon(QIcon::fromTheme(QLatin1String("digikam")));
        d->progressPixmap = QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                    QLatin1String("digikam/data/banner-digikam.png")));
    }
    else
    {
        setIcon(QIcon::fromTheme(QLatin1String("showfoto")));
        d->progressPixmap = QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                    QLatin1String("showfoto/data/banner-showfoto.png")));
    }

    d->alignOnright  = alignOnright;
    d->progressTimer = new QTimer(this);
    d->progressTimer->setSingleShot(true);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

DLogoAction::~DLogoAction()
{
    delete d;
}

void DLogoAction::start()
{
    d->progressCount = 0;
    d->progressTimer->start(100);
}

void DLogoAction::stop()
{
    d->progressCount = 0;
    d->progressTimer->stop();

    if (d->urlLabel)
    {
        d->urlLabel->updateData(DAboutData::webProjectUrl(),
                                d->progressPixmap.copy(0, 0, 144, 32).toImage());
    }
}

bool DLogoAction::running() const
{
    return d->progressTimer->isActive();
}

void DLogoAction::slotProgressTimerDone()
{
    QPixmap anim(d->progressPixmap.copy(0, d->progressCount*32, 144, 32));
    d->progressCount++;

    if (d->progressCount == 36)
    {
        d->progressCount = 0;
    }

    if (d->urlLabel)
    {
        d->urlLabel->updateData(DAboutData::webProjectUrl(), anim.toImage());
    }

    d->progressTimer->start(100);
}

QWidget* DLogoAction::createWidget(QWidget* parent)
{
    QWidget* const container  = new QWidget(parent);
    QHBoxLayout* const layout = new QHBoxLayout(container);
    d->urlLabel               = new DActiveLabel(DAboutData::webProjectUrl(), QString(), container);
    d->urlLabel->setToolTip(i18n("Visit digiKam project website"));
    d->urlLabel->updateData(DAboutData::webProjectUrl(), d->progressPixmap.copy(0, 0, 144, 32).toImage());

    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    if (d->alignOnright)
    {
        layout->addStretch();
    }

    layout->addWidget(d->urlLabel);

    return container;
}

void DLogoAction::deleteWidget(QWidget* widget)
{
    stop();
    d->urlLabel = 0;
    QWidgetAction::deleteWidget(widget);
}

} // namespace Digikam
