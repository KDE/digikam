/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setup.moc"

// Qt includes

#include <QPointer>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <kdebug.h>

// Local includes

#include "albumsettings.h"
#include "thumbsgenerator.h"
#include "setupalbumview.h"
#include "setupcamera.h"
#include "setupcategory.h"
#include "setupcollections.h"
#include "setupdcraw.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setupiofiles.h"
#include "setuplighttable.h"
#include "setupmetadata.h"
#include "setupmime.h"
#include "setupmisc.h"
#include "setupplugins.h"
#include "setupslideshow.h"
#include "setuptooltip.h"
#include "setupdatabase.h"
#include "setupfacetags.h"
#include "setupversioning.h"

#ifdef USE_SCRIPT_IFACE
#include "setupscriptmanager.h"
#endif

namespace Digikam
{

class Setup::SetupPrivate
{
public:

    SetupPrivate() :
        page_database(0),
        page_collections(0),
        page_albumView(0),
        page_tooltip(0),
        page_metadata(0),
        page_template(0),
        page_category(0),
        page_mime(0),
        page_lighttable(0),
        page_editor(0),
        page_dcraw(0),
        page_iofiles(0),
        page_slideshow(0),
        page_icc(0),
        page_camera(0),
        page_misc(0),
        page_plugins(0),
#ifdef USE_SCRIPT_IFACE
        page_scriptmanager(0),
#endif
        page_facetags(0),
        page_versioning(0),
        databasePage(0),
        collectionsPage(0),
        albumViewPage(0),
        tooltipPage(0),
        metadataPage(0),
        templatePage(0),
        categoryPage(0),
        mimePage(0),
        lighttablePage(0),
        editorPage(0),
        dcrawPage(0),
        iofilesPage(0),
        slideshowPage(0),
        iccPage(0),
        cameraPage(0),
        //faceTagsPage(0),
        miscPage(0),
        pluginsPage(0),
#ifdef USE_SCRIPT_IFACE
        scriptManagerPage(0),
#endif
        versioningPage(0)
    {
    }

    KPageWidgetItem*    page_database;
    KPageWidgetItem*    page_collections;
    KPageWidgetItem*    page_albumView;
    KPageWidgetItem*    page_tooltip;
    KPageWidgetItem*    page_metadata;
    KPageWidgetItem*    page_template;
    KPageWidgetItem*    page_category;
    KPageWidgetItem*    page_mime;
    KPageWidgetItem*    page_lighttable;
    KPageWidgetItem*    page_editor;
    KPageWidgetItem*    page_dcraw;
    KPageWidgetItem*    page_iofiles;
    KPageWidgetItem*    page_slideshow;
    KPageWidgetItem*    page_icc;
    KPageWidgetItem*    page_camera;
    KPageWidgetItem*    page_misc;
    KPageWidgetItem*    page_plugins;
#ifdef USE_SCRIPT_IFACE
    KPageWidgetItem*    page_scriptmanager;
#endif
    KPageWidgetItem*    page_facetags;
    KPageWidgetItem*    page_versioning;

    SetupDatabase*      databasePage;
    SetupCollections*   collectionsPage;
    SetupAlbumView*     albumViewPage;
    SetupToolTip*       tooltipPage;
    SetupMetadata*      metadataPage;
    SetupTemplate*      templatePage;
    SetupCategory*      categoryPage;
    SetupMime*          mimePage;
    SetupLightTable*    lighttablePage;
    SetupEditor*        editorPage;
    SetupDcraw*         dcrawPage;
    SetupIOFiles*       iofilesPage;
    SetupSlideShow*     slideshowPage;
    SetupICC*           iccPage;
    SetupCamera*        cameraPage;
    SetupMisc*          miscPage;
    SetupPlugins*       pluginsPage;
#ifdef USE_SCRIPT_IFACE
    SetupScriptManager* scriptManagerPage;
#endif
    //SetupFaceTags*      faceTagsPage;
    SetupVersioning*    versioningPage;

public:

