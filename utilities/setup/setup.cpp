/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Album
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Album Public License for more details.
 *
 * ============================================================ */

#include "setup.h"
#include "setup.moc"

// KDE includes.

#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kvbox.h>

// Local includes.

#include "batchthumbsgenerator.h"
#include "setupcollections.h"
#include "setupalbumview.h"
#include "setuptooltip.h"
#include "setupmetadata.h"
#include "setupidentity.h"
#include "setupcategory.h"
#include "setupmime.h"
#include "setuplighttable.h"
#include "setupeditor.h"
#include "setupdcraw.h"
#include "setupiofiles.h"
#include "setupslideshow.h"
#include "setupicc.h"
#include "setupplugins.h"
#include "setupcamera.h"
#include "setupmisc.h"

namespace Digikam
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        page_collections = 0;
        page_albumView   = 0;
        page_tooltip     = 0;
        page_metadata    = 0;
        page_identity    = 0;
        page_category    = 0;
        page_mime        = 0;
        page_lighttable  = 0;
        page_editor      = 0;
        page_dcraw       = 0;
        page_iofiles     = 0;
        page_slideshow   = 0;
        page_icc         = 0;
        page_plugins     = 0;
        page_camera      = 0;
        page_misc        = 0;

        collectionsPage  = 0;
        albumViewPage    = 0;
        tooltipPage      = 0;
        metadataPage     = 0;
        identityPage     = 0;
        categoryPage     = 0;
        mimePage         = 0;
        lighttablePage   = 0;
        editorPage       = 0;
        dcrawPage        = 0;
        iofilesPage      = 0;
        slideshowPage    = 0;
        iccPage          = 0;
        cameraPage       = 0;
        miscPage         = 0;
        pluginsPage      = 0;
    }

    KPageWidgetItem  *page_collections;
    KPageWidgetItem  *page_albumView;
    KPageWidgetItem  *page_tooltip;
    KPageWidgetItem  *page_metadata;
    KPageWidgetItem  *page_identity;
    KPageWidgetItem  *page_category;
    KPageWidgetItem  *page_mime;
    KPageWidgetItem  *page_lighttable;
    KPageWidgetItem  *page_editor;
    KPageWidgetItem  *page_dcraw;
    KPageWidgetItem  *page_iofiles;
    KPageWidgetItem  *page_slideshow;
    KPageWidgetItem  *page_icc;
    KPageWidgetItem  *page_plugins;
    KPageWidgetItem  *page_camera;
    KPageWidgetItem  *page_misc;

    SetupCollections *collectionsPage;
    SetupAlbumView   *albumViewPage;
    SetupToolTip     *tooltipPage;
    SetupMetadata    *metadataPage;
    SetupIdentity    *identityPage;
    SetupCategory    *categoryPage;
    SetupMime        *mimePage;
    SetupLightTable  *lighttablePage;
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
     : KPageDialog(parent), d(new SetupPrivate)
{
    setObjectName(name);
    setCaption(i18n("Configure"));
    setButtons(Help|Ok|Cancel );
    setDefaultButton(Ok);
    setHelp("setupdialog.anchor", "digikam");
    setFaceType(List);
    setModal(true);

    d->collectionsPage  = new SetupCollections(this);
    d->page_collections = addPage(d->collectionsPage, i18n("Collections"));
    d->page_collections->setHeader(i18n("<qt>Collections Settings<br/>"
                                   "<i>Set root albums and database locations</i></qt>"));
    d->page_collections->setIcon(KIcon("folder-image"));

    d->albumViewPage  = new SetupAlbumView();
    d->page_albumView = addPage(d->albumViewPage, i18n("Album View"));
    d->page_albumView->setHeader(i18n("<qt>Album View Settings<br/>"
                                 "<i>Customize the look of albums list</i></qt>"));
    d->page_albumView->setIcon(KIcon("view-list-icons"));

    d->categoryPage  = new SetupCategory();
    d->page_category = addPage(d->categoryPage, i18n("Album Category"));
    d->page_category->setHeader(i18n("<qt>Album Category Settings<br/>"
                                "<i>Assign categories to albums used to sort them</i></qt>"));
    d->page_category->setIcon(KIcon("view-calendar-list"));

    d->tooltipPage  = new SetupToolTip();
    d->page_tooltip = addPage(d->tooltipPage, i18n("Tool-Tip"));
    d->page_tooltip->setHeader(i18n("<qt>Album Items Tool-Tip Settings<br/>"
                               "<i>Customize information in tool-tips</i></qt>"));
    d->page_tooltip->setIcon(KIcon("dialog-information"));

    d->metadataPage  = new SetupMetadata();
    d->page_metadata = addPage(d->metadataPage, i18n("Metadata"));
    d->page_metadata->setHeader(i18n("<qt>Embedded Image Information Management<br/>"
                                "<i>Setup relations between images and metadata</i></qt>"));
    d->page_metadata->setIcon(KIcon("exifinfo"));

    d->identityPage  = new SetupIdentity();
    d->page_identity = addPage(d->identityPage, i18n("Identity"));
    d->page_identity->setHeader(i18n("<qt>Default identity information<br/>"
                                "<i>Customize your personal information</i></qt>"));
    d->page_identity->setIcon(KIcon("user-identity"));

    d->mimePage  = new SetupMime();
    d->page_mime = addPage(d->mimePage, i18n("MIME Types"));
    d->page_mime->setHeader(i18n("<qt>Supported File Settings<br/>"
                            "<i>Add new file types to show as album items</i></qt>"));
    d->page_mime->setIcon(KIcon("system-file-manager"));

    d->lighttablePage  = new SetupLightTable();
    d->page_lighttable = addPage(d->lighttablePage, i18n("Light Table"));
    d->page_lighttable->setHeader(i18n("<qt>Light Table Settings<br/>"
                                  "<i>Customize tool used to compare images</i></qt>"));
    d->page_lighttable->setIcon(KIcon("lighttable"));

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage(d->editorPage, i18n("Image Editor"));
    d->page_editor->setHeader(i18n("<qt>Image Editor Settings<br/>"
                              "<i>Customize image editor behavior</i></qt>"));
    d->page_editor->setIcon(KIcon("editimage"));

    d->iofilesPage  = new SetupIOFiles();
    d->page_iofiles = addPage(d->iofilesPage, i18n("Save Images"));
    d->page_iofiles->setHeader(i18n("<qt>Image Editor: Settings for Saving Image Files<br/>"
                               "<i>Set default configuration used to save images with image editor</i></qt>"));
    d->page_iofiles->setIcon(KIcon("document-save-all"));

    d->dcrawPage = new SetupDcraw();
    d->page_dcraw = addPage(d->dcrawPage, i18n("RAW decoding"));
    d->page_dcraw->setHeader(i18n("<qt>Image Editor: RAW Files Decoding Settings<br/>"
                             "<i>Customize default RAW decoding settings of image editor</i></qt>"));
    d->page_dcraw->setIcon(KIcon("kdcraw"));

    d->iccPage  = new SetupICC(0, this);
    d->page_icc = addPage(d->iccPage, i18n("Color Management"));
    d->page_icc->setHeader(i18n("<qt>Image Editor: Settings for Color Management<br/>"
                           "<i>Customize color management settings of image editor</i></qt>"));
    d->page_icc->setIcon(KIcon("colormanagement"));

    d->pluginsPage  = new SetupPlugins();
    d->page_plugins = addPage(d->pluginsPage, i18n("Kipi Plugins"));
    d->page_plugins->setHeader(i18n("<qt>Main Interface Plug-in Settings<br/>"
                               "<i>Set which plugins will be accessible from main interface</i></qt>"));
    d->page_plugins->setIcon(KIcon("kipi"));

    d->slideshowPage  = new SetupSlideShow();
    d->page_slideshow = addPage(d->slideshowPage, i18n("Slide Show"));
    d->page_slideshow->setHeader(i18n("<qt>Slide Show Settings<br/>"
                                 "<i>Customize slideshow settings</i></qt>"));
    d->page_slideshow->setIcon(KIcon("view-presentation"));

    d->cameraPage  = new SetupCamera();
    d->page_camera = addPage(d->cameraPage, i18n("Cameras"));
    d->page_camera->setHeader(i18n("<qt>Camera Settings<br/>"
                              "<i>Manage your camera devices</i></qt>"));
    d->page_camera->setIcon(KIcon("camera-photo"));

    d->miscPage  = new SetupMisc();
    d->page_misc = addPage(d->miscPage, i18n("Miscellaneous"));
    d->page_misc->setHeader(i18n("<qt>Miscellaneous Settings<br/>"
                            "<i>Customize behavior of other parts of digiKam</i></qt>"));
    d->page_misc->setIcon(KIcon("preferences-other"));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));

    if (page != LastPageUsed)
        showPage(page);
    else
    {
        showPage((Page)group.readEntry("Setup Page", (int)CollectionsPage));
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
    d->collectionsPage->applySettings();
    d->albumViewPage->applySettings();
    d->tooltipPage->applySettings();
    d->metadataPage->applySettings();
    d->identityPage->applySettings();
    d->categoryPage->applySettings();
    d->mimePage->applySettings();
    d->cameraPage->applySettings();
    d->lighttablePage->applySettings();
    d->editorPage->applySettings();
    d->dcrawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();

    if (d->metadataPage->exifAutoRotateAsChanged())
    {
        QString msg = i18n("The Exif auto-rotate thumbnails option has been changed.\n"
                           "Do you want to rebuild all albums' items' thumbnails now?\n\n"
                           "Note: thumbnail processing can take a while! You can start "
                           "this job later from the \"Tools\" menu.");
        int result = KMessageBox::warningYesNo(this, msg);
        if (result != KMessageBox::Yes)
            return;

        BatchThumbsGenerator *thumbsGenerator = new BatchThumbsGenerator(this);
        thumbsGenerator->show();
    }

    close();
}

