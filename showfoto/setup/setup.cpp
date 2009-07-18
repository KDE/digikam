/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : showFoto setup dialog.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setup.h"
#include "setup.moc"

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>

// Local includes

#include "setupdcraw.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setupiofiles.h"
#include "setupslideshow.h"
#include "setuptooltip.h"
#include "setupmetadata.h"

namespace ShowFoto
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        editorPage     = 0;
        metadataPage   = 0;
        toolTipPage    = 0;
        dcrawPage      = 0;
        iofilesPage    = 0;
        slideshowPage  = 0;
        iccPage        = 0;
        page_editor    = 0;
        page_metadata  = 0;
        page_tooltip   = 0;
        page_dcraw     = 0;
        page_iofiles   = 0;
        page_slideshow = 0;
        page_icc       = 0;
    }

    KPageWidgetItem*         page_editor;
    KPageWidgetItem*         page_metadata;
    KPageWidgetItem*         page_tooltip;
    KPageWidgetItem*         page_dcraw;
    KPageWidgetItem*         page_iofiles;
    KPageWidgetItem*         page_slideshow;
    KPageWidgetItem*         page_icc;

    SetupEditor*             editorPage;
    SetupMetadata*           metadataPage;
    SetupToolTip*            toolTipPage;

    Digikam::SetupDcraw*     dcrawPage;
    Digikam::SetupIOFiles*   iofilesPage;
    Digikam::SetupSlideShow* slideshowPage;
    Digikam::SetupICC*       iccPage;
};

Setup::Setup(QWidget* parent, const char* name, Setup::Page page)
     : KPageDialog(parent), d(new SetupPrivate)
{
    setObjectName(name);
    setCaption(i18n("Configure"));
    setButtons( KDialog::Help|KDialog::Ok|KDialog::Cancel );
    setDefaultButton(KDialog::Ok);
    setHelp("setupdialog.anchor", "showfoto");
    setFaceType(KPageDialog::List);
    setModal(true);

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage(d->editorPage, i18nc("general settings tab", "General"));
    d->page_editor->setHeader(i18n("<qt>General Settings<br/>"
                              "<i>Customize general behavior</i></qt>"));
    d->page_editor->setIcon(KIcon("showfoto"));

    d->metadataPage  = new SetupMetadata();
    d->page_metadata = addPage(d->metadataPage, i18n("Metadata"));
    d->page_metadata->setHeader(i18n("<qt>Embedded Image Information Management<br/>"
                                "<i>Setup relations between images and metadata</i></qt>"));
    d->page_metadata->setIcon(KIcon("exifinfo"));

    d->toolTipPage = new SetupToolTip();
    d->page_tooltip = addPage(d->toolTipPage, i18n("Tool Tip"));
    d->page_tooltip->setHeader(i18n("<qt>Thumbbar Items Tool-Tip Settings<br/>"
                               "<i>Customize information in tool-tips</i></qt>"));
    d->page_tooltip->setIcon(KIcon("dialog-information"));

    d->dcrawPage  = new Digikam::SetupDcraw();
    d->page_dcraw = addPage(d->dcrawPage, i18n("RAW Decoding"));
    d->page_dcraw->setHeader(i18n("<qt>RAW Files Decoding Settings<br/>"
                             "<i>Customize default RAW decoding settings</i></qt>"));
    d->page_dcraw->setIcon(KIcon("kdcraw"));

    d->iccPage  = new Digikam::SetupICC(0, this);
    d->page_icc = addPage(d->iccPage, i18n("Color Management"));
    d->page_icc->setHeader(i18n("<qt>Settings for Color Management<br/>"
                           "<i>Customize color management settings</i></qt>"));
    d->page_icc->setIcon(KIcon("colormanagement"));

    d->iofilesPage  = new Digikam::SetupIOFiles();
    d->page_iofiles = addPage(d->iofilesPage, i18n("Save Images"));
    d->page_iofiles->setHeader(i18n("<qt>Settings for Saving Image Files<br/>"
                               "<i>Set default configuration used to save images</i></qt>"));
    d->page_iofiles->setIcon(KIcon("document-save-all"));

    d->slideshowPage  = new Digikam::SetupSlideShow();
    d->page_slideshow = addPage(d->slideshowPage, i18n("Slide Show"));
    d->page_slideshow->setHeader(i18n("<qt>Slide Show Settings<br/>"
                                 "<i>Customize slideshow settings</i></qt>"));
    d->page_slideshow->setIcon(KIcon("view-presentation"));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));

    if (page != LastPageUsed)
        showPage(page);
    else
    {
        showPage((Page)group.readEntry("Setup Page", (int)EditorPage));
    }

    restoreDialogSize(group);

    show();
}

Setup::~Setup()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));
    group.writeEntry("Setup Page", (int)activePageIndex());
    saveDialogSize(group);
    config->sync();
    delete d;
}

void Setup::slotOkClicked()
{
    d->editorPage->applySettings();
    d->metadataPage->applySettings();
    d->toolTipPage->applySettings();
    d->dcrawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    close();
}

void Setup::showPage(Setup::Page page)
{
    switch(page)
    {
        case ToolTipPage:
            setCurrentPage(d->page_tooltip);
            break;
        case DcrawPage:
            setCurrentPage(d->page_dcraw);
            break;
        case IOFilesPage:
            setCurrentPage(d->page_iofiles);
            break;
        case SlideshowPage:
            setCurrentPage(d->page_slideshow);
            break;
        case ICCPage:
            setCurrentPage(d->page_icc);
            break;
        case MetadataPage:
            setCurrentPage(d->page_metadata);
            break;
        default:
            setCurrentPage(d->page_editor);
            break;
    }
}

Setup::Page Setup::activePageIndex()
{
    KPageWidgetItem *cur = currentPage();
    if (cur == d->page_tooltip)   return ToolTipPage;
    if (cur == d->page_dcraw)     return DcrawPage;
    if (cur == d->page_iofiles)   return IOFilesPage;
    if (cur == d->page_slideshow) return SlideshowPage;
    if (cur == d->page_icc)       return ICCPage;
    if (cur == d->page_metadata)  return MetadataPage;

    return EditorPage;
}

}   // namespace ShowFoto
