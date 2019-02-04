/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert 8 bits color depth to 16.
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

#ifndef DIGIKAM_CONVERT8TO16TOOL_PLUGIN_H
#define DIGIKAM_CONVERT8TO16TOOL_PLUGIN_H

// Local includes

#include "dplugineditor.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.editor.Convert8To16Tool"

using namespace Digikam;

namespace DigikamEditorConvert8To16ToolPlugin
{

class Convert8To16ToolPlugin : public DPluginEditor
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginEditor)

public:

    explicit Convert8To16ToolPlugin(QObject* const parent = 0);
    ~Convert8To16ToolPlugin();

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;

    void setup(QObject* const) override;

private Q_SLOTS:

    void slotConvert8To16();
};

} // namespace DigikamEditorConvert8To16ToolPlugin

#endif // DIGIKAM_CONVERT8TO16TOOL_PLUGIN_H
