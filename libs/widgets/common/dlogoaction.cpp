/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : an tool bar action object to display logo
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QPixmap>
#include <QBoxLayout>

// KDE includes.

#include <kurllabel.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
#include <klocale.h>

// Local includes.

#include "dlogoaction.h"

namespace Digikam 
{

DLogoAction::DLogoAction(QObject* parent)
           : KAction(parent)
{
}

QWidget* DLogoAction::createWidget( QWidget * parent )
{
    QToolBar *bar = qobject_cast<QToolBar*>(parent);
    
    // This action should only be used in a toolbar
    Q_ASSERT(bar != NULL);
    
    QWidget* container    = new QWidget(parent);
    QHBoxLayout* layout   = new QHBoxLayout(container);
    KUrlLabel *pixmapLogo = new KUrlLabel("http://www.digikam.org", QString(), bar);
    pixmapLogo->setMargin(0);
    pixmapLogo->setScaledContents(false);
    pixmapLogo->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    pixmapLogo->setToolTip(i18n("Visit digiKam project website"));
    pixmapLogo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png")));
    pixmapLogo->setFocusPolicy(Qt::NoFocus);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(pixmapLogo);
    
    connect(pixmapLogo, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(slotProcessUrl(const QString&)));

    return container;
}

void DLogoAction::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

} // namespace Digikam
