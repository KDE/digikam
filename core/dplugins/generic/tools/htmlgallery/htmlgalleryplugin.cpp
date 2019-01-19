/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to generate HTML image galleries.
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

#include "htmlgalleryplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "htmlwizard.h"

namespace Digikam
{

HtmlGalleryPlugin::HtmlGalleryPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

HtmlGalleryPlugin::~HtmlGalleryPlugin()
{
}

QString HtmlGalleryPlugin::name() const
{
    return i18n("Html Gallery");
}

QString HtmlGalleryPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HtmlGalleryPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-html"));
}

QString HtmlGalleryPlugin::description() const
{
    return i18n("A tool to generate HTML gallery from images");
}

QString HtmlGalleryPlugin::details() const
{
    return i18n("<p>This tool permit to back-process items (as resize) before to create W3C compliant html gallery.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Themable HTML template with different layout can be used to assemble files on a gallery.</p>");
}

QList<DPluginAuthor> HtmlGalleryPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2019"),
                             i18n("Developer and Maintainer"))
            << DPluginAuthor(QString::fromUtf8("Aurelien Gateau"),
                             QString::fromUtf8("agateau at kde dot org"),
                             QString::fromUtf8("(C) 2006-2009"),
                             i18n("Former Author and Maintainer"))
            << DPluginAuthor(QString::fromUtf8("Gianluca Urgese"),
                             QString::fromUtf8("giasone dot 82 at gmail dot com"),
                             QString::fromUtf8("(C) 2010"))
            ;
}

void HtmlGalleryPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create Html gallery..."));
    ac->setObjectName(QLatin1String("htmlgallery"));
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_H);
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotHtmlGallery()));

    addAction(ac);
}

void HtmlGalleryPlugin::slotHtmlGallery()
{
    QPointer<HTMLWizard> wzrd = new HTMLWizard(0, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
