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

#include "daboutdata.h"

namespace Digikam
{

DLogoAction::DLogoAction(QObject* parent, bool alignOnright)
           : KAction(parent)
{
    m_alignOnright = alignOnright;
    setText(i18n("digikam.org"));
    setIcon(KIcon("digikam"));
}

QWidget* DLogoAction::createWidget( QWidget * parent )
{
    QToolBar *bar = qobject_cast<QToolBar*>(parent);

    // This action should only be used in a toolbar
    Q_ASSERT(bar != NULL);

    QWidget* container  = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(container);
    m_pixmapLogo        = new KUrlLabel(webProjectUrl().url(), QString(), bar);
    m_pixmapLogo->setMargin(0);
    m_pixmapLogo->setScaledContents(false);
    m_pixmapLogo->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    m_pixmapLogo->setToolTip(i18n("Visit digiKam project website"));
    m_pixmapLogo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/banner-digikam.png")));
    m_pixmapLogo->setFocusPolicy(Qt::NoFocus);

    layout->setMargin(0);
    layout->setSpacing(0);

    if (m_alignOnright)
        layout->addStretch();

    layout->addWidget(m_pixmapLogo);

    connect(m_pixmapLogo, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(slotProcessUrl(const QString&)));

    return container;
}

void DLogoAction::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

} // namespace Digikam
