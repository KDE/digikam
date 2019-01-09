/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Flickr web-service.
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

#include "flickrplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "flickrwindow.h"

namespace Digikam
{

FlickrPlugin::FlickrPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

FlickrPlugin::~FlickrPlugin()
{
    delete m_toolDlg;
}

QString FlickrPlugin::name() const
{
    return i18n("Flickr");
}

QString FlickrPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FlickrPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-flickr"));
}

QString FlickrPlugin::description() const
{
    return i18n("A tool to export to Flickr web-service");
}

QString FlickrPlugin::details() const
{
    return i18n("<p>This tool permit to export items to Flickr web-service.</p>"
                "<p>See Flickr web site for details: <a href='https://box.com/'>https://box.com/</a></p>");
}

QList<DPluginAuthor> FlickrPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2008-2019"))
            << DPluginAuthor(QLatin1String("Maik Qualmann"),
                             QLatin1String("metzpinguin at gmail dot com"),
                             QLatin1String("(C) 2017-2019"))
            << DPluginAuthor(QLatin1String("Vardhman Jain"),
                             QLatin1String("vardhman at gmail dot com"),
                             QLatin1String("(C) 2005-2008"))
            << DPluginAuthor(QLatin1String("Luka Renko"),
                             QLatin1String("lure at kubuntu dot org"),
                             QLatin1String("(C) 2009"))
            << DPluginAuthor(QLatin1String("Shourya Singh Gupta"),
                             QLatin1String("shouryasgupta at gmail dot com"),
                             QLatin1String("(C) 2015"))
            ;
}

void FlickrPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Flickr..."));
    ac->setObjectName(QLatin1String("export_flickr"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_R);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFlickr()));

    addAction(ac);
}

void FlickrPlugin::slotFlickr()
{
    if (!reactivateToolDialog())
    {
        delete m_toolDlg;
        m_toolDlg = new FlickrWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
