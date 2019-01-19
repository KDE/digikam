/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to create print compositions.
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

#include "printcreatorplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintwizard.h"

namespace Digikam
{

PrintCreatorPlugin::PrintCreatorPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

PrintCreatorPlugin::~PrintCreatorPlugin()
{
}

QString PrintCreatorPlugin::name() const
{
    return i18n("Print Creator");
}

QString PrintCreatorPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PrintCreatorPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("document-print"));
}

QString PrintCreatorPlugin::description() const
{
    return i18n("A tool to create print composition from images");
}

QString PrintCreatorPlugin::details() const
{
    return i18n("<p>This tool permit to back-process items (as assemble) before to print.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Different pre-defined paper sizes and layouts can be used to process files.</p>");
}

QList<DPluginAuthor> PrintCreatorPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"),
                             i18n("Developer and Maintainer"))
            << DPluginAuthor(QString::fromUtf8("Todd Shoemaker"),
                             QString::fromUtf8("todd at theshoemakers dot net"),
                             QString::fromUtf8("(C) 2003-2004"),
                             i18n("Author"))
            << DPluginAuthor(QString::fromUtf8("Angelo Naselli"),
                             QString::fromUtf8("anaselli at linux dot it"),
                             QString::fromUtf8("(C) 2007-2013"))
            << DPluginAuthor(QString::fromUtf8("Andreas Trink"),
                             QString::fromUtf8("atrink at nociaro dot org"),
                             QString::fromUtf8("(C) 2010"),
                             i18n("Contributor"))
            ;
}

void PrintCreatorPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Print Creator..."));
    ac->setObjectName(QLatin1String("printcreator"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPrintCreator()));

    addAction(ac);
}

void PrintCreatorPlugin::slotPrintCreator()
{
    QPointer<AdvPrintWizard> wzrd = new AdvPrintWizard(0, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