    KPageWidgetItem* pageItem(Setup::Page page) const;
};

Setup::Setup(QWidget* parent)
    : KPageDialog(parent), d(new SetupPrivate)
{
    setCaption(i18n("Configure"));
    setButtons(Help | Ok | Cancel);
    setDefaultButton(Ok);
    setHelp("setupdialog.anchor", "digikam");
    setFaceType(List);
    setModal(true);

    d->databasePage     = new SetupDatabase(this);
    d->page_database    = addPage(d->databasePage, i18n("Database"));
    d->page_database->setHeader(i18n("<qt>Database Settings<br/>"
                                     "<i>Customize database settings</i></qt>"));
    d->page_database->setIcon(KIcon("server-database"));

    d->collectionsPage  = new SetupCollections(this);
    d->page_collections = addPage(d->collectionsPage, i18n("Collections"));
    d->page_collections->setHeader(i18n("<qt>Collections Settings<br/>"
                                        "<i>Set root albums and database locations</i></qt>"));
    d->page_collections->setIcon(KIcon("folder-image"));

    d->albumViewPage  = new SetupAlbumView();
    d->page_albumView = addPage(d->albumViewPage, i18n("Album View"));
    d->page_albumView->setHeader(i18n("<qt>Album View Settings<br/>"
                                      "<i>Customize the look of the albums list</i></qt>"));
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

    /*
        d->faceTagsPage  = new SetupFaceTags();
        d->page_facetags = addPage(d->faceTagsPage, i18n("People Tags"));
        d->page_facetags->setHeader(i18n("<qt>People Tags<br/>"
                                         "<i>Configure digiKam's face detection and recognition</i></qt>"));
        d->page_facetags->setIcon(KIcon("face-smile"));
    */

    d->metadataPage  = new SetupMetadata();
    d->page_metadata = addPage(d->metadataPage, i18n("Metadata"));
    d->page_metadata->setHeader(i18n("<qt>Embedded Image Information Management<br/>"
                                     "<i>Setup relations between images and metadata</i></qt>"));
    d->page_metadata->setIcon(KIcon("exifinfo"));

    d->templatePage  = new SetupTemplate();
    d->page_template = addPage(d->templatePage, i18n("Templates"));
    d->page_template->setHeader(i18n("<qt>Metadata templates<br/>"
                                     "<i>Manage your collection of metadata templates</i></qt>"));
    d->page_template->setIcon(KIcon("user-identity"));

    d->mimePage  = new SetupMime();
    d->page_mime = addPage(d->mimePage, i18n("MIME Types"));
    d->page_mime->setHeader(i18n("<qt>Supported File Settings<br/>"
                                 "<i>Add new file types to show as album items</i></qt>"));
    d->page_mime->setIcon(KIcon("system-file-manager"));

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage(d->editorPage, i18n("Editor Window"));
    d->page_editor->setHeader(i18n("<qt>Image Editor Window Settings<br/>"
                                   "<i>Customize the image editor window</i></qt>"));
    d->page_editor->setIcon(KIcon("editimage"));

    d->versioningPage  = new SetupVersioning();
    d->page_versioning = addPage(d->versioningPage, i18n("Editing Images"));
    d->page_versioning->setHeader(i18n("<qt>Editing Images<br/>"
                                       "<i>Configure non-destructive editing and versioning</i></qt>"));
    d->page_versioning->setIcon(KIcon("view-catalog"));

    d->dcrawPage  = new SetupDcraw();
    d->page_dcraw = addPage(d->dcrawPage, i18n("RAW Decoding"));
    d->page_dcraw->setHeader(i18n("<qt>Image Editor: RAW File Decoding<br/>"
                                  "<i>Configure RAW decoding settings of the image editor</i></qt>"));
    d->page_dcraw->setIcon(KIcon("kdcraw"));

    d->iofilesPage  = new SetupIOFiles();
    d->page_iofiles = addPage(d->iofilesPage, i18n("Saving Images"));
    d->page_iofiles->setHeader(i18n("<qt>Image Editor: Settings for Saving Image Files<br/>"
                                    "<i>Set default configuration used to save images with the image editor</i></qt>"));
    d->page_iofiles->setIcon(KIcon("document-save-all"));

    d->iccPage  = new SetupICC(0, this);
    d->page_icc = addPage(d->iccPage, i18n("Color Management"));
    d->page_icc->setHeader(i18n("<qt>Settings for Color Management<br/>"
                                "<i>Customize the color management settings</i></qt>"));
    d->page_icc->setIcon(KIcon("colormanagement"));

    d->lighttablePage  = new SetupLightTable();
    d->page_lighttable = addPage(d->lighttablePage, i18n("Light Table"));
    d->page_lighttable->setHeader(i18n("<qt>Light Table Settings<br/>"
                                       "<i>Customize tool used to compare images</i></qt>"));
    d->page_lighttable->setIcon(KIcon("lighttable"));

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

    d->pluginsPage  = new SetupPlugins();
    d->page_plugins = addPage(d->pluginsPage, i18n("Kipi Plugins"));
    d->page_plugins->setHeader(i18n("<qt>Main Interface Plug-in Settings<br/>"
                                    "<i>Set which plugins will be accessible from the main interface</i></qt>"));
    d->page_plugins->setIcon(KIcon("kipi"));

#ifdef USE_SCRIPT_IFACE
    d->scriptManagerPage  = new SetupScriptManager();
    d->page_scriptmanager = addPage(d->scriptManagerPage , i18n("Script Manager"));
    d->page_scriptmanager->setHeader(i18n("<qt>Script Manager<br/>"
                                          "<i>Add/Remove and Manage Digikam Scripts</i></qt>"));
    d->page_scriptmanager->setIcon(KIcon("application-x-shellscript"));
#endif

    d->miscPage  = new SetupMisc();
    d->page_misc = addPage(d->miscPage, i18n("Miscellaneous"));
    d->page_misc->setHeader(i18n("<qt>Miscellaneous Settings<br/>"
                                 "<i>Customize behavior of the other parts of digiKam</i></qt>"));
    d->page_misc->setIcon(KIcon("preferences-other"));

    for (int i = 0; i != SetupPageEnumLast; ++i)
    {
        KPageWidgetItem* item = d->pageItem((Page)i);

        if (!item)
        {
            continue;
        }

        QWidget* wgt            = item->widget();
        QScrollArea* scrollArea = qobject_cast<QScrollArea*>(wgt);

        if (scrollArea)
        {
            scrollArea->setFrameShape(QFrame::NoFrame);
        }
    }

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));
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

