/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert 8 bits color depth to 16
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

#include "convert8to16toolplugin.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"

namespace Digikam
{

Convert8To16ToolPlugin::Convert8To16ToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

Convert8To16ToolPlugin::~Convert8To16ToolPlugin()
{
}

QString Convert8To16ToolPlugin::name() const
{
    return i18n("Convert to 16 bits");
}

QString Convert8To16ToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert8To16ToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth8to16"));
}

QString Convert8To16ToolPlugin::description() const
{
    return i18n("A tool to convert color depth to 16 bits");
}

QString Convert8To16ToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can convert image color depth to 16 bits.</p>");
}

QList<DPluginAuthor> Convert8To16ToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2005-2019"))
            ;
}

void Convert8To16ToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "16 bits"));
    ac->setObjectName(QLatin1String("editorwindow_convertto16bits"));
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotConvert8To16()));

    addAction(ac);
}

void Convert8To16ToolPlugin::slotConvert8To16()
{
    ImageIface iface;

    if (iface.originalSixteenBit())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("This image is already using a depth of 16 bits / color / pixel."));
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(64);
    qApp->restoreOverrideCursor();}

} // namespace Digikam
