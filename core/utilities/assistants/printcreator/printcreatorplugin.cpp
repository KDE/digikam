/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : plugin to create prints.
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
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintwizard.h"

namespace Digikam
{

PrintCreatorPlugin::PrintCreatorPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

QString PrintCreatorPlugin::name() const
{
    return i18n("Print Creator");
}

QString PrintCreatorPlugin::id() const
{
    return QLatin1String("PrintCreator");
}

QString PrintCreatorPlugin::version() const
{
    return QLatin1String("1.0");
}

QString PrintCreatorPlugin::description() const
{
    return i18n("A Tool to Create Prints.");
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
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"),
                             i18n("Developer and Maintainer"))
            << DPluginAuthor(QLatin1String("Todd Shoemaker"),
                             QLatin1String("todd at theshoemakers dot net"),
                             QLatin1String("(C) 2003-2004"),
                             i18n("Author"))
            << DPluginAuthor(QLatin1String("Angelo Naselli"),
                             QLatin1String("anaselli at linux dot it"),
                             QLatin1String("(C) 2007-2013"))
            << DPluginAuthor(QLatin1String("Andreas Trink"),
                             QLatin1String("atrink at nociaro dot org"),
                             QLatin1String("(C) 2010"),
                             i18n("Contributor"))
            ;
}

void PrintCreatorPlugin::setup()
{
    DPluginAction* const ac = new DPluginAction(this);
    ac->setIcon(QIcon::fromTheme(QLatin1String("document-print")));
    ac->setText(i18nc("@action", "Print Creator..."));
    ac->setActionName(QLatin1String("printcreator"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPrintCreator()));

    addAction(ac);
}

void PrintCreatorPlugin::slotPrintCreator()
{
    QPointer<AdvPrintWizard> wzrd = new AdvPrintWizard(0, infoIface());
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
