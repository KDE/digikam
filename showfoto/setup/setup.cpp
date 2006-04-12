/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-02
 * Description : showfoto setup dialog.
 *
 * Copyright 2005-2006 by Gilles Caulier
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
#include "setupiofiles.h"
#include "setupimgplugins.h"
#include "setupslideshow.h"
#include "setupicc.h"
#include "setup.h"

namespace ShowFoto
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        slideshowPage   = 0;
        imgpluginsPage  = 0;
        iofilesPage     = 0;
        iccPage         = 0;
        editorPage      = 0;
        page_icc        = 0;
        page_slideshow  = 0;
        page_imgplugins = 0;
        page_iofiles    = 0;
        page_editor     = 0;
    }

    QFrame                   *page_editor;
    QFrame                   *page_iofiles;
    QFrame                   *page_imgplugins;
    QFrame                   *page_slideshow;
    QFrame                   *page_icc;
    
    SetupEditor              *editorPage;

    Digikam::SetupICC        *iccPage;
    Digikam::SetupIOFiles    *iofilesPage;
    Digikam::SetupImgPlugins *imgpluginsPage;
    Digikam::SetupSlideShow  *slideshowPage;
};

Setup::Setup(QWidget* parent, const char* name, Setup::Page page)
     : KDialogBase(IconList, i18n("Configure"), Help|Ok|Cancel, Ok, parent,
                   name, true, true )
{
    d = new SetupPrivate;
    setHelp("setupdialog.anchor", "showfoto");

    d->page_editor = addPage(i18n("General"), i18n("General Settings"),
                             BarIcon("showfoto", KIcon::SizeMedium));
    d->editorPage = new SetupEditor(d->page_editor);

    d->page_iofiles = addPage(i18n("IO files"), i18n("IO Image Files Settings"),
                              BarIcon("pipe", KIcon::SizeMedium));
    d->iofilesPage = new Digikam::SetupIOFiles(d->page_iofiles);
    
    d->page_imgplugins = addPage(i18n("Image Plugins"), i18n("Image Plugins Settings"),
                                 BarIcon("digikamimageplugins", KIcon::SizeMedium));
    d->imgpluginsPage = new Digikam::SetupImgPlugins(d->page_imgplugins);

    d->page_slideshow = addPage(i18n("Slide Show"), i18n("Slide Show Settings"),
                                BarIcon("slideshow", KIcon::SizeMedium));
    d->slideshowPage = new Digikam::SetupSlideShow(d->page_slideshow);

    d->page_icc = addPage(i18n("ICC Profiles"), i18n("Color Profiles Management"),
                          BarIcon("colorize", KIcon::SizeMedium));
    d->iccPage = new Digikam::SetupICC(d->page_icc, this);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    showPage((int) page);
    show();
}

Setup::~Setup()
{
    delete d;
}

void Setup::slotOkClicked()
{
    d->editorPage->applySettings();
    d->iofilesPage->applySettings();
    d->imgpluginsPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    close();
}

Digikam::SetupImgPlugins* Setup::imagePluginsPage()
{
    return d->imgpluginsPage;
}

}   // namespace ShowFoto

#include "setup.moc"
