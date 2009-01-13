/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : a tool bar action object to display animated logo
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dlogoaction.moc"

// C++ includes.

#include <cmath>

// Qt includes.

#include <QPalette>
#include <QPixmap>
#include <QBoxLayout>
#include <QTimer>
#include <QPainter>

// KDE includes.

#include <kurllabel.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <klocale.h>

// Local includes.

#include "daboutdata.h"

namespace Digikam
{

class DLogoActionPriv
{
public:

    DLogoActionPriv()
    {
        alignOnright = true;
        timer        = 0;
        urlLabel     = 0;
        angle        = 0;

        // NOTE: rotation of logo is adapted to an exported PNG file generated from 
        // digikam/data/pics/banner-digikam.svgz and digikam/data/pics/banner-showfoto.svgz
        // using height of 33 pixels.
        logoCenter   = QPoint(125, 16);
        logoRect     = QRect(109, 0, 33, 33);
    }

    bool       alignOnright;

    int        angle;

    QTimer    *timer;

    QPoint     logoCenter;
    QRect      logoRect;

    QPixmap    bannerPix;
    QPixmap    logoPix;
    QPixmap    animPix;

    KUrlLabel *urlLabel;
};

DLogoAction::DLogoAction(QObject* parent, bool alignOnright)
           : KAction(parent), d(new DLogoActionPriv)
{
    setText("digikam.org");
    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
    {
        setIcon(KIcon("digikam"));
        d->bannerPix = QPixmap(KStandardDirs::locate("data", "digikam/data/banner-digikam.png"));
    }
    else
    {
        setIcon(KIcon("showfoto"));
        d->bannerPix = QPixmap(KStandardDirs::locate("data", "digikam/data/banner-showfoto.png"));
    }

    d->logoPix      = d->bannerPix.copy(d->logoRect);
    d->alignOnright = alignOnright;
    d->timer        = new QTimer();

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));
}

DLogoAction::~DLogoAction()
{
    delete d;
}

void DLogoAction::start()
{
    d->angle = 0;
    d->timer->start(100);
}

void DLogoAction::stop()
{
    d->angle = 0;
    d->timer->stop();
    if (d->urlLabel)
        d->urlLabel->setPixmap(d->bannerPix);
}

bool DLogoAction::running() const
{
    return d->timer->isActive();
}

void DLogoAction::slotTimeout()
{
    d->angle   = (d->angle + 10) % 360;
    d->animPix = d->bannerPix;

    QPainter p(&d->animPix);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipRect(d->logoRect);
    p.translate(d->logoCenter);
    p.rotate(d->angle);
    p.drawPixmap(-d->logoCenter.y(), -d->logoCenter.y(), d->logoPix);
    p.end();

    if (d->urlLabel)
        d->urlLabel->setPixmap(d->animPix);
}

QWidget* DLogoAction::createWidget(QWidget * parent)
{
    QToolBar *bar = qobject_cast<QToolBar*>(parent);

    // This action should only be used in a toolbar
    Q_ASSERT(bar != NULL);

    QWidget* container  = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(container);
    d->urlLabel         = new KUrlLabel(webProjectUrl().url(), QString(), bar);
    d->urlLabel->setMargin(0);
    d->urlLabel->setScaledContents(false);
    d->urlLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    d->urlLabel->setToolTip(i18n("Visit digiKam project website"));
    d->urlLabel->setPixmap(d->bannerPix);
    d->urlLabel->setFocusPolicy(Qt::NoFocus);

    layout->setMargin(0);
    layout->setSpacing(0);

    if (d->alignOnright)
        layout->addStretch();

    layout->addWidget(d->urlLabel);

    connect(d->urlLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(slotProcessUrl(const QString&)));

    return container;
}

void DLogoAction::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

} // namespace Digikam
