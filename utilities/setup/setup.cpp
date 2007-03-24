/* ============================================================
 * Authors: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2003-02-03
 * Description : digiKam setup dialog.
 * 
 * Copyright 2003-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "batchthumbsgenerator.h"
#include "setupgeneral.h"
#include "setuptooltip.h"
#include "setupmetadata.h"
#include "setupidentity.h"
#include "setupcollections.h"
#include "setupmime.h"
#include "setupeditor.h"
#include "setupdcraw.h"
#include "setupiofiles.h"
#include "setupslideshow.h"
#include "setupicc.h"
#include "setupplugins.h"
#include "setupcamera.h"
#include "setupmisc.h"
#include "setup.h"
#include "setup.moc"

namespace Digikam
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        page_general     = 0;
        page_tooltip     = 0;
        page_metadata    = 0;
        page_identity    = 0;
        page_collections = 0;
        page_mime        = 0;
        page_editor      = 0;
        page_dcraw       = 0;
        page_iofiles     = 0;
        page_slideshow   = 0;
        page_icc         = 0;
        page_plugins     = 0;
        page_camera      = 0;
        page_misc        = 0;

        generalPage      = 0;
        tooltipPage      = 0;
        metadataPage     = 0;
        identityPage     = 0;
        collectionsPage  = 0;
        mimePage         = 0;
        editorPage       = 0;
        dcrawPage        = 0;
        iofilesPage      = 0;
        slideshowPage    = 0;
        iccPage          = 0;
        cameraPage       = 0;
        miscPage         = 0;
        pluginsPage      = 0;
    }

    QFrame           *page_general;
    QFrame           *page_tooltip;
    QFrame           *page_metadata;
    QFrame           *page_identity;
    QFrame           *page_collections;
    QFrame           *page_mime;
    QFrame           *page_editor;
    QFrame           *page_dcraw;
    QFrame           *page_iofiles;
    QFrame           *page_slideshow;
    QFrame           *page_icc;
    QFrame           *page_plugins;
    QFrame           *page_camera;
    QFrame           *page_misc;

    SetupGeneral     *generalPage;
    SetupToolTip     *tooltipPage;
    SetupMetadata    *metadataPage;
    SetupIdentity    *identityPage;
    SetupCollections *collectionsPage;
    SetupMime        *mimePage;
    SetupEditor      *editorPage;
    SetupDcraw       *dcrawPage;
    SetupIOFiles     *iofilesPage;
    SetupSlideShow   *slideshowPage;
    SetupICC         *iccPage;
    SetupCamera      *cameraPage;
    SetupMisc        *miscPage;
    SetupPlugins     *pluginsPage;
};

Setup::Setup(QWidget* parent, const char* name, Setup::Page page)
     : KDialogBase(IconList, i18n("Configure"), Help|Ok|Cancel, Ok, parent,
                   name, true, true )
{
    d = new SetupPrivate;
    setHelp("setupdialog.anchor", "digikam");

    d->page_general = addPage(i18n("Albums"), i18n("Album Settings"),
                              BarIcon("folder_image", KIcon::SizeMedium));
    d->generalPage = new SetupGeneral(d->page_general, this);

    d->page_collections = addPage(i18n("Collections"), i18n("Album Collections"),
                                  BarIcon("fileopen", KIcon::SizeMedium));
    d->collectionsPage = new SetupCollections(d->page_collections);

    d->page_identity = addPage(i18n("Identity"), i18n("Default IPTC identity information"),
                               BarIcon("identity", KIcon::SizeMedium));
    d->identityPage = new SetupIdentity(d->page_identity);

    d->page_metadata = addPage(i18n("Metadata"), i18n("Embedded Image Information Management"),
                               BarIcon("exifinfo", KIcon::SizeMedium));
    d->metadataPage = new SetupMetadata(d->page_metadata);

    d->page_tooltip = addPage(i18n("Tool Tip"), i18n("Album Items Tool Tip Settings"),
                              BarIcon("filetypes", KIcon::SizeMedium));
    d->tooltipPage = new SetupToolTip(d->page_tooltip);

    d->page_mime = addPage(i18n("Mime Types"), i18n("File (MIME) Types Settings"),
                           BarIcon("filetypes", KIcon::SizeMedium));
    d->mimePage = new SetupMime(d->page_mime);

    d->page_editor = addPage(i18n("Image Editor"), i18n("Image Editor General Settings"),
                             BarIcon("image", KIcon::SizeMedium));
    d->editorPage = new SetupEditor(d->page_editor);

    d->page_iofiles = addPage(i18n("Save Images"), i18n("Image Editor Save Images Files Settings"),
                              BarIcon("filesave", KIcon::SizeMedium));
    d->iofilesPage = new SetupIOFiles(d->page_iofiles);

    d->page_dcraw = addPage(i18n("RAW decoding"), i18n("RAW Files Decoding Settings"),
                              BarIcon("kdcraw", KIcon::SizeMedium));
    d->dcrawPage = new SetupDcraw(d->page_dcraw);

    d->page_icc = addPage(i18n("Color Management"), i18n("Image Editor Color Management"),
                          BarIcon("colorize", KIcon::SizeMedium));
    d->iccPage = new SetupICC(d->page_icc, this);

    d->page_plugins = addPage(i18n("Kipi Plugins"), i18n("Main Interface Plug-in Settings"),
                              BarIcon("kipi", KIcon::SizeMedium));
    d->pluginsPage = new SetupPlugins(d->page_plugins);

    d->page_slideshow = addPage(i18n("Slide Show"), i18n("Slide Show Settings"),
                                BarIcon("slideshow", KIcon::SizeMedium));
    d->slideshowPage = new SetupSlideShow(d->page_slideshow);

    d->page_camera = addPage(i18n("Cameras"), i18n("Camera Settings"),
                             BarIcon("digitalcam", KIcon::SizeMedium));
    d->cameraPage = new SetupCamera(d->page_camera);

    d->page_misc = addPage(i18n("Miscellaneous"), i18n("Miscellaneous Settings"),
                           BarIcon("misc", KIcon::SizeMedium));
    d->miscPage = new SetupMisc(d->page_misc);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    if (page != LastPageUsed)
        showPage((int) page);
    else 
    {
        KConfig* config = kapp->config();
        config->setGroup("General Settings");
        showPage(config->readNumEntry("Setup Page", General));
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
    d->generalPage->applySettings();
    d->tooltipPage->applySettings();
    d->metadataPage->applySettings();
    d->identityPage->applySettings();
    d->collectionsPage->applySettings();
    d->mimePage->applySettings();
    d->cameraPage->applySettings();
    d->editorPage->applySettings();
    d->dcrawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();

    if (d->metadataPage->exifAutoRotateAsChanged())
    {
        QString msg = i18n("The Exif auto-rotate thumbnails option has been changed.\n"
                           "Do you want to rebuild all albums items thumbnails now?\n\n"
                           "Note: thumbnail processing can take a while! You can start "
                           "this job later using \"Tools\" menu.");
        int result = KMessageBox::warningYesNo(this, msg);
        if (result != KMessageBox::Yes)
            return;

        BatchThumbsGenerator *thumbsGenerator = new BatchThumbsGenerator(this);
        thumbsGenerator->exec();
    }

    close();
}

SetupPlugins* Setup::kipiPluginsPage()
{
    return d->pluginsPage;
}

}  // namespace Digikam

