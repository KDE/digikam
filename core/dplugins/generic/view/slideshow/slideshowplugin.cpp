/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to render slideshow.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "slideshowplugin.h"

// Qt includes

#include <QPointer>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "slideshow.h"

namespace GenericDigikamSlideShowPlugin
{

SlideShowPlugin::SlideShowPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

SlideShowPlugin::~SlideShowPlugin()
{
}

QString SlideShowPlugin::name() const
{
    return i18n("SlideShow");
}

QString SlideShowPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SlideShowPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-presentation"));
}

QString SlideShowPlugin::description() const
{
    return i18n("A tool to render slideshow");
}

QString SlideShowPlugin::details() const
{
    return i18n("<p>This tool render a serie of items as a simple slide-show.</p>"
                "<p>Plenty of items properties can be displayed as overlay while running.</p>"
                "<p>This tool can play album contents in recursive mode with children albums if any.</p>");
}

QList<DPluginAuthor> SlideShowPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Enrico Ros <eros dot kde at email dot it>"),
                             QString::fromUtf8("eros dot kde at email dot it"),
                             QString::fromUtf8("(C) 2004"))
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void SlideShowPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setObjectName(QLatin1String("slideshow"));
    ac->setActionCategory(DPluginAction::GenericView);

    DInfoInterface* const iface = infoIface(parent);

    if (iface && iface->supportAlbums())
    {
        QMenu* const slideShowAction = new QMenu(i18n("Slideshow"), parent);
        slideShowAction->setIcon(icon());
        ac->setMenu(slideShowAction);

        QAction* const slideShowAllAction = new QAction(i18n("All"), parent);
        slideShowAllAction->setObjectName(QLatin1String("slideshow_all"));
        slideShowAllAction->setShortcut(Qt::Key_F9);
        slideShowAction->addAction(d->slideShowAllAction);

        connect(slideShowAllAction, SIGNAL(triggered()),
                this, SLOT(slotMenuSlideShowAll()));

        QCation* const slideShowSelectionAction = new QAction(i18n("Selection"), parent);
        slideShowSelectionAction->setObjectName(QLatin1String("slideshow_selected"));
        slideShowSelectionAction->setShortcut(Qt::ALT+Qt::Key_F9);
        slideShowAction->addAction(d->slideShowSelectionAction);

        connect(slideShowSelectionAction, SIGNAL(triggered()),
                this, SLOT(slotMenuSlideShowSelection()));

        QAction* const slideShowRecursiveAction = new QAction(i18n("With All Sub-Albums"), parent);
        slideShowRecursiveAction->setObjectName(QLatin1String("slideshow_recursive"));
        slideShowRecursiveAction->setShortcut(Qt::SHIFT+Qt::Key_F9);
        slideShowAction->addAction(d->slideShowRecursiveAction);

        connect(slideShowRecursiveAction, SIGNAL(triggered()),
                this, SLOT(slotMenuSlideShowRecursive()));
    }
    else
    {
        ac->setText(i18nc("@action", "Slideshow"));
        ac->setShortcut(Qt::Key_F9);

        connect(ac, SIGNAL(triggered(bool)),
                this, SLOT(slotSlideShow()));
    }

    addAction(ac);
}

void SlideShowPlugin::slotSlideShow()
{
    DInfoInterface* const iface = infoIface(sender());
}

void SlideShowPlugin::slotMenuSlideShowAll()
{
    /*
    DInfoInterface* const iface = infoIface(sender());

    QPointer<PresentationMngr> mngr = new PresentationMngr(this);

    foreach (const QUrl& url, iface->currentSelectedItems())
    {
        DItemInfo info(iface->itemInfo(url));
        mngr->addFile(url, info.comment());
        qApp->processEvents();
    }

    mngr->setPlugin(this);
    mngr->showConfigDialog();
    */
}

void SlideShowPlugin::slotMenuSlideShowSelection()
{
}

void SlideShowPlugin::slotMenuSlideShowRecursive()
{
}

} // namespace GenericDigikamSlideShowPlugin