void Setup::setTemplate(const Template& t)
{
    if (d->templatePage)
    {
        d->templatePage->setTemplate(t);
    }
}

QSize Setup::sizeHint() const
{
    // The minimum size is very small. But the default initial size is such
    // that some important tabs get a scroll bar, although the dialog could be larger
    // on a normal display (QScrollArea size hint does not take widget into account)
    // Adjust size hint here so that certain selected tabs are display full per default.
    QSize hint          = KPageDialog::sizeHint();
    int maxHintHeight   = 0;
    int maxWidgetHeight = 0;

    for (int page = 0; page != SetupPageEnumLast; ++page)
    {
        // only take tabs into account here that should better be displayed without scrolling
        if (page == CollectionsPage ||
            page == AlbumViewPage   ||
            page == TemplatePage    ||
            page == MimePage        ||
            page == LightTablePage  ||
            page == EditorPage      ||
            page == IOFilesPage     ||
            page == DcrawPage       ||
            page == MiscellaneousPage)
        {
            KPageWidgetItem* item   = d->pageItem((Page)page);

            if (!item)
            {
                continue;
            }

            QWidget* page           = item->widget();
            maxHintHeight           = qMax(maxHintHeight, page->sizeHint().height());
            QScrollArea* scrollArea = qobject_cast<QScrollArea*>(page);

            if (scrollArea)
            {
                maxWidgetHeight = qMax(maxWidgetHeight, scrollArea->widget()->sizeHint().height());
            }
        }
    }

    // The additional 20 is a hack to make it work.
    // Don't know why, the largest page would have scroll bars without this
    if (maxWidgetHeight > maxHintHeight)
    {
        hint.setHeight(hint.height() + (maxWidgetHeight - maxHintHeight) + 20);
    }

    return hint;
}

bool Setup::exec(Page page)
{
    return exec(0, page);
}

bool Setup::exec(QWidget* parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);
    bool success          = setup->KPageDialog::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

bool Setup::execSinglePage(Page page)
{
    return execSinglePage(0, page);
}

bool Setup::execSinglePage(QWidget* parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);
    setup->setFaceType(Plain);
    bool success          = setup->KPageDialog::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

bool Setup::execTemplateEditor(QWidget* parent, const Template& t)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(TemplatePage);
    setup->setFaceType(Plain);
    setup->setTemplate(t);
    bool success          = setup->KPageDialog::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

void Setup::slotButtonClicked(int button)
{
    if (button == KDialog::Ok)
    {
        okClicked();
    }
    else
    {
        KDialog::slotButtonClicked(button);
    }
}

