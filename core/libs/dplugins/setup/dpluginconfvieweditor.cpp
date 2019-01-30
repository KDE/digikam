/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external editor plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dpluginconfvieweditor.h"

// Local includes

#include "dplugineditor.h"
#include "dpluginloader.h"

namespace Digikam
{

DPluginConfViewEditor::DPluginConfViewEditor(QWidget* const parent)
    : DPluginConfView(parent)
{
    loadPlugins();
}

DPluginConfViewEditor::~DPluginConfViewEditor()
{
}

void DPluginConfViewEditor::loadPlugins()
{
    DPluginLoader* const loader = DPluginLoader::instance();

    if (loader)
    {
        foreach (DPlugin* const tool, loader->allPlugins())
        {
            DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(tool);

            if (edit)
            {
                appendPlugin(edit);
            }
        }
    }

    // Sort items by plugin names.
    sortItems(0, Qt::AscendingOrder);
}

} // namespace Digikam
