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
qMakePairmake
// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "slideshow.h"

namespace Digikam
{

class Q_DECL_HIDDEN SlideShowPlugin::Private
{

public:

    explicit Private()
        : slideShowAction(0),
          slideShowAllAction(0),
          slideShowSelectionAction(0),
          slideShowRecursiveAction(0)
    {
    }

    QMenu*   slideShowAction;
    QAction* slideShowAllAction;
    QAction* slideShowSelectionAction;
    QAction* slideShowRecursiveAction;
};

SlideShowPlugin::SlideShowPlugin(QObject* const parent)
    : DPluginGeneric(parent),
      d(new Private)
{
}

SlideShowPlugin::~SlideShowPlugin()
{
    delete d;
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
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void SlideShowPlugin::setup(QObject* const parent)
{
    d->slideShowAction = new QMenu(i18n("Slideshow"), parent);
    d->slideShowAction->setIcon(icon());

    d->slideShowAllAction = new QAction(i18n("All"), parent);
    d->slideShowAllAction->setObjectName(QLatin1String("slideshow_all"));
    d->slideShowAllAction->setShortcut(Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowAllAction);

    connect(d->slideShowAllAction, SIGNAL(triggered()),
            this, SLOT(slotSlideShowAll()));

    d->slideShowSelectionAction = new QAction(i18n("Selection"), parent);
    d->slideShowSelectionAction->setObjectName(QLatin1String("slideshow_selected"));
    d->slideShowSelectionAction->setShortcut(Qt::ALT+Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowSelectionAction);

    connect(d->slideShowSelectionAction, SIGNAL(triggered()),
            this, SLOT(slotSlideShowSelection()));

    d->slideShowRecursiveAction = new QAction(i18n("With All Sub-Albums"), parent);
    d->slideShowRecursiveAction->setObjectName(QLatin1String("slideshow_recursive"));
    d->slideShowRecursiveAction->setShortcut(Qt::SHIFT+Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowRecursiveAction);

    connect(d->slideShowRecursiveAction, SIGNAL(triggered()),
            this, SLOT(slotSlideShowRecursive()));

    DPluginAction* const ac = new DPluginAction(parent);
    ac->setMenu(d->slideShowAction);
    ac->setObjectName(QLatin1String("slideshow"));
    ac->setActionCategory(DPluginAction::GenericView);

    addAction(ac);
}

void SlideShowPlugin::slotSlideShowAll()
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

void SlideShowPlugin::slotSlideShowSelection()
{
}

void SlideShowPlugin::slotSlideShowRecursive()
{
}

} // namespace Digikam
