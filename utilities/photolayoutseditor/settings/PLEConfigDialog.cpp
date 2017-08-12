/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "PLEConfigDialog.h"

#include <QIcon>

#include "PLEConfigSkeleton.h"
#include "PLEConfigViewWidget.h"

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::PLEConfigDialogPrivate
{
    PLEConfigViewWidget * confVWdg;

    friend class PLEConfigDialog;
};

PLEConfigDialog::PLEConfigDialog(QWidget * parent) :
    KConfigDialog(parent, QLatin1String("settings"), PLEConfigSkeleton::self()),
    d(new PLEConfigDialogPrivate)
{
    d->confVWdg = new PLEConfigViewWidget( 0, i18n("View") );
    addPage( d->confVWdg, i18n("View") )->setIcon(QIcon(QLatin1String(":/view.png")));
}

PLEConfigDialog::~PLEConfigDialog()
{
   delete d;
}

void PLEConfigDialog::updateSettings()
{
    d->confVWdg->updateSettings();
}

void PLEConfigDialog::updateWidgets()
{
    d->confVWdg->updateWidgets();
}
