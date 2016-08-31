/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup Image Editor page
 *
 * Copyright (C) 2007-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupeditor.h"

// Qt includes

#include <QTabWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "setupeditoriface.h"
#include "setupiofiles.h"
#include "setupversioning.h"

namespace Digikam
{

class SetupEditor::Private
{
public:


    Private() :
        tab(0),
        iface(0),
        iofiles(0),
        versioning(0)
    {
    }

    QTabWidget*       tab;

    SetupEditorIface* iface;
    SetupIOFiles*     iofiles;
    SetupVersioning*  versioning;
};

SetupEditor::SetupEditor(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->tab = new QTabWidget;

    // --------------------------------------------------------

    d->iface      = new SetupEditorIface(d->tab);
    d->versioning = new SetupVersioning(d->tab);
    d->iofiles    = new SetupIOFiles(d->tab);

    // --------------------------------------------------------

    d->tab->addTab(d->iface,      i18nc("@title:tab", "Interface"));
    d->tab->addTab(d->versioning, i18nc("@title:tab", "Versioning"));
    d->tab->addTab(d->iofiles,    i18nc("@title:tab", "Save Settings"));

    // --------------------------------------------------------

    setWidget(d->tab);
    setWidgetResizable(true);
    readSettings();
}

SetupEditor::~SetupEditor()
{
    delete d;
}

void SetupEditor::applySettings()
{
    d->iface->applySettings();
    d->versioning->applySettings();
    d->iofiles->applySettings();
}

void SetupEditor::readSettings()
{
    // Nothing todo. All is already processed in widget contructors
}

}  // namespace Digikam
