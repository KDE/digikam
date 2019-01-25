/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to ImageShack web-service.
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

#include "imageshackplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageshackwindow.h"

namespace GenericDigikamImageShackPlugin
{

ImageShackPlugin::ImageShackPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

ImageShackPlugin::~ImageShackPlugin()
{
    delete m_toolDlg;
}

QString ImageShackPlugin::name() const
{
    return i18n("ImageShack");
}

QString ImageShackPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ImageShackPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-imageshack"));
}

QString ImageShackPlugin::description() const
{
    return i18n("A tool to export to ImageShack web-service");
}

QString ImageShackPlugin::details() const
{
    return i18n("<p>This tool permit to export items to ImageShack web-service.</p>"
                "<p>See ImageShack web site for details: <a href='https://imageshack.us/'>https://imageshack.us/</a></p>");
}

QList<DPluginAuthor> ImageShackPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Dodon Victor"),
                             QString::fromUtf8("dodonvictor at gmail dot com"),
                             QString::fromUtf8("(C) 2012"))
            ;
}

void ImageShackPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Imageshack..."));
    ac->setObjectName(QLatin1String("export_imageshack"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_M);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotImageShack()));

    addAction(ac);
}

void ImageShackPlugin::slotImageShack()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new ImageShackWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace GenericDigikamImageShackPlugin
