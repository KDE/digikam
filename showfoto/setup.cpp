/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
 * Date   : 2005-04-02
 * Description : showfoto setup dialog.
 * 
 * Copyright 2005 by Gilles Caulier
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
 
// Qt includes.

#include <qtabwidget.h>
#include <qapplication.h>
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "setupeditor.h"
#include "setupplugins.h"
#include "setup.h"

Setup::Setup(QWidget* parent, const char* name, Setup::Page page)
     : KDialogBase(IconList, i18n("Configure"), Help|Ok|Cancel, Ok, parent,
                   name, true, true )
{
    setHelp("setupdialog.anchor", "showfoto");

    page_editor = addPage(i18n("General"), i18n("General Settings"),
                        BarIcon("showfoto", KIcon::SizeMedium));
    editorPage_ = new SetupEditor(page_editor);

    page_plugins = addPage(i18n("Image Plugins"), i18n("Image Plugins List"),
                           BarIcon("digikamimageplugins", KIcon::SizeMedium));
    pluginsPage_ = new SetupPlugins(page_plugins);
        
    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    showPage((int) page);
    show();
}

Setup::~Setup()
{
}

void Setup::slotOkClicked()
{
    editorPage_->applySettings();
    pluginsPage_->applySettings();
    close();
}

#include "setup.moc"