SetupPlugins* Setup::kipiPluginsPage()
{
    return d->pluginsPage;
}

void Setup::showPage(Setup::Page page)
{
    switch(page)
    {
        case AlbumViewPage:
            setCurrentPage(d->page_albumView);
            break;
        case ToolTipPage:
            setCurrentPage(d->page_tooltip);
            break;
        case MetadataPage:
            setCurrentPage(d->page_metadata);
            break;
        case IdentifyPage:
            setCurrentPage(d->page_identity);
            break;
        case CategoryPage:
            setCurrentPage(d->page_category);
            break;
        case MimePage:
            setCurrentPage(d->page_mime);
            break;
        case LightTablePage:
            setCurrentPage(d->page_lighttable);
            break;
        case EditorPage:
            setCurrentPage(d->page_editor);
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
        case KipiPluginsPage:
            setCurrentPage(d->page_plugins);
            break;
        case CameraPage:
            setCurrentPage(d->page_camera);
            break;
        case MiscellaneousPage:
            setCurrentPage(d->page_misc);
            break;
        default:
            setCurrentPage(d->page_collections);
            break;
    }
}

Setup::Page Setup::activePageIndex()
{
    KPageWidgetItem *cur = currentPage();

    if (cur == d->page_albumView)   return AlbumViewPage;
    if (cur == d->page_tooltip)     return ToolTipPage;
    if (cur == d->page_metadata)    return MetadataPage;
    if (cur == d->page_identity)    return IdentifyPage;
    if (cur == d->page_category)    return CategoryPage;
    if (cur == d->page_mime)        return MimePage;
    if (cur == d->page_lighttable)  return LightTablePage;
    if (cur == d->page_editor)      return EditorPage;
    if (cur == d->page_dcraw)       return DcrawPage;
    if (cur == d->page_iofiles)     return IOFilesPage;
    if (cur == d->page_slideshow)   return SlideshowPage;
    if (cur == d->page_icc)         return ICCPage;
    if (cur == d->page_plugins)     return KipiPluginsPage;
    if (cur == d->page_camera)      return CameraPage;
    if (cur == d->page_misc)        return MiscellaneousPage;

    return CollectionsPage;
}

}  // namespace Digikam
