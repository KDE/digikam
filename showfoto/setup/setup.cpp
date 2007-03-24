/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-02
 * Description : showfoto setup dialog.
 *
 * Copyright 2005-2007 by Gilles Caulier
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
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "setupeditor.h"
#include "setupdcraw.h"
#include "setupiofiles.h"
#include "setupslideshow.h"
#include "setupicc.h"
#include "setup.h"
#include "setup.moc"

namespace ShowFoto
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        editorPage      = 0;
        dcrawPage       = 0;
        iofilesPage     = 0;
        slideshowPage   = 0;
        iccPage         = 0;
        page_editor     = 0;
        page_dcraw      = 0;
        page_iofiles    = 0;
        page_slideshow  = 0;
        page_icc        = 0;
    }

    QFrame                   *page_editor;
    QFrame                   *page_dcraw;
    QFrame                   *page_iofiles;
    QFrame                   *page_slideshow;
    QFrame                   *page_icc;
    
    SetupEditor              *editorPage;

    Digikam::SetupDcraw      *dcrawPage;
    Digikam::SetupIOFiles    *iofilesPage;
    Digikam::SetupSlideShow  *slideshowPage;
    Digikam::SetupICC        *iccPage;
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

    d->page_dcraw = addPage(i18n("RAW decoding"), i18n("RAW Files Decoding Settings"),
                              BarIcon("kdcraw", KIcon::SizeMedium));
    d->dcrawPage = new Digikam::SetupDcraw(d->page_dcraw);

    d->page_icc = addPage(i18n("ICC Profiles"), i18n("Color Management Profiles"),
                          BarIcon("colorize", KIcon::SizeMedium));
    d->iccPage = new Digikam::SetupICC(d->page_icc, this);

    d->page_iofiles = addPage(i18n("Save Images"), i18n("Image Editor Save Images Files Settings"),
                              BarIcon("filesave", KIcon::SizeMedium));
    d->iofilesPage = new Digikam::SetupIOFiles(d->page_iofiles);
    
    d->page_slideshow = addPage(i18n("Slide Show"), i18n("Slide Show Settings"),
                                BarIcon("slideshow", KIcon::SizeMedium));
    d->slideshowPage = new Digikam::SetupSlideShow(d->page_slideshow);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    if (page != LastPageUsed)
        showPage((int) page);
    else 
    {
        KConfig* config = kapp->config();
        config->setGroup("General Settings");
        showPage(config->readNumEntry("Setup Page", EditorPage));        
    }
    
    show();
}

Setup::~Setup()
{
    KConfig* config = kapp->config();
    config->setGroup("General Settings");
    config->writeEntry("Setup Page", activePageIndex());
    config->sync();    
    delete d;
}

void Setup::slotOkClicked()
{
    d->editorPage->applySettings();
    d->dcrawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    close();
}

}   // namespace ShowFoto
