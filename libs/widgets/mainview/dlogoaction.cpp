/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : a tool bar action object to display animated logo
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dlogoaction.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QPalette>
#include <QPixmap>
#include <QBoxLayout>
#include <QTimer>
#include <QPainter>

// KDE includes

#include <kurllabel.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <klocale.h>

// Local includes

#include "daboutdata.h"

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

    bool       alignOnright;

    int        progressCount;         // Position of animation.

    QTimer*    progressTimer;

    QPixmap    progressPixmap;

    KUrlLabel* urlLabel;
};

DLogoAction::DLogoAction(QObject* const parent, bool alignOnright)
    : KAction(parent), d(new Private)
{
    setText("digikam.org");

    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
    {
        setIcon(KIcon("digikam"));
        d->progressPixmap = QPixmap(KStandardDirs::locate("data", "digikam/data/banner-digikam.png"));
    }
    else
    {
        setIcon(KIcon("showfoto"));
        d->progressPixmap = QPixmap(KStandardDirs::locate("data", "showfoto/data/banner-showfoto.png"));
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
        d->urlLabel->setPixmap(d->progressPixmap.copy(0, 0, 144, 32));
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
        d->urlLabel->setPixmap(anim);
    }

    d->progressTimer->start(100);
}

QWidget* DLogoAction::createWidget(QWidget* parent)
{
    QWidget* const container  = new QWidget(parent);
    QHBoxLayout* const layout = new QHBoxLayout(container);
    d->urlLabel               = new KUrlLabel(DAboutData::webProjectUrl().url(), QString(), container);
    d->urlLabel->setMargin(0);
    d->urlLabel->setScaledContents(false);
    d->urlLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    d->urlLabel->setToolTip(i18n("Visit digiKam project website"));
    d->urlLabel->setPixmap(d->progressPixmap.copy(0, 0, 144, 32));
    d->urlLabel->setFocusPolicy(Qt::NoFocus);

    layout->setMargin(0);
    layout->setSpacing(0);

    if (d->alignOnright)
    {
        layout->addStretch();
    }

    layout->addWidget(d->urlLabel);

    connect(d->urlLabel, SIGNAL(leftClickedUrl(QString)),
            this, SLOT(slotProcessUrl(QString)));

    return container;
}

void DLogoAction::deleteWidget(QWidget* widget)
{
    stop();
    d->urlLabel = 0;
    KAction::deleteWidget(widget);
}

void DLogoAction::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

} // namespace Digikam
