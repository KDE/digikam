/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image plugins interface for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin.h"

// Local includes

#include "editortool.h"
#include "editortooliface.h"

namespace Digikam
{

ImagePlugin::ImagePlugin(QObject* const parent, const QString& name)
    : QObject(parent)
{
    setObjectName(name);
}

ImagePlugin::~ImagePlugin()
{
}

void ImagePlugin::setEnabledSelectionActions(bool)
{
}

void ImagePlugin::setEnabledActions(bool)
{
}

void ImagePlugin::loadTool(EditorTool* const tool)
{
    EditorToolIface::editorToolIface()->loadTool(tool);

    connect(tool, SIGNAL(okClicked()),
            this, SLOT(slotToolDone()));

    connect(tool, SIGNAL(cancelClicked()),
            this, SLOT(slotToolDone()));
}

void ImagePlugin::slotToolDone()
{
    EditorToolIface::editorToolIface()->unLoadTool();
}

QString ImagePlugin::actionCategory() const
{
    QString val = property("ActionCategory").toString();

    if (val.isNull() || val.isEmpty())
    {
        val = QLatin1String("__INVALID__");
    }

    return val;
}

void ImagePlugin::setActionCategory(const QString& cat)
{
    QString val = actionCategory();

    if (val == QLatin1String("__INVALID__"))
    {
         setProperty("ActionCategory", cat);
    }
}

}  // namespace Digikam
