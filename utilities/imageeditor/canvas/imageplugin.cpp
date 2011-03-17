/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image plugins interface for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin.moc"

// Local includes

#include "editortool.h"
#include "editortooliface.h"

namespace Digikam
{

class ImagePlugin::ImagePluginPriv
{
public:

    ImagePluginPriv()
    {}

    QString actionCategory;
};

// --------------------------------------------------------

ImagePlugin::ImagePlugin(QObject* parent, const char* name)
    : QObject(parent), d(new ImagePluginPriv())
{
    setObjectName(name);
}

ImagePlugin::~ImagePlugin()
{
    delete d;
}

void ImagePlugin::setEnabledSelectionActions(bool)
{
}

void ImagePlugin::setEnabledActions(bool)
{
}

void ImagePlugin::loadTool(EditorTool* tool)
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
    // FIXME: crash on 64bit systems?
    // According to bug #214718 digiKam crashes when using QString::isEmpty() on the returned string.
    // But shouldn't the string be valid when the Priv class is created?
    // I will add an extra check here to see if the crash disappears.
    // In general I would say this is a Qt bug?

    if (!d || d->actionCategory.isNull() || d->actionCategory.isEmpty())
    {
        return QString("__INVALID__");
    }

    return d->actionCategory;
}

void ImagePlugin::setActionCategory(const QString& name)
{
    // only set once
    if (d && d->actionCategory.isEmpty())
    {
        d->actionCategory = name;
    }
}
}  // namespace Digikam
