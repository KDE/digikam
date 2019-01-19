/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to resize an image with content analysis
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

#include "contentawareresizetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "contentawareresizetool.h"

namespace Digikam
{

ContentAwareResizeToolPlugin::ContentAwareResizeToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

ContentAwareResizeToolPlugin::~ContentAwareResizeToolPlugin()
{
}

QString ContentAwareResizeToolPlugin::name() const
{
    return i18n("Liquid Rescale");
}

QString ContentAwareResizeToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ContentAwareResizeToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-scale"));
}

QString ContentAwareResizeToolPlugin::description() const
{
    return i18n("A tool to resize an image with content analysis");
}

QString ContentAwareResizeToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can resize an image with content analysis.</p>");
}

QList<DPluginAuthor> ContentAwareResizeToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Julien Pontabry"),
                             QString::fromUtf8("julien dot pontabry at ulp dot u-strasbg dot fr"),
                             QString::fromUtf8("(C) 2009-2010"))
            << DPluginAuthor(QString::fromUtf8("Julien Narboux"),
                             QString::fromUtf8("julien at narboux dot fr"),
                             QString::fromUtf8("(C) 2009-2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"))
            ;
}

void ContentAwareResizeToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Liquid Rescale..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_contentawareresizing"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotContentAwareResize()));

    addAction(ac);
}

void ContentAwareResizeToolPlugin::slotContentAwareResize()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ContentAwareResizeTool* const tool = new ContentAwareResizeTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
