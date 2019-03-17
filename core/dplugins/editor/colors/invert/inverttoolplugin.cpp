/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to invert colors.
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

#include "inverttoolplugin.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"
#include "invertfilter.h"

namespace DigikamEditorInvertToolPlugin
{

InvertToolPlugin::InvertToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

InvertToolPlugin::~InvertToolPlugin()
{
}

QString InvertToolPlugin::name() const
{
    return i18n("Invert Colors");
}

QString InvertToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon InvertToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("edit-select-invert"));
}

QString InvertToolPlugin::description() const
{
    return i18n("A tool to invert image colors");
}

QString InvertToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can invert colors from image.</p>");
}

QList<DPluginAuthor> InvertToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void InvertToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Invert"));
    ac->setObjectName(QLatin1String("editorwindow_color_invert"));
    // NOTE: Photoshop 7 use CTRL+I.
    ac->setShortcut(Qt::CTRL + Qt::Key_I);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotInvert()));

    addAction(ac);
}

void InvertToolPlugin::slotInvert()
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    ImageIface iface;
    InvertFilter invert(iface.original(), 0L);
    invert.startFilterDirectly();
    iface.setOriginal(i18n("Invert"), invert.filterAction(), invert.getTargetImage());

    qApp->restoreOverrideCursor();
}

} // namespace DigikamEditorInvertToolPlugin
