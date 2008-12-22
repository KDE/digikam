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

// Qt includes.

#include <qtooltip.h>
#include <qpixmap.h>

// KDE includes.

#include <kurllabel.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>

// Local includes.

#include "daboutdata.h"
#include "dlogoaction.h"

namespace Digikam
{

DLogoAction::DLogoAction(QObject* parent, const char* name)
           : KAction(parent, name)
{
    setText("digikam.org");
    setIcon("digikam");
}

int DLogoAction::plug(QWidget *widget, int index)
{
    if (kapp && !kapp->authorizeKAction(name()))
        return -1;

    if ( widget->inherits( "KToolBar" ) )
    {
        KToolBar *bar         = (KToolBar *)widget;
        int id                = getToolButtonID();
        KURLLabel *pixmapLogo = new KURLLabel(Digikam::webProjectUrl(), QString(), bar);
        pixmapLogo->setMargin(0);
        pixmapLogo->setScaledContents(false);
        pixmapLogo->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
        QToolTip::add(pixmapLogo, i18n("Visit digiKam project website"));
        KGlobal::dirs()->addResourceType("banner-digikam", KGlobal::dirs()->kde_default("data") + "digikam/data");
        QString directory = KGlobal::dirs()->findResourceDir("banner-digikam", "banner-digikam.png");
        pixmapLogo->setPixmap(QPixmap( directory + "banner-digikam.png" ));
        pixmapLogo->setFocusPolicy(QWidget::NoFocus);

        bar->insertWidget(id, pixmapLogo->width(), pixmapLogo);
        bar->alignItemRight(id);

        addContainer(bar, id);

        connect(bar, SIGNAL(destroyed()),
                this, SLOT(slotDestroyed()));

        connect(pixmapLogo, SIGNAL(leftClickedURL(const QString&)),
                this, SLOT(slotProcessURL(const QString&)));

        return containerCount() - 1;
    }

    int containerId = KAction::plug( widget, index );

    return containerId;
}

void DLogoAction::slotProcessURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

} // namespace Digikam