void Setup::okClicked()
{
    if (!d->cameraPage->checkSettings())
    {
        showPage(CameraPage);
        return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);

    d->cameraPage->applySettings();
    d->databasePage->applySettings();
    d->collectionsPage->applySettings();
    d->albumViewPage->applySettings();
    d->tooltipPage->applySettings();
    d->metadataPage->applySettings();
    d->templatePage->applySettings();
    d->categoryPage->applySettings();
    d->mimePage->applySettings();
    d->lighttablePage->applySettings();
    d->editorPage->applySettings();
    d->dcrawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();
    d->pluginsPage->applyPlugins();
    //d->faceTagsPage->applySettings();
    d->versioningPage->applySettings();

#ifdef USE_SCRIPT_IFACE
    d->scriptManagerPage->applySettings();
#endif

    AlbumSettings::instance()->emitSetupChanged();

    kapp->restoreOverrideCursor();

    if (d->metadataPage->exifAutoRotateAsChanged())
    {
        QString msg = i18n("The Exif auto-rotate thumbnails option has been changed.\n"
                           "Do you want to rebuild all albums' items' thumbnails now?\n\n"
                           "Note: thumbnail processing can take a while. You can start "
                           "this job later from the \"Tools\" menu.");
        int result = KMessageBox::warningYesNo(this, msg);

        if (result != KMessageBox::Yes)
        {
            return;
        }

        new ThumbsGenerator();
    }

    accept();
}

void Setup::showPage(Setup::Page page)
{
    KPageWidgetItem* item = 0;

    if (page == LastPageUsed)
    {
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group        = config->group(QString("Setup Dialog"));

        item = d->pageItem((Page)group.readEntry("Setup Page", (int)CollectionsPage));
    }
    else
    {
        item = d->pageItem(page);
    }

    if (!item)
    {
        item = d->pageItem(CollectionsPage);
    }

    setCurrentPage(item);
}

Setup::Page Setup::activePageIndex() const
{
    KPageWidgetItem* cur = currentPage();

    if (cur == d->page_collections)
    {
        return CollectionsPage;
    }

    if (cur == d->page_albumView)
    {
        return AlbumViewPage;
    }

    if (cur == d->page_tooltip)
    {
        return ToolTipPage;
    }

    if (cur == d->page_metadata)
    {
        return MetadataPage;
    }

    if (cur == d->page_template)
    {
        return TemplatePage;
    }

    if (cur == d->page_category)
    {
        return CategoryPage;
    }

    if (cur == d->page_mime)
    {
        return MimePage;
    }

    if (cur == d->page_lighttable)
    {
        return LightTablePage;
    }

    if (cur == d->page_editor)
    {
        return EditorPage;
    }

    if (cur == d->page_dcraw)
    {
        return DcrawPage;
    }

    if (cur == d->page_iofiles)
    {
        return IOFilesPage;
    }

    if (cur == d->page_slideshow)
    {
        return SlideshowPage;
    }

    if (cur == d->page_icc)
    {
        return ICCPage;
    }

    if (cur == d->page_plugins)
    {
        return KipiPluginsPage;
    }

    if (cur == d->page_camera)
    {
        return CameraPage;
    }

    if (cur == d->page_facetags)
    {
        return FaceTagsPage;
    }

    if (cur == d->page_misc)
    {
        return MiscellaneousPage;
    }

    if (cur == d->page_versioning)
    {
        return VersioningPage;
    }

#ifdef USE_SCRIPT_IFACE

    if (cur == d->page_scriptmanager)
    {
        return ScriptManagerPage;
    }

#endif

    return DatabasePage;
}

KPageWidgetItem* Setup::SetupPrivate::pageItem(Setup::Page page) const
{
    switch (page)
    {
        case Setup::DatabasePage:
            return page_database;

        case Setup::CollectionsPage:
            return page_collections;

        case Setup::AlbumViewPage:
            return page_albumView;

        case Setup::ToolTipPage:
            return page_tooltip;

        case Setup::MetadataPage:
            return page_metadata;

        case Setup::TemplatePage:
            return page_template;

        case Setup::CategoryPage:
            return page_category;

        case Setup::MimePage:
            return page_mime;

        case Setup::LightTablePage:
            return page_lighttable;

        case Setup::EditorPage:
            return page_editor;

        case Setup::DcrawPage:
            return page_dcraw;

        case Setup::IOFilesPage:
            return page_iofiles;

        case Setup::SlideshowPage:
            return page_slideshow;

        case Setup::ICCPage:
            return page_icc;

        case Setup::KipiPluginsPage:
            return page_plugins;

        case Setup::CameraPage:
            return page_camera;

        case Setup::FaceTagsPage:
            return page_facetags;

        case Setup::MiscellaneousPage:
            return page_misc;

        case Setup::VersioningPage:
            return page_versioning;

#ifdef USE_SCRIPT_IFACE

        case Setup::ScriptManagerPage:
            return page_scriptmanager;
#endif

        default:
            return 0;
    }
}

}  // namespace Digikam
