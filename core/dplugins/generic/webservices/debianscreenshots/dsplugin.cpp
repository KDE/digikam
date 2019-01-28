/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a tool to export images to Debian Screenshots site
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

#include "dsplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dswindow.h"

namespace DigikamGenericDebianScreenshotsPlugin
{

DSPlugin::DSPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

DSPlugin::~DSPlugin()
{
    delete m_toolDlg;
}

QString DSPlugin::name() const
{
    return i18n("DebianScreenshots");
}

QString DSPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DSPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-debianscreenshots"));
}

QString DSPlugin::description() const
{
    return i18n("A tool to export to Debian Screenshots site");
}

QString DSPlugin::details() const
{
    return i18n("<p>This tool permit to export items to Debian Screenshots site.</p>"
                "<p>See Debian Screenshots web site for details: <a href='https://screenshots.debian.net/'>https://screenshots.debian.net/</a></p>");
}

QList<DPluginAuthor> DSPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Pau Garcia i Quiles"),
                             QString::fromUtf8("pgquiles at elpauer dot org"),
                             QString::fromUtf8("(C) 2010"))
            ;
}

void DSPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Debian Screenshots..."));
    ac->setObjectName(QLatin1String("export_debianscreenshot"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotDebianScreenshots()));

    addAction(ac);
}

void DSPlugin::slotDebianScreenshots()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new DSWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericDebianScreenshotsPlugin
