/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : an tool bar action object to display logo
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QPalette>
#include <QPixmap>
#include <QBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QTransform>

// KDE includes.

#include <kurllabel.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
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
        pos          = 0;
    }

    bool       alignOnright;

    int        pos;

    QTimer    *timer;

    QPixmap    logoPix;
    QPixmap    animPix;

    KUrlLabel *urlLabel;
};

DLogoAction::DLogoAction(QObject* parent, bool alignOnright)
           : KAction(parent), d(new DLogoActionPriv)
{
    setText("digikam.org");
    setIcon(KIcon("digikam"));

    d->logoPix      = QPixmap(KStandardDirs::locate("data", "digikam/data/banner-digikam.png"));
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
    d->pos = 0;
    d->timer->start(100);
}

void DLogoAction::stop()
{
    d->pos = 0;
    d->timer->stop();
    d->urlLabel->setPixmap(d->logoPix);
}

bool DLogoAction::running() const
{
    return d->timer->isActive();
}

void DLogoAction::slotTimeout()
{
    QTransform trans;
    trans.rotate(d->pos);
    const int size = d->logoPix.height();
    d->pos         = (d->pos + 10) % 360;
    d->animPix     = d->logoPix;
    QPixmap logo   = d->logoPix.copy(d->logoPix.width()-size, 0, size, size);
    logo           = logo.transformed(trans, Qt::SmoothTransformation);
    logo           = logo.copy((logo.width()/2) - (size/2), (logo.height()/2) - (size/2), size, size);

    QPainter p(&d->animPix);
    p.drawPixmap(d->logoPix.width()-size, 0, logo);
    p.end();

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
    d->urlLabel->setPixmap(d->logoPix);
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
